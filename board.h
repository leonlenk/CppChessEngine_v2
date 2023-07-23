#pragma once
#include <string>
#include <unordered_map>
#include <SFML/Graphics.hpp>
#include <utility>
#include "pieceMaps.h"
#include "globals.h"


class Board
{
public:
	Board();
	Board(std::string startingFEN);
	~Board();
	
	void generateAllLegalMoves();

	// returns true if valid move was made
	bool makeMove(U64 fromWhere, U64 whereTo);
	
	// gets the piece at the location of the mask
	PieceMaps* getPieceAtMask(const U64 mask) const;
	// gets the pieces index
	int getPieceIndexAtMask(const U64 mask) const;
	PieceMaps* getPieceMapAtIndex(int index) const;
	
	// accessor functions
	U64 get_LegalMovesFor(U64 piece);
	U64 get_allBlackAttacks() const;
	U64 get_allWhiteAttacks() const;
	bool get_isWhitesMove() const;
	void set_isWhitesMove(bool setter);

private:
	void setLegalMoveFlags();
	void findPinsAndChecks(bool isForWhite);
	void removeFromLegalMoves(U64 piecesToRemove);
	// stores the pieces position and maps to all the legal moves it can make
	std::unordered_map <U64, U64> legalMoves;
	PieceMaps* allPieces[NUM_PIECES];
	// for where pieces can move
	U64 emptySquares;
	U64 whitePieceOccupancy;
	U64 blackPieceOccupancy;
	U64 allPieceOccupancy;
	U64 attackableSquares;
	U64 attackableAndEmptySquares;
	// attacks
	U64 allTempAttacks;
	U64 allBlackAttacks;
	U64 allWhiteAttacks;
	// en passent
	U64 enPassentSquare;
	// checks and pins
	U64 locOfChecks;
	U64 checkMoveMask;
	U64 slidingCheckMask; // ensures king can't go behind itself
	// game state info
	int halfTurnNum;
	bool isWhitesMove;
	bool isCheckMate;
	// flags denote that these castles are still possible in this order
	// white kingside, white queen side, black king side, black queen side
	bool canCastle[4];

friend class WPawnMap;
friend class BPawnMap;
friend class KnightMap;
friend class BishopMap;
friend class RookMap;
friend class QueenMap;
friend class KingMap;
};

inline PieceMaps* Board::getPieceMapAtIndex(int index) const { return allPieces[index]; }

// setters and getters 

inline U64 Board::get_LegalMovesFor(U64 piece) { return legalMoves[piece]; }
inline U64 Board::get_allBlackAttacks() const { return allBlackAttacks; }
inline U64 Board::get_allWhiteAttacks() const { return allWhiteAttacks; }
inline bool Board::get_isWhitesMove() const { return isWhitesMove; }
inline void Board::set_isWhitesMove(bool setter) { isWhitesMove = setter; }

// draws the board
void renderGame(Board* myBoard);
