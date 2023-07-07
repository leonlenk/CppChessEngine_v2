#pragma once
#include <string>
#include <memory>
#include <SFML/Graphics.hpp>
#include "globals.h"

class WPawnMap;
class BPawnMap;
class KnightMap;
class KingMap;
class PieceMaps;

class Board
{
public:
	Board();
	Board(std::string startingFEN);
	~Board();

	// gets the piece at the location of the mask
	PieceMaps* getPieceAtMask(const U64 mask) const;
	// gets the pieces index
	int getPieceIndexAtMask(const U64 mask) const;
	
private:
	PieceMaps* allPieces[numOfPieces];
};

// draws a board
void renderGame(Board* myBoard);
