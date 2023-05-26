#!/usr/bin/env bash

make clean > /dev/null 2>&1

if [ "$1" == "debug" ]
then
    docker build . -f Dockerfile.debug -t timothyliuxf/bgp_platform:debug
elif [ "$1" == "model" ]
then
    docker build . -f Dockerfile.model -t timothyliuxf/bgp_platform_model
else
    docker build . -f Dockerfile -t timothyliuxf/bgp_platform
fi
