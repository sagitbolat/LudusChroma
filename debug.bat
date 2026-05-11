@echo off
mkdir .\%1\debug
pushd .\%1\debug
g++ -g ..\%2.cpp -IC:\SDL2\include -LC:\SDL2\lib -Wall -lmingw32 -lSDL2main -lSDL2 
popd
