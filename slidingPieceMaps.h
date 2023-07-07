#pragma once

#include "pieceMaps.h"

class RookMap : public PieceMaps
{
public:
	RookMap() {}
	RookMap(U64 intialState) { set_pieceLoc(intialState); }
	virtual ~RookMap() {}
	virtual void updateAttackMap();
};

class BishopMap : public PieceMaps
{
public:
	BishopMap() {}
	BishopMap(U64 intialState) { set_pieceLoc(intialState); }
	virtual ~BishopMap() {}
	virtual void updateAttackMap();
};

class QueenMap : public PieceMaps
{
public:
	QueenMap() {}
	QueenMap(U64 intialState) { set_pieceLoc(intialState); }
	virtual ~QueenMap() {}
	virtual void updateAttackMap();
};