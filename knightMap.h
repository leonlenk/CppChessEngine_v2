#pragma once
#include "pieceMaps.h"

class KnightMap : public PieceMaps
{
public:
	KnightMap() {}
	KnightMap(U64 intialState) { set_pieceLoc(intialState); }
	virtual ~KnightMap() {}
	virtual void updateAttackMap();
};