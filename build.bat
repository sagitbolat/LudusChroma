@echo off

:: === Configurable Variables ===
set BUILD_DIR=%1\build_win
set GAME_SOURCE=%2.cpp
set MODE=%3

:: Capture current date and time for log file name
for /f "tokens=1-4 delims=/ " %%a in ("%DATE%") do set TODAY=%%c-%%a-%%b
for /f "tokens=1-2 delims=:." %%a in ("%TIME%") do set CURTIME=%%a-%%b

:: Replace any invalid characters (like colon ":") with valid ones in time
set CURTIME=%CURTIME: =%
set CURTIME=%CURTIME::=-%


set SDL_INCLUDE=C:\SDL2\include
set SDL_LIB=C:\SDL2\lib
set GLEW_INCLUDE=C:\glew\include
set GLEW_LIB=C:\glew\lib
set GLM_INCLUDE=C:\glm
set OPENAL_INCLUDE=C:\OpenAL_SDK\include
set OPENAL_LIB=C:\OpenAL_SDK\libs\Win64

set COMMON_FLAGS=-lmingw32 -lSDL2main -lSDL2 -lglew32 -lopengl32 -static-libgcc -static-libstdc++

:: === Create Build Directory and Clear Log ===
mkdir %BUILD_DIR% 2>nul
mkdir %BUILD_DIR%\logs 2>nul

:: Create a unique log file name using date and time
set LOG_FILE=build_log_%TODAY%_%CURTIME%.log

