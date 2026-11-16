//! The offset dataset (`offsets.json`) that Apex scans against and rewrites.
//!
//! Schema version 1 is documented in the project's `contracts.md`. The file is
//! shipped with the release (next to the packaged runtime) and next to the
//! application during development, so Apex always edits the file the build uses.

use std::fs;
use std::path::{Path, PathBuf};
use std::process::Command;

use serde::{Deserialize, Serialize};

use crate::ApexError;

pub const SCHEMA_VERSION: u32 = 1;
const DATASET_NAME: &str = "n1ssan-offsets";

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct Dataset {
    pub schema_version: u32,
    pub dataset: String,
    pub version: String,
    pub updated_at: String,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub note: Option<String>,
    pub entries: Vec<DatasetEntry>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct DatasetEntry {
    pub name: String,
    pub module: String,
    pub pattern: String,
    /// Byte offset of the rel32 field inside the matched instruction.
    pub displacement: u32,
    /// Absolute module offset of the global the instruction references.
    #[serde(default)]
    pub rva: Option<u64>,
    /// Confidence Apex recorded when it last resolved and validated this entry.
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub confidence: Option<String>,
    /// Timestamp of that validation.
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub verified_at: Option<String>,
}

impl Dataset {
    pub fn load(path: &Path) -> Result<Self, ApexError> {
        let raw = fs::read_to_string(path).map_err(|error| {
            ApexError::new(
                "E_APEX_DATASET",
                format!("Cannot read {}: {error}", path.display()),
            )
        })?;

        Self::parse(&raw, &path.display().to_string())
    }

    /// Parses and validates a dataset document from any origin (file or fetch).
    pub fn parse(raw: &str, origin: &str) -> Result<Self, ApexError> {
        let dataset: Dataset = serde_json::from_str(raw).map_err(|error| {
            ApexError::new(
                "E_APEX_DATASET",
                format!("{origin} is not a valid offset dataset: {error}"),
            )
        })?;

        if dataset.schema_version != SCHEMA_VERSION {
            return Err(ApexError::new(
                "E_APEX_DATASET",
                format!(
                    "Dataset schema version {} is not supported (expected {SCHEMA_VERSION})",
                    dataset.schema_version
                ),
            ));
        }

        if dataset.dataset != DATASET_NAME {
            return Err(ApexError::new(
                "E_APEX_DATASET",
                format!(
                    "{origin} is dataset '{}', not '{DATASET_NAME}'",
                    dataset.dataset
                ),
            ));
        }

        if dataset.entries.is_empty() {
            return Err(ApexError::new(
                "E_APEX_DATASET",
                format!("{origin} contains no offset entries"),
            ));
        }

        Ok(dataset)
    }

    /// Writes the dataset, keeping the previous file as `<name>.bak`.
    pub fn save(&self, path: &Path) -> Result<Option<PathBuf>, ApexError> {
        let mut backup = None;

        if path.exists() {
            let backup_path = path.with_extension("json.bak");
            fs::copy(path, &backup_path).map_err(|error| {
                ApexError::new(
                    "E_APEX_WRITE",
                    format!("Cannot back up {}: {error}", path.display()),
                )
            })?;
            backup = Some(backup_path);
        }

        let mut json = serde_json::to_string_pretty(self).map_err(|error| {
            ApexError::new(
                "E_APEX_WRITE",
                format!("Cannot serialize the dataset: {error}"),
            )
        })?;
        json.push('\n');

        fs::write(path, json).map_err(|error| {
            ApexError::new(
                "E_APEX_WRITE",
                format!("Cannot write {}: {error}", path.display()),
            )
        })?;

        Ok(backup)
    }

    /// `2026.11.13.1` becomes `2026.11.13.2`; anything unparsable gains `.1`.
    pub fn bump_version(&mut self) {
        let (head, tail) = match self.version.rsplit_once('.') {
            Some((head, tail)) => (head, tail),
            None => ("", self.version.as_str()),
        };

        match tail.parse::<u32>() {
            Ok(revision) => self.version = format!("{head}.{}", revision + 1),
            Err(_) => self.version = format!("{}.1", self.version),
        }
    }
}

/// Locates the dataset the app should read and write.
///
/// Order: `N1SSAN_APEX_DATASET` override, a packaged `offsets.json` beside the
/// executable, then the development `definitions/offsets.json` by walking up.
pub fn resolve_dataset_path() -> Result<PathBuf, ApexError> {
    if let Ok(explicit) = std::env::var("N1SSAN_APEX_DATASET") {
        let path = PathBuf::from(explicit);
        if path.is_file() {
            return Ok(path);
        }
        return Err(ApexError::new(
            "E_APEX_DATASET",
            format!(
                "N1SSAN_APEX_DATASET points at a missing file: {}",
                path.display()
            ),
        ));
    }

    let exe = std::env::current_exe().map_err(|error| {
        ApexError::new(
            "E_APEX_DATASET",
            format!("Cannot resolve the executable path: {error}"),
        )
    })?;

    let mut directory = exe.parent().map(Path::to_path_buf);

    for _ in 0..6 {
        let Some(current) = directory.clone() else {
            break;
        };

        for candidate in [
            current.join("offsets.json"),
            current.join("data").join("offsets.json"),
            current.join("resources").join("offsets.json"),
            current.join("definitions").join("offsets.json"),
        ] {
            if candidate.is_file() {
                return Ok(candidate);
            }
        }

        directory = current.parent().map(Path::to_path_buf);
    }

    Err(ApexError::new(
        "E_APEX_DATASET",
        "offsets.json was not found beside the executable or in definitions/",
    ))
}

// ---------------------------------------------------------------- fetch source

/// Where updated definitions come from. The workspace ships no endpoint of its
/// own, so the source is configured rather than hard-coded.
#[derive(Debug, Clone)]
pub enum Source {
    File(PathBuf),
    Url(String),
}

impl Source {
    pub fn describe(&self) -> String {
        match self {
            Source::File(path) => path.display().to_string(),
            Source::Url(url) => url.clone(),
        }
    }
}

fn source_config_path() -> Option<PathBuf> {
    let base = std::env::var("LOCALAPPDATA").ok()?;
    Some(
        PathBuf::from(base)
            .join("n1ssan")
            .join("configs")
            .join("apex-source.json"),
    )
}

fn parse_source_value(value: &str) -> Option<Source> {
    let trimmed = value.trim();
    if trimmed.is_empty() {
        return None;
    }
    if trimmed.starts_with("http://") || trimmed.starts_with("https://") {
        return Some(Source::Url(trimmed.to_string()));
    }
    if let Some(rest) = trimmed.strip_prefix("file://") {
        return Some(Source::File(PathBuf::from(rest)));
    }
    Some(Source::File(PathBuf::from(trimmed)))
}

#[derive(Deserialize)]
#[serde(rename_all = "camelCase")]
struct SourceConfig {
    #[serde(default)]
    url: Option<String>,
    #[serde(default)]
    file: Option<String>,
    #[serde(default)]
    path: Option<String>,
}

/// Resolution order: `N1SSAN_APEX_SOURCE`, then
/// `%LOCALAPPDATA%\n1ssan\configs\apex-source.json` with `url`, `file`, or `path`.
pub fn resolve_source() -> Result<Source, ApexError> {
    if let Ok(explicit) = std::env::var("N1SSAN_APEX_SOURCE") {
        return parse_source_value(&explicit).ok_or_else(|| {
            ApexError::new(
                "E_APEX_FETCH",
                "N1SSAN_APEX_SOURCE is empty; set it to a URL or a dataset path.",
            )
        });
    }

    let Some(config_path) = source_config_path() else {
        return Err(ApexError::new(
            "E_APEX_FETCH",
            "No definitions source is configured. Set N1SSAN_APEX_SOURCE or create apex-source.json.",
        ));
    };

    if !config_path.is_file() {
        return Err(ApexError::new(
            "E_APEX_FETCH",
            format!(
                "No definitions source is configured. Create {} with a \"url\" or \"file\" field.",
                config_path.display()
            ),
        ));
    }

    let raw = fs::read_to_string(&config_path).map_err(|error| {
        ApexError::new(
            "E_APEX_FETCH",
            format!("Cannot read {}: {error}", config_path.display()),
        )
    })?;

    let config: SourceConfig = serde_json::from_str(&raw).map_err(|error| {
        ApexError::new(
            "E_APEX_FETCH",
            format!("{} is not valid JSON: {error}", config_path.display()),
        )
    })?;

    for value in [config.url, config.file, config.path].into_iter().flatten() {
        if let Some(source) = parse_source_value(&value) {
            return Ok(source);
        }
    }

    Err(ApexError::new(
        "E_APEX_FETCH",
        format!(
            "{} names no source. Add a \"url\" or \"file\" field.",
            config_path.display()
        ),
    ))
}

/// Reads the configured source and returns its raw document.
///
/// HTTP sources use the `curl.exe` that ships with Windows 10 and later, so the
/// app gains no TLS dependency and no network code of its own.
pub fn read_source(source: &Source) -> Result<String, ApexError> {
    match source {
        Source::File(path) => fs::read_to_string(path).map_err(|error| {
            ApexError::new(
                "E_APEX_FETCH",
                format!("Cannot read {}: {error}", path.display()),
            )
        }),
        Source::Url(url) => {
            let output = Command::new("curl.exe")
                .args(["-fsSL", "--max-time", "20", url])
                .output()
                .map_err(|error| {
                    ApexError::new(
                        "E_APEX_FETCH",
                        format!("Cannot run curl.exe for {url}: {error}"),
                    )
                })?;

            if !output.status.success() {
                let detail = String::from_utf8_lossy(&output.stderr);
                let detail = detail.trim();
                return Err(ApexError::new(
                    "E_APEX_FETCH",
                    if detail.is_empty() {
                        format!("Fetching {url} failed with {}.", output.status)
                    } else {
                        format!("Fetching {url} failed: {detail}")
                    },
                ));
            }

            String::from_utf8(output.stdout).map_err(|_| {
                ApexError::new("E_APEX_FETCH", format!("{url} did not return UTF-8 JSON."))
            })
        }
    }
}
