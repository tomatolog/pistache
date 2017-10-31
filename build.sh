#!/bin/bash

git submodule update --init
mkdir -p build
mkdir -p replicator/build
cd replicator/
git submodule update --init
cd ..

docker run -v `pwd`/replicator:/rocksplicator -v `pwd`:/pistache -w /rocksplicator/build angxu/rocksplicator-build:latest bash -c "cmake ..; make -j4"
docker run -v `pwd`/replicator:/rocksplicator -v `pwd`:/pistache -w /pistache/build angxu/rocksplicator-build:latest bash -c "cmake ..; make -j4"
