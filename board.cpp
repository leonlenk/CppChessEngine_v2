#include <string>
#include <unordered_map>
#include <iostream>
#include <bit>
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
	// by default all pawns promote to queens
	wPawnPromote = W_QUEEN_INDEX;
	bPawnPromote = B_QUEEN_INDEX;
	halfTurnNum = 0;
	allWhiteAttacks = 0;
	allBlackAttacks = 0;
	attackableSquares = 0;
	enPassentSquare = 0;
	fullMoveNum = 0;
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

	if (startingFen[letter] != '-')
	{
		enPassentSquare = algLoc2Mask(startingFen[letter], startingFen[letter + 1]);
		letter++;
	}

	letter += 2;
	string turnSubString = "";
	while (startingFen[letter] != ' ')
	{
		turnSubString += startingFen[letter];
		letter++;
	}

	halfTurnNum = stoi(turnSubString);

	letter++;

	turnSubString = "";
	while (letter != startingFen.length())
	{
		turnSubString += startingFen[letter];
		letter++;
	}

	fullMoveNum = stoi(turnSubString);

	generateAllLegalMoves();
}

Board::~Board()
{
	for (int i = 0; i < NUM_PIECES; i++)
		if (allPieces[i] != nullptr)
			delete allPieces[i];

	while (history.empty() == false)
	{
		delete history.top();
		history.pop();
	}
}

string Board::exportFEN()
{
	string FENString = "";
	U64 shift;
	int pieceIndex;
	char spaces = '0';

	// add piece position
	for (int row = 7; row >= 0; row--)
	{
		shift = ONE << row * 8;
		for (int col = 0; col < BOARD_WIDTH; col++)
		{
			pieceIndex = getPieceIndexAtMask(shift);
			if (pieceIndex != -1)
			{
				if (spaces != '0')
				{
					FENString += spaces;
					spaces = '0';
				}
				FENString += FEN_PIECES[pieceIndex];
			}
			else
				spaces++;

			shift <<= 1;
		}
		if (spaces != '0')
		{
			FENString += spaces;
			spaces = '0';
		}
		FENString += '/';
	}

	// remove last / from end of string
	FENString.pop_back();
	FENString += ' ';
	FENString += isWhitesMove ? 'w' : 'b';
	FENString += ' ';
	
	// add castling rights
	for (int i = 0; i < 4; i++)
		if (canCastle[i])
			FENString += CASTLE_NOTATION[i];
	
	// if no castling rights are left there should be a dash
	if (FENString.back() == ' ')
		FENString += '-';

	FENString += ' ';

	// add enpassent
	FENString += mask2AlgLoc(enPassentSquare);

	FENString += ' ';

	FENString += to_string(halfTurnNum);
	FENString += ' ';
	FENString += to_string(fullMoveNum);

	return FENString;
}

string Board::bitMap2Alg(U64 fromWhere, U64 whereTo)
{
	HistoryFrame tempHistory;
	tempHistory.startLoc = fromWhere;
	tempHistory.endLoc = whereTo;
	tempHistory.enPassentSquare = whereTo & enPassentSquare;
	tempHistory.movingPieceIndex = getPieceIndexAtMask(fromWhere);
	tempHistory.capturedPieceIndex = getPieceIndexAtMask(whereTo);

	if (tempHistory.movingPieceIndex == W_PAWN_INDEX && whereTo & RANK_8)
		tempHistory.pawnPromotion = wPawnPromote;
	else if (tempHistory.movingPieceIndex == B_PAWN_INDEX && whereTo & RANK_1)
		tempHistory.pawnPromotion = bPawnPromote;
	else
		tempHistory.pawnPromotion = -1;

	return history2Alg(&tempHistory);
}

string Board::history2Alg(HistoryFrame* myMove)
{
	if (myMove == nullptr)
		return "Error";
	string tempAlg = "";
	tempAlg += FEN_PIECES[myMove->movingPieceIndex];
	tempAlg += mask2AlgLoc(myMove->startLoc);

	// handle captured pieces
	if (myMove->capturedPieceIndex != -1)
	{
		tempAlg += 'x';
		tempAlg += FEN_PIECES[myMove->capturedPieceIndex];
	}

	tempAlg += mask2AlgLoc(myMove->endLoc);
	
	// in case of pawn promotion
	if (myMove->pawnPromotion != -1)
	{
		tempAlg += '=';
		tempAlg += tolower(FEN_PIECES[myMove->pawnPromotion]);
	}

	// if in check add plus
	if (myMove->inCheck)
		tempAlg += '+';

	// in case of castling
	if (myMove->movingPieceIndex == W_KING_INDEX || myMove->movingPieceIndex == B_KING_INDEX)
	{
		if (myMove->startLoc << 2 == myMove->endLoc) // king side castle
			tempAlg = "0-0";
		if (myMove->startLoc >> 2 == myMove->endLoc) // queen side
			tempAlg = "0-0-0";
	}

	return tempAlg;
}

string Board::mask2AlgLoc(U64 mask)
{
	if (popcount(mask) != 1) return "-";
	string algNota = "";
	int numShifts = countr_zero(mask);
	algNota += char(numShifts % 8 + 'a');
	algNota += char(numShifts / 8 + '1');
	return algNota;
}

U64 Board::algLoc2Mask(char first, char second)
{
	return ONE << ((first - 'a') + (second - '1') * 8);
}

U64 Board::algLoc2Mask(string algLoc)
{
	if (algLoc.length() != 2) return 0;
	return ONE << ((algLoc[0] - 'a') + (algLoc[1] - '1') * 8);
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

