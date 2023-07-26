package main

import (
	"encoding/json"

	"github.com/rs/zerolog/log"
)

type InferenceRequest struct {
	// TODO: inference timeout
	ID     string `json:"id"`
	Width  int    `json:"width"`
	Height int    `json:"height"`
	Input  Frames `json:"input"`
}

type Frames struct {
	L0Health      [][]int   `json:"l0_health"`
	L1Bodies      [][]int   `json:"l1_bodies"`
	L2Segments    [][]int   `json:"l2_segments"`
	L3SnakeLength [][]int   `json:"l3_snake_length"`
	L4Food        [][]int   `json:"l4_food"`
	L5Board       [][]int   `json:"l5_board"`
	L6HeadMask    [][]int   `json:"l6_head_mask"`
	L7TailMask    [][]int   `json:"l7_tail_mask"`
	L8BodiesGTE   [][]int   `json:"l8_bodies_gte"`
	L9BodiesLT    [][]int   `json:"l9_bodies_lt"`
	AliveCount    [][][]int `json:"alive_count"`
}

type InferenceResponse struct {
	Action SnakeMove `json:"action"`
	Value  float64   `json:"value"`
	Error  string    `json:"error"`
}

// TODO: implement redundant RL model support

func getRLMove(uuid string, state GameState) (InferenceResponse, error) {
	log.Info().Str("uuid", uuid).
		Msg("Generating RL move...")

	// TODO: get logger for particular function/game/turn

	// prepare InferenceRequest
	req := InferenceRequest{
		ID:     uuid,
		Width:  state.Board.Width,
		Height: state.Board.Height,
	}

	// populate frames using game state data

	// LAYER REFERENCE from gamewrapper.cpp
	/**
	layer0: snake health on heads {0,...,100}
	layer1: snake bodies {0,1}
	layer2: segment numbers {0,...,255}
	layer3: snake length >= player {0,1}
	layer4: food {0,1}
	layer5: gameboard {0,1}
	layer6: head_mask {0,1}
	layer7: double_tail_mask {0,1}
	layer8: snake bodies >= us {0,1 + them - us}
	layer9: snake bodies < us {0,-them + us}
	layer10-16: Alive count {0,1}
	*/

	// calculate offset when snake is centred
	// NOTE: the CNN board dimensions is 23x23 so the 12th block is the centre
	//   and the snake coordinates are 0-indexed
	x_offset := 12 - (state.You.Head.X + 1)
	y_offset := 12 - (state.You.Head.Y + 1)

	frames := Frames{}

	// initialize all 23 x 23 matrices
	frames.L0Health = get_layer(23, 23)
	frames.L1Bodies = get_layer(23, 23)
	frames.L2Segments = get_layer(23, 23)
	frames.L3SnakeLength = get_layer(23, 23)
	frames.L4Food = get_layer(23, 23)
	frames.L5Board = get_layer(23, 23)
	frames.L6HeadMask = get_layer(23, 23)
	frames.L7TailMask = get_layer(23, 23)
	frames.L8BodiesGTE = get_layer(23, 23)
	frames.L9BodiesLT = get_layer(23, 23)

	//////////////////////////////////////////////////////////////////////////////////////////
	// GAMEBOARD
	//////////////////////////////////////////////////////////////////////////////////////////

	// find own snake's id
	my_id := state.You.ID

	// process all opponent snakes
	num_of_opponents := len(state.Board.Snakes) - 1

	// iterate through all snakes on the board
	eliminated := true // used to check if board has own snake
	doubled_tails := 0
	for n, snake := range state.Board.Snakes {
		if snake.ID == my_id && snake.Health > 0 {
			eliminated = false
		}

		//////////////////////////////////////////////////////////////////////////////////////////
		// HEAD
		//////////////////////////////////////////////////////////////////////////////////////////

		// add snake head health to layer0
		if snake.Health > 0 {
			frames.L0Health[snake.Head.X+x_offset][snake.Head.Y+y_offset] = snake.Health
		} else { // skip dead snakes
			continue
		}

		// look for equal or longer snakes' head to layer3
		if snake.Length >= state.You.Length {
			frames.L3SnakeLength[snake.Head.X+x_offset][snake.Head.Y+y_offset] = 1
		}

		// apply head hask on layer6
		frames.L6HeadMask[snake.Head.X+x_offset][snake.Head.Y+y_offset] = 1

		//////////////////////////////////////////////////////////////////////////////////////////
		// ALL BODY SEGMENTS (including head + tail)
		//////////////////////////////////////////////////////////////////////////////////////////

		// NOTE: bodies are ordered head -> tail
		// NOTE: a snake has three "parts": head, body, tail
		// NOTE: it's possible a "body" DNE if the snake is <2 long
		// NOTE: it's also possible a snake doesn't have a "tail" if it's 1 long
		total_segmants := len(snake.Body)

		for i := 0; i < total_segmants; i++ { // process all parts
			// add snake bodies to layer1
			frames.L1Bodies[snake.Body[i].X+x_offset][snake.Body[i].Y+y_offset] = 1

			// add segment numbers to layer2 (in descending order to 1)
			frames.L2Segments[snake.Body[i].X+x_offset][snake.Body[i].Y+y_offset] = total_segmants - i

			// look for equal or longer snakes' bodies to layer8
			if snake.Length >= state.You.Length {
				frames.L8BodiesGTE[snake.Body[i].X+x_offset][snake.Body[i].Y+y_offset] = 1
			} else { // otherwise add them to layer9
				frames.L9BodiesLT[snake.Body[i].X+x_offset][snake.Body[i].Y+y_offset] = 1
			}

		}

		//////////////////////////////////////////////////////////////////////////////////////////
		// TAIL ONLY
		//////////////////////////////////////////////////////////////////////////////////////////

		// check if snake is long enough to have tail
		if total_segmants > 1 {
			// look for doubled existing tails to layer7 using layer2
			// iterate on all snakes up to current snake
			for j := 0; j < n; j++ {
				// check if current snake's tail is equal to previous snake's tail
				tail_a := state.Board.Snakes[j].Body[total_segmants-1] // tail of some previous snake
				tail_b := snake.Body[total_segmants-1]                 // tail of current snake
				if tail_a.X == tail_b.X && tail_a.Y == tail_b.Y {
					// add to layer7
					frames.L7TailMask[tail_a.X+x_offset][tail_a.Y+y_offset] = 1

					doubled_tails++
				}
			}
		}
	}

	if eliminated {
		// TODO: return error
		log.Error().Msg("snake is eliminated")
	}

	// if there are more doubled tails than snakes
	if doubled_tails > num_of_opponents/2 {
		// TODO: report potential error?
		log.Warn().Msg("doubled tails > num_of_opponents / 2")
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// ALIVE LAYER (layer10-16)
	//////////////////////////////////////////////////////////////////////////////////////////

	// calculate alive count
	alive_count := num_of_opponents
	// set alive layer to 1 grid of width x height
	frames.AliveCount = make([][][]int, 7)
	for i := 0; i < 7; i++ {
		frames.AliveCount[i] = get_layer(23, 23)
	}

	// TODO improve efficiency
	for x := 0; x < 23; x++ {
		for y := 0; y < 23; y++ {
			// ignore coordinates excluded by offset
			if x < x_offset || y < y_offset {
				continue
			}
			// ignore coordinates excluded by offset + board size
			if x >= x_offset+state.Board.Width || y >= y_offset+state.Board.Height {
				continue
			}
			// enable board mask for gameboard layer5
			frames.L5Board[x][y] = 1

			// enable board mask for alive layer
			frames.AliveCount[alive_count][x][y] = 1
		}
	}

	// TODO: sanity check alive_count
	if alive_count <= 0 || alive_count > 7 {
		// TODO: warning unexpected alive_count
		log.Warn().Int("alive_count", alive_count).
			Msg("unexpected alive_count")
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// FOOD (layer4)
	//////////////////////////////////////////////////////////////////////////////////////////

	// get food and hazards from state.Board
	for _, food := range state.Board.Food {
		frames.L4Food[food.X][food.Y] = 1
	}

	req.Input = frames

	// send HTTP request

	// marshal InferenceRequest into json
	reqBody, err := json.Marshal(req)

	// write reqBody into a local file
	// err = ioutil.WriteFile("req.json", reqBody, 0644)

	if err != nil {
		return InferenceResponse{}, err
	}

	ch := make(chan []byte)

	job := PostRequest{
		Url:     RL_API + "/api/predict",
		Data:    reqBody,
		Channel: ch,
	}

	// send request
	err = job.PostJSON()
	if err != nil {
		return InferenceResponse{}, err
	}

	// parse json into InferenceResponse
	resObj := InferenceResponse{}
	resBody := <-ch

	err = json.Unmarshal(resBody, &resObj)
	if err != nil {
		return resObj, err
	}

	// TODO:
	// num_of_food := len(state.Board.Food)
	// num_of_hazards := len(state.Board.Hazards)

	return resObj, nil
}

func get_layer(width int, height int) [][]int {
	// initialize a width x height matrix
	layer := make([][]int, width)
	for i := range layer {
		layer[i] = make([]int, height)
	}

	return layer
}
