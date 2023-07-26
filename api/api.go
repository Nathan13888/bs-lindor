package main

import (
	"crypto/md5"
	"encoding/hex"

	"github.com/gofiber/fiber/v2"
	"github.com/rs/zerolog/log"
)

type SnakeMove int

const (
	MOVE_LEFT  SnakeMove = 0
	MOVE_RIGHT SnakeMove = 1
	MOVE_UP    SnakeMove = 2
	MOVE_DOWN  SnakeMove = 3
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

func postMove(c *fiber.Ctx) error {
	// TODO: (parallel) board size, game mode, move turn, move time
	// TODO: sanity check - Battlesnake.Latency
	// TODO: auto tune using BS latency
	// TODO: compute time remaining after requests

	// send job requests to RL API and tree search

	// force move after timeout

	// tree_scores = tree.compute_scores(game_state)# Check if the policy action is a loser in the future
	// if tree_scores[policy_action] == -Infinity:
	// 		return best_action(tree_scores)# Check if tree search found a winning move
	// if any tree_scores is Infinity:
	// 		return best_action(tree_scores)# Base case: use policy network move
	// return policy_action

	state, err := getState(c)
	if err != nil {
		log.Error().Err(err).Msg("failed to get GameState")
	}

	uuid := genUUID(state)

	log.Info().Str("uuid", uuid).Msg("Moving snake.")

	// query RL API

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

	move := "up" // TODO: default move

	switch infRes.Action {
	case MOVE_LEFT:
		log.Debug().Msg("moving left")
		move = "left"
	case MOVE_RIGHT:
		log.Debug().Msg("moving right")
		move = "right"
	case MOVE_UP:
		log.Debug().Msg("moving up")
		move = "up"
	case MOVE_DOWN:
		log.Debug().Msg("moving down")
		move = "down"
	default:
		log.Error().Msg("invalid move")
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
