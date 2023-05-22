#!/usr/bin/env bash

make clean > /dev/null 2>&1

if [ "$1" == "debug" ]
then
    docker build . -f Dockerfile.debug -t timothyliuxf/bgp_platform:debug
elif [ "$1" == "experiment" ]
then
    docker build . -f Dockerfile -t timothyliuxf/bgp_platform:experiment
elif [ "$1" == "" ] || [ "$1" == "latest" ]
then
    docker build . -f Dockerfile -t timothyliuxf/bgp_platform
else
    echo "Usage: ./build_docker.sh [debug|experiment|latest|]" >&2
    exit 1
fi
