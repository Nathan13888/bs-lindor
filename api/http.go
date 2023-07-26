package main

import (
	"bytes"
	"io/ioutil"
	"net/http"
	"time"
)

// TODO: make parallized HTTP workers
// TODO: auto time request and response length

type PostRequest struct {
	Url  string
	Data []byte
}

// const HTTP_TIMEOUT = 350
const HTTP_TIMEOUT = 2000

func (job PostRequest) PostJSON() ([]byte, error) {
	var resBody []byte

	// io.Reader as body
	body := bytes.NewBuffer(job.Data)

	// set up request
	req, err := http.NewRequest("POST", job.Url, body)
	if err != nil {
		return resBody, err
	}

	req.Header.Set("Content-Type", "application/json")

	// create http client
	client := &http.Client{
		Timeout: time.Duration(HTTP_TIMEOUT) * time.Millisecond,
	}

	// send request with body
	res, err := client.Do(req)
	if err != nil {
		return resBody, err
	}

	// parse response body
	defer res.Body.Close()

	resBody, err = ioutil.ReadAll(res.Body)
	if err != nil {
		return resBody, err
	}

	return resBody, nil
}
