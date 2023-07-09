#pragma once
#include <string>
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

	// returns true if valid move was made
	bool makeMove(U64 fromWhere, U64 whereTo, int pieceIndex);

	void makeAttackMaps();

	// gets the piece at the location of the mask
	PieceMaps* getPieceAtMask(const U64 mask) const;
	// gets the pieces index
	int getPieceIndexAtMask(const U64 mask) const;
	PieceMaps* getPieceMapAtIndex(int index) const;
	
	bool get_isWhitesMove() const;
	void set_isWhitesMove(bool setter);

private:
	PieceMaps* allPieces[NUM_PIECES];
	bool isWhitesMove;
	int halfTurnNum;
};

inline PieceMaps* Board::getPieceMapAtIndex(int index) const { return allPieces[index]; }

// setters and getters 
inline bool Board::get_isWhitesMove() const { return isWhitesMove; }
inline void Board::set_isWhitesMove(bool setter) { isWhitesMove = setter; }

// draws the board
void renderGame(Board* myBoard);
