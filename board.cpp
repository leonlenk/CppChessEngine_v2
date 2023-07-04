#include <string>
#include <memory>
#include "board.h"
#include "globals.h"
#include "pieceMaps.h"
#include "pawnMap.h"
#include "knightMap.h"
#include "kingMap.h"

using namespace std;

Board::Board()
{
	wPawns = new WPawnMap(65280);
	wKnights = new KnightMap(66);
	wKing = new KingMap(16);
	bPawns = new BPawnMap(71776119061217280);
	bKnights = new KnightMap(4755801206503243776);
	bKing = new KingMap(1152921504606846976);
}

Board::Board(string startingFen)
{
	
}

Board::~Board()
{
	delete wPawns;
	delete bPawns;
	delete wKnights;
	delete bKnights;
	delete wKing;
	delete bKing;
}

PieceMaps* Board::getPieceAtMask(const U64 mask)
{
	if (mask & (wPawns->get_pieceLoc())) return wPawns;
	else if (mask & (bPawns->get_pieceLoc())) return bPawns;
	else if (mask & (wKnights->get_pieceLoc())) return wKnights;
	else if (mask & (bKnights->get_pieceLoc())) return bKnights;
	else if (mask & (wKing->get_pieceLoc())) return wKing;
	else if (mask & (bKing->get_pieceLoc())) return bKing;

	return nullptr;
}