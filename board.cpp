#include <string>
#include <unordered_map>
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
	isWhitesMove = true;
	isCheckMate = false;
	halfTurnNum = 1;
	getAllLegalMoves();
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

		if (file > BOARD_WIDTH) { rank--; file = 1; }
		letter++;
	}

	letter++;
	if (startingFen[letter] == 'w')
		isWhitesMove = true;
	else
		isWhitesMove = false;

	getAllLegalMoves();
}

void Board::getAllLegalMoves()
{
	getAllPsuedoLegalMoves();
}

bool Board::makeMove(U64 fromWhere, U64 whereTo)
{
	if ((legalMoves[fromWhere] & whereTo) == 0)
		return false;
	int pieceIndex = getPieceIndexAtMask(fromWhere);
	allPieces[pieceIndex]->set_pieceLoc((allPieces[pieceIndex]->get_pieceLoc() ^ fromWhere) | whereTo);
	isWhitesMove = !isWhitesMove;
	getAllLegalMoves();
	halfTurnNum++;
	return true;
}

void Board::getAllPsuedoLegalMoves()
{
	int pieceIndex;

	// find moves only for the active player 
	if (isWhitesMove) pieceIndex = 6; 
	else pieceIndex = 12;

	findPieceOccupancy();
	for (int i = pieceIndex - 6; i < pieceIndex; i++)
		allPieces[i]->getPsuedoLegalMoves(this);
}

void Board::findPieceOccupancy()
{
	whitePieceOccupancy = 0;
	blackPieceOccupancy = 0;
	for (int i = 0; i < FIRST_BLACK_INDEX; i++)
		whitePieceOccupancy |= allPieces[i]->get_pieceLoc();
	for (int i = FIRST_BLACK_INDEX; i < NUM_PIECES; i++)
		blackPieceOccupancy |= allPieces[i]->get_pieceLoc();
	emptySquares = ~(blackPieceOccupancy | whitePieceOccupancy);

	if (isWhitesMove) attackableSquares = blackPieceOccupancy | emptySquares;
	else attackableSquares = whitePieceOccupancy | emptySquares;
	allPieceOccupancy = whitePieceOccupancy | blackPieceOccupancy;
	attackableAndEmptySquares = emptySquares | attackableSquares;
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

