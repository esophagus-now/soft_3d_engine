main.exe: main.cpp
	g++ -o main -Wall -fno-diagnostics-show-caret main.cpp -ISDL/include -L. -LSDL/lib/win32 -lSDL2main -lSDL2

segments.exe: segments.cpp
	g++ -g -o segments -Wall -fno-diagnostics-show-caret segments.cpp 
