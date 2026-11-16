//! N1ssan Sync (Operation Apex): checks the signature/offset dataset against
//! the offsets actually present in a running CS2 client, and writes the
//! discovered offsets back into the dataset the release build ships.

mod definitions;
mod process;
mod scan;

use std::path::PathBuf;
use std::sync::Mutex;

use serde::Serialize;
use tauri::{Emitter, Manager};

use definitions::{Dataset, DatasetEntry};

const GAME_EXE: &str = "cs2.exe";
const CLIENT_MODULE: &str = "client.dll";

/// Every failure carries the taxonomy code documented in `failure-taxonomy.md`.
#[derive(Debug, Clone, Serialize)]
pub struct ApexError {
    pub code: String,
    pub message: String,
}

impl ApexError {
    pub fn new(code: &str, message: impl Into<String>) -> Self {
        Self {
            code: code.to_string(),
            message: message.into(),
        }
    }

    fn internal(error: impl std::fmt::Display) -> Self {
        Self::new("E_APEX_INTERNAL", error.to_string())
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize)]
#[serde(rename_all = "lowercase")]
pub enum EntryStatus {
    /// The scanned RVA equals the dataset's RVA.
    Match,
    /// The signature resolved somewhere else than the dataset records.
    Moved,
    /// The pattern no longer exists in the module.
    Missing,
    /// The dataset has no baseline RVA yet.
    New,
}

/// How much confidence the runtime check gives a resolved address.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize)]
#[serde(rename_all = "lowercase")]
pub enum Confidence {
    /// The global holds an aligned, plausible user-mode pointer.
    High,
    /// The global holds a pointer, but it is not 16-byte aligned.
    Medium,
    /// The address could not be read or does not hold a usable value.
    Low,
    /// No address was resolved at all.
    None,
}

impl Confidence {
    fn label(self) -> &'static str {
        match self {
            Confidence::High => "high",
            Confidence::Medium => "medium",
            Confidence::Low => "low",
            Confidence::None => "none",
        }
    }

    fn trusted(self) -> bool {
        matches!(self, Confidence::High | Confidence::Medium)
    }
}

#[derive(Debug, Clone, Serialize)]
#[serde(rename_all = "camelCase")]
pub struct EntryResult {
    pub name: String,
    pub module: String,
    pub status: EntryStatus,
    pub stored_rva: Option<u64>,
    pub found_rva: Option<u64>,
    pub confidence: Confidence,
    pub validation: String,
}

#[derive(Debug, Clone, Serialize)]
#[serde(rename_all = "camelCase")]
pub struct ScanReport {
    pub entries: Vec<EntryResult>,
    pub total: usize,
    pub moved: usize,
    pub missing: usize,
    /// Entries with no stored baseline yet; they still need applying.
    pub new_count: usize,
    /// Runtime validation counts across every entry.
    pub confidence_high: usize,
    pub confidence_medium: usize,
    pub confidence_low: usize,
    pub dataset_version: String,
    pub dataset_path: String,
    pub scanned_at: String,
}

#[derive(Debug, Clone, Serialize)]
#[serde(rename_all = "camelCase")]
pub struct ApplyReport {
    pub dataset_path: String,
    pub backup_path: Option<String>,
    pub dataset_version: String,
    pub changed: Vec<String>,
    /// Entries the runtime check could not trust, so they were not written.
    pub blocked: Vec<String>,
    pub updated_at: String,
}

#[derive(Debug, Clone, Serialize)]
#[serde(rename_all = "camelCase")]
pub struct FetchReport {
    pub dataset_path: String,
    pub backup_path: Option<String>,
    pub dataset_version: String,
    pub source: String,
    pub changed: Vec<String>,
    pub updated_at: String,
}

#[derive(Debug, Clone, Serialize)]
#[serde(rename_all = "camelCase")]
pub struct ApexStatus {
    pub game_running: bool,
    pub game_pid: Option<u32>,
    pub client_module: bool,
    pub dataset_path: String,
    pub dataset_version: String,
    pub dataset_entries: usize,
    pub app_version: String,
}

#[derive(Debug, Clone, Serialize)]
#[serde(rename_all = "camelCase")]
pub struct ProgressEvent {
    pub percent: u32,
    pub label: String,
    pub message: String,
}

#[derive(Default)]
struct AppState {
    last_scan: Mutex<Option<ScanReport>>,
}

fn emit_progress(app: &tauri::AppHandle, percent: u32, label: &str, message: &str) {
    let _ = app.emit(
        "apex-progress",
        ProgressEvent {
            percent,
            label: label.to_string(),
            message: message.to_string(),
        },
    );
}

