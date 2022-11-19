CXX="g++"
CXXFLAGS="-fdiagnostics-color=always -g"
INCLUDE="-I./engine -I./external/stb -I./external/glm"
SLIB=""
LIB=""


case $OSTYPE in
  msys)
    INCLUDE="$INCLUDE -I./external/SDL2-2.24.2/x86_64-w64-mingw32/include"
    INCLUDE="$INCLUDE -I./external/SDL2-2.24.2/x86_64-w64-mingw32/include/SDL2"
    INCLUDE="$INCLUDE -I./external/glew-2.1.0/include"
    SLIB="$SLIB -L./external/SDL2-2.24.2/x86_64-w64-mingw32/lib"
    SLIB="$SLIB -L./external/glew-2.1.0/lib/Release/x64"
    LIB="$LIB -lmingw32 -lSDL2main -lSDL2 -lglew32 -lopengl32 -lglu32 -lstdc++ -static-libgcc -static-libstdc++"
    OUTPUT="build/engine.exe"
    ;;
  linux-gnu)
    CXXFLAGS="$CXXFLAGS -lGLEW -lGL"
    OUTPUT="build/engine"
    ;;
  *)
    echo "Unknown OS: $OSTYPE"
    exit 1
    ;;
esac

MAIN="engine/main.cpp"


CMD="$CXX $CXXFLAGS $INCLUDE $SLIB $MAIN $LIB -o $OUTPUT"
$CMD
echo "$CMD"