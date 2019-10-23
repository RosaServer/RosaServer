<img src="https://i.imgur.com/4N3PMTS.png" width="600">

A server scripting API for [Sub Rosa](http://subrosagame.com/).

**⚠ Updating to Alpha 36! This is not ready yet.**

RosaServer uses [LuaJIT](http://luajit.org/); this means there's no hit to performance while being able to create anything from moderation tools to complex custom games with easy-to-write version agnostic code.

# Documentation
For complete reference on using the Lua API, go to the [wiki](https://github.com/RosaServer/RosaServer/wiki).

# Building
If you want to build RosaServer yourself and contribute:
1. Clone the source, including LuaJIT.
2. Use [MSVC 2019](https://visualstudio.microsoft.com/vs/).
3. Run `LuaJIT/src/msvcbuild.bat` in a developer command prompt to compile it.
4. Compile RosaServer.
5. Copy `Release/RosaServer.dll` and `LuaJIT/src/lua51.dll` into your server directory.
6. Either use something like [Stud_PE](http://www.cgsoftlabs.ro/studpe.html) to add RosaServer.dll to subrosadedicated.exe's import table, or inject it manually after launch.

---

Thanks to these open source libraries:
- [Sol3](https://github.com/ThePhD/sol2)
- [Detours](https://github.com/microsoft/Detours)
- [cpp-httplib](https://github.com/yhirose/cpp-httplib)
- [TinyCon](https://github.com/unix-ninja/hackersandbox)