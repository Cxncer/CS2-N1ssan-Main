//! Locates `cs2.exe`, maps `client.dll`, and reads its bytes.
//!
//! The app only reads memory: it never writes to the game process, so Apex can
//! run safely before, during, or after a CS2 update.

#[cfg(windows)]
mod platform {
    use std::ffi::c_void;
    use std::mem::size_of;

    use windows_sys::Win32::Foundation::CloseHandle;
    use windows_sys::Win32::System::Diagnostics::Debug::ReadProcessMemory;
    use windows_sys::Win32::System::Diagnostics::ToolHelp::{
        CreateToolhelp32Snapshot, Module32FirstW, Module32NextW, Process32FirstW, Process32NextW,
        MODULEENTRY32W, PROCESSENTRY32W, TH32CS_SNAPMODULE, TH32CS_SNAPMODULE32,
        TH32CS_SNAPPROCESS,
    };
    use windows_sys::Win32::System::Threading::{
        OpenProcess, PROCESS_QUERY_INFORMATION, PROCESS_VM_READ,
    };

    fn wide_to_string(buffer: &[u16]) -> String {
        let end = buffer.iter().position(|unit| *unit == 0).unwrap_or(buffer.len());
        String::from_utf16_lossy(&buffer[..end])
    }

    /// Process ID of a running executable, matched case-insensitively.
    pub fn find_process(exe_name: &str) -> Option<u32> {
        unsafe {
            let snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            if snapshot.is_null() {
                return None;
            }

            let mut entry: PROCESSENTRY32W = std::mem::zeroed();
            entry.dwSize = size_of::<PROCESSENTRY32W>() as u32;

            let mut found = None;
            let mut next = Process32FirstW(snapshot, &mut entry) != 0;

            while next {
                if wide_to_string(&entry.szExeFile).eq_ignore_ascii_case(exe_name) {
                    found = Some(entry.th32ProcessID);
                    break;
                }
                next = Process32NextW(snapshot, &mut entry) != 0;
            }

            CloseHandle(snapshot);
            found
        }
    }

    /// Base address and size of a module loaded into `pid`.
    pub fn find_module(pid: u32, module_name: &str) -> Option<(u64, usize)> {
        unsafe {
            let snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
            if snapshot.is_null() {
                return None;
            }

            let mut entry: MODULEENTRY32W = std::mem::zeroed();
            entry.dwSize = size_of::<MODULEENTRY32W>() as u32;

            let mut found = None;
            let mut next = Module32FirstW(snapshot, &mut entry) != 0;

            while next {
                if wide_to_string(&entry.szModule).eq_ignore_ascii_case(module_name) {
                    found = Some((entry.modBaseAddr as u64, entry.modBaseSize as usize));
                    break;
                }
                next = Module32NextW(snapshot, &mut entry) != 0;
            }

            CloseHandle(snapshot);
            found
        }
    }

    pub struct GameProcess {
        handle: *mut c_void,
        base: u64,
        size: usize,
    }

    impl Drop for GameProcess {
        fn drop(&mut self) {
            unsafe {
                CloseHandle(self.handle);
            }
        }
    }

    impl GameProcess {
        pub fn open(pid: u32, module_name: &str) -> Option<Self> {
            let (base, size) = find_module(pid, module_name)?;

            unsafe {
                let handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 0, pid);
                if handle.is_null() {
                    return None;
                }
                Some(Self { handle, base, size })
            }
        }

        pub fn read_module(&self) -> Option<Vec<u8>> {
            let mut buffer = vec![0u8; self.size];
            let mut read = 0usize;

            let ok = unsafe {
                ReadProcessMemory(
                    self.handle,
                    self.base as *const c_void,
                    buffer.as_mut_ptr() as *mut c_void,
                    self.size,
                    &mut read,
                )
            };

            if ok == 0 || read == 0 {
                return None;
            }

            buffer.truncate(read);
            Some(buffer)
        }
    }
}

#[cfg(not(windows))]
mod platform {
    pub fn find_process(_exe_name: &str) -> Option<u32> {
        None
    }

    pub fn find_module(_pid: u32, _module_name: &str) -> Option<(u64, usize)> {
        None
    }

    pub struct GameProcess;

    impl GameProcess {
        pub fn open(_pid: u32, _module_name: &str) -> Option<Self> {
            None
        }

        pub fn read_module(&self) -> Option<Vec<u8>> {
            None
        }
    }
}

pub use platform::{find_module, find_process, GameProcess};
