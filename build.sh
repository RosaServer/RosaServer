#!/bin/bash

STYLE="\e[36;1m\e[1m"
RESET="\e[0m"

TYPE=${TYPE:-Release}

mkdir -p release
cd release

echo -e "${STYLE}Compiling moonjit...${RESET}"
pushd ../moonjit/src
	make XCFLAGS+=-DLUAJIT_ENABLE_LUA52COMPAT -j${nproc}
popd

echo -e "${STYLE}Compiling RosaServer (${TYPE})...${RESET}"
cmake -DCMAKE_BUILD_TYPE=${TYPE} -DOPENSSL_INCLUDE_DIR=/usr/lib -DOPENSSL_SSL_LIBRARY=/usr/lib/libssl.so -DOPENSSL_CRYPTO_LIBRARY=/usr/lib/libcrypto.so ..
make -j${nproc}
