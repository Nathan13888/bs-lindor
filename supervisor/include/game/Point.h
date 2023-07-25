//
// Created by ethan on 7/24/2023.
//

#ifndef SUPERVISOR_POINT_H
#define SUPERVISOR_POINT_H

#include "defs.h"

class Point {
    public:
        int x;
        int y;
        Point();
        Point(int x, int y);
        Point addMove(Direction dir);
        bool operator ==(const Point& p) const;
        bool operator !=(const Point& p) const;
        bool operator < (const Point& p)  const;
};

namespace std {
    template <>
    class hash<Point> {
        public:
            size_t operator()(const Point& p) const {
                return hash<int>()(p.x) ^ hash<int>()(p.y);
            }
    };
};


#endif //SUPERVISOR_POINT_H
