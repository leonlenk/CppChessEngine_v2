#include <unordered_map>
#include <bit>
#include <iostream>
#include "pieceMaps.h"
#include "board.h"
#include "globals.h"

using namespace std;

inline U64 getKnightAttacks(U64 mask);
inline U64 getBishopAttacks(U64 mask, U64 allPieceOccupancy, int numShifts);
inline U64 getRookAttacks(U64 mask, U64 allPieceOccupancy, int numShifts);
U64 reverseBitMap(U64 forward);

void WPawnMap::getPsuedoLegalMoves(Board* myBoard)
{
	U64 attacks;
	U64 forwardPushes;
	for (U64 mask = 1; mask != 0; mask <<= 1)
		if (mask & pieceLoc)
		{
			attacks = (((mask & NOT_FILE_A) << 9) | ((mask & NOT_FILE_H) << 7));
			forwardPushes = (mask << 8) & myBoard->emptySquares;
			forwardPushes |= ((forwardPushes & WHITE_PAWN_FIRST_MOVE) << 8) & myBoard->emptySquares;
			myBoard->legalMoves[mask] = (attacks & (myBoard->blackPieceOccupancy | myBoard->enPassentSquare) | forwardPushes) & myBoard->checkMoveMask;
			myBoard->allTempAttacks |= attacks;
		}
}

void BPawnMap::getPsuedoLegalMoves(Board* myBoard)
{ 
	U64 attacks;
	U64 forwardPushes;
	for (U64 mask = 1; mask != 0; mask <<= 1)
		if (mask & pieceLoc)
		{
			attacks = (((mask & NOT_FILE_H) >> 9) | ((mask & NOT_FILE_A) >> 7));
			forwardPushes = (mask >> 8) & myBoard->emptySquares;
			forwardPushes |= ((forwardPushes & BLACK_PAWN_FIRST_MOVE) >> 8) & myBoard->emptySquares;
			myBoard->legalMoves[mask] = (attacks & (myBoard->whitePieceOccupancy | myBoard->enPassentSquare) | forwardPushes) & myBoard->checkMoveMask;
			myBoard->allTempAttacks |= attacks;
		}
}

void KnightMap::getPsuedoLegalMoves(Board* myBoard)
{
	U64 attacks;
	for (U64 mask = 1; mask != 0; mask <<= 1)
		if (mask & pieceLoc)
		{
			attacks = getKnightAttacks(mask);
			myBoard->legalMoves[mask] = (attacks & myBoard->attackableSquares) & myBoard->checkMoveMask;
			myBoard->allTempAttacks |= attacks;
		}
}

void BishopMap::getPsuedoLegalMoves(Board* myBoard)
{
	U64 attacks;
	U64 mask = 1;
	for (int i = 0; i < NUM_SQUARES; i++)
	{
		if (mask & pieceLoc)
		{
			attacks = getBishopAttacks(mask, myBoard->allPieceOccupancy, i);
			myBoard->legalMoves[mask] = (attacks & myBoard->attackableAndEmptySquares) & myBoard->checkMoveMask;
			myBoard->allTempAttacks |= attacks;
		}
		mask <<= 1;
	}
}

void RookMap::getPsuedoLegalMoves(Board* myBoard)
{
	U64 attacks;
	U64 mask = 1;
	for (int i = 0; i < NUM_SQUARES; i++)
	{
		if (mask & pieceLoc)
		{
			attacks = getRookAttacks(mask, myBoard->allPieceOccupancy, i);
			myBoard->legalMoves[mask] = (attacks & myBoard->attackableAndEmptySquares) & myBoard->checkMoveMask;
			myBoard->allTempAttacks |= attacks;
		}
		mask <<= 1;
	}
}

void QueenMap::getPsuedoLegalMoves(Board* myBoard)
{
	U64 attacks;
	U64 mask = 1;

	for (int i = 0; i < NUM_SQUARES; i++)
	{
		if (mask & pieceLoc)
		{
			attacks = getBishopAttacks(mask, myBoard->allPieceOccupancy, i) | getRookAttacks(mask, myBoard->allPieceOccupancy, i);
			// insert them into the legal move map
			myBoard->legalMoves[mask] = (attacks & myBoard->attackableAndEmptySquares) & myBoard->checkMoveMask;
			myBoard->allTempAttacks |= attacks;
		}
		mask <<= 1;
	}
}

void KingMap::getPsuedoLegalMoves(Board* myBoard)
{
	U64 attacks;
	attacks = (pieceLoc & NOT_FILE_A) << 1 | (pieceLoc & NOT_FILE_H) >> 1 | pieceLoc; // west one and east one
	// pushes the three bits in a row north and south then excludes the center
	myBoard->legalMoves[pieceLoc] = (((attacks ^ get_pieceLoc()) | attacks << 8 | attacks >> 8) & myBoard->attackableSquares) & (~myBoard->slidingCheckMask);
	myBoard->allTempAttacks |= ((attacks ^ get_pieceLoc()) | attacks << 8 | attacks >> 8);
}


