#include <iostream>
#include <string>
#include "board.h"

using namespace std;

int main()
{
    string test = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8";
    Board myGame;
    renderGame(&myGame);
} 

