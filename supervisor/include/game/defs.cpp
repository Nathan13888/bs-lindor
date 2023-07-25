//
// Created by ethan on 7/24/2023.
//
#include "defs.h"

ostream& operator<<(std::ostream& o, Direction c) {
    cout << static_cast<int>(c);
    return o;
}