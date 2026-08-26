#pragma once

class DialogControlDimension
{
public:
    int X = 0;
    int Y = 0;
    int W = 0;
    int H = 0;

    void Reset()
    {
        X = 0;
        Y = 0;
        W = 0;
        H = 0;
    }

    bool Point_Is_Inside_Dimensions(const int x, const int y) const
    {
        return (x >= X && x <= X + W) && (y >= Y && y <= Y + H);
    }
};
