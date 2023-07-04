#pragma once

#include "globals.h"

class PieceMaps
{
public:
	PieceMaps() 
	{
		pieceLoc = 0;
		attackMap = 0;
	}
	virtual ~PieceMaps() {}
	virtual void updateAttackMap() = 0;

	// accessor functions
	U64 get_pieceLoc();
	void set_pieceLoc(U64 newLoc);
	U64 get_attackMap();
	void set_attackMap(U64 newAttack);
private:
	U64 pieceLoc;
	U64 attackMap;
};

inline U64 PieceMaps::get_pieceLoc() { return pieceLoc; }
inline void PieceMaps::set_pieceLoc(U64 newLoc) { pieceLoc = newLoc; };
inline U64 PieceMaps::get_attackMap() { return attackMap; }
inline void PieceMaps::set_attackMap(U64 newAttack) { attackMap = newAttack; }