:: Log start time
echo [%TIME%] Build Started
echo ======================================== >> %BUILD_DIR%\logs\%LOG_FILE%
echo ===== Build started at %TIME% ===== >> %BUILD_DIR%\logs\%LOG_FILE%
echo ======================================== >> %BUILD_DIR%\logs\%LOG_FILE%
echo[ >> %BUILD_DIR%\logs\%LOG_FILE%

:: === Determine Build Mode ===
if "%MODE%" == ""     (GOTO CompileGame)
if "%MODE%" == "-f"   (GOTO CompileUI)
if "%MODE%" == "-ui"  (GOTO CompileUI)
if "%MODE%" == "-gl"  (GOTO CompileGL)
if "%MODE%" == "-al"  (GOTO CompileAL)

:: === UI Compilation ===
:CompileUI
echo [%TIME%] +++++ Compiling UI +++++  >> %BUILD_DIR%\logs\%LOG_FILE%
echo[ >> %BUILD_DIR%\logs\%LOG_FILE%
echo [%TIME%] Compiling UI...
g++ -c Engine\UI\sky_ui_imgui.cpp -I%SDL_INCLUDE% -I%GLEW_INCLUDE% -I%GLM_INCLUDE% -o sky_ui_imgui.o %COMMON_FLAGS% >>%BUILD_DIR%\logs\%LOG_FILE% 2>&1
if errorlevel 1 (GOTO BuildFailed)
if errorlevel 0 (echo [%TIME%] 0 Errors >> %BUILD_DIR%\logs\%LOG_FILE%)
move sky_ui_imgui.o %BUILD_DIR% >nul
echo[ >> %BUILD_DIR%\logs\%LOG_FILE%
echo [%TIME%] ===== UI Compiled =====  >> %BUILD_DIR%\logs\%LOG_FILE%
if "%MODE%" == "-f" (GOTO CompileGL) else (GOTO CompileGame)

:: === OpenGL Compilation ===
:CompileGL
echo [%TIME%] +++++ Compiling Rendering Layer +++++ >> %BUILD_DIR%\logs\%LOG_FILE%
echo[ >> %BUILD_DIR%\logs\%LOG_FILE%
echo [%TIME%] Compiling OpenGL Renderer...
g++ -c Engine\OpenGL_renderer2D.cpp -I%SDL_INCLUDE% -I%GLEW_INCLUDE% -I%GLM_INCLUDE% -o OpenGL_renderer2D.o %COMMON_FLAGS% >>%BUILD_DIR%\logs\%LOG_FILE% 2>&1
if errorlevel 1 (GOTO BuildFailed)
if errorlevel 0 (echo [%TIME%] 0 Errors >> %BUILD_DIR%\logs\%LOG_FILE%)
move OpenGL_renderer2D.o %BUILD_DIR% >nul
XCOPY Engine\assets\shader.vs %BUILD_DIR% /D /Y >nul
XCOPY Engine\assets\shader.fs %BUILD_DIR% /D /Y >nul
echo[ >> %BUILD_DIR%\logs\%LOG_FILE%
echo [%TIME%] ===== Render Layer Compiled =====  >> %BUILD_DIR%\logs\%LOG_FILE%
if "%MODE%" == "-f" (GOTO CompileAL) else (GOTO CompileGame)

:: === OpenAL Compilation ===
:CompileAL
echo [%TIME%] +++++ Compiling Audio Layer +++++ >> %BUILD_DIR%\logs\%LOG_FILE%
echo[ >> %BUILD_DIR%\logs\%LOG_FILE%
echo [%TIME%] Compiling OpenAL Sound...
g++ -c Engine\OpenAL_sound.cpp -I%SDL_INCLUDE% -I%GLEW_INCLUDE% -I%OPENAL_INCLUDE% -o OpenAL_sound.o %COMMON_FLAGS% -lOpenAL32 >>%BUILD_DIR%\logs\%LOG_FILE% 2>&1
if errorlevel 1 (GOTO BuildFailed)
if errorlevel 0 (echo [%TIME%] 0 Errors >> %BUILD_DIR%\logs\%LOG_FILE%)
move OpenAL_sound.o %BUILD_DIR% >nul
echo[ >> %BUILD_DIR%\logs\%LOG_FILE%
echo [%TIME%] ===== Audio Layer Compiled =====  >> %BUILD_DIR%\logs\%LOG_FILE%

:: === Copy Assets === We only do it if -f is envoked because this only needs to happen on the first compilation
if NOT "%MODE%"=="-f" (GOTO CompileGame)
XCOPY Engine\assets\startup.png %BUILD_DIR% /D /Y >nul
XCOPY Engine\assets\glew32.dll %BUILD_DIR% /D /Y >nul
XCOPY Engine\assets\SDL2.dll %BUILD_DIR% /D /Y >nul
XCOPY Engine\assets\OpenAL32.dll %BUILD_DIR% /D /Y >nul
XCOPY Engine\assets\libwinpthread-1.dll %BUILD_DIR% /D /Y >nul

:: === Final Compilation ===
:CompileGame
echo [%TIME%] +++++ Compiling Project +++++ >> %BUILD_DIR%\logs\%LOG_FILE%
echo[ >> %BUILD_DIR%\logs\%LOG_FILE%
echo [%TIME%] Compiling and linking final executable...
pushd %BUILD_DIR%
g++ OpenGL_renderer2D.o OpenAL_sound.o sky_ui_imgui.o ..\%GAME_SOURCE% ^
  -I%SDL_INCLUDE% -I%GLEW_INCLUDE% -I%GLM_INCLUDE% -I%OPENAL_INCLUDE% ^
  -L%SDL_LIB% -L%GLEW_LIB% -L%OPENAL_LIB% ^
  -Wall %COMMON_FLAGS% -lOpenAL32 -o %2.exe >>logs\%LOG_FILE% 2>&1
if errorlevel 1 (GOTO BuildFailed)
if errorlevel 0 (echo [%TIME%] 0 Errors >>logs\%LOG_FILE%)

echo[ >>logs\%LOG_FILE%
echo [%TIME%] ===== Project Compiled =====  >>logs\%LOG_FILE%
echo[ >>logs\%LOG_FILE%
echo ==========================================>>logs\%LOG_FILE%
echo ===== Build completed at %TIME% =====>>logs\%LOG_FILE%
echo ==========================================>>logs\%LOG_FILE%
echo [%TIME%] Build Successful
exit /b

:: === Build Failed ===
:BuildFailed
echo[ >>logs\%LOG_FILE%
echo =======================================>>logs\%LOG_FILE%
echo ===== Build failed at %TIME% =====>>logs\%LOG_FILE%
echo =======================================>>logs\%LOG_FILE%
echo [%TIME%] Build Failed
exit /b
