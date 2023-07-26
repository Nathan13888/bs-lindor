FROM golang:1.18-alpine as builder
WORKDIR /build
COPY . .
RUN apk add make
# for correct build time
RUN apk add --no-cache tzdata bash
RUN cp /usr/share/zoneinfo/America/Toronto /etc/localtime

# cached layers until build
RUN go mod download && go mod verify
ENV CGO_ENABLED=0 GOOS=linux GOARCH=amd64
RUN make build



FROM alpine:latest
WORKDIR /app
RUN apk add --no-cache tzdata bash
RUN cp /usr/share/zoneinfo/America/Toronto /etc/localtime
# copy built binary
COPY --from=builder /build/bin/api /app/api

# variables
ENV LISTEN_ADDR=":7800"
ENV OWNER="wocrekcatta"
ENV HEAD="default"
ENV TAIL="default"
ENV RL_API="http://localhost:7801"

EXPOSE 7800/tcp

CMD /app/api