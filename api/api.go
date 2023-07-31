package main

import (
	"crypto/md5"
	"encoding/hex"

	"github.com/gofiber/fiber/v2"
	"github.com/rs/zerolog/log"
)

type SnakeMove int

const (
	// {'u', 'd', 'l', 'r'}
	MOVE_LEFT  SnakeMove = 2
	MOVE_RIGHT SnakeMove = 3
	MOVE_UP    SnakeMove = 1
	MOVE_DOWN  SnakeMove = 0
	MOVE_ERROR SnakeMove = -1 // a suitable move couldn't be found
)

func postStart(c *fiber.Ctx) error {
	state, err := getState(c)
	if err != nil {
		log.Error().Err(err).Msg("failed to get GameState")
	}
	log.Info().Str("uuid", genUUID(state)).Msg("Starting game.")

	// TODO: sanity check - board size, move turn, move time

	// job := PostRequest{
	// 	Url:     RL_API + "/api/start",
	// 	Data:    []byte{},
	// 	Channel: nil,
	// }

	// err = job.PostJSON()
	// if err != nil {
	// 	log.Error().Err(err).Msg("failed to post start request")
	// }

	return c.SendStatus(200)
}

func postEnd(c *fiber.Ctx) error {
	state, err := getState(c)
	if err != nil {
		log.Error().Err(err).Msg("failed to get GameState")
	}
	log.Info().Str("uuid", genUUID(state)).
		Int("length", state.Turn).
		Msg("Ending game.")

	// job := PostRequest{
	// 	Url:     RL_API + "/api/end",
	// 	Data:    []byte{},
	// 	Channel: nil,
	// }

	// err = job.PostJSON()
	// if err != nil {
	// 	log.Error().Err(err).Msg("failed to post start request")
	// }

	return c.SendStatus(200)
}

// TODO: implement on the fly options
// TODO: implement redundant APIs
// TODO: implement backup RL API
// TODO: health check
// TODO: API stats: processes requests, avg res time, etc

func postMove(c *fiber.Ctx) error {
	// TODO: (parallel) board size, game mode, move turn, move time
	// TODO: sanity check - Battlesnake.Latency
	// TODO: auto tune using BS latency
	// TODO: compute time remaining after requests

	// send job requests to RL API and tree search

	state, err := getState(c)
	if err != nil {
		log.Error().Err(err).Msg("failed to get GameState")
	}

	uuid := genUUID(state)
	log.Info().Str("uuid", uuid).Msg("Moving snake.")

	// TODO:
	// force move after timeout

	// query RL API
	// TODO: query backup RL

	infRes, err := getRLMove(uuid, state)
	if err != nil {
		log.Error().Err(err).Msg("failed to get RL move")
	}

	if infRes.Action == MOVE_ERROR {
		log.Error().Str("error", infRes.Error).Msg("failed to get RL move")
	}

	log.Debug().Int("action", int(infRes.Action)).
		Float64("value", infRes.Value).
		Msg("got RL move")

	action := infRes.Action

	// TODO: query tree search

	move := "up" // TODO: default move

	switch action {
	case MOVE_LEFT:
		log.Debug().Str("uuid", uuid).Msg("moving left")
		move = "left"
	case MOVE_RIGHT:
		log.Debug().Str("uuid", uuid).Msg("moving right")
		move = "right"
	case MOVE_UP:
		log.Debug().Str("uuid", uuid).Msg("moving up")
		move = "up"
	case MOVE_DOWN:
		log.Debug().Str("uuid", uuid).Msg("moving down")
		move = "down"
	default:
		log.Error().Str("uuid", uuid).Msg("invalid move")
	}

	log.Info().Str("uuid", uuid).
		Str("move", move).
		Msg("sending move")

	applyContext(c)

	return c.JSON(&fiber.Map{
		"move": move,
		// TODO: "shout": "<taunt them>",
		// TODO: get random taunt from taunts.json
	})
}

func getState(c *fiber.Ctx) (GameState, error) {
	// parse state
	state := GameState{}
	err := c.BodyParser(&state)
	return state, err
}

func applyContext(c *fiber.Ctx) {
	c.Accepts("json", "text")
	c.Accepts("text/plain", "application/json")
}

func genUUID(state GameState) string {
	game_id := state.Game.ID
	snake_id := state.You.ID
	new_id := game_id + snake_id

	// create MD5 hash based on both IDs
	md5_hash := md5.Sum([]byte(new_id))

	return hex.EncodeToString(md5_hash[:])
}
