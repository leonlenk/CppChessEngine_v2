#pragma once

#include <unordered_map>
#include "globals.h"

class Board;

class PieceMaps
{
public:
	PieceMaps() 
	{
		pieceLoc = 0;
	}
	virtual ~PieceMaps() {}
	virtual void getPsuedoLegalMoves(Board* myBoard) = 0;

	// accessor functions
	U64 get_pieceLoc();
	void set_pieceLoc(U64 newLoc);
protected:
	U64 pieceLoc;
};

inline U64 PieceMaps::get_pieceLoc() { return pieceLoc; }
inline void PieceMaps::set_pieceLoc(U64 newLoc) { pieceLoc = newLoc; };


class WPawnMap : public PieceMaps
{
public:
	WPawnMap() {}
	WPawnMap(U64 intialState) { set_pieceLoc(intialState); }
	virtual ~WPawnMap() {}
	virtual void getPsuedoLegalMoves(Board* myBoard);
};

class BPawnMap : public PieceMaps
{
public:
	BPawnMap() {}
	BPawnMap(U64 intialState) { set_pieceLoc(intialState); }
	virtual ~BPawnMap() {}
	virtual void getPsuedoLegalMoves(Board* myBoard);
};

class KnightMap : public PieceMaps
{
public:
	KnightMap() {}
	KnightMap(U64 intialState) { set_pieceLoc(intialState); }
	virtual ~KnightMap() {}
	virtual void getPsuedoLegalMoves(Board* myBoard);
};

class BishopMap : public PieceMaps
{
public:
	BishopMap() {}
	BishopMap(U64 intialState) { set_pieceLoc(intialState); }
	virtual ~BishopMap() {}
	virtual void getPsuedoLegalMoves(Board* myBoard);
};

class RookMap : public PieceMaps
{
public:
	RookMap() {}
	RookMap(U64 intialState) { set_pieceLoc(intialState); }
	virtual ~RookMap() {}
	virtual void getPsuedoLegalMoves(Board* myBoard);
};

class QueenMap : public PieceMaps
{
public:
	QueenMap() {}
	QueenMap(U64 intialState) { set_pieceLoc(intialState); }
	virtual ~QueenMap() {}
	virtual void getPsuedoLegalMoves(Board* myBoard);
};

class KingMap : public PieceMaps
{
public:
	KingMap() {}
	KingMap(U64 intialState) { set_pieceLoc(intialState); }
	virtual ~KingMap() {}
	virtual void getPsuedoLegalMoves(Board* myBoard);
	void findPinsAndChecks();
};
