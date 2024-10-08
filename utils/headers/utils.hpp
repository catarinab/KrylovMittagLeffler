#ifndef UTILS_HPP
#define UTILS_HPP

#define ROOT 0

#define ENDTAG (-1)

#define MV 2
#define VV 3
#define ADD 4
#define NORM 5

#define EPS16 2.220446049250313e-16
#define EPS52 1e-52

#define PI 3.141592653589793238466

#include <complex>

#include "dense_matrix.hpp"
#include "dense_vector.hpp"

struct SparseTriplet {
    long long int col;
    long long int row;
    double value;
    SparseTriplet(long long int row, long long int col, double value) : col(col), row(row), value(value) {}
    SparseTriplet() : col(0), row(0), value(0) {}
};

inline bool operator<(const SparseTriplet& a, const SparseTriplet& b) {
    if(a.row < b.row || (a.row == b.row && a.col < b.col))
        return true;
    return false;
}

inline double factorial(int n) {
    double result = 1;
    for(int i = 1; i <= n; i++) {
        result *= i;
    }
    return result;
}

inline double falling_factorial(double n, int k) {
    double result = 1;
    for(int i = 0; i < k ; i++) {
        result *= (n - i);
    }
    return result;
}


#endif // UTILS_HPP