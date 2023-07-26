package main

import "runtime"

var (
	BuildVersion = "development"
	BuildTime    = "unknown"
	BuildUser    = "unknown"
	BuildGOOS    = "unknown"
	BuildARCH    = "unknown"
	GOOS         = runtime.GOOS
	GOARCH       = runtime.GOARCH
)
