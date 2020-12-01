#!/bin/bash

STYLE="\e[36;1m\e[1m"
RESET="\e[0m"

TYPE=${TYPE:-Release}

mkdir -p release
cd release

echo -e "${STYLE}Compiling moonjit...${RESET}"
pushd ../moonjit/src
	make XCFLAGS+=-DLUAJIT_ENABLE_LUA52COMPAT
popd

echo -e "${STYLE}Compiling RosaServer (${TYPE})...${RESET}"
cmake -DCMAKE_BUILD_TYPE=${TYPE} ..
make