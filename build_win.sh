
#passing -f will compile full source
#passing -ui will compile ui module + the game
#passing -gl will compile gl module + the game
#passing -al will compile al module + the game
#passing nothing will compile just the game

exec 3>&1     
exec > /dev/null
mkdir -p ./$1/build_win

start_time=$(date +%s%N)


# Compile the UI API
if [ "$3" = "-f" ] || [ "$3" = "-ui" ]; then
    g++ -c -IC:\SDL2\include -IC:\glew\include -IC:\glm Engine/UI/sky_ui_imgui.cpp -Wall -o sky_ui_imgui.o -lmingw32 -lSDL2main -lSDL2 -lglew32 -lopengl32 
    mv sky_ui_imgui.o $1/buidl_win
fi
# Compile the 2D renderer
if [ "$3" = "-f" ] || [ "$3" = "-gl" ]; then
    g++ -c -IC:\SDL2\include -IC:\glew\include -IC:glm Engine/OpenGL_renderer2D.cpp -Wall -o OpenGL_renderer2D.o -lmingw32 -lSDL2main -lSDL2 -lglew32 -lopengl32 
    mv OpenGL_renderer2D.o $1/buidl_win
fi
# Compile the sound system
if [ "$3" = "-f" ] || [ "$3" = "-al" ]; then
    g++ -c Engine/OpenAL_sound.cpp -Wall -o OpenAL_sound.o -lopenal
    mv OpenAL_sound.o $1/buidl_win
fi




pushd ./$1/build_win
    # Compile the main program
    g++ -Wall sky_ui_imgui.o OpenGL_renderer2D.o OpenAL_sound.o ../$2.cpp -o $2 -lmingw32 -lSDL2main -lSDL2 -lglew32 -lopenl32 -lopenal


end_time=$(date +%s%N)
execution_time_ns=$((end_time - start_time))
execution_time_ms=$((execution_time_ns / 1000000))

exec 1>&3     

echo -e "\033[1;34mCompiled in:\033[0m $execution_time_ms ms"

./$2
popd
