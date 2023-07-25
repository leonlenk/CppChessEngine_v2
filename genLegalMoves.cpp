#include <string>
#include <unordered_map>
#include <bit>
#include <iostream>
#include "board.h"
#include "globals.h"
#include "pieceMaps.h"

using namespace std;

void Board::generateAllLegalMoves()
{
	setLegalMoveFlags();

	// get white legal moves
	allTempAttacks = 0;
	attackableSquares = blackPieceOccupancy | emptySquares;
	attackableAndEmptySquares = emptySquares | attackableSquares;
	findPinsAndChecks(true);
	if (popcount(locOfChecks) > 1) // if there is more than one check only the king can move
	{
		removeFromLegalMoves(whitePieceOccupancy);
		allPieces[W_KING_INDEX]->getPsuedoLegalMoves(this);
	}
	else
		for (int i = 0; i < FIRST_BLACK_INDEX; i++)
			allPieces[i]->getPsuedoLegalMoves(this);
	allWhiteAttacks = allTempAttacks;

	// get black legal moves
	allTempAttacks = 0;
	attackableSquares = whitePieceOccupancy | emptySquares;
	attackableAndEmptySquares = emptySquares | attackableSquares;
	findPinsAndChecks(false);
	if (popcount(locOfChecks) > 1)
	{
		removeFromLegalMoves(blackPieceOccupancy);
		allPieces[B_KING_INDEX]->getPsuedoLegalMoves(this);
	}
	else
		for (int i = FIRST_BLACK_INDEX; i < NUM_PIECES; i++)
			allPieces[i]->getPsuedoLegalMoves(this);
	allBlackAttacks = allTempAttacks;

	// make sure king doesn't move into an attacked square
	legalMoves[allPieces[W_KING_INDEX]->get_pieceLoc()] &= ~allBlackAttacks;
	legalMoves[allPieces[B_KING_INDEX]->get_pieceLoc()] &= ~allWhiteAttacks;

	// restrict the movment of pinned pieces
	while (numOfPins > 0)
	{
		numOfPins--;
		legalMoves[pinnedPieceLoc[numOfPins]] &= pinnedMovementMasks[numOfPins];
	}
	
	// white king side castling
	if ((allPieceOccupancy & CASTLING_MASKS[0]) == 0 && (allBlackAttacks & CASTLING_MASKS[0]) == 0 && canCastle[0] && (allPieces[W_KING_INDEX]->get_pieceLoc() & allBlackAttacks) == 0)
		legalMoves[W_KING_START] |= W_KING_START << 2;
	// white queen side castling
	if ((allPieceOccupancy & CASTLING_MASKS[1]) == 0 && (allBlackAttacks & CASTLING_MASKS[1]) == 0 && canCastle[1] && (allPieces[W_KING_INDEX]->get_pieceLoc() & allBlackAttacks) == 0)
		legalMoves[W_KING_START] |= W_KING_START >> 2;

	// black king side castling
	if ((allPieceOccupancy & CASTLING_MASKS[2]) == 0 && (allWhiteAttacks & CASTLING_MASKS[2]) == 0 && canCastle[2] && (allPieces[B_KING_INDEX]->get_pieceLoc() & allWhiteAttacks) == 0)
		legalMoves[B_KING_START] |= B_KING_START << 2;
	// black queen side castling
	if ((allPieceOccupancy & CASTLING_MASKS[3]) == 0 && (allWhiteAttacks & CASTLING_MASKS[3]) == 0 && canCastle[3] && (allPieces[B_KING_INDEX]->get_pieceLoc() & allWhiteAttacks) == 0)
		legalMoves[B_KING_START] |= B_KING_START >> 2;
}

void Board::removeFromLegalMoves(U64 piecesToRemove)
{
	U64 tempMask;
	U64 tempPiecesToRemove = piecesToRemove;
	U64 one = 1;
	while (tempPiecesToRemove)
	{
		tempMask = one << countr_zero(tempPiecesToRemove);
		legalMoves[tempMask] = 0;
		tempPiecesToRemove ^= tempMask;
	}
}

void Board::setLegalMoveFlags()
{
	numOfPins = 0;
	whitePieceOccupancy = 0;
	blackPieceOccupancy = 0;
	for (int i = 0; i < FIRST_BLACK_INDEX; i++)
		whitePieceOccupancy |= allPieces[i]->get_pieceLoc();
	for (int i = FIRST_BLACK_INDEX; i < NUM_PIECES; i++)
		blackPieceOccupancy |= allPieces[i]->get_pieceLoc();
	emptySquares = ~(blackPieceOccupancy | whitePieceOccupancy);
	allPieceOccupancy = whitePieceOccupancy | blackPieceOccupancy;
}