/// `YYYY-MM-DDTHH:MM:SSZ` without pulling in a date-time crate.
pub(crate) fn now_iso8601() -> String {
    let seconds = std::time::SystemTime::now()
        .duration_since(std::time::UNIX_EPOCH)
        .map(|elapsed| elapsed.as_secs() as i64)
        .unwrap_or(0);

    let days = seconds.div_euclid(86_400);
    let time_of_day = seconds.rem_euclid(86_400);

    // Howard Hinnant's civil_from_days.
    let z = days + 719_468;
    let era = if z >= 0 { z } else { z - 146_096 } / 146_097;
    let day_of_era = z - era * 146_097;
    let year_of_era =
        (day_of_era - day_of_era / 1_460 + day_of_era / 36_524 - day_of_era / 146_096) / 365;
    let mut year = year_of_era + era * 400;
    let day_of_year = day_of_era - (365 * year_of_era + year_of_era / 4 - year_of_era / 100);
    let month_prime = (5 * day_of_year + 2) / 153;
    let day = day_of_year - (153 * month_prime + 2) / 5 + 1;
    let month = if month_prime < 10 {
        month_prime + 3
    } else {
        month_prime - 9
    };
    if month <= 2 {
        year += 1;
    }

    format!(
        "{year:04}-{month:02}-{day:02}T{:02}:{:02}:{:02}Z",
        time_of_day / 3_600,
        (time_of_day % 3_600) / 60,
        time_of_day % 60
    )
}

fn status_impl() -> ApexStatus {
    let pid = process::find_process(GAME_EXE);
    let client_module = pid
        .and_then(|id| process::find_module(id, CLIENT_MODULE))
        .is_some();

    let (dataset_path, dataset_version, dataset_entries) =
        match definitions::resolve_dataset_path() {
            Ok(path) => match Dataset::load(&path) {
                Ok(dataset) => (path.display().to_string(), dataset.version, dataset.entries.len()),
                Err(_) => (path.display().to_string(), String::new(), 0),
            },
            Err(_) => (String::new(), String::new(), 0),
        };

    ApexStatus {
        game_running: pid.is_some(),
        game_pid: pid,
        client_module,
        dataset_path,
        dataset_version,
        dataset_entries,
        app_version: env!("CARGO_PKG_VERSION").to_string(),
    }
}

/// Scans every dataset entry against the live `client.dll`.
fn scan_impl(app: &tauri::AppHandle) -> Result<ScanReport, ApexError> {
    emit_progress(app, 4, "DATASET", "");

    let dataset_path = definitions::resolve_dataset_path()?;
    let dataset = Dataset::load(&dataset_path)?;

    emit_progress(app, 14, "PROCESS", "");

    let pid = process::find_process(GAME_EXE).ok_or_else(|| {
        ApexError::new(
            "E_APEX_NO_GAME",
            "cs2.exe is not running. Start the game, then check again.",
        )
    })?;

    let game = process::GameProcess::open(pid, CLIENT_MODULE).ok_or_else(|| {
        ApexError::new(
            "E_APEX_NO_MODULE",
            "client.dll is not loaded yet. Wait until CS2 reaches the main menu.",
        )
    })?;

    emit_progress(app, 24, "READING", "");

    let module = game.read_module().ok_or_else(|| {
        ApexError::new(
            "E_APEX_NO_MODULE",
            "client.dll memory could not be read. Try running Apex as administrator.",
        )
    })?;

    let mut entries = Vec::with_capacity(dataset.entries.len());
    let total = dataset.entries.len();

    for (index, entry) in dataset.entries.iter().enumerate() {
        let percent = 30 + ((index as u32) * 60) / total.max(1) as u32;
        emit_progress(app, percent, "SCANNING", "");

        let result = scan_entry(&module, entry);
        entries.push(result);
    }

    emit_progress(app, 96, "RESOLVING", "");

    let resolved = entries
        .iter()
        .filter(|entry| entry.found_rva.is_some())
        .count();

    if resolved == 0 {
        return Err(ApexError::new(
            "E_APEX_SCAN_EMPTY",
            "None of the dataset signatures matched client.dll. The patterns themselves are outdated and need new signatures, not new offsets.",
        ));
    }

    let report = ScanReport {
        moved: entries.iter().filter(|e| e.status == EntryStatus::Moved).count(),
        missing: entries.iter().filter(|e| e.status == EntryStatus::Missing).count(),
        new_count: entries.iter().filter(|e| e.status == EntryStatus::New).count(),
        confidence_high: entries.iter().filter(|e| e.confidence == Confidence::High).count(),
        confidence_medium: entries.iter().filter(|e| e.confidence == Confidence::Medium).count(),
        confidence_low: entries
            .iter()
            .filter(|e| matches!(e.confidence, Confidence::Low | Confidence::None))
            .count(),
        total: entries.len(),
        entries,
        dataset_version: dataset.version,
        dataset_path: dataset_path.display().to_string(),
        scanned_at: now_iso8601(),
    };

    if let Some(state) = app.try_state::<AppState>() {
        if let Ok(mut last) = state.last_scan.lock() {
            *last = Some(report.clone());
        }
    }

    emit_progress(app, 100, "DONE", "");
    Ok(report)
}

