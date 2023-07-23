# Treesearch Sidecar


A proxy from the API to the C++ treesearch algorithm.


## How it works

All requests from Battlesnake go through `api` and separated to either the python RL inference server AND the tree search algorithm written in C++.

The tree search is compiled as an executable and a serialized JSON string is piped into STDIN. The treesearch then outputs the search into STDOUT.

## Why we need it

It's best if the API doesn't need to execute the tree search executable directly. Thus, **a sidecar** could proxy the requests on behalf of the API instead.