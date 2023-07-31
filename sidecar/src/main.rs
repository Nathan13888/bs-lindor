#[macro_use]
extern crate rocket;
use rocket::{get, post, http::Status, serde::json::Json, serde::Deserialize, serde::Serialize};

use serde_json::Value;
use std::collections::HashMap;
use std::env;

mod logic;

#[derive(Deserialize, Serialize, Debug)]
pub struct Game {
    id: String,
    ruleset: HashMap<String, Value>,
    timeout: u32,
}

#[derive(Deserialize, Serialize, Debug)]
pub struct Board {
    height: u32,
    width: u32,
    food: Vec<Coord>,
    snakes: Vec<Battlesnake>,
    hazards: Vec<Coord>,
}

#[derive(Deserialize, Serialize, Debug)]
pub struct Battlesnake {
    id: String,
    name: String,
    health: u32,
    body: Vec<Coord>,
    head: Coord,
    length: u32,
    latency: String,
    shout: Option<String>,
}

#[derive(Deserialize, Serialize, Debug)]
pub struct Coord {
    x: u32,
    y: u32,
}

#[derive(Deserialize, Serialize, Debug)]
pub struct GameState {
    game: Game,
    turn: u32,
    board: Board,
    you: Battlesnake,
}

#[get("/")]
fn hello() -> Result<Json<String>, Status> {
    Ok(Json(String::from("Hello from sidecar")))
}

#[post("/move", format = "json", data = "<move_req>")]
fn make_move(move_req: Json<Value>) -> Json<Value> {
    let move_resp = logic::run(move_req);
    return Json(move_resp);
}

#[launch]
fn rocket() -> _ {
    if let Ok(port) = env::var("PORT") {
        env::set_var("ROCKET_PORT", &port);
    }

    rocket::build().mount("/", routes![hello, make_move])
}
