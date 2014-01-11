/*
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */
#include <stdio.h>
#include "caching.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded.
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
  int i, n, m,j,p;
   int temp,nn,mm;
    
    // for M = 32, N = 32, use blockings=8*8 to optimize. 
    if (M == 32 && N == 32)
      {
      for (i = 0; i < 32; i += 8)
	{
	  for (p = 0; p < 32; p += 8)
	    {
		for (n = i; n < i + 8; n++)
		  {	
                    for (m = p; m < p + 8; m++)
		      {
			//skip diagonal for now so we can take advantage
			//of the fact that A B have already had the diagonal part
		        //loaded in cache after the m loop
			        if (n != m)
				  {
				    B[m][n] = A[n][m];

				  }
		      }
            //find the diagonal part and that's when n in [m-8,m]
		    if (n >= (m-8) && n <= m)
		 {
                    B[n][n] = A[n][n];
		 }
		  }
	    }
	}
    }
    //for M=64, N=64, use blockings of 8*8 and then use blockings of 4*4 to optimize
    //for the 8*8 blockings, we want to isolate the diagonal blockings because to take advantage of the diagonal hits
    //further more, for the 4*4 blockings we want to fill in the number in clockwise motion because it will increase loaded data 
    //and reduce miss. 
    //then we handle the diagonal matrixes seperately
    else if (M == 64 && N == 64)
      {
        for (n = 0; n < 64; n +=8)
	  {
            for (m = 0; m < 64; m += 8)
	      {
		//for matrixes not on diagonal 
		if (m != n)
		  {
		    nn = n; 
		    //for the first two 4*4 blockings
                    for ( mm = m; mm <= m + 4; mm += 4)
		      {
                        for (i = nn; i < nn+4; i++)
			  {
			    for (j = mm; j < mm+4; j++)
			      {
				    B[j][i] = A[i][j];
			      }
			  }
		      }
		    //now for the lower two 4*4 blockings, we want to count from right to left
		    //as clockwise counting reduces miss
		    //because the third blocking is already loaded for B, and A is conveniently preloaded too
		    nn = n+4;
		    for (mm= m + 4 ; mm >= m ; mm -= 4)
		      {
			for (i = nn; i < nn+4; i++)
			  {
			    for (j = mm; j < mm+4; j++)
			      {
				B[j][i] = A[i][j];
			      }              
			  }      
		      }
		  }
	      }      
          }
	
	//for the 8*8 blockings at the diagonal
        for (n = 0; n < 64; n += 8) {
	  nn = n;
	  for (p=0; p < 2; p++) {
	    nn = n + p*4;
	    //for the 4*4 blockings at the diagonal
                 for (i = nn; i < nn + 4; i++) {
                     for (j = nn; j < nn + 4; j++) {
                         if (i == j){
			   //leave the diagonal to the end so it's preloaded in B, so to reduce miss
                             temp = A[i][j];
                         }
                         if (i != j){
                             B[j][i] = A[i][j];
                         }
                     }
                     B[i][i] = temp;
                 }
		 //for the 4*4 blockings not at the diagonal
                 for (i = nn; i < nn+4; i++) {
                     if (p == 0) {
                         for (j = nn+4; j < nn+8; j++) {
                             if ((i - nn) == (j - nn - 4)) {
			       //same as above to preload the diagonal
                                 temp = A[i][j];
                             }
                             else
                                 B[j][i] = A[i][j];
                         }
                         B[i+4][i] = temp;
                     }
                     else {
		       //because the 4*4 blocking is divided by the 4*4 blocking that is at the diagonal
		       //so it's a bit messy here, **might be better way of assigning numbers for the 4*4 blockings
                         for (j = nn-4; j < nn; j++) {
                             if ((i - nn)  == (j - nn + 4)) {
			       //the same to handle the diagonal
                                 temp = A[i][j];
                             }
                             else
                                 B[j][i] = A[i][j];
                         }
                         B[i-4][i] = temp;
                     }
                 }
            }
        } 
    }
 


    //for M=61, N=67, we will use 8*8 blockings to optimize
    //as shown before, it's the same mechanism as the 32*32 one. the only difference is that it will
    //break at 61 and 67
    else if (M == 61 && N == 67)
      {
	for (j = 0; j < 61; j += 8)
	  {
	    for (i = 0; i < 67; i += 8)
	      {
		for (n = i; n < i + 8; n++)
		  {
                    if (n > 66) break;	
                    for (m = j; m < j + 8; m++)
		      {
 			if (m > 60) break;
 			B[m][n] = A[n][m];
		      }
		  }
	      }
	  }
      }
}

/*
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started.
 */

/*
 * trans - A simple baseline transpose function, not optimized for the cache.
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
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc);

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc);

}

/*
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
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

