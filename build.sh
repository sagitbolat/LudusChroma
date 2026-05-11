
#passing -f will compile full source
#passing -ui will compile ui module + the game
#passing -gl will compile gl module + the game
#passing -al will compile al module + the game
#passing nothing will compile just the game

exec 3>&1     
exec > /dev/null
mkdir -p ./$1/build

start_time=$(date +%s%N)


# Compile the UI API
if [ "$3" = "-f" ] || [ "$3" = "-ui" ]; then
    g++ -c Engine/UI/sky_ui_imgui.cpp -Wall -o sky_ui_imgui.o -lSDL2 -lGLEW  -lGL
    mv sky_ui_imgui.o $1/build
fi
# Compile the 2D renderer
if [ "$3" = "-f" ] || [ "$3" = "-gl" ]; then
    g++ -c Engine/OpenGL_renderer2D.cpp -Wall -o OpenGL_renderer2D.o -lSDL2 -lGLEW  -lGL
    mv OpenGL_renderer2D.o $1/build
    cp Engine/assets/shader.vs $1/build
    cp Engine/assets/shader.fs $1/build
fi
# Compile the sound system
if [ "$3" = "-f" ] || [ "$3" = "-al" ]; then
    g++ -c Engine/OpenAL_sound.cpp -Wall -o OpenAL_sound.o -lopenal
    mv OpenAL_sound.o $1/build
fi


cp Engine/assets/glew32.dll $1/build
cp Engine/assets/SDL2.dll $1/build
cp Engine/assets/startup.png $1/build

pushd ./$1/build
    # Compile the main program
    g++ -Wall sky_ui_imgui.o OpenGL_renderer2D.o OpenAL_sound.o ../$2.cpp -o $2 -lSDL2 -lGLEW  -lGL -lopenal


end_time=$(date +%s%N)
execution_time_ns=$((end_time - start_time))
execution_time_ms=$((execution_time_ns / 1000000))

exec 1>&3     

echo -e "\033[1;34mCompiled in:\033[0m $execution_time_ms ms"

./$2
popd