/// A signature hit alone does not prove the address is the global the entry
/// claims. The module is mapped, so the value at the resolved RVA can be read
/// back: a plausible, aligned user-mode pointer is what a real global holds at
/// the point its signature is scanned.
fn validate_global(module: &[u8], rva: Option<u64>) -> (Confidence, String) {
    let Some(rva) = rva else {
        return (
            Confidence::None,
            "No address was resolved from the signature.".to_string(),
        );
    };

    let start = match usize::try_from(rva) {
        Ok(start) => start,
        Err(_) => {
            return (
                Confidence::Low,
                "The resolved address does not fit the module.".to_string(),
            )
        }
    };

    let Some(end) = start.checked_add(8) else {
        return (
            Confidence::Low,
            "The resolved address does not fit the module.".to_string(),
        );
    };

    let Some(bytes) = module.get(start..end) else {
        return (
            Confidence::Low,
            "The resolved address is outside client.dll.".to_string(),
        );
    };

    let pointer = u64::from_le_bytes([
        bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5], bytes[6], bytes[7],
    ]);

    if pointer == 0 {
        return (
            Confidence::Low,
            "The global at this address is null.".to_string(),
        );
    }

    // Canonical user-space addresses on x64 sit below the 0x7FFF_FFFF_FFFF mark.
    if pointer < 0x1_0000 || pointer > 0x0000_7FFF_FFFF_FFFF {
        return (
            Confidence::Low,
            "The global does not hold a user-mode pointer.".to_string(),
        );
    }

    if pointer % 16 == 0 {
        (
            Confidence::High,
            format!("Global holds an aligned pointer (0x{pointer:X})."),
        )
    } else {
        (
            Confidence::Medium,
            format!("Global holds a pointer (0x{pointer:X}) that is not 16-byte aligned."),
        )
    }
}

fn scan_entry(module: &[u8], entry: &DatasetEntry) -> EntryResult {
    let pattern = match scan::Pattern::parse(&entry.pattern) {
        Ok(pattern) => pattern,
        Err(error) => {
            return EntryResult {
                name: entry.name.clone(),
                module: entry.module.clone(),
                status: EntryStatus::Missing,
                stored_rva: entry.rva,
                found_rva: None,
                confidence: Confidence::None,
                validation: format!("The dataset pattern is invalid: {error}"),
            }
        }
    };

    let found_rva = scan::find(module, &pattern)
        .and_then(|offset| scan::resolve_rip32(module, offset, entry.displacement));

    let (confidence, validation) = validate_global(module, found_rva);

    let status = match (found_rva, entry.rva) {
        (None, _) => EntryStatus::Missing,
        (Some(_), None) => EntryStatus::New,
        (Some(found), Some(stored)) if found == stored => EntryStatus::Match,
        (Some(_), Some(_)) => EntryStatus::Moved,
    };

    EntryResult {
        name: entry.name.clone(),
        module: entry.module.clone(),
        status,
        stored_rva: entry.rva,
        found_rva,
        confidence,
        validation,
    }
}

