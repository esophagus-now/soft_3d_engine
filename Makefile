main.exe: main.cpp main.h 3d_types.h 3d_types.cpp sbuf.h sbuf.cpp poly_raster.h poly_raster.cpp
	g++ -g -o main -Wall -fno-diagnostics-show-caret *.cpp -ISDL/include -L. -LSDL/lib/win32 -lSDL2main -lSDL2
