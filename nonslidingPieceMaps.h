#pragma once

#include "globals.h"
#include "pieceMaps.h"

class WPawnMap : public PieceMaps
{
public:
	WPawnMap() {}
	WPawnMap(U64 intialState) { set_pieceLoc(intialState); }
	virtual ~WPawnMap() {}
	virtual void updateAttackMap();
};

inline void WPawnMap::updateAttackMap() { set_attackMap(((get_pieceLoc() & notAFile) << 9) | ((get_pieceLoc() & notHFile) << 7)); }

class BPawnMap : public PieceMaps
{
public:
	BPawnMap() {}
	BPawnMap(U64 intialState) { set_pieceLoc(intialState); }
	virtual ~BPawnMap() {}
	virtual void updateAttackMap();
};

inline void BPawnMap::updateAttackMap() { set_attackMap(((get_pieceLoc() & notHFile) >> 9) | ((get_pieceLoc() & notAFile) >> 7)); }

class KnightMap : public PieceMaps
{
public:
	KnightMap() {}
	KnightMap(U64 intialState) { set_pieceLoc(intialState); }
	virtual ~KnightMap() {}
	virtual void updateAttackMap();
};

class KingMap : public PieceMaps
{
public:
	KingMap() {}
	KingMap(U64 intialState) { set_pieceLoc(intialState); }
	virtual ~KingMap() {}
	virtual void updateAttackMap();
};