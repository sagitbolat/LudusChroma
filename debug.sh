mkdir ./$1/debug
pushd ./$1/debug
    g++ -Wall -Werror -g ../../Engine/UI/sky_ui_imgui.cpp ../../Engine/OpenGL_renderer2D.cpp ../../Engine/OpenAL_sound.cpp ../$2.cpp -w -lSDL2 -lGLEW  -lGL -lopenal
    gdb a.out
popd
