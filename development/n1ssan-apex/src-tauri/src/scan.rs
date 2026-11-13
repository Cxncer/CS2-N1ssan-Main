//! IDA-style signature patterns (`48 8B 1D ? ? ? ? ...`) and a plain matcher
//! over a module's bytes. The same pattern text the native DLL uses, so a hit
//! here means the DLL's own scan will hit too.

#[derive(Debug, Clone)]
pub struct Pattern {
    bytes: Vec<u8>,
    wildcards: Vec<bool>,
}

impl Pattern {
    pub fn parse(text: &str) -> Result<Self, String> {
        let mut bytes = Vec::new();
        let mut wildcards = Vec::new();

        for token in text.split_whitespace() {
            if token == "?" || token == "??" {
                bytes.push(0);
                wildcards.push(true);
                continue;
            }

            let value = u8::from_str_radix(token, 16)
                .map_err(|_| format!("invalid pattern byte '{token}'"))?;
            bytes.push(value);
            wildcards.push(false);
        }

        if bytes.is_empty() {
            return Err("empty pattern".to_string());
        }

        Ok(Self { bytes, wildcards })
    }

    pub fn len(&self) -> usize {
        self.bytes.len()
    }

    /// First non-wildcard byte, used to skip ahead quickly.
    fn anchor(&self) -> Option<(usize, u8)> {
        self.wildcards
            .iter()
            .position(|wildcard| !wildcard)
            .map(|index| (index, self.bytes[index]))
    }
}

/// Returns the offset of the first match inside `haystack`.
pub fn find(haystack: &[u8], pattern: &Pattern) -> Option<usize> {
    let length = pattern.len();
    if length == 0 || haystack.len() < length {
        return None;
    }

    let last_start = haystack.len() - length;
    let mut start = 0;

    while start <= last_start {
        if let Some((anchor_index, anchor_byte)) = pattern.anchor() {
            match haystack[start + anchor_index..=last_start + anchor_index]
                .iter()
                .position(|byte| *byte == anchor_byte)
            {
                Some(skip) => start += skip,
                None => return None,
            }
        }

        let matched = pattern
            .bytes
            .iter()
            .zip(pattern.wildcards.iter())
            .enumerate()
            .all(|(index, (expected, wildcard))| *wildcard || haystack[start + index] == *expected);

        if matched {
            return Some(start);
        }

        start += 1;
    }

    None
}

/// Reads the little-endian rel32 at `displacement` and returns the address the
/// instruction references, relative to the module base.
pub fn resolve_rip32(module: &[u8], match_offset: usize, displacement: u32) -> Option<u64> {
    let start = match_offset + displacement as usize;
    let bytes = module.get(start..start + 4)?;
    let rel = i32::from_le_bytes([bytes[0], bytes[1], bytes[2], bytes[3]]);
    let next = match_offset as i64 + displacement as i64 + 4;
    let target = next + rel as i64;

    if target < 0 || target as usize >= module.len() {
        return None;
    }

    Some(target as u64)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn parses_wildcards_and_bytes() {
        let pattern = Pattern::parse("48 8B 1D ? ? ? ? 48 89 1D").unwrap();
        assert_eq!(pattern.len(), 10);
        assert!(pattern.wildcards[3]);
        assert!(!pattern.wildcards[0]);
    }

    #[test]
    fn finds_a_signature_after_noise() {
        let module = [0u8, 1, 2, 3, 0x48, 0x8B, 0x1D, 0xAA, 0xBB, 0xCC, 0xDD, 0x90];
        let pattern = Pattern::parse("48 8B 1D ? ? ? ?").unwrap();
        assert_eq!(find(&module, &pattern), Some(4));
    }

    #[test]
    fn reports_no_match_for_absent_signature() {
        let module = [0x90u8; 16];
        let pattern = Pattern::parse("48 8B 1D ? ? ? ?").unwrap();
        assert_eq!(find(&module, &pattern), None);
    }

    #[test]
    fn resolves_rip_relative_target() {
        // lea rax, [rip+0x10] at offset 4: displacement 0x10 lands 4 + 7 + 0x10.
        let mut module = vec![0u8; 64];
        module[4] = 0x48;
        module[5] = 0x8D;
        module[6] = 0x05;
        module[7..11].copy_from_slice(&0x10i32.to_le_bytes());

        let pattern = Pattern::parse("48 8D 05 ? ? ? ?").unwrap();
        let hit = find(&module, &pattern).unwrap();
        assert_eq!(resolve_rip32(&module, hit, 3), Some(27));
    }

    #[test]
    fn rejects_targets_outside_the_module() {
        let mut module = vec![0u8; 16];
        module[0] = 0x48;
        module[1] = 0x8B;
        module[2] = 0x05;
        module[3..7].copy_from_slice(&0x7FFF_FFFFi32.to_le_bytes());
        assert_eq!(resolve_rip32(&module, 0, 3), None);
    }
}
