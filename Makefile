graphics_headers = -I./SFML-2.6.1/include/
graphics_objects = -L./SFML-2.6.1/lib -L/usr/lib/x86_64-linux-gnu -lsfml-graphics-s -lsfml-window-s -lsfml-system-s -lfreetype -l:libudev.so.1 -l:libX11.so.6 -l:libXcursor.so.1 -l:libXrandr.so.2

output = ./build/
objects = $(output)main.o $(output)board.o $(output)renderGame.o $(output)genMoves.o \
	$(output)moves.o $(output)perft.o

chessEngine : $(objects) 
	g++ -std=c++20 -o chessEngine $(objects) $(graphics_objects)

$(output)main.o : main.cpp board.h
	g++ -std=c++20 -c main.cpp -o $@ 

$(output)board.o : board.cpp board.h globals.h pieceMaps.h
	g++ -std=c++20 -c board.cpp $(graphics_headers) -o $@ 

$(output)renderGame.o : renderGame.cpp board.h pieceMaps.h globals.h
	g++ -std=c++20 -c renderGame.cpp $(graphics_headers) -o $@ 

$(output)genMoves.o : genMoves.cpp pieceMaps.h board.h globals.h
	g++ -std=c++20 -c genMoves.cpp -o $@ 

$(output)moves.o : moves.cpp board.h globals.h pieceMaps.h
	g++ -std=c++20 -c moves.cpp -o $@ 

$(output)perft.o : perft.cpp board.h globals.h
	g++ -std=c++20 -c perft.cpp -o $@ 

.PHONY : clean
clean :
	-rm chessEngine $(objects)
