//
// Created by ethan on 7/24/2023.
//
#include "defs.h"

std::ostream& operator<<(std::ostream& o, Direction c) {
    std::cout << static_cast<int>(c);
    return o;
}