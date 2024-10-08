#ifndef IO_OPS_HPP
#define IO_OPS_HPP

#include <string>
#include <vector>
#include "utils.hpp"
using namespace std;

pair<double, double> readHeader(string& inputFile);
vector<vector<SparseTriplet>> readFileFullMtx(const string& inputFile, long long int *rows, long long int *cols, long long int *nz);
vector<vector<SparseTriplet>> readFilePartialMtx(const string& inputFile, long long int * rows, long long int * cols, long long int * nz,
                                                 const int * displs, const int * counts, int me);
#endif // IO_OPS_HPP