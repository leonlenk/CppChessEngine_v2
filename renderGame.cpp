#include <string>
#include <SFML/Graphics.hpp>
#include <memory>
#include <iostream>
#include "board.h"
#include "pieceMaps.h"
#include "pawnMap.h"
#include "knightMap.h"
#include "kingMap.h"
#include "globals.h"

using namespace std;

void drawBoard(int cellSize, sf::RenderWindow &window);
void drawHighlights(U64 sqaures);
void renderSprite(sf::RenderWindow& window, unique_ptr<sf::Sprite> tempSprites[64], sf::Texture pieceTextures[12], int row, int col, int cellSize, int textureNum);
void displayToolTips();

void Board::renderGame()
{
    int frameRate = 30;
    int windowWidth, windowHeight;
    // make sure this is evenly divisiable by 8 so that the board properly renders in the window
    windowHeight = windowWidth = 800;
    int cellSize = windowHeight / 8;
    sf::Clock clock;

    // board highlighting toggled flags
    bool attackMapsToggled = false;

    bool isDragging = false;
    int draggedSpriteIndex = -1;

    // load in all the piece files
    sf::Texture pieceTextures[12];
    string pieceNames[12] = {"P", "N", "B", "R", "Q", "K", "p", "n", "b", "r", "q", "k"};
    unique_ptr<sf::Sprite> tempSprites[64];
    for (int i = 0; i < 64; i++)
        tempSprites[i] = nullptr;

    for (int i = 0; i < 6; i++)
        pieceTextures[i].loadFromFile("whitePieces/" + pieceNames[i] + ".png");
    for (int i = 6; i < 12; i++)
        pieceTextures[i].loadFromFile("blackPieces/" + pieceNames[i] + ".png");
    for (int i = 0; i < 12; i++)
        pieceTextures[i].setSmooth(true);

    // opens the window
    sf::RenderWindow window(sf::VideoMode(windowWidth, windowWidth), "Chess Board");
    displayToolTips();

    while (window.isOpen())
    {
        // for each frame
        if (clock.getElapsedTime().asMilliseconds() % (1000 / frameRate) == 0)
        {
            // event handler
            sf::Event event;
            while (window.pollEvent(event))
            {
                // close window
                if (event.type == sf::Event::Closed)
                    window.close();

                // handles all key presses
                if (event.type == sf::Event::KeyPressed)
                {
                    // escape closes the window
                    if (event.key.code == sf::Keyboard::Escape)
                        window.close();

                    // h is for help
                    if (event.key.code == sf::Keyboard::H)
                        displayToolTips();

                    if (event.key.code == sf::Keyboard::A)
                        attackMapsToggled = !attackMapsToggled;
                }

                // implements drag of drag and drop of the pieces
                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
                {
                    auto mouse_pos = sf::Mouse::getPosition(window); // Mouse position relative to the window
                    auto translated_pos = window.mapPixelToCoords(mouse_pos); // Mouse position translated into world coordinates
                    for (int i = 0; i < 64; i++)
                        if (tempSprites[i] != nullptr && tempSprites[i]->getGlobalBounds().contains(translated_pos)) // Rectangle-contains-point check // Mouse is inside the sprite.
                        {
                            draggedSpriteIndex = i;
                            isDragging = true;
                        }
                }

                // implements the drop
                if (isDragging && event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left)
                {
                    U64 movedPiece = 1;
                    // To be completly honest, not really sure why the -15 is nessacary but it works so... don't look a gifted horse in the mouth?
                    movedPiece <<= (sf::Mouse::getPosition(window).x / cellSize) + (7 - ((sf::Mouse::getPosition(window).y / cellSize) * 8)) - 15;                    
                    
                    // decides the pieces old position in order to find which piece is being moved
                    U64 oldPos = 1; 
                    oldPos <<=  8 * (draggedSpriteIndex / 8) + (draggedSpriteIndex - ((draggedSpriteIndex / 8) * 8));
                    
                    PieceMaps* movingPiecesMap = getPieceAtMask(oldPos);
                    movingPiecesMap->set_pieceLoc((movingPiecesMap->get_pieceLoc() ^ oldPos) | movedPiece);

                    draggedSpriteIndex = -1;
                    isDragging = false;
                }
            }

            // window drawer
            drawBoard(cellSize, window);
            drawPieces(cellSize, window, pieceTextures, tempSprites, draggedSpriteIndex);
            window.display();

            clock.restart();
        }
    }
}

