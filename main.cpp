#include <iostream>
#include <string>
#include "board.h"

using namespace std;

int main()
{
    string test = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8";
    Board myGame(test);
    //myGame.makeMove("Nb1d2");
    //myGame.makeMove("d8e8");
    //renderGame(&myGame); // Pd7xqe8=r+
    //myGame.runSplitPERFT(1);
    //cout << myGame.runPERFT(2) << " should be 400" << endl;
    //cout << myGame.runPERFT(3) << " should be 8,902" << endl;
    myGame.runSplitPERFT(4); //	2,103,487 
    //cout << myGame.runPERFT(5) << " should be 4,865,609" << endl;
    //cout << myGame.runPERFT(6) << " should be 119,060,324" << endl;
} 