/// Writes the offsets found by the last scan into the dataset.
fn apply_impl(app: &tauri::AppHandle) -> Result<ApplyReport, ApexError> {
    let report = app
        .try_state::<AppState>()
        .and_then(|state| state.last_scan.lock().ok().and_then(|last| last.clone()))
        .ok_or_else(|| {
            ApexError::new(
                "E_APEX_SCAN_EMPTY",
                "Run Find Offset before updating; there is no verified scan to apply.",
            )
        })?;

    if report.dataset_path.is_empty() {
        return Err(ApexError::new(
            "E_APEX_DATASET",
            "The dataset path is unknown; restart Apex and try again.",
        ));
    }

    let path = PathBuf::from(&report.dataset_path);
    let mut dataset = Dataset::load(&path)?;
    let mut changed = Vec::new();
    let mut blocked = Vec::new();
    let verified_at = now_iso8601();

    for entry in dataset.entries.iter_mut() {
        let Some(result) = report.entries.iter().find(|item| item.name == entry.name) else {
            continue;
        };

        if !matches!(result.status, EntryStatus::Moved | EntryStatus::New) {
            continue;
        }

        let Some(found) = result.found_rva else {
            continue;
        };

        // Nothing that failed the runtime check reaches the dataset the release
        // ships; it stays as a candidate for the next scan instead.
        if !result.confidence.trusted() {
            blocked.push(entry.name.clone());
            continue;
        }

        if entry.rva != Some(found) {
            entry.rva = Some(found);
            entry.confidence = Some(result.confidence.label().to_string());
            entry.verified_at = Some(verified_at.clone());
            changed.push(entry.name.clone());
        }
    }

    let mut backup_path = None;

    if !changed.is_empty() {
        dataset.bump_version();
        dataset.updated_at = now_iso8601();

        emit_progress(app, 60, "WRITING", "");
        backup_path = dataset.save(&path)?.map(|path| path.display().to_string());

        if let Some(state) = app.try_state::<AppState>() {
            if let Ok(mut last) = state.last_scan.lock() {
                *last = None;
            }
        }
    }

    emit_progress(app, 100, "SYNCED", "");

    Ok(ApplyReport {
        dataset_path: path.display().to_string(),
        backup_path,
        dataset_version: dataset.version,
        changed,
        blocked,
        updated_at: dataset.updated_at,
    })
}

/// Replaces the dataset with the definitions from the configured source.
///
/// The fetched document is validated before anything is written, the previous
/// file is kept as `.bak`, and the scan state is cleared so an apply can never
/// pair definitions from one file with findings from another.
fn fetch_impl(app: &tauri::AppHandle) -> Result<FetchReport, ApexError> {
    emit_progress(app, 8, "SOURCE", "");
    let source = definitions::resolve_source()?;

    emit_progress(app, 20, "DOWNLOAD", "");
    let raw = definitions::read_source(&source)?;

    emit_progress(app, 55, "VALIDATING", "");
    let fetched = Dataset::parse(&raw, &source.describe())?;

    let path = definitions::resolve_dataset_path()?;
    let previous = Dataset::load(&path).ok();

    let mut changed = Vec::new();
    match &previous {
        Some(previous) => {
            for entry in &fetched.entries {
                let unchanged = previous
                    .entries
                    .iter()
                    .find(|existing| existing.name == entry.name)
                    .map(|existing| existing.rva == entry.rva && existing.pattern == entry.pattern)
                    .unwrap_or(false);
                if !unchanged {
                    changed.push(entry.name.clone());
                }
            }
        }
        None => changed.extend(fetched.entries.iter().map(|entry| entry.name.clone())),
    }

    emit_progress(app, 82, "WRITING", "");
    let backup_path = fetched.save(&path)?;

    if let Some(state) = app.try_state::<AppState>() {
        if let Ok(mut last) = state.last_scan.lock() {
            *last = None;
        }
    }

    emit_progress(app, 100, "FETCHED", "");

    Ok(FetchReport {
        dataset_path: path.display().to_string(),
        backup_path: backup_path.map(|path| path.display().to_string()),
        dataset_version: fetched.version,
        source: source.describe(),
        changed,
        updated_at: fetched.updated_at,
    })
}

#[tauri::command]
async fn apex_status() -> Result<ApexStatus, ApexError> {
    tauri::async_runtime::spawn_blocking(status_impl)
        .await
        .map_err(ApexError::internal)
}

#[tauri::command]
async fn apex_scan(app: tauri::AppHandle) -> Result<ScanReport, ApexError> {
    let handle = app.clone();
    tauri::async_runtime::spawn_blocking(move || scan_impl(&handle))
        .await
        .map_err(ApexError::internal)?
}

#[tauri::command]
async fn apex_apply(app: tauri::AppHandle) -> Result<ApplyReport, ApexError> {
    let handle = app.clone();
    tauri::async_runtime::spawn_blocking(move || apply_impl(&handle))
        .await
        .map_err(ApexError::internal)?
}

#[tauri::command]
async fn apex_fetch(app: tauri::AppHandle) -> Result<FetchReport, ApexError> {
    let handle = app.clone();
    tauri::async_runtime::spawn_blocking(move || fetch_impl(&handle))
        .await
        .map_err(ApexError::internal)?
}

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    tauri::Builder::default()
        .manage(AppState::default())
        .invoke_handler(tauri::generate_handler![
            apex_status,
            apex_scan,
            apex_apply,
            apex_fetch
        ])
        .run(tauri::generate_context!())
        .expect("error while running N1ssan Sync");
}
