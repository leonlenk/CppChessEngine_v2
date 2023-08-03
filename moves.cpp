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

bool Board::makeMove(std::string algNotation)
{
	// check if castling
	U64 kingStart = isWhitesMove ? W_KING_START : B_KING_START;
	if (algNotation == "0-0") // kingside
		return makeMove(kingStart, kingStart << 2);
	if (algNotation == "0-0-0") // queen side
		return makeMove(kingStart, kingStart >> 2);
	
	U64 fromWhere;
	U64 whereTo;
	auto strItr = algNotation.begin();
	// ignore the piece indicator
	if (isalpha(strItr[0]) && isalpha(strItr[1]))
		strItr++;

	fromWhere = algLoc2Mask(strItr[0], strItr[1]);
	strItr += 2;
	// ignore captures
	strItr += tolower(strItr[0]) == 'x';
	// ignore captured piece name
	if (isalpha(strItr[0]) && isalpha(strItr[1]))
		strItr++;

	whereTo = algLoc2Mask(strItr[0], strItr[1]);
	return makeMove(fromWhere, whereTo);
}

bool Board::makeMove(U64 fromWhere, U64 whereTo)
{
	// check if move is legal
	if ((legalMoves[fromWhere] & whereTo) == 0) return false;
	int pieceIndex = getPieceIndexAtMask(fromWhere);
	int capturedPiece;
	// ensure the correct player is playing
	if (isWhitesMove && pieceIndex >= FIRST_BLACK_INDEX) return false;
	else if (!isWhitesMove && pieceIndex < FIRST_BLACK_INDEX) return false;

	HistoryFrame* tempHistory = new HistoryFrame;

	// check if en passent is being preformed
	if (whereTo == enPassentSquare && pieceIndex == W_PAWN_INDEX)
	{
		capturedPiece = B_PAWN_INDEX;
		allPieces[capturedPiece]->set_pieceLoc(allPieces[capturedPiece]->get_pieceLoc() ^ enPassentSquare >> 8);
		tempHistory->enPassentSquare = enPassentSquare;
	}
	else if (whereTo == enPassentSquare && pieceIndex == B_PAWN_INDEX)
	{
		capturedPiece = W_PAWN_INDEX;
		allPieces[capturedPiece]->set_pieceLoc(allPieces[capturedPiece]->get_pieceLoc() ^ enPassentSquare << 8);
		tempHistory->enPassentSquare = enPassentSquare;
	}
	else
	{
		capturedPiece = getPieceIndexAtMask(whereTo);
		// if piece is being captured remove it
		if (capturedPiece != -1)
			allPieces[capturedPiece]->set_pieceLoc(allPieces[capturedPiece]->get_pieceLoc() ^ whereTo);
	}

	// set history
	tempHistory->startLoc = fromWhere;
	tempHistory->endLoc = whereTo;
	tempHistory->movingPieceIndex = pieceIndex;
	tempHistory->capturedPieceIndex = capturedPiece;

	// check if an en passentable move was made
	if (pieceIndex == W_PAWN_INDEX && whereTo == fromWhere << 16)
		enPassentSquare = fromWhere << 8;
	else if (pieceIndex == B_PAWN_INDEX && whereTo == fromWhere >> 16)
		enPassentSquare = fromWhere >> 8;
	else
		enPassentSquare = 0;

	// move rook for castling
	// white king side castling
	if (pieceIndex == W_KING_INDEX && canCastle[0] && allPieces[W_KING_INDEX]->get_pieceLoc() & W_KING_START && whereTo == (fromWhere << 2))
		allPieces[W_ROOK_INDEX]->set_pieceLoc((allPieces[W_ROOK_INDEX]->get_pieceLoc() | W_KING_START << 1) ^ ROOK_START_ARRAY[0]);
	// white queen side castling
	else if (pieceIndex == W_KING_INDEX && canCastle[1] && allPieces[W_KING_INDEX]->get_pieceLoc() & W_KING_START && whereTo == (fromWhere >> 2))
		allPieces[W_ROOK_INDEX]->set_pieceLoc((allPieces[W_ROOK_INDEX]->get_pieceLoc() | W_KING_START >> 1) ^ ROOK_START_ARRAY[1]);
	// black king side castling
	else if (pieceIndex == B_KING_INDEX && canCastle[2] && allPieces[B_KING_INDEX]->get_pieceLoc() & B_KING_START && whereTo == (fromWhere << 2))
		allPieces[B_ROOK_INDEX]->set_pieceLoc((allPieces[B_ROOK_INDEX]->get_pieceLoc() | B_KING_START << 1) ^ ROOK_START_ARRAY[2]);
	// black queen side castling
	else if (pieceIndex == B_KING_INDEX && canCastle[3] && allPieces[B_KING_INDEX]->get_pieceLoc() & B_KING_START && whereTo == (fromWhere >> 2))
		allPieces[B_ROOK_INDEX]->set_pieceLoc((allPieces[B_ROOK_INDEX]->get_pieceLoc() | B_KING_START >> 1) ^ ROOK_START_ARRAY[3]);

	// move the piece
	allPieces[pieceIndex]->set_pieceLoc((allPieces[pieceIndex]->get_pieceLoc() ^ fromWhere) | whereTo);
	// check if pawn is being promoted
	if (pieceIndex == W_PAWN_INDEX && whereTo & RANK_8)
	{
		allPieces[W_PAWN_INDEX]->set_pieceLoc(allPieces[W_PAWN_INDEX]->get_pieceLoc() ^ whereTo);
		allPieces[wPawnPromote]->set_pieceLoc(allPieces[wPawnPromote]->get_pieceLoc() | whereTo);
		tempHistory->pawnPromotion = wPawnPromote;
	}
	else if (pieceIndex == B_PAWN_INDEX && whereTo & RANK_1)
	{
		allPieces[B_PAWN_INDEX]->set_pieceLoc(allPieces[B_PAWN_INDEX]->get_pieceLoc() ^ whereTo);
		allPieces[bPawnPromote]->set_pieceLoc(allPieces[bPawnPromote]->get_pieceLoc() | whereTo);
		tempHistory->pawnPromotion = bPawnPromote;
	}

	for (int i = 0; i < 4; i++)
		tempHistory->canCastle[i] = canCastle[i];
	history.push(tempHistory);

	// check if castling is still legal
	if (fromWhere & W_KING_START) { canCastle[0] = false; canCastle[1] = false; }
	if (fromWhere & B_KING_START) { canCastle[2] = false; canCastle[3] = false; }
	for (int i = 0; i < 4; i++)
		if (fromWhere & ROOK_START_ARRAY[i] || whereTo & ROOK_START_ARRAY[i])
			canCastle[i] = false;

	// set appropriate flags and regen legal moves
	// increments on blacks move
	fullMoveNum += !isWhitesMove;
	// half move clock is reset if pawn move or piece captured
	if (capturedPiece != -1 || pieceIndex == W_PAWN_INDEX || pieceIndex == B_PAWN_INDEX)
		halfTurnNum = 0;
	else halfTurnNum++;
	isWhitesMove = !isWhitesMove;
	generateAllLegalMoves();
	// checks if the move puts the king in check
	tempHistory->inCheck = isWhitesMove ? allPieces[W_KING_INDEX]->get_pieceLoc() & allBlackAttacks : allPieces[B_KING_INDEX]->get_pieceLoc() & allWhiteAttacks;
	return true;
}

