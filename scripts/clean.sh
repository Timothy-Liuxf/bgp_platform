#!/usr/bin/env bash

make distclean
rm -rf build
find . -iname .deps | xargs rm -rf
