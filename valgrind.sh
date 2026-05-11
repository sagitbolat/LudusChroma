mkdir ./$1/build
pushd ./$1/build
    g++ -Wall -Werror -g sky_ui_imgui.o OpenGL_renderer2D.o OpenAL_sound.o ../$2.cpp -o $2 -lSDL2 -lGLEW -lGL -openal
    valgrind ./a.out
popd
