g++ \
    -fdiagnostics-color=always \
    -g \
    -I./external/stb \
    -I./external/glm \
    -lSDL2 \
    -lGLEW \
    -lGL \
    ./engine/main.cpp \
    -o ./build/main
