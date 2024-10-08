#include <vector>

#include "headers/mtx_ops_mkl.hpp"
#include "headers/scaling_and_squaring.hpp"


using namespace std;

//find suitable m value for pade approximation
int findM(double norm, double theta , int * power) {
    int m = 1; //2^0
    *power = 0; //2^0

    while(norm / m >= theta) {
        m *= 2;
        (*power)++;
    }

    return m;
}

//get pade coefficients for m = 3, 5, 7, 9, 13
vector<double> get_pade_coefficients(int m) {
    vector<double> coeff;
    if(m == 3)
        coeff = {120, 60, 12, 1};
    else if(m == 5)
        coeff = {30240, 15120, 3360, 420, 30, 1};
    else if(m == 7)
        coeff = {17297280, 8648640, 1995840, 277200, 25200, 1512, 56, 1};
    else if (m == 9)
        coeff = {17643225600, 8821612800, 2075673600, 302702400, 30270240,
                2162160, 110880, 3960, 90, 1};
    else if (m == 13)
        coeff = {6.476475253248e16, 3.238237626624e16, 7771770303897600,
                1187353796428800,  129060195264000,   10559470521600,
                670442572800,      33522128640,       1323241920,
                40840800,          960960,            16380,  182,  1};
    return coeff;
}

//calculate all necessary parameters for pade approximation
dense_matrix definePadeParams(vector<dense_matrix> * powers, int * m, int * power, dense_matrix A) {
    vector<double> theta = {
    1.495585217958292e-002, // m_val = 3
    2.539398330063230e-001,  // m_val = 5
    9.504178996162932e-001,  // m_val = 7
    2.097847961257068e+000,  // m_val = 9
    5.371920351148152e+000}; // m_val = 13

    dense_matrix finalMtx = A;

    *m = 0;

    dense_matrix identity = dense_matrix(A.getColVal(), A.getRowVal());
    identity.setIdentity();

    double normA = A.getNorm2();

    if(normA < theta[0])
        *m = 3;
    else if(normA < theta[1])
        *m = 5;
    else if(normA < theta[2])
        *m = 7;
    else if(normA < theta[3])
        *m = 9;
    else if(normA < theta[4])
        *m = 13;

    if(*m == 0) {
        int s = findM(normA, theta[4], power);
        *m = 13;
        finalMtx = A / (s);
    }

    auto powIter = powers->begin();

    *powIter++ = identity;
    *powIter++ = finalMtx;
    dense_matrix A_2 = denseMatrixMult(finalMtx, finalMtx);
    *powIter++ = A_2;
    if(*m == 3)
        return finalMtx;
    dense_matrix A_4 = denseMatrixMult(A_2, A_2);
    *++powIter = A_4;
    if(*m == 5)
        return finalMtx;
    *(powIter + 2) = denseMatrixMult(A_2, A_4);
    if(*m == 7 || *m == 13)
        return finalMtx;
    *(powIter + 4) = denseMatrixMult(A_4, A_4);

    return finalMtx;

}

//Calculate the Pade approximation of the exponential of matrix H.
dense_matrix scalingAndSquaring(const dense_matrix& inputH) {
    vector<dense_matrix> powers(9);
    int s = 0, m = 0;
    dense_matrix H = definePadeParams(&powers, &m, &s, inputH);

    dense_matrix identity = dense_matrix(H.getColVal(), H.getRowVal());
    identity.setIdentity();

    dense_matrix U;
    dense_matrix V;

    vector<double> coeff = get_pade_coefficients(m);


    if(m!= 13) {
        U = identity * coeff[1];
        V = identity * coeff[0];

        for(int j = m; j >= 3; j-=2) {
            U = denseMatrixAdd(U, powers[j-1] * coeff[j]);
            V = denseMatrixAdd(V, powers[j-1] * coeff[j-1]);
        }
    }

    if(m == 13){
        dense_matrix op1 = denseMatrixAdd(powers[6]*coeff[7], powers[4]*coeff[5]);
        dense_matrix op2 = denseMatrixAdd(powers[2]*coeff[3], identity*coeff[1]);
        dense_matrix sum1 = denseMatrixAdd(op1, op2);
        op1 = denseMatrixAdd(powers[6]*coeff[13], powers[4]*coeff[11]);
        op2 = denseMatrixAdd(op1, powers[2]*coeff[9]);

        dense_matrix sum2 = denseMatrixMult(powers[6], op2);

        U = denseMatrixAdd(sum1, sum2);

        op1 = denseMatrixAdd(powers[6]*coeff[6], powers[4]*coeff[4]);
        op2 = denseMatrixAdd(powers[2]*coeff[2], identity*coeff[0]);
        sum1 = denseMatrixAdd(op1, op2);
        op1 = denseMatrixAdd(powers[6]*coeff[12], powers[4]*coeff[10]);
        op2 = denseMatrixAdd(op1, powers[2]*coeff[8]);

        sum2 = denseMatrixMult(powers[6], op2);
        
        V = denseMatrixAdd(sum1, sum2);

    }


    U = denseMatrixMult(H, U);

    dense_matrix num1 = denseMatrixSub(V, U);
    dense_matrix num2 = denseMatrixAdd(V, U);

    dense_matrix res = solveEq(num1, num2);

    if(s != 0)
        for (int i = 0; i < s; i++)
            res = denseMatrixMult(res, res);

    return res;
}