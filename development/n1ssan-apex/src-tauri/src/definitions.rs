//! The offset dataset (`offsets.json`) that Apex scans against and rewrites.
//!
//! Schema version 1 is documented in the project's `contracts.md`. The file is
//! shipped with the release (next to the packaged runtime) and next to the
//! application during development, so Apex always edits the file the build uses.

use std::fs;
use std::path::{Path, PathBuf};

use serde::{Deserialize, Serialize};

use crate::ApexError;

pub const SCHEMA_VERSION: u32 = 1;

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
}

impl Dataset {
    pub fn load(path: &Path) -> Result<Self, ApexError> {
        let raw = fs::read_to_string(path).map_err(|error| {
            ApexError::new(
                "E_APEX_DATASET",
                format!("Cannot read {}: {error}", path.display()),
            )
        })?;

        let dataset: Dataset = serde_json::from_str(&raw).map_err(|error| {
            ApexError::new(
                "E_APEX_DATASET",
                format!("{} is not a valid offset dataset: {error}", path.display()),
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

        if dataset.entries.is_empty() {
            return Err(ApexError::new(
                "E_APEX_DATASET",
                format!("{} contains no offset entries", path.display()),
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
            ApexError::new("E_APEX_WRITE", format!("Cannot serialize the dataset: {error}"))
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
            format!("N1SSAN_APEX_DATASET points at a missing file: {}", path.display()),
        ));
    }

    let exe = std::env::current_exe().map_err(|error| {
        ApexError::new("E_APEX_DATASET", format!("Cannot resolve the executable path: {error}"))
    })?;

    let mut directory = exe.parent().map(Path::to_path_buf);

    for _ in 0..6 {
        let Some(current) = directory.clone() else {
            break;
        };

        for candidate in [
            current.join("offsets.json"),
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
