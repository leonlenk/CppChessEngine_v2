# CppChessEngine_v2

## Introduction
I had played chess on and off for the past several years (I was even once the president of my chess team) and I've always had a fondness for the game so when I learned to program I of course wanted to make a chess bot. However instead of instantly leaping into making a bot (with deep learning) I wanted to understand the framework I would be building on, so I made my own engine.

## How To Run It/ How To Play
All you need to do is run make in your directory, this will generate the object files from the cpp files into a directory called build, the graphics library (SFML) has it's object files and headers in the SFML-2.6.1 directory. Then run ./chessEngine and the graphical display will show up. 
Run these commands:
```
make
./chessEngine
```
The consol will also show short cuts and you can display the short cuts by hitting h form within the window that poped-up. To play you drag and drop the pieces, legal moves for the pieces will be highlighted, but only legal moves made by the player who's turn it is will be registered. Additionally every move that is made will make the terminal display the algebraic notation for that move.

## Implementation details
If you look through my github you might notice that there is another chess engine implemented in python with pytorch tensors representing the board. Needless to say this was really *really* slow. So after I took my first course in C++ I decided to migrate it over and build my new engine with speed in mind. I also tried to make it a bit cleaner by making it object oriented. Here are a list of optimizations I made:
- In the backend all pieces are represented by bitmaps, that is all pawns are ones in a 64 bit (unsigned long long) int
- When checking where sliding pieces can move I use the o^(o-2r) method
- Because that method requires reversing the bits in a 64 bit int I created a function that split it into 8 8 bit strings and used a look up table to reverse those 8 bit strings and stacked them in reverse order (this actually netted me a 11x speed up over looping over all the bits in the 64 bit word)
- Instead of making and unmaking moves to calculate pinned pieces and check I instead make my king move like all the pieces and check if it runs into any opposing pieces for a large speed up

## Going forward
My next step pretaining to this engine is a little more optimiztion (stuff like adding parallelism), cleaning up the code, and determining the optimization flags I will use. For the project as a whole I'm going to try to put my engine on my website and then write a deep learning powered bot that plays on my engine.

## Preview
I included several features into the graphical display, one of which was the ability to use hotkeys to display all the squares white attacked (to help with development) so the image below has that toggled to show the board highlighting and the board itself.
![image of chess board showing white's attacks highlighted](https://github.com/leonlenk/CppChessEngine_v2/blob/master/github_assets/highlight_preveiw.png)
