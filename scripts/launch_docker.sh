#!/usr/bin/env bash

docker run -v $(pwd)/config:/app/config --network $1 -v $(pwd)/info:/app/info -v $2:/app/data -d timothyliuxf/bgp_platform
