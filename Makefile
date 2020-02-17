main.exe: main.cpp main.h 3d_types.h 3d_types.cpp sbuf.h poly_raster.cpp
	g++ -o main -Wall -fno-diagnostics-show-caret *.cpp -ISDL/include -L. -LSDL/lib/win32 -lSDL2main -lSDL2
