#include <string>
#include <unordered_map>
#include <iostream>
#include "board.h"
#include "globals.h"
#include "pieceMaps.h"

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

	// sets all pieces to have no legal moves in the beginning
	U64 one = 1;
	for (int i = 0; i < NUM_PIECES; i++)
		legalMoves[one << i] = 0;
	// all castling rights are preserved at the beginning of the game
	for (int i = 0; i < 4; i++)
		canCastle[i] = true;
	isWhitesMove = true;
	isCheckMate = false;
	halfTurnNum = 0;
	allWhiteAttacks = 0;
	allBlackAttacks = 0;
	attackableSquares = 0;
	enPassentSquare = 0;
	generateAllLegalMoves();
}

Board::Board(string startingFen) : Board()
{
	int letter = 0;
	int rank = 8;
	int file = 1;
	U64 pieceLoc = 1;
	size_t pieceIndex;

	for (int i = 0; i < NUM_PIECES; i++)
		allPieces[i]->set_pieceLoc(0);

	if (startingFen == "empty") return;

	// assigns piece position in bit maps based on FEN string
	while (startingFen[letter] != ' ')
	{
		if (isalpha(startingFen[letter]))
		{
			pieceIndex = FEN_PIECES.find(startingFen[letter]);
			pieceLoc = 1;
			pieceLoc <<= 8 * (rank - 1) + (file - 1);
			allPieces[pieceIndex]->set_pieceLoc(allPieces[pieceIndex]->get_pieceLoc() | pieceLoc);
			file++;
		}
		else if (isdigit(startingFen[letter]))
			file += startingFen[letter] - '0';

		if (file > BOARD_WIDTH) { rank--; file = 1; }
		letter++;
	}

	letter++;
	if (startingFen[letter] == 'w')
		isWhitesMove = true;
	else
		isWhitesMove = false;
	letter += 2;

	// find castling permissions
	for (int i = 0; i < 4; i++)
		canCastle[i] = false;
	if (startingFen[letter] != '-')
		for (int i = 0; i < 4; i++)
			switch (startingFen[letter + i])
			{
			case ' ': break;
			case 'K': canCastle[0] = true; break;
			case 'Q': canCastle[1] = true; break;
			case 'k': canCastle[2] = true; break;
			case 'q': canCastle[3] = true; break;
			}

	for (int i = 0; i < 4; i++)
		if (canCastle[i] == true)
			letter++;
	letter++;

	generateAllLegalMoves();
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

