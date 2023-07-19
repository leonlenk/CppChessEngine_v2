#include <unordered_map>
#include "pieceMaps.h"
#include "board.h"
#include "globals.h"

using namespace std;

U64 reverseBitMap(U64 forward);

void WPawnMap::getPsuedoLegalMoves(Board* myBoard)
{
	U64 attacks;
	U64 forwardPushes;
	for (U64 mask = 1; mask != 0; mask <<= 1)
		if (mask & pieceLoc)
		{
			attacks = (((mask & NOT_FILE_A) << 9) | ((mask & NOT_FILE_H) << 7));
			forwardPushes = mask << 8 | (mask & WHITE_PAWN_START) << 16;
			myBoard->legalMoves[mask] = attacks & myBoard->blackPieceOccupancy | forwardPushes & myBoard->emptySquares;
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
			forwardPushes = mask >> 8 | (mask & BLACK_PAWN_START) >> 16;
			myBoard->legalMoves[mask] = attacks & myBoard->blackPieceOccupancy | forwardPushes & myBoard->emptySquares;
		}
}

void KnightMap::getPsuedoLegalMoves(Board* myBoard)
{
	U64 h1, h2;
	for (U64 mask = 1; mask != 0; mask <<= 1)
		if (mask & pieceLoc)
		{
			// adds horizontal moves (accounting for overflow) and then reflects them across the y-axis
			h1 = ((mask >> 1) & NOT_FILE_A) | ((mask << 1) & NOT_FILE_H); // up and down 1
			h2 = ((mask >> 2) & NOT_FILE_AB) | ((mask << 2) & NOT_FILE_GH); // up and down 2
			myBoard->legalMoves[mask] = ((h1 << 16) | (h1 >> 16) | (h2 << 8) | (h2 >> 8)) & myBoard->attackableSquares;
		}
}

void BishopMap::getPsuedoLegalMoves(Board* myBoard)
{
	U64 left;
	U64 right;
	U64 diagonal;
	U64 antidiagonal;
	U64 diagonalMask;
	U64 antidiagonalMask;
	U64 mask = 1;
	for (int i = 0; i < NUM_SQUARES; i++)
	{
		if (mask & pieceLoc)
		{
			diagonalMask = DIAGONAL_MASKS[i / 8 + i % 8];
			antidiagonalMask = ANTIDIAGONAL_MASKS[i / 8 + 7 - i % 8];
			left = (myBoard->allPieceOccupancy & diagonalMask) - 2 * mask;
			right = reverseBitMap(reverseBitMap(myBoard->allPieceOccupancy & diagonalMask) - 2 * reverseBitMap(mask));
			diagonal = (left ^ right)& diagonalMask;
			left = (myBoard->allPieceOccupancy & antidiagonalMask) - 2 * mask;
			right = reverseBitMap(reverseBitMap(myBoard->allPieceOccupancy & antidiagonalMask) - 2 * reverseBitMap(mask));
			antidiagonal = (left ^ right)& antidiagonalMask;
			myBoard->legalMoves[mask] = (diagonal | antidiagonal) & myBoard->attackableAndEmptySquares;
		}
		mask <<= 1;
	}
}

void RookMap::getPsuedoLegalMoves(Board* myBoard)
{
	U64 left;
	U64 right;
	U64 vertical;
	U64 horizontal;
	U64 file;
	U64 rank;
	U64 mask = 1;
	for (int i = 0; i < NUM_SQUARES; i++)
	{
		if (mask & pieceLoc)
		{
			rank = RANK_MASKS[i/8];
			file = FILE_MASKS[i % 8];
			left = (myBoard->allPieceOccupancy & rank) - 2 * mask;
			right = reverseBitMap(reverseBitMap(myBoard->allPieceOccupancy & rank) - 2 * reverseBitMap(mask));
			horizontal = (left ^ right) & rank;
			left = (myBoard->allPieceOccupancy & file) - 2 * mask;
			right = reverseBitMap(reverseBitMap(myBoard->allPieceOccupancy & file) - 2 * reverseBitMap(mask));
			vertical = (left ^ right) & file;
			myBoard->legalMoves[mask] = (horizontal | vertical) & myBoard->attackableAndEmptySquares;
		}
		mask <<= 1;
	}
}

void QueenMap::getPsuedoLegalMoves(Board* myBoard)
{
	U64 left;
	U64 right;
	U64 diagonal;
	U64 antidiagonal;
	U64 vertical;
	U64 horizontal;
	U64 file;
	U64 rank;
	U64 diagonalMask;
	U64 antidiagonalMask;
	U64 mask = 1;

	for (int i = 0; i < NUM_SQUARES; i++)
	{
		if (mask & pieceLoc)
		{
			// get bishop moves
			diagonalMask = DIAGONAL_MASKS[i / 8 + i % 8];
			antidiagonalMask = ANTIDIAGONAL_MASKS[i / 8 + 7 - i % 8];
			left = (myBoard->allPieceOccupancy & diagonalMask) - 2 * mask;
			right = reverseBitMap(reverseBitMap(myBoard->allPieceOccupancy & diagonalMask) - 2 * reverseBitMap(mask));
			diagonal = (left ^ right) & diagonalMask;
			left = (myBoard->allPieceOccupancy & antidiagonalMask) - 2 * mask;
			right = reverseBitMap(reverseBitMap(myBoard->allPieceOccupancy & antidiagonalMask) - 2 * reverseBitMap(mask));
			antidiagonal = (left ^ right) & antidiagonalMask;

			// get rook moves
			rank = RANK_MASKS[i / 8];
			file = FILE_MASKS[i % 8];
			left = (myBoard->allPieceOccupancy & rank) - 2 * mask;
			right = reverseBitMap(reverseBitMap(myBoard->allPieceOccupancy & rank) - 2 * reverseBitMap(mask));
			horizontal = (left ^ right) & rank;
			left = (myBoard->allPieceOccupancy & file) - 2 * mask;
			right = reverseBitMap(reverseBitMap(myBoard->allPieceOccupancy & file) - 2 * reverseBitMap(mask));
			vertical = (left ^ right) & file;

			// insert them into the legal move map
			myBoard->legalMoves[mask] = (horizontal | vertical | diagonal | antidiagonal) & myBoard->attackableAndEmptySquares;
		}
		mask <<= 1;
	}
}

void KingMap::getPsuedoLegalMoves(Board* myBoard)
{
	U64 attacks;
	for (U64 mask = 1; mask != 0; mask <<= 1)
		if (mask & pieceLoc)
		{
			attacks = (mask & NOT_FILE_A) << 1 | (mask & NOT_FILE_H) >> 1 | mask; // west one and east one
			// pushes the three bits in a row north and south then excludes the center
			myBoard->legalMoves[mask] = ((attacks ^ get_pieceLoc()) | attacks << 8 | attacks >> 8) & myBoard->attackableSquares;
		}
}

// utility functions
U64 reverseBitMap(U64 forward)
{
	U64 backwards = 0;
	for (int i = 0; i < 64; i++)
		backwards |= ((forward >> i) & 1) << (63 - i);
	return backwards;
}