void drawBoard(int cellSize, sf::RenderWindow &window)
{
    window.clear(sf::Color(118, 150, 86));
    // draw the checkerboard

    for (int row = 0; row < cellSize; row++)
        for (int col = 0; col < cellSize; col++)
        {
            if ((row + col) % 2 == 0)
            {
                sf::RectangleShape rectangle(sf::Vector2f(cellSize, cellSize));
                rectangle.setPosition(cellSize * row, cellSize * col);
                rectangle.setFillColor(sf::Color(238, 238, 210));
                window.draw(rectangle);
            }
        }
}

void drawHighlights(U64 squares)
{
    for (int row = 0; row < 8; row++)
        for (int col = 0; col < 8; col++)
        {
            squares >>= 1;
        }
}

void Board::drawPieces(int cellSize, sf::RenderWindow& window, sf::Texture pieceTextures[12], unique_ptr<sf::Sprite> tempSprites[64], int draggedSpriteIndex)
{
    U64 mask = 1;

    for(int row = 0; row < 8; row++)
        for(int col = 0; col < 8; col++)
        {
            // if the sprite is being dragged
            if (draggedSpriteIndex != -1 && draggedSpriteIndex == (row * 8) + col)
                tempSprites[draggedSpriteIndex]->setPosition(sf::Mouse::getPosition(window).x - (cellSize/2), sf::Mouse::getPosition(window).y - (cellSize / 2));
            // otherwise check where all the other pieces are
            else if (mask & (wPawns->get_pieceLoc()))
                renderSprite(window, tempSprites, pieceTextures, row, col, cellSize, 0);
            else if (mask & (bPawns->get_pieceLoc()))
                renderSprite(window, tempSprites, pieceTextures, row, col, cellSize, 6);
            else if (mask & (wKnights->get_pieceLoc()))
                renderSprite(window, tempSprites, pieceTextures, row, col, cellSize, 1);
            else if (mask & (bKnights->get_pieceLoc()))
                renderSprite(window, tempSprites, pieceTextures, row, col, cellSize, 7);
            else if (mask & (wKing->get_pieceLoc()))
                renderSprite(window, tempSprites, pieceTextures, row, col, cellSize, 5);
            else if (mask & (bKing->get_pieceLoc()))
                renderSprite(window, tempSprites, pieceTextures, row, col, cellSize, 11);
            else
                tempSprites[(row * 8) + col] = nullptr;

            mask <<= 1;
        }

    if (draggedSpriteIndex != -1)
        window.draw(*tempSprites[draggedSpriteIndex]);
}


void renderSprite(sf::RenderWindow& window, unique_ptr<sf::Sprite> tempSprites[64], sf::Texture pieceTextures[12], int row, int col, int cellSize, int textureNum)
{
    int index = (row * 8) + col;
    tempSprites[index] = make_unique<sf::Sprite>();
    tempSprites[index]->setTexture(pieceTextures[textureNum]);
    tempSprites[index]->setPosition(cellSize * col, cellSize * (7 - row));
    tempSprites[index]->setScale(.003f * cellSize, .003f * cellSize);
    window.draw(*tempSprites[index]);
}

void displayToolTips()
{
    cout << endl << "h: help" << endl;
    cout << "esc: close window" << endl;
    cout << "a: toggle attack maps" < endl;

}