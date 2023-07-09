#pragma once

#include <string>

// 64 bit unsigned int
typedef unsigned long long U64;

int const NUM_PIECES = 12;
int const BOARD_SIZE = 8;

// initial board set up FEN string
std::string const START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

// mask of ones without A file (col 1)
U64 const notAFile = 0x7f7f7f7f7f7f7f7f;
// mask of ones without H file (col 8)
U64 const notHFile = 0xfefefefefefefefe;
// mask without A or B file (col 1 and 2)
U64 const notABFile = 0x3f3f3f3f3f3f3f3f;
// mask without H or G file (col 7 and 8)
U64 const notGHFile = 0xfcfcfcfcfcfcfcfc;
// mask without A, B, or C file (col 1, 2, and 3)
U64 const notABCFile = 0x1f1f1f1f1f1f1f1f;
// mask without F, G, or H files (col 6, 7, and 8)
U64 const notFGHFile = 0xf8f8f8f8f8f8f8f8;


// masks for only the files
U64 const Afile = 0x0101010101010101;
U64 const Bfile = 0x0202020202020202;
U64 const Cfile = 0x0404040404040404;
U64 const Dfile = 0x0808080808080808;
U64 const Efile = 0x1010101010101010;
U64 const Ffile = 0x2020202020202020;
U64 const Gfile = 0x4040404040404040;
U64 const Hfile = 0x8080808080808080;

U64 const fullRank = 0x00000000000000ff;