bool Board::makeMove(U64 fromWhere, U64 whereTo)
{
	// check if move is legal
	if ((legalMoves[fromWhere] & whereTo) == 0) return false;
	int pieceIndex = getPieceIndexAtMask(fromWhere);
	int capturedPiece;
	// check if en passent is being preformed
	if (whereTo == enPassentSquare && pieceIndex == W_PAWN_INDEX)
	{
		capturedPiece = B_PAWN_INDEX;
		allPieces[capturedPiece]->set_pieceLoc(allPieces[capturedPiece]->get_pieceLoc() ^ enPassentSquare >> 8);
	}
	else if (whereTo == enPassentSquare && pieceIndex == B_PAWN_INDEX)
	{
		capturedPiece = W_PAWN_INDEX;
		allPieces[capturedPiece]->set_pieceLoc(allPieces[capturedPiece]->get_pieceLoc() ^ enPassentSquare << 8);
	}
	else
	{
		capturedPiece = getPieceIndexAtMask(whereTo);
		// if piece is being captured remove it
		if (capturedPiece != -1)
			allPieces[capturedPiece]->set_pieceLoc(allPieces[capturedPiece]->get_pieceLoc() ^ whereTo);
	}

	// check for en passentable pawn moves
	if (pieceIndex == W_PAWN_INDEX && whereTo == fromWhere << 16)
		enPassentSquare = fromWhere << 8;
	else if (pieceIndex == B_PAWN_INDEX && whereTo == fromWhere >> 16)
		enPassentSquare = fromWhere >> 8;
	else
		enPassentSquare = 0;

	// move rook for castling
	// white king side castling
	if (pieceIndex == W_KING_INDEX && canCastle[0] && allPieces[W_KING_INDEX]->get_pieceLoc() & W_KING_START && whereTo == (W_KING_START << 2))
		allPieces[W_ROOK_INDEX]->set_pieceLoc((allPieces[W_ROOK_INDEX]->get_pieceLoc() | W_KING_START << 1) ^ ROOK_START_ARRAY[0]);
	// white queen side castling
	else if (pieceIndex == W_KING_INDEX && canCastle[1] && allPieces[W_KING_INDEX]->get_pieceLoc() & W_KING_START && whereTo == (W_KING_START >> 2))
		allPieces[W_ROOK_INDEX]->set_pieceLoc((allPieces[W_ROOK_INDEX]->get_pieceLoc() | W_KING_START >> 1) ^ ROOK_START_ARRAY[1]);
	// black king side castling
	else if (pieceIndex == B_KING_INDEX && canCastle[2] && allPieces[B_KING_INDEX]->get_pieceLoc() & B_KING_START && whereTo == (B_KING_START << 2))
		allPieces[B_ROOK_INDEX]->set_pieceLoc((allPieces[B_ROOK_INDEX]->get_pieceLoc() | B_KING_START << 1) ^ ROOK_START_ARRAY[2]);
	// black queen side castling
	else if (pieceIndex == B_KING_INDEX && canCastle[3] && allPieces[B_KING_INDEX]->get_pieceLoc() & B_KING_START && whereTo == (B_KING_START >> 2))
		allPieces[B_ROOK_INDEX]->set_pieceLoc((allPieces[B_ROOK_INDEX]->get_pieceLoc() | B_KING_START >> 1) ^ ROOK_START_ARRAY[3]);

	// move the piece
	allPieces[pieceIndex]->set_pieceLoc((allPieces[pieceIndex]->get_pieceLoc() ^ fromWhere) | whereTo);

	// check if castling is still legal
	if (fromWhere & W_KING_START) { canCastle[0] = false; canCastle[1] = false; }
	if (fromWhere & B_KING_START) { canCastle[2] = false; canCastle[3] = false; }
	for (int i = 0; i < 4; i++)
		if (fromWhere & ROOK_START_ARRAY[i] || whereTo & ROOK_START_ARRAY[i])
			canCastle[i] = false;

	// set appropriate flags and regen legal moves
	isWhitesMove = !isWhitesMove;
	generateAllLegalMoves();
	halfTurnNum++;
	return true;
}
