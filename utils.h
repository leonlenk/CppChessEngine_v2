#pragma once
#include <string>
#include "globals.h"

// bit maps to algebraic notation
std::string bits2Alg(U64 loc);

// algebraic noation to bit maps
U64 alg2Bits(std::string alg);