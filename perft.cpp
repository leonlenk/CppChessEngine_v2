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
	int absoluteTotal = 0;
	int branchTotal = 0;
	auto startClock = chrono::steady_clock::now();

	while (activePieces)
	{
		pieceMask = ONE << countr_zero(activePieces);
		potentialMoves = legalMoves[pieceMask];
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
	int total = 0;
	while (activePieces)
	{
		pieceMask = ONE << countr_zero(activePieces);
		potentialMoves = legalMoves[pieceMask];
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
