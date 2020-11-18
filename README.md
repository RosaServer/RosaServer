<img src="https://i.imgur.com/4N3PMTS.png" width="600">

A linux server scripting API for [Sub Rosa](http://subrosagame.com/).

**⚠ Early in development, APIs can change at any time.**

RosaServer uses LuaJIT/[moonjit](https://github.com/moonjit/moonjit); this means there's no hit to performance while being able to create anything from moderation tools to complex custom games with easy-to-write version agnostic code.

# Getting Started

## Installing

- Build the library or download the latest [Release](https://github.com/RosaServer/RosaServer/releases).
- Your directory should contain `libluajit.so`, `librosaserver.so`, `subrosadedicated.x64`, and the `data` folder (the last two can be found with your game install).
  - You will also need the `rosaserversatellite` binary if you plan to use the ChildProcess API. Make sure it has execute permissions.
- There's a 99% chance you'll also want to use [RosaServerCore](https://github.com/RosaServer/RosaServerCore).

## Running

```bash
LD_PRELOAD="$(pwd)/libluajit.so $(pwd)/librosaserver.so" ./subrosadedicated.x64
```

The server will start as normal and `main/init.lua` will be executed.

# Documentation

For complete reference on using the Lua API, go to the [wiki](https://github.com/RosaServer/RosaServer/wiki).

# Building

Make sure all submodules are cloned, and run `./build`

## Required Packages
- `build-essential` on Debian/Ubuntu
- `cmake`
- `libssl-dev`

Here's a basic script I use to copy the required files after they're compiled. For example, `./build && ./postbuild`
```bash
#!/bin/bash

DEST=~/testing/

cp ./moonjit/src/libluajit.so "$DEST"
cp ./release/RosaServer/librosaserver.so "$DEST"
cp ./release/RosaServerSatellite/rosaserversatellite "$DEST"
```

---

Thanks to these open source libraries:

- [moonjit](https://github.com/moonjit/moonjit)
- [Sol3](https://github.com/ThePhD/sol2)
- [SubHook](https://github.com/Zeex/subhook)
- [cpp-httplib](https://github.com/yhirose/cpp-httplib)
- [stb](https://github.com/nothings/stb)
