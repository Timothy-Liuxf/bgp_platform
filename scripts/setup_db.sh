#!/usr/bin/env bash
docker run --name postgres -e POSTGRES_PASSWORD=bgp1234567 -p 5432:5432 -d postgres
