#include "pieceMaps.h"
#include "knightMap.h"
#include "kingMap.h"

void KnightMap::updateAttackMap() 
{ 
	// adds horizontal moves (accounting for overflow) and then reflects them across the y-axis
	U64 h1 = ((get_pieceLoc() >> 1) & notAFile) | ((get_pieceLoc() << 1) & notHFile); // up and down 1
	U64 h2 = ((get_pieceLoc() >> 2) & notABFile) | ((get_pieceLoc() << 2) & notGHFile); // up and down 2
	set_attackMap((h1 << 16) | (h1 >> 16) | (h2 << 8) | (h2 >> 8)); 
}

void KingMap::updateAttackMap()
{
	U64 attacks = (get_pieceLoc() & notAFile) << 1 | (get_pieceLoc() & notHFile) >> 1 | get_pieceLoc(); // west one and east one
	// pushes the three bits in a row north and south then excludes the center
	set_attackMap((attacks ^ get_pieceLoc()) | attacks << 8 | attacks >> 8);
}