#pragma once
#include <string>
#include <memory>
#include <SFML/Graphics.hpp>
#include "globals.h"

class WPawnMap;
class BPawnMap;
class KnightMap;
class KingMap;
class PieceMaps;

class Board
{
public:
	Board();
	Board(std::string startingFEN);
	~Board();

	// gets the piece at the location of the mask
	PieceMaps* getPieceAtMask(const U64 mask);

	// draws the board
	void renderGame();
	// renderer helper function
	void drawPieces(int cellSize, sf::RenderWindow& window, sf::Texture pieceTextures[12], std::unique_ptr<sf::Sprite> tempSprites[64], int draggedSpriteIndex);

private:
	WPawnMap* wPawns;
	BPawnMap* bPawns;
	KnightMap* wKnights;
	KnightMap* bKnights;
	KingMap* wKing;
	KingMap* bKing;
};
