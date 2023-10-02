#include <iostream>
#include <vector>
#include <cstdlib>
#include <omp.h>
#include <mpi.h>
#include <bits/stdc++.h>

#ifndef MTX_OPS
    #define MTX_OPS 1
    #include "mtx_ops.cpp"
#endif

#define ENDTAG 0
#define IDLETAG 1
#define FUNCTAG 2

#define MV 3
#define VV 4
#define SUB 6
#define ADD 7


void helpProccess(int helpDest, CSR_Matrix A, Vector b, int me, int size) {
    int func = -1;
    int helpSize = 0;
    double dotProd = 0;
    int begin = 0;
    int end = 0;
    int savedVec = 0;

    MPI_Status status;
    MPI_Request sendIdleReq;
    
    Vector auxBuf(0);
    Vector auxBuf2(0);

    MPI_Recv(&func, 1, MPI_INT, helpDest, FUNCTAG, MPI_COMM_WORLD, &status);
    MPI_Recv(&helpSize, 1, MPI_INT, helpDest, FUNCTAG, MPI_COMM_WORLD, &status);

    if(func == VV || func == SUB || func == ADD){
        auxBuf.resize(helpSize);
        auxBuf2.resize(helpSize);
        MPI_Recv(&auxBuf.values[0], helpSize, MPI_DOUBLE, helpDest, FUNCTAG, MPI_COMM_WORLD, &status);
        MPI_Recv(&auxBuf2.values[0], helpSize, MPI_DOUBLE, helpDest, FUNCTAG, MPI_COMM_WORLD, &status);
    }
    else if(func == MV){
        auxBuf.resize(size);
        MPI_Recv(&auxBuf.values[0], size, MPI_DOUBLE, helpDest, FUNCTAG, MPI_COMM_WORLD, &status);  
        MPI_Recv(&begin, 1, MPI_DOUBLE, helpDest, FUNCTAG, MPI_COMM_WORLD, &status);
        MPI_Recv(&end, 1, MPI_DOUBLE, helpDest, FUNCTAG, MPI_COMM_WORLD, &status);
    }
    
    switch(func){
        case MV:
            
            auxBuf2 = sparseMatrixVector(A, auxBuf, begin, end, size);

            MPI_Send(&auxBuf2.values[0], helpSize, MPI_DOUBLE, helpDest, MV, MPI_COMM_WORLD);
            break;
        case VV:
            dotProd = dotProduct(auxBuf, auxBuf2, 0, helpSize);

            MPI_Send(&dotProd, 1, MPI_DOUBLE, helpDest, VV, MPI_COMM_WORLD);
            break;
        case SUB:
            auxBuf = subtractVec(auxBuf, auxBuf2, 0, helpSize);
            
            MPI_Send(&auxBuf.values[0], helpSize, MPI_DOUBLE, helpDest, SUB, MPI_COMM_WORLD);
            break;
        case ADD:
        
            auxBuf = addVec(auxBuf, auxBuf2, 0, helpSize);
            
            MPI_Send(&auxBuf.values[0], helpSize, MPI_DOUBLE, helpDest, ADD, MPI_COMM_WORLD);
            break;
        default: 
            cout << "Proccess number: " << me << " Received wrong function tag from node " << helpDest << endl;
            break;
    }
}