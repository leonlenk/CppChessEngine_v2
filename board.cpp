#include <string>
#include <memory>
#include "board.h"
#include "globals.h"
#include "pieceMaps.h"
#include "nonslidingPieceMaps.h"
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

	isWhitesMove = true;
	halfTurnNum = 1;
	makeAttackMaps();
}

Board::Board(string startingFen) : Board()
{
	int letter = 0;
	int rank = 8;
	int file = 1;

	for (int i = 0; i < NUM_PIECES; i++)
		allPieces[i]->set_pieceLoc(0);

	if (startingFen == "empty") return;

	// assigns piece position in bit maps based on FEN string
	while (startingFen[letter] != ' ')
	{
		if (isalpha(startingFen[letter]))
		{
			//AddPiece(rank, file, letter);
			file++;
		}
		else if (isdigit(startingFen[letter]))
			file += startingFen[letter] - '0';

		if (file > BOARD_SIZE) { rank--; file = 1; }
		letter++;
	}

	letter++;
	if (startingFen[letter] == 'w')
		isWhitesMove = true;
	else
		isWhitesMove = false;

	makeAttackMaps();
}

bool Board::makeMove(U64 fromWhere, U64 whereTo, int pieceIndex)
{
	if (fromWhere == whereTo)
		return false;
	allPieces[pieceIndex]->set_pieceLoc((allPieces[pieceIndex]->get_pieceLoc() ^ fromWhere) | whereTo);
	makeAttackMaps();
	halfTurnNum++;
	isWhitesMove = !isWhitesMove;
	return true;
}

void Board::makeAttackMaps()
{
	int i;
	int pieceIndex;

	// makes attack maps only for the other player 
	if (isWhitesMove) { i = 6; pieceIndex = 12; }
	else { i = 0; pieceIndex = 6; }

	for (; i < pieceIndex; i++)
		allPieces[i]->updateAttackMap();
}

Board::~Board()
{
	for (int i = 0; i < NUM_PIECES; i++)
		if (allPieces[i] != nullptr)
			delete allPieces[i];
}

PieceMaps* Board::getPieceAtMask(const U64 mask) const
{
	for (int i = 0; i < NUM_PIECES; i++)
		if ((mask & allPieces[i]->get_pieceLoc()) != 0)
			return allPieces[i];
	return nullptr;
}

int Board::getPieceIndexAtMask(const U64 mask) const
{
	for (int i = 0; i < NUM_PIECES; i++)
		if ((mask & allPieces[i]->get_pieceLoc()) != 0)
			return i;
	return -1;
}

