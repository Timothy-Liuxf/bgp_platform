#!/usr/bin/env bash

pushd src && find . -iname "*.hpp" -or -iname "*.cc" | xargs clang-format -i && popd