void Board::findPinsAndChecks(bool isForWhite)
{
	U64 diagonalMask, antidiagonalMask, fileMask, rankMask;
	U64 kingLoc = (isForWhite ? allPieces[W_KING_INDEX]->get_pieceLoc() : allPieces[B_KING_INDEX]->get_pieceLoc());
	int kingNumShifts = countr_zero(kingLoc);
	// finds where in all pieces array to access enemy pieces
	int enemyPieceOffSet = (isForWhite ? 6 : 0);
	auto pawnAttacks = (isForWhite ? [](U64 kingLoc) { return (((kingLoc & NOT_FILE_H) >> 9) | ((kingLoc & NOT_FILE_A) >> 7)); } : 
		[](U64 kingLoc) { return (((kingLoc & NOT_FILE_A) << 9) | ((kingLoc & NOT_FILE_H) << 7)); });

	U64 bishopQueenLoc = allPieces[W_BISHOP_INDEX + enemyPieceOffSet]->get_pieceLoc() | allPieces[W_QUEEN_INDEX + enemyPieceOffSet]->get_pieceLoc();
	U64 rookQueenLoc = allPieces[W_ROOK_INDEX + enemyPieceOffSet]->get_pieceLoc() | allPieces[W_QUEEN_INDEX + enemyPieceOffSet]->get_pieceLoc();
	U64 tempPieceOccupancy = allPieceOccupancy;
	U64 tempCheckMoveMask = 0;
	U64 tempSlidingCheckMask = 0;
	auto evalSlidingPiece = ([tempPieceOccupancy, kingLoc, &tempCheckMoveMask, &tempSlidingCheckMask](U64 attackMask, U64 pieceOfIntrest) -> U64
	{
		U64 left, right;
		U64 leftPiece, rightPiece;
		left = (tempPieceOccupancy & attackMask) - 2 * kingLoc;
		leftPiece = left & attackMask & pieceOfIntrest;
		right = reverseBitMap(reverseBitMap(tempPieceOccupancy & attackMask) - 2 * reverseBitMap(kingLoc));
		rightPiece = right & attackMask & pieceOfIntrest;
		if (leftPiece) { tempCheckMoveMask = right & attackMask; tempSlidingCheckMask |= attackMask; }
		if (rightPiece) { tempCheckMoveMask = left & attackMask; tempSlidingCheckMask |= attackMask; }
		return leftPiece | rightPiece;
	});
	locOfChecks = 0;
	checkMoveMask = 0;

	// masks
	diagonalMask = DIAGONAL_MASKS[kingNumShifts / 8 + kingNumShifts % 8];
	antidiagonalMask = ANTIDIAGONAL_MASKS[kingNumShifts / 8 + 7 - kingNumShifts % 8];
	rankMask = RANK_MASKS[kingNumShifts / 8];
	fileMask = FILE_MASKS[kingNumShifts % 8];

	locOfChecks |= pawnAttacks(kingLoc) & allPieces[W_PAWN_INDEX + enemyPieceOffSet]->get_pieceLoc();
	locOfChecks |= getKnightAttacks(kingLoc) & allPieces[W_KNIGHT_INDEX + enemyPieceOffSet]->get_pieceLoc();
	// bishop moves
	locOfChecks |= evalSlidingPiece(diagonalMask, bishopQueenLoc);
	locOfChecks |= evalSlidingPiece(antidiagonalMask, bishopQueenLoc);
	// rook moves
	locOfChecks |= evalSlidingPiece(rankMask, rookQueenLoc);
	locOfChecks |= evalSlidingPiece(fileMask, rookQueenLoc);

	slidingCheckMask = tempSlidingCheckMask;
	if (locOfChecks == 0) checkMoveMask = ~0;
	else checkMoveMask = locOfChecks | tempCheckMoveMask;
}

inline U64 getKnightAttacks(U64 mask)
{
	U64 h1, h2;
	// adds horizontal moves (accounting for overflow) and then reflects them across the y-axis
	h1 = ((mask >> 1) & NOT_FILE_A) | ((mask << 1) & NOT_FILE_H); // up and down 1
	h2 = ((mask >> 2) & NOT_FILE_AB) | ((mask << 2) & NOT_FILE_GH); // up and down 2
	return (h1 << 16) | (h1 >> 16) | (h2 << 8) | (h2 >> 8);
}

inline U64 getBishopAttacks(U64 mask, U64 allPieceOccupancy, int numShifts)
{
	U64 left;
	U64 right;
	U64 diagonal;
	U64 antidiagonal;
	U64 diagonalMask;
	U64 antidiagonalMask;
	diagonalMask = DIAGONAL_MASKS[numShifts / 8 + numShifts % 8];
	antidiagonalMask = ANTIDIAGONAL_MASKS[numShifts / 8 + 7 - numShifts % 8];
	left = (allPieceOccupancy & diagonalMask) - 2 * mask;
	right = reverseBitMap(reverseBitMap(allPieceOccupancy & diagonalMask) - 2 * reverseBitMap(mask));
	diagonal = (left ^ right) & diagonalMask;
	left = (allPieceOccupancy & antidiagonalMask) - 2 * mask;
	right = reverseBitMap(reverseBitMap(allPieceOccupancy & antidiagonalMask) - 2 * reverseBitMap(mask));
	antidiagonal = (left ^ right) & antidiagonalMask;
	return diagonal | antidiagonal;
}

inline U64 getRookAttacks(U64 mask, U64 allPieceOccupancy, int numShifts)
{
	U64 left;
	U64 right;
	U64 vertical;
	U64 horizontal;
	U64 file;
	U64 rank;
	rank = RANK_MASKS[numShifts / 8];
	file = FILE_MASKS[numShifts % 8];
	left = (allPieceOccupancy & rank) - 2 * mask;
	right = reverseBitMap(reverseBitMap(allPieceOccupancy & rank) - 2 * reverseBitMap(mask));
	horizontal = (left ^ right) & rank;
	left = (allPieceOccupancy & file) - 2 * mask;
	right = reverseBitMap(reverseBitMap(allPieceOccupancy & file) - 2 * reverseBitMap(mask));
	vertical = (left ^ right) & file;
	return horizontal | vertical;
}


// utility functions
U64 reverseBitMap(U64 forward)
{
	U64 backwards = 0;
	for (int i = 0; i < 64; i++)
		backwards |= ((forward >> i) & 1) << (63 - i);
	return backwards;
}