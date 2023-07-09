#include <string>
#include <SFML/Graphics.hpp>
#include <memory>
#include <iostream>
#include "board.h"
#include "pieceMaps.h"
#include "nonslidingPieceMaps.h"
#include "globals.h"

using namespace std;

void renderGame(Board* myBoard);
void regenAttackMaps(Board* myBoard);
void drawBoard(int cellSize, sf::RenderWindow &window);
void drawHighlights(int cellSize, sf::RenderWindow& window, U64 squares);
void drawPieces(int cellSize, sf::RenderWindow& window, sf::Texture pieceTextures[12], std::unique_ptr<sf::Sprite> tempSprites[64], int draggedSpriteIndex, Board* myBoard);
void renderSprite(sf::RenderWindow& window, unique_ptr<sf::Sprite> tempSprites[65], sf::Texture pieceTextures[12], int row, int col, int cellSize, int textureNum);
void displayToolTips();

void renderGame(Board* myBoard)
{
    int frameRate = 60;
    int windowWidth, windowHeight;
    // make sure this is evenly divisiable by 8 otherwise borders render weirdly
    windowHeight = windowWidth = 800;
    int cellSize = windowHeight / BOARD_SIZE;
    sf::Clock clock;

    // set font
    sf::Font font;
    font.loadFromFile("OpenSans-Bold.ttf");

    // board highlighting toggle flags
    bool attackMapsToggled = false;
    bool devModeToggled = false;
    bool removePieceDevToggled = false;

    // dev mode helper flags
    bool devWhite = true;
    int devIndex = 4;
    int absoluteDevIndex = devIndex;

    int draggedSpriteIndex = -1;
    U64 oldPos = -1;
    PieceMaps* movingPiecesMap = nullptr;

    // load in all the piece files
    sf::Texture pieceTextures[12];
    string pieceNames[12] = {"P", "N", "B", "R", "Q", "K", "p", "n", "b", "r", "q", "k"};
    unique_ptr<sf::Sprite> tempSprites[65];
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

                    // a for attack maps
                    if (event.key.code == sf::Keyboard::A)
                    {
                        attackMapsToggled = !attackMapsToggled;
                        if (attackMapsToggled) cout << "Attack maps highlighting on" << endl << endl;
                        else cout << "Attack maps highlighting off" << endl << endl;
                    }

                    // d for dev mode: add pieces
                    if (event.key.code == sf::Keyboard::D)
                    {
                        devModeToggled = !devModeToggled;
                        if (devModeToggled) cout << "Dev mode on" << endl << endl;
                        else cout << "Dev mode off" << endl << endl;
                    }

                    // if dev mode is toggled allow user to switch piece they are adding
                    if (devModeToggled)
                    {
                        if (event.key.code == sf::Keyboard::B) devWhite = false;
                        if (event.key.code == sf::Keyboard::W) devWhite = true;
                        if (event.key.code == sf::Keyboard::Num1) { devIndex = 0; removePieceDevToggled = false; }
                        if (event.key.code == sf::Keyboard::Num2) { devIndex = 1; removePieceDevToggled = false; }
                        if (event.key.code == sf::Keyboard::Num3) { devIndex = 2; removePieceDevToggled = false; }
                        if (event.key.code == sf::Keyboard::Num4) { devIndex = 3; removePieceDevToggled = false; }
                        if (event.key.code == sf::Keyboard::Num5) { devIndex = 4; removePieceDevToggled = false; }
                        if (event.key.code == sf::Keyboard::Num6) { devIndex = 5; removePieceDevToggled = false; }
                        if (event.key.code == sf::Keyboard::R) removePieceDevToggled = !removePieceDevToggled;

                        absoluteDevIndex = devIndex;
                        if (!devWhite) absoluteDevIndex += 6;
                    }
                }

                // implements drag of drag and drop of the pieces
                // and dev tools
                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
                {
                    auto mouse_pos = sf::Mouse::getPosition(window); // Mouse position relative to the window in pixels
                    auto translated_pos = window.mapPixelToCoords(mouse_pos); // Mouse position translated into world coordinates
                    for (int i = 0; i < 64; i++)
                        if (tempSprites[i] != nullptr && tempSprites[i]->getGlobalBounds().contains(translated_pos)) // Rectangle-contains-point check // Mouse is inside the sprite.
                        {
                            draggedSpriteIndex = i;
                            // decides the pieces old position in order to find which piece is being moved
                            oldPos = 1;
                            oldPos <<= 8 * (draggedSpriteIndex / 8) + (draggedSpriteIndex - ((draggedSpriteIndex / 8) * 8));
                            movingPiecesMap = myBoard->getPieceAtMask(oldPos);
                            if (devModeToggled && removePieceDevToggled)
                            {
                                tempSprites[draggedSpriteIndex] = nullptr;
                                movingPiecesMap->set_pieceLoc(movingPiecesMap->get_pieceLoc() ^ oldPos);
                                draggedSpriteIndex = -1;
                                regenAttackMaps(myBoard);
                            }
                            break;
                        }
                    // if in dev mode and a piece isnt being dragged add the held piece to the board
                    if (devModeToggled && draggedSpriteIndex == -1 && !removePieceDevToggled)
                    {
                        U64 mousePos = 1;
                        mousePos <<= (mouse_pos.x / cellSize) + 8 * (8 - (mouse_pos.y / cellSize) - 1);
                        // add the piece to the board
                        myBoard->getPieceMapAtIndex(absoluteDevIndex)->set_pieceLoc(myBoard->getPieceMapAtIndex(absoluteDevIndex)->get_pieceLoc() | mousePos);
                        // generate attack maps for both sides in case a is toggled
                        regenAttackMaps(myBoard);
                    }
                }

                // implements the drop
                if (draggedSpriteIndex != -1 && event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left)
                {
                    U64 newPos = 1;
                    // To be completly honest, not really sure why the -16 is nessacary but it works so... don't look a gifted horse in the mouth?                   
                    newPos <<= (sf::Mouse::getPosition(window).x / cellSize) + (8 - ((sf::Mouse::getPosition(window).y / cellSize) * 8 )) - 16;
                    
                    int piecesIndex = myBoard->getPieceIndexAtMask(oldPos);
                    myBoard->makeMove(oldPos, newPos, piecesIndex);

                    oldPos = -1;
                    movingPiecesMap = nullptr;
                    draggedSpriteIndex = -1;
                }
            }

            // Draw window every frame
            drawBoard(cellSize, window);
            if (attackMapsToggled && draggedSpriteIndex != -1 && movingPiecesMap != nullptr)
                drawHighlights(cellSize, window, movingPiecesMap->get_attackMap());
            drawPieces(cellSize, window, pieceTextures, tempSprites, draggedSpriteIndex, myBoard);

            // render dev pieces
            if (devModeToggled)
            {
                
                if (removePieceDevToggled)
                {
                    sf::Text text("X", font);
                    text.setCharacterSize(cellSize * 1.2);
                    text.setStyle(sf::Text::Bold);
                    text.setFillColor(sf::Color::Red);
                    // the text aligns weirdly with the mouse which is why the alignment values are odd, they are gathered experimentally
                    text.setPosition(sf::Mouse::getPosition(window).x - (cellSize / 2.5), sf::Mouse::getPosition(window).y - (cellSize / 1.4));
                    window.draw(text);
                }
                else
                { 
                    renderSprite(window, tempSprites, pieceTextures, 0, 64, cellSize, absoluteDevIndex);
                    tempSprites[64]->setPosition(sf::Mouse::getPosition(window).x - (cellSize / 2),
                        sf::Mouse::getPosition(window).y - (cellSize / 2));
                    window.draw(*tempSprites[64]);
                }
            }

            window.display();
            clock.restart();
        }
    }
}

