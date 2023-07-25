//
// Created by ethan on 7/24/2023.
//

#ifndef SUPERVISOR_PATH_H
#define SUPERVISOR_PATH_H

#include <deque>
#include "Point.h"

class Path {
    public:
        deque<Point> path;
        void add(Point p);
        bool exists();
        size_t length();
        Direction getStepDir(int step);
};


#endif //SUPERVISOR_PATH_H
