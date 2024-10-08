#ifndef ARNOLDI_ITERATION_SHARED_HPP
#define ARNOLDI_ITERATION_SHARED_HPP

#include "csr_matrix.hpp"
#include "dense_matrix.hpp"
#include "dense_vector.hpp"


int arnoldiIteration(const csr_matrix& A, dense_vector& initVec, int k_total, int m, dense_matrix * V,
                     dense_matrix * H);

#endif // ARNOLDI_ITERATION_SHARED_HPP