void regenAttackMaps(Board* myBoard)
{
    myBoard->set_isWhitesMove(!myBoard->get_isWhitesMove());
    myBoard->makeAttackMaps();
    myBoard->set_isWhitesMove(!myBoard->get_isWhitesMove());
    myBoard->makeAttackMaps();
}

void drawBoard(int cellSize, sf::RenderWindow &window)
{
    window.clear(sf::Color(118, 150, 86)); // dark squares
    // draw the checkerboard

    for (int row = 0; row < cellSize; row++)
        for (int col = 0; col < cellSize; col++)
        {
            if ((row + col) % 2 == 0)
            {
                sf::RectangleShape rectangle(sf::Vector2f(cellSize, cellSize));
                rectangle.setPosition(cellSize * row, cellSize * col);
                rectangle.setFillColor(sf::Color(238, 238, 210)); // light square
                window.draw(rectangle);
            }
        }
}

void drawHighlights(int cellSize, sf::RenderWindow& window, U64 squares)
{
    // convert U64 location to board location
    for (int row = 7; row >= 0; row--)
        for (int col = 0; col < 8; col++)
        {
            if (squares & 1)
            {
                sf::RectangleShape rectangle(sf::Vector2f(cellSize, cellSize));
                rectangle.setPosition(cellSize * col, cellSize * row);
                if ((row + col) % 2 == 0)
                    rectangle.setFillColor(sf::Color(102, 178, 178)); // light square highlight
                else
                    rectangle.setFillColor(sf::Color(0, 102, 102)); // dark square highlight
                window.draw(rectangle);
            }
            squares >>= 1;
        }
}

void drawPieces(int cellSize, sf::RenderWindow& window, sf::Texture pieceTextures[12], unique_ptr<sf::Sprite> tempSprites[64], int draggedSpriteIndex, Board* myBoard)
{
    U64 mask = 1;
    int maskIndex = -1;

    for(int row = 0; row < BOARD_SIZE; row++)
        for(int col = 0; col < BOARD_SIZE; col++)
        {
            maskIndex = myBoard->getPieceIndexAtMask(mask);
            // if the sprite is being dragged
            if (draggedSpriteIndex != -1 && draggedSpriteIndex == (row * BOARD_SIZE) + col)
                tempSprites[draggedSpriteIndex]->setPosition(sf::Mouse::getPosition(window).x - (cellSize/2), sf::Mouse::getPosition(window).y - (cellSize / 2));
            // otherwise check where all the other pieces are
            else if (maskIndex != -1)
                renderSprite(window, tempSprites, pieceTextures, row, col, cellSize, maskIndex);
            else
                tempSprites[(row * BOARD_SIZE) + col] = nullptr;

            mask <<= 1;
        }

    if (draggedSpriteIndex != -1)
        window.draw(*tempSprites[draggedSpriteIndex]);
}


void renderSprite(sf::RenderWindow& window, unique_ptr<sf::Sprite> tempSprites[65], sf::Texture pieceTextures[12], int row, int col, int cellSize, int textureNum)
{
    int index = (row * BOARD_SIZE) + col;
    tempSprites[index] = make_unique<sf::Sprite>();
    tempSprites[index]->setTexture(pieceTextures[textureNum]);
    tempSprites[index]->setPosition(cellSize * col, cellSize * (7 - row));
    tempSprites[index]->setScale(.003f * cellSize, .003f * cellSize);
    window.draw(*tempSprites[index]);
}

void displayToolTips()
{
    cout << "h: help" << endl;
    cout << "esc: close window" << endl;
    cout << "a: toggle attack maps" << endl;
    cout << "d: click to add a piece using 1,2,...,6 to select type and w/b to select color and r to remove" << endl;

    cout << endl;
}