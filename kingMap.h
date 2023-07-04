#pragma once
#include "pieceMaps.h"

class KingMap : public PieceMaps
{
public:
	KingMap() {}
	KingMap(U64 intialState) { set_pieceLoc(intialState); }
	virtual ~KingMap() {}
	virtual void updateAttackMap();
};