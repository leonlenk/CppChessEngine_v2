#include <string>
#include <iostream>
#include <chrono>
#include "board.h"
#include "globals.h"

using namespace std;

int Board::runSplitPERFT(int depth)
{
	cout << "Running split PERFT to depth: " << depth << endl;
	U64 activePieces = isWhitesMove ? whitePieceOccupancy : blackPieceOccupancy;
	U64 pieceMask, moveMask, potentialMoves;
	U64 promotionRank = isWhitesMove ? RANK_7 : RANK_2;
	int pawnIndex = isWhitesMove ? W_PAWN_INDEX : B_PAWN_INDEX;
	int absoluteTotal = 0;
	int branchTotal = 0;
	auto startClock = chrono::steady_clock::now();

	while (activePieces)
	{
		pieceMask = ONE << countr_zero(activePieces);
		potentialMoves = legalMoves[pieceMask];
		if (pieceMask & allPieces[pawnIndex]->get_pieceLoc() && pieceMask & promotionRank) // if pawn is about to promote
			while (potentialMoves)
			{
				moveMask = ONE << countr_zero(potentialMoves);
				for (int i = W_KNIGHT_INDEX + pawnIndex; i <= W_QUEEN_INDEX + pawnIndex; i++)
				{
					wPawnPromote = i;
					bPawnPromote = i;
					makeMove(pieceMask, moveMask);
					branchTotal = runPERFT(depth - 1);
					absoluteTotal += branchTotal;
					cout << history2Alg(get_mostRecentMove()) << ": " << branchTotal << endl;
					undoMove();
				}
				potentialMoves ^= moveMask;
			}
		else
			while (potentialMoves)
			{
				moveMask = ONE << countr_zero(potentialMoves);
				makeMove(pieceMask, moveMask);
				branchTotal = runPERFT(depth - 1);
				absoluteTotal += branchTotal;
				cout << history2Alg(get_mostRecentMove()) << ": " << branchTotal << endl;
				undoMove();
				potentialMoves ^= moveMask;
			}
		activePieces ^= pieceMask;
	}

	auto stopClock = chrono::steady_clock::now();

	cout << "Found a total of: " << absoluteTotal << " nodes" << endl;
	cout << "In: " << chrono::duration_cast<chrono::milliseconds>(stopClock - startClock).count() << " ms" << endl;
	return absoluteTotal;
}

int Board::runPERFT(int depth)
{
	if (depth <= 0)
		return 1;
	U64 activePieces = isWhitesMove ? whitePieceOccupancy : blackPieceOccupancy;
	U64 pieceMask, moveMask, potentialMoves;
	U64 promotionRank = isWhitesMove ? RANK_7 : RANK_2;
	int pawnIndex = isWhitesMove ? W_PAWN_INDEX : B_PAWN_INDEX;
	int total = 0;

	while (activePieces)
	{
		pieceMask = ONE << countr_zero(activePieces);
		potentialMoves = legalMoves[pieceMask];
		if (pieceMask & allPieces[pawnIndex]->get_pieceLoc() && pieceMask & promotionRank) // if pawn is about to promote
			while (potentialMoves)
			{
				moveMask = ONE << countr_zero(potentialMoves);
				for (int i = W_KNIGHT_INDEX + pawnIndex; i <= W_QUEEN_INDEX + pawnIndex; i++)
				{
					wPawnPromote = i;
					bPawnPromote = i;
					makeMove(pieceMask, moveMask);
					total += runPERFT(depth - 1);
					undoMove();
				}
				potentialMoves ^= moveMask;
			}
		else
			while (potentialMoves)
			{
				moveMask = ONE << countr_zero(potentialMoves);
				makeMove(pieceMask, moveMask);
				total += runPERFT(depth - 1);
				undoMove();
				potentialMoves ^= moveMask;
			}
		activePieces ^= pieceMask;
	}

	return total;
}
