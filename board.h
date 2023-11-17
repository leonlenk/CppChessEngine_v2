#pragma once
#include <string>
#include <unordered_map>
//#include <SFML/Graphics.hpp>
#include <utility>
#include <stack>
#include "pieceMaps.h"
#include "globals.h"

class Board
{
public:
	Board();
	Board(std::string startingFEN);
	~Board();
	struct HistoryFrame;

	// runs perft with each path printed
	int runSplitPERFT(int depth);
	int runPERFT(int depth);
	// returns true if move succesfully undone
	bool undoMove();

	std::string exportFEN();

	// returns algebraic notation from a move
	// bitmap2alg does not account for checks and will not add a +
	std::string bitMap2Alg(U64 fromWhere, U64 whereTo);
	std::string history2Alg(HistoryFrame* myMove);

	// algebraic notation for piece locations
	std::string mask2AlgLoc(U64 mask);
	U64 algLoc2Mask(char first, char second);
	U64 algLoc2Mask(std::string algLoc);

	void generateAllLegalMoves();

	// returns true if valid move was made
	bool makeMove(std::string algNotation);
	bool makeMove(U64 fromWhere, U64 whereTo);
	
	// gets the piece at the location of the mask
	PieceMaps* getPieceAtMask(const U64 mask) const;
	// gets the pieces index
	int getPieceIndexAtMask(const U64 mask) const;
	PieceMaps* getPieceMapAtIndex(int index) const;
	
	// accessor functions
	HistoryFrame* get_mostRecentMove();
	U64 get_LegalMovesFor(U64 piece);
	U64 get_allBlackAttacks() const;
	U64 get_allWhiteAttacks() const;
	bool get_isWhitesMove() const;
	void set_isWhitesMove(bool setter);

	struct HistoryFrame
	{
		HistoryFrame() : canCastle{ 1, 1, 1, 1 } { startLoc = 0; endLoc = 0; enPassentSquare = 0; movingPieceIndex = -1; capturedPieceIndex = -1; pawnPromotion = -1; inCheck = false; }
		HistoryFrame(U64 start, U64 end, int moving) : HistoryFrame() { startLoc = start; endLoc = end; movingPieceIndex = moving; }
		~HistoryFrame() {}
		U64 startLoc;
		U64 endLoc;
		U64 enPassentSquare;
		int movingPieceIndex;
		int capturedPieceIndex;
		int pawnPromotion;
		bool canCastle[4];
		bool inCheck;
	};
	
private:
	void setLegalMoveFlags();
	void findPinsAndChecks(bool isForWhite);
	void removeFromLegalMoves(U64 piecesToRemove);
	// moves the king as a sliding piece in all 8 directions (called in each below) and evaluates if its getting attacked
	// from each direction then makes masks
	U64 findSlidingPieceChecksPins(U64 attackMask, U64 attackingPiece, U64 kingLoc, U64 friendlyPieces);
	std::stack<HistoryFrame*> history;
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
	// checks
	U64 locOfChecks; // the location of the pieces checking the king
	U64 checkMoveMask; // where pieces can move to stop the check
	U64 slidingCheckMask; // ensures king can't go behind itself
	// pinned pieces
	U64 pinnedPieceLoc[16]; // there are at most 8 pins possible for each king at once
	U64 pinnedMovementMasks[16]; // in a one to one correspondance with the above array
	int numOfPins;
	// pawn promotion
	int wPawnPromote;
	int bPawnPromote;
	// game state info
	int fullMoveNum;
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
inline Board::HistoryFrame* Board::get_mostRecentMove() { if (history.empty()) return nullptr; return history.top(); }
inline U64 Board::get_LegalMovesFor(U64 piece) { return legalMoves[piece]; }
inline U64 Board::get_allBlackAttacks() const { return allBlackAttacks; }
inline U64 Board::get_allWhiteAttacks() const { return allWhiteAttacks; }
inline bool Board::get_isWhitesMove() const { return isWhitesMove; }
inline void Board::set_isWhitesMove(bool setter) { isWhitesMove = setter; }

// draws the board
void renderGame(Board* myBoard);
