#include "headers/arnoldi_iteration_shared_nu.hpp"
#include <string.h>

/*  Parameters
    ----------
    A : An m × m array (csr_matrix)
    b : Initial mx1 vector (dense_vector)
    n : Degree of the Krylov space (int)
    m : Dimension of the matrix (int)

    Returns
    -------
    V : An m x n array (dense_matrix), where the columns are an orthonormal basis of the Krylov subspace.
    H : An n x n array (dense_matrix). A on basis V. It is upper Hessenberg.
*/
int arnoldiIteration(const csr_matrix &A, dense_vector &initVec, int k_total, int m, dense_matrix *V,
                     dense_matrix *H, int nu) {

    int stat = mkl_sparse_set_mv_hint(A.getMKLSparseMatrix(),SPARSE_OPERATION_NON_TRANSPOSE,A.getMKLDescription(),
                                      k_total*nu);

    if (stat != SPARSE_STATUS_SUCCESS) {
        cerr << "Error in mkl_sparse_set_mv_hint" << endl;
        return 1;
    }

    stat = mkl_sparse_optimize(A.getMKLSparseMatrix());

    if (stat != SPARSE_STATUS_SUCCESS) {
        cerr << "Error in mkl_sparse_optimize" << endl;
        return 1;
    }

    V->setCol(0, initVec);

    int k;

    //auxiliary
    dense_vector w(m);
    dense_vector temp(m);
    auto * dotProd = new double[k_total + 1]();
    double *vCol;

    for(k = 1; k < k_total + 1; k++) {
        double tempNorm = 0;
        V->getCol(k-1, &temp);
        memset(dotProd, 0, k * sizeof(double));
        for(int mult = 0; mult < nu; mult++) {
            mkl_sparse_d_mv(SPARSE_OPERATION_NON_TRANSPOSE, 1.0, A.getMKLSparseMatrix(), A.getMKLDescription(),
                            temp.values.data(), 0.0, w.values.data());
            if(mult != nu - 1)
                memcpy(temp.values.data(), w.values.data(), m * sizeof(double));
        }



        #pragma omp parallel shared(dotProd) private(vCol)
        {
            for(int j = 0; j < k; j++) {
                //dotprod entre w e V->getCol(j)
                V->getCol(j, &vCol);

                #pragma omp for reduction(+:dotProd[j:j+1])
                for (int i = 0; i < m; i++) {
                    dotProd[j] += (w.values[i] * vCol[i]);
                }

                #pragma omp for simd nowait
                for(int i = 0; i < m; i++) {
                    w.values[i] = w.values[i] - vCol[i] * dotProd[j];
                }
            }

            if(k < k_total) {
                #pragma omp for reduction(+:tempNorm)
                for(int i = 0; i < m; i++){
                    tempNorm += w.values[i] * w.values[i];
                }
                double wNorm = sqrt(tempNorm);
                if(wNorm != 0) {
                    //V(:, k) = w / wNorm
                    V->getCol(k, &vCol);
                    #pragma omp for nowait
                    for (int i = 0; i < m; i++) {
                        vCol[i] = w.values[i] / wNorm;
                    }
                    //H(k, k-1) = wNorm
                }
            }
        }

        H->setValue(k, k - 1, sqrt(tempNorm));

        //H(:, k-1) = dotProds
        for (int i = 0; i < k; i++) {
            H->setValue(i, k - 1, dotProd[i]);
        }
    }
    return k;
}