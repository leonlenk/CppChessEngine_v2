#include <string>
#include <memory>
#include "board.h"
#include "globals.h"
#include "pieceMaps.h"
#include "pawnMap.h"
#include "knightMap.h"
#include "kingMap.h"
#include "slidingPieceMaps.h"

using namespace std;

Board::Board()
{
	// white pieces
	allPieces[0] = new WPawnMap(65280);
	allPieces[1] = new KnightMap(66);
	allPieces[2] = new BishopMap(36);
	allPieces[3] = new RookMap(129);
	allPieces[4] = new QueenMap(8);
	allPieces[5] = new KingMap(16);

	// black pieces
	allPieces[6] = new BPawnMap(71776119061217280);
	allPieces[7] = new KnightMap(4755801206503243776);
	allPieces[8] = new BishopMap(2594073385365405696);
	allPieces[9] = new RookMap(9295429630892703744);
	allPieces[10] = new QueenMap(576460752303423488);
	allPieces[11] = new KingMap(1152921504606846976);
}

Board::Board(string startingFen)
{
	
}

Board::~Board()
{
	for (int i = 0; i < numOfPieces; i++)
		if (allPieces[i] != nullptr)
			delete allPieces[i];
}

PieceMaps* Board::getPieceAtMask(const U64 mask) const
{
	for (int i = 0; i < numOfPieces; i++)
		if ((mask & allPieces[i]->get_pieceLoc()) != 0)
			return allPieces[i];
	return nullptr;
}

int Board::getPieceIndexAtMask(const U64 mask) const
{
	for (int i = 0; i < numOfPieces; i++)
		if ((mask & allPieces[i]->get_pieceLoc()) != 0)
			return i;
	return -1;
}