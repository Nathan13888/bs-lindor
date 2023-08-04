use std::io::Write;
use std::process::{Command, Stdio};

use rocket::serde::json::Json;
use serde_json::{Value, json};

const EXE_PATH: &str = "../supervisor/cmake-build-release/supervisor.exe";

pub fn run(move_req: Json<Value>) -> Value {
    let mut child = Command::new(EXE_PATH)
        .stdin(Stdio::piped())
        .stdout(Stdio::piped())
        .spawn()
        .expect("Failed to spawn child process");

    let mut stdin = child.stdin.take().expect("Failed to open stdin");
    std::thread::spawn(move || {
        stdin
            .write_all(move_req.to_string().as_bytes())
            .expect("Failed to write to stdin");
    });

    let output = child
        .wait_with_output()
        .expect("Failed to read stdout");

    println!("stdout: {}", String::from_utf8_lossy(&output.stdout));

    let resp_obj: Value = serde_json::from_str(match std::str::from_utf8(&output.stdout) {
        Ok(v) => v,
        Err(e) => panic!("Invalid UTF-8 sequence: {}", e),
    }).unwrap();

    return json!(resp_obj);
}