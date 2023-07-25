use std::io::Write;
use std::process::{Command, Stdio};

const EXE_PATH: &str = "../supervisor/cmake-build-release/supervisor.exe";

let mut child = Command::new(EXE_PATH)
    .stdin(Stdio::piped())
    .stdout(Stdio::piped())
    .spawn()
    .expect("Failed to spawn child process");

let mut stdin = child.stdin.take().expect("Failed to open stdin");
std::thread::spawn(move || {
    stdin.write_all("Hello, world!".as_bytes()).expect("Failed to write to stdin");
});

let output = child.wait_with_output().expect("Failed to read stdout");