bool Board::undoMove()
{
	if (history.empty()) return false;
	HistoryFrame* tempHistory = history.top();
	// add moving piece to its original position and remove it from the old one
	allPieces[tempHistory->movingPieceIndex]->set_pieceLoc((allPieces[tempHistory->movingPieceIndex]->get_pieceLoc() | tempHistory->startLoc) ^ tempHistory->endLoc);
	// check for castling
	if (tempHistory->movingPieceIndex == W_KING_INDEX) // white
	{
		if (tempHistory->startLoc << 2 == tempHistory->endLoc) // king side castle
			allPieces[W_ROOK_INDEX]->set_pieceLoc((allPieces[W_ROOK_INDEX]->get_pieceLoc() | W_KING_ROOK_START) ^ W_KING_START << 1);
		if (tempHistory->startLoc >> 2 == tempHistory->endLoc) // queen side
			allPieces[W_ROOK_INDEX]->set_pieceLoc((allPieces[W_ROOK_INDEX]->get_pieceLoc() | W_QUEEN_ROOK_START) ^ W_KING_START >> 1);
	}
	else if (tempHistory->movingPieceIndex == B_KING_INDEX) //black
	{
		if (tempHistory->startLoc << 2 == tempHistory->endLoc) // king side
			allPieces[B_ROOK_INDEX]->set_pieceLoc((allPieces[B_ROOK_INDEX]->get_pieceLoc() | B_KING_ROOK_START) ^ B_KING_START << 1);
		if (tempHistory->startLoc >> 2 == tempHistory->endLoc) // queen side
			allPieces[B_ROOK_INDEX]->set_pieceLoc((allPieces[B_ROOK_INDEX]->get_pieceLoc() | B_QUEEN_ROOK_START) ^ B_KING_START >> 1);
	}

	// add back captured piece
	// load enpassent specially
	if (tempHistory->capturedPieceIndex != -1)
	{
		if (tempHistory->enPassentSquare != 0)
		{
			// negated to denote that we want to know if it was white capturing last turn
			U64 enPassentPawnLoc = isWhitesMove ? tempHistory->endLoc << 8 : tempHistory->endLoc >> 8;
			allPieces[tempHistory->capturedPieceIndex]->set_pieceLoc(allPieces[tempHistory->capturedPieceIndex]->get_pieceLoc() | enPassentPawnLoc);
		}
		else
			allPieces[tempHistory->capturedPieceIndex]->set_pieceLoc(allPieces[tempHistory->capturedPieceIndex]->get_pieceLoc() | tempHistory->endLoc);
	}

	// undo pawn promotion
	if (tempHistory->pawnPromotion != -1)
	{
		allPieces[tempHistory->pawnPromotion]->set_pieceLoc(allPieces[tempHistory->pawnPromotion]->get_pieceLoc() ^ tempHistory->endLoc);
		int pawnColor = isWhitesMove ? B_PAWN_INDEX : W_PAWN_INDEX; // check last turn
		allPieces[pawnColor]->set_pieceLoc(allPieces[pawnColor]->get_pieceLoc() ^ tempHistory->endLoc);
	}
	enPassentSquare = tempHistory->enPassentSquare;
	for (int i = 0; i < 4; i++)
		canCastle[i] = tempHistory->canCastle[i];
	history.pop();
	delete tempHistory;
	// only decrement on blacks move
	fullMoveNum -= !isWhitesMove;
	if (halfTurnNum > 0)
		halfTurnNum--;
	isWhitesMove = !isWhitesMove;
	generateAllLegalMoves();
	return true;
}