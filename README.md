# RosaServer
A server scripting API for Sub Rosa.

**⚠ Updating to Alpha 36! This is not ready yet.**

# Building
1. Download the source, including LuaJIT.
2. Use [MSVC 2019](https://visualstudio.microsoft.com/vs/).
3. Run `LuaJIT/src/msvcbuild.bat` in a developer command prompt to compile it.
4. Compile RosaServer.
5. Copy `Release/RosaServer.dll` and `LuaJIT/src/lua51.dll` into your server directory.
6. Either use something like [Stud_PE](http://www.cgsoftlabs.ro/studpe.html) to add RosaServer.dll to subrosadedicated.exe's import table, or inject it manually after launch.