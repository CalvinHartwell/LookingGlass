use crossbeam::{Receiver, bounded};
use crate::commands::{Frame, Command};
use log::*;
use std::fs;
use std::ffi::CString;
use std::os::raw::c_int;
use std::os::unix::fs::OpenOptionsExt;
use crate::bindings::mode_t;
use std::fs::OpenOptions;
use std::io::Read;

pub fn pipe_thread() {
    let path = CString::new(crate::PIPE_LOCATION).unwrap();
    let mode: u32 = 0o644;
    let result: c_int = unsafe { libc::mkfifo(path.as_ptr(), mode as mode_t) };
    if result != 0 {
        warn!("Failed to create pipe at {} with error code {}", crate::PIPE_LOCATION, result);
    } else {
        info!("Created pipe at {}", crate::PIPE_LOCATION);
    }

    let mut pipe: fs::File = OpenOptions::new()
        .read(true)
        .custom_flags(libc::O_NONBLOCK | libc::O_RDONLY | libc::O_SYNC)
        .open(crate::PIPE_LOCATION)
        .expect(format!("Failed to open named pipe at {}", crate::PIPE_LOCATION).as_str());

    info!("Opened pipe at {}", crate::PIPE_LOCATION);


    loop {
        let mut buf = [0u8; 500000];

        let len = match pipe.read(&mut buf) {
            Err(err) => {
                error!("Error reading from pipe: {}", err);
                continue;
            }
            Ok(len) => len
        };

        // nothing was sent
        if len == 0 {
            continue;
        }

        debug!("Read {} bytes", len);
        dbg!(&buf[0..len]);

        let command: Command = match bincode::deserialize(&buf[0..len]) {
            Err(err) => {
                error!("Error deserializing buffer of length {}: {}", len, err);
                continue;
            }
            Ok(cmd) => cmd
        };

        debug!("Received command: {:?}", command);
        unsafe { command.handle() };
    }
}