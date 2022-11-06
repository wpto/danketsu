g++ \
  -fdiagnostics-color=always \
  -g \
  engine/main.cpp \
  -I./external/SDL2-2.24.2/x86_64-w64-mingw32/include \
  -L./external/SDL2-2.24.2/x86_64-w64-mingw32/lib \
  -I./external/glew-2.1.0/include \
  -L./external/glew-2.1.0/lib/Release/x64 \
  -lmingw32 \
  -lSDL2main \
  -lSDL2 \
  -lglew32 \
  -lopengl32 \
  -lglu32 \
  -lstdc++ \
  -static-libgcc \
  -static-libstdc++ \
  -o build/engine.exe

