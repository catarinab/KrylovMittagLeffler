#define ROOT 0

#define ENDTAG -1
#define IDLETAG 1
#define FUNCTAG 2

#define MV 3
#define VV 4
#define SUB 6
#define ADD 7

struct SparseTriplet {
    int col;
    int row;
    double value;
    SparseTriplet(int row, int col, double value) : row(row), col(col), value(value) {}
    SparseTriplet() : row(0), col(0), value(0) {}
};

bool operator<(const SparseTriplet& a, const SparseTriplet& b) {
    if(a.row < b.row)
        return true;
    else if(a.row == b.row && a.col < b.col)
        return true;
    else
        return false;
}