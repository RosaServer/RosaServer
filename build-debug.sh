#!/bin/bash

STYLE="\e[36;1m\e[1m"
RESET="\e[0m"

TYPE=${TYPE:-Debug}

mkdir -p debug
cd debug

echo -e "${STYLE}Compiling moonjit...${RESET}"
pushd ../moonjit/src
	make XCFLAGS+=-DLUAJIT_ENABLE_LUA52COMPAT -j${nproc}
popd

echo -e "${STYLE}Compiling RosaServer (${TYPE})...${RESET}"
cmake -DCMAKE_BUILD_TYPE=${TYPE} ..
make -j${nproc}
