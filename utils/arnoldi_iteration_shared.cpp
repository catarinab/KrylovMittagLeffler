#include "headers/arnoldi_iteration_shared.hpp"

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
int arnoldiIteration(const csr_matrix& A, dense_vector& initVec, int k_total, int m, dense_matrix * V,
                     dense_matrix * H) {

    int stat = mkl_sparse_set_mv_hint(A.getMKLSparseMatrix(), SPARSE_OPERATION_NON_TRANSPOSE, A.getMKLDescription(),
                                      k_total);

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
    auto *w = (double *) malloc(m * sizeof(double));
    double *vCol;
    double dotProd;
    double tempNorm;

    for (k = 1; k < k_total + 1; k++) {
        tempNorm = 0;
        dotProd = 0;
        V->getCol(k - 1, &vCol);
        mkl_sparse_d_mv(SPARSE_OPERATION_NON_TRANSPOSE, 1.0, A.getMKLSparseMatrix(), A.getMKLDescription(),
                        vCol, 0.0, w);


        #pragma omp parallel shared(V, H, w, dotProd, tempNorm) private(vCol)
        {
            for (int j = 0; j < k; j++) {
                V->getCol(j, &vCol);

                //dotprod between w and V->getCol(j)
                #pragma omp for reduction(+:dotProd)
                for (int i = 0; i < m; i++) {
                    dotProd += (w[i] * vCol[i]);
                }

                #pragma omp for
                for (int i = 0; i < m; i++) {
                    w[i] -= vCol[i] * dotProd;
                }

                #pragma omp single
                {
                    H->setValue(j, k - 1, dotProd);
                    dotProd = 0;
                };
            }

            if (k < k_total) {
                //calculate ||w||
                #pragma omp for reduction(+:tempNorm)
                for (int i = 0; i < m; i++) {
                    tempNorm += w[i] * w[i];
                }
                double wNorm = sqrt(tempNorm);

                //only change V and H if ||w|| != 0
                if (wNorm != 0) {
                    //V(:, k) = w / ||w||
                    V->getCol(k, &vCol);
                    #pragma omp for nowait
                    for (int i = 0; i < m; i++) {
                        vCol[i] = w[i] / wNorm;
                    }

                    //H(k, k-1) = wNorm
                    #pragma omp single
                    H->setValue(k, k - 1, wNorm);
                }
            }
        }
    }

    free(w);

    return k;
}