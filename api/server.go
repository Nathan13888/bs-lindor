package main

import (
	"os"
	"time"

	"github.com/gofiber/fiber/v2"
	"github.com/rs/zerolog"
	"github.com/rs/zerolog/log"
	"github.com/rs/zerolog/pkgerrors"
)

var (
	LISTEN_ADDR = ":7800"
	OWNER       = "wocrekcatta"
	HEAD        = "default"
	TAIL        = "default"
	COLOR       = "#6BCAE2"
	RL_API      = "http://localhost:7801"
)

func main() {
	// env variables
	LISTEN_ADDR = os.Getenv("LISTEN_ADDR")
	if len(LISTEN_ADDR) == 0 {
		LISTEN_ADDR = ":7800"
	}
	OWNER = os.Getenv("OWNER")
	if len(OWNER) == 0 {
		OWNER = "wocrekcatta"
	}
	HEAD = os.Getenv("HEAD")
	if len(HEAD) == 0 {
		HEAD = "default"
	}
	TAIL = os.Getenv("TAIL")
	if len(TAIL) == 0 {
		TAIL = "default"
	}

	RL_API = os.Getenv("RL_API")
	if len(RL_API) == 0 {
		RL_API = "http://localhost:7801"
	}

	// TODO: log env variables

	configureLogger(true)

	// fiber app
	app := fiber.New()

	// health/debug endpoints
	app.Get("/health", getHealth)
	app.Get("/healthz", getHealth)
	app.Get("/alive", getHealth)
	app.Get("/status", getStatus)

	// battle snake api
	app.Get("/", func(c *fiber.Ctx) error {
		return c.JSON(&BattlesnakeInfoResponse{
			APIVersion: "1",
			Author:     OWNER, // must match the snake's owner
			Color:      COLOR,
			Head:       HEAD,
			Tail:       TAIL,
		})
	})

	app.Post("/start", postStart)
	app.Post("/end", postEnd)
	app.Post("/move", postMove)

	// telem api
	// TODO: implement

	// TODO: logging middleware with zerolog
	// TODO: jaegar tracing

	err := app.Listen(LISTEN_ADDR)
	if err != nil {
		log.Fatal().Err(err).Msg("Failed to start server.")
	}
}

var timeFormat = time.RFC3339

func configureLogger(debug bool) {
	zerolog.SetGlobalLevel(zerolog.InfoLevel)
	if debug {
		zerolog.SetGlobalLevel(zerolog.DebugLevel)
	}
	zerolog.TimeFieldFormat = timeFormat
	zerolog.ErrorStackMarshaler = pkgerrors.MarshalStack

	consoleWriter := zerolog.ConsoleWriter{Out: os.Stdout}
	multi := zerolog.MultiLevelWriter(consoleWriter) // TODO: add extra logging outputs
	log.Logger = zerolog.New(multi).With().Timestamp().Logger()
}

// API endpoints

func getHealth(c *fiber.Ctx) error {
	log.Info().Msg("Health check.")

	return c.SendStatus(200)
}

type StatusResponse struct {
	Status       string `json:"status"`
	BuildVersion string `json:"build_version"`
	BuildTime    string `json:"build_time"`
	BuildUser    string `json:"build_user"`
	BuildGOOS    string `json:"build_goos"`
	BuildARCH    string `json:"build_arch"`
	GOOS         string `json:"goos"`
	GOARCH       string `json:"goarch"`
}

func getStatus(c *fiber.Ctx) error {
	log.Info().Msg("Status check.")

	return c.JSON(&StatusResponse{
		Status:       "ok",
		BuildVersion: BuildVersion,
		BuildTime:    BuildTime,
		BuildUser:    BuildUser,
		BuildGOOS:    BuildGOOS,
		BuildARCH:    BuildARCH,
		GOOS:         GOOS,
		GOARCH:       GOARCH,
	})
}
