.DEFAULT_GOAL := run

run:
	LISTEN_ADDR=":7800" go run .

bench:
	echo "implement"

build:
	go build -o bin/api -ldflags "\
		-X 'github.com/Nathan13888/bs-lindor/api/v2/main.BuildVersion=$$(git rev-parse --abbrev-ref HEAD)' \
		-X 'github.com/Nathan13888/bs-lindor/api/v2/main.BuildUser=$$(id -u -n)' \
		-X 'github.com/Nathan13888/bs-lindor/api/v2/main.BuildTime=$$(date)' \
		-X 'github.com/Nathan13888/bs-lindor/api/v2/main.BuildGOOS=$$(go env GOOS)' \
		-X 'github.com/Nathan13888/bs-lindor/api/v2/main.BuildARCH=$$(go env GOARCH)' \
		-s -w"

docker-build:
	docker build \
		--label "org.opencontainers.image.source=https://github.com/Nathan13888/bs-lindor" \
		-t bs-lindor-api .

	docker tag bs-lindor-api:latest ghcr.io/nathan13888/bs-lindor-api:latest

publish:
	make docker-build
	make publish-ghcr

publish-ghcr:
	docker push ghcr.io/nathan13888/bs-lindor-api:latest
	docker tag ghcr.io/nathan13888/bs-lindor-api:latest ghcr.io/nathan13888/bs-lindor-api:0.0
	docker push ghcr.io/nathan13888/bs-lindor-api:0.0

clean:
	echo "implement"
