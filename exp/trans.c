/* 
 * trans.c - 矩阵转置B=A^T
 *每个转置函数都必须具有以下形式的原型：
 *void trans（int M，int N，int a[N][M]，int B[M][N]）；
 *通过计算，块大小为32字节的1KB直接映射缓存上的未命中数来计算转置函数。
 */ 
#include <stdio.h>
#include "cachelab.h"
int is_transpose(int M, int N, int A[N][M], int B[M][N]);
char transpose_submit_desc[] = "Transpose submission";  //请不要修改“Transpose_submission”


void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    int row,col,i,j;
    int a0,a1,a2,a3,a4,a5,a6,a7;
    if(N==32&&M==32){
        for(row=0;row<N;row=row+8){
            for(col=0;col<M;col=col+8){  
                for(i=row;i<row+8&&i<N;i++){
                    if(row==col){ 
                        a0 =A[i][col];
                        a1 =A[i][col+1];
                        a2 =A[i][col+2];
                        a3 =A[i][col+3];
                        a4 =A[i][col+4];
                        a5 =A[i][col+5];
                        a6 =A[i][col+6];
                        a7 =A[i][col+7];
                        
                        B[col][i] =a0;
                        B[col+1][i] =a1;
                        B[col+2][i] =a2;
                        B[col+3][i] =a3;
                        B[col+4][i] =a4;
                        B[col+5][i] =a5;
                        B[col+6][i] =a6;
                        B[col+7][i] =a7;
                    }
                    else{
                        for(j=col;j<col+8&&j<M;j++){
                            B[j][i] = A[i][j];
                        }
                    }
                    
                }
            }   
        }
    }
    if(N==64&&M==64){
        for(row=0;row<N;row=row+8){
            for(col=0;col<M;col=col+8){
                for(i=row;i<row+4;i++){
                    a0 = A[i][col];
                    a1 = A[i][col+1];
                    a2 = A[i][col+2];
                    a3 = A[i][col+3];
                    a4 = A[i][col+4];
                    a5 = A[i][col+5];
                    a6 = A[i][col+6];
                    a7 = A[i][col+7];

                    B[col][i] = a0;
                    B[col+1][i] = a1;
                    B[col+2][i] = a2;
                    B[col+3][i] = a3;
                    B[col][i+4] = a4;
                    B[col+1][i+4] = a5;
                    B[col+2][i+4] = a6;
                    B[col+3][i+4] = a7;
                }
                for(j=col;j<col+4;j++){
                    a0 = A[row+4][j];
                    a1 = A[row+5][j];
                    a2 = A[row+6][j];
                    a3 = A[row+7][j];

                    a4 = B[j][row+4];
                    a5 = B[j][row+5];
                    a6 = B[j][row+6];
                    a7 = B[j][row+7];

                    B[j][row+4] = a0;
                    B[j][row+5] = a1;
                    B[j][row+6] = a2;
                    B[j][row+7] = a3;
                
                    B[j+4][row] = a4;
                    B[j+4][row+1] = a5;
                    B[j+4][row+2] = a6;
                    B[j+4][row+3] = a7;
                }
                for(i=row+4;i<row+8;i++){
                    a4 = A[i][col+4];
                    a5 = A[i][col+5];
                    a6 = A[i][col+6];
                    a7 = A[i][col+7];

                    B[col+4][i] = a4;
                    B[col+5][i] = a5;
                    B[col+6][i] = a6;
                    B[col+7][i] = a7;
                }
            }
        }
    }
    if(N==67&&M==61){
        for(row=0;row<N;row=row+17){
            for(col=0;col<M;col=col+17){
                for(i=row;i<row+17&&i<N;i++){
                    for(j=col;j<col+17&&j<M;j++){
                        B[j][i]=A[i][j];
                    }
                }
            }
        }
    }
}

/* 
 * 我们在下面定义了一个简单的方法来帮助您开始，您可以根据下面的例子把上面值置补充完整。
 */ 

/* 
 * 简单的基线转置功能，未针对缓存进行优化。
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions-此函数向驱动程序注册转置函数。
 *在运行时，驱动程序将评估每个注册的函数并总结它们的性能。这是一种试验不同转置策略的简便方法。
 */
void registerFunctions()
{
    /* 注册解决方案函数  */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* 注册任何附加转置函数 */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - 函数检查B是否是A的转置。在从转置函数返回之前，可以通过调用它来检查转置的正确性。
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}
