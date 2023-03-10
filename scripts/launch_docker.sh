#!/usr/bin/env bash

if [ "$1" == "debug" ]
then
    TAG=debug
else
    TAG=latest
fi

docker run -v $(pwd)/config:/app/config --network $1 -v $(pwd)/info:/app/info -v $2:/app/data -d timothyliuxf/bgp_platform:$TAG
