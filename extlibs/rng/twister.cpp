/*
 * twister.cpp  Mex wrapper for the Mersenne Twister RNG.
 *
 *  Copyright 2009 The MathWorks, Inc.
 *  Revision: 1.0  Date: 2004/12/23
 *
 *  Requires MATLAB� R13.

%TWISTER   Uniformly distributed pseudo-random numbers.
%   R = TWISTER(N) returns an N-by-N matrix containing pseudo-random values
%   drawn from a uniform distribution on the unit interval.  TWISTER(M,N) or
%   TWISTER([M,N]) returns an M-by-N matrix.  TWISTER(M,N,P,...) or
%   TWISTER([M,N,P,...]) generates an M-by-N-by-P-by-... array.  TWISTER with
%   no arguments returns a scalar.  TWISTER(SIZE(A)) returns an array the same
%   size as A.
%
%   TWISTER produces pseudo-random numbers using the Mersenne Twister
%   algorithm by Nishimura and Matsumoto, and is an alternative to the
%   built-in function RAND in MATLAB.  It creates double precision values in
%   the closed interval [0, 1-2^(-53)], and can generate 2^19937 - 1 values
%   before repeating itself.
%
%   The sequence of numbers generated is determined by the internal state of
%   the generator.  Since MATLAB resets the state at start-up, the sequence of
%   numbers generated will be the same in each session unless the state is
%   changed.  Setting the generator to different states leads to unique
%   computations, but does not improve any statistical properties.  Setting
%   the generator to the same fixed state allows computations to be repeated.
%
%   TWISTER('state',J), where J is a scalar integer, initializes the state of
%   the generator.  There is no simple connection between the sequence of
%   random numbers generated from TWISTER('state',J) and TWISTER('state',J+1).
%   TWISTER('state',0) resets the generator to its initial state.  J may also
%   be an array of integers with length less than 625.
%
%   S = TWISTER('state') returns a 625-element vector of UINT32 values
%   containing the current state of the uniform generator.
%
%   TWISTER('state',S), where S is the output of TWISTER('state'), sets the
%   state of the generator to S.
%
%    Examples:
%
%       Three ways to initialize TWISTER differently each time:
%          twister('state',sum(100*clock))
%          twister('state',100*clock)
%          twister('state',2^32*rand(n,1)) % where n < 625.
%
%       Generate 100 values, reset the state, and repeat the sequence:
%          s = twister('state');
%          u1 = twister(100,1);
%          twister('state',s);
%          u2 = twister(100,1); % contains exactly the same values as u1
%
%       Generate uniform values from the interval [a, b]:
%          r = a + (b-a).*twister(100,1);
%
%       Generate integers uniform on the set 1:n:
%          r = 1 + floor(n.*twister(100,1));
%
%       Generate standard normal random values using the inversion method:
%          z = -sqrt(2).*erfcinv(2*twister(100,1));
%
%   Mex file derived from a copyrighted C program by Takuji Nishimura and
%   Makoto Matsumoto.
%
%   Reference: M. Matsumoto and T. Nishimura, "Mersenne Twister: A
%   623-Dimensionally Equidistributed Uniform Pseudo-Random Number Generator",
%   ACM Transactions on Modeling and Computer Simulation, Vol. 8, No. 1,
%   January 1998, pp 3--30.
%
%   See also RAND, RANDN.

%   Note:  Initializing TWISTER to the scalar integer state 0 actually
%   corresponds to the C call init_genrand(5489).

 */


#include "mex.h"
#include "matrix.h"
#include <math.h>
#include <string.h>
#include <ctype.h>


/****************************************************************************
 *
 * Mersenne Twister code:
 */


/*
   A C-program for MT19937, with initialization improved 2002/1/26.
   Coded by Takuji Nishimura and Makoto Matsumoto.

   Before using, initialize the state by using init_genrand(seed)
   or init_by_array(init_key, key_length).

   Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

     1. Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

     3. The names of its contributors may not be used to endorse or promote
        products derived from this software without specific prior written
        permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


   Any feedback is very welcome.
   http://www.math.keio.ac.jp/matumoto/emt.html
   email: matumoto@math.keio.ac.jp
*/

/* Period parameters */
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */

static unsigned long mt[N]; /* the array for the state vector  */
static int mti=N+1; /* mti==N+1 means mt[N] is not initialized */

/* initializes mt[N] with a seed */
void init_genrand(unsigned long s)
{
    mt[0]= s & 0xffffffffUL;
    for (mti=1; mti<N; mti++) {
        mt[mti] =
	    (1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti);
        /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
        /* In the previous versions, MSBs of the seed affect   */
        /* only MSBs of the array mt[].                        */
        /* 2002/01/09 modified by Makoto Matsumoto             */
        mt[mti] &= 0xffffffffUL;
        /* for >32 bit machines */
    }
}

/* initialize by an array with array-length */
/* init_key is the array for initializing keys */
/* key_length is its length */
void init_by_array(unsigned long init_key[], unsigned long key_length)
{
    int i, j, k;
    init_genrand(19650218UL);
    i=1; j=0;
    k = (N>key_length ? N : key_length);
    for (; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1664525UL))
          + init_key[j] + j; /* non linear */
        mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++; j++;
        if (i>=N) { mt[0] = mt[N-1]; i=1; }
        if (j>=key_length) j=0;
    }
    for (k=N-1; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1566083941UL))
          - i; /* non linear */
        mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++;
        if (i>=N) { mt[0] = mt[N-1]; i=1; }
    }

    mt[0] = 0x80000000UL; /* MSB is 1; assuring non-zero initial array */
}

/* generates a random number on [0,0xffffffff]-interval */
unsigned long genrand_int32(void)
{
    unsigned long y;
    static unsigned long mag01[2]={0x0UL, MATRIX_A};
    /* mag01[x] = x * MATRIX_A  for x=0,1 */

    if (mti >= N) { /* generate N words at one time */
        int kk;

        if (mti == N+1)   /* if init_genrand() has not been called, */
            init_genrand(5489UL); /* a default initial seed is used */

        for (kk=0;kk<N-M;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        for (;kk<N-1;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
        mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

        mti = 0;
    }

    y = mt[mti++];

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return y;
}

/* generates a random number on [0,0x7fffffff]-interval */
long genrand_int31(void)
{
    return (long)(genrand_int32()>>1);
}

/* generates a random number on [0,1]-real-interval */
double genrand_real1(void)
{
    return genrand_int32()*(1.0/4294967295.0);
    /* divided by 2^32-1 */
}

/* generates a random number on [0,1)-real-interval */
double genrand_real2(void)
{
    return genrand_int32()*(1.0/4294967296.0);
    /* divided by 2^32 */
}

/* generates a random number on (0,1)-real-interval */
double genrand_real3(void)
{
    return (((double)genrand_int32()) + 0.5)*(1.0/4294967296.0);
    /* divided by 2^32 */
}

/* generates a random number on [0,1) with 53-bit resolution*/
double genrand_res53(void)
{
    unsigned long a=genrand_int32()>>5, b=genrand_int32()>>6;
    return(a*67108864.0+b)*(1.0/9007199254740992.0);
}
/* These real versions are due to Isaku Wada, 2002/01/09 added */


/****************************************************************************
 *
 * MATLAB code:
 */

/* the gateway function */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    int errMsgNum = 0;

    /* First time through.
     */
    if (mti == N+1) init_genrand(5489UL);

    if (nlhs > 1) {
        mexErrMsgIdAndTxt("twister:TooManyOutputs", "Too many output arguments.");
    }

    /* No args given, return a scalar.
     */
    if (nrhs == 0) {
        plhs[0] = mxCreateDoubleScalar(genrand_res53());

    } else if ((nrhs > 0) && (mxIsChar(prhs[0]))) {
        char theString[10], *p = theString;
        if (mxGetString(prhs[0],theString,sizeof(theString))) {
            mexErrMsgIdAndTxt("twister:BadStringArg", "Invalid string arg.");
        }
        for (;*p;p++) *p = tolower(*p); /* need strncasecmp */
        if (strncmp(theString,"state",sizeof(theString))) {
            mexErrMsgIdAndTxt("twister:BadStringArg", "Invalid string arg.");
        } else if (nrhs > 2) {
            mexErrMsgIdAndTxt("twister:TooManyInputs", "Too many input arguments.");
        }

        /* Return the old state when setting the state only if asked for, but
         * always return current state when reading the state.
         */
        if ((nlhs > 0) || (nrhs == 1)) {
            plhs[0] = mxCreateNumericMatrix(1,N+1,mxUINT32_CLASS,mxREAL);
            unsigned long *q = mt, *p = (unsigned long*)mxGetData(plhs[0]);
            for (int i=N; i; i--) *p++ = *q++;
            *p++ = mti;
        }

        /* Init or set the state.
         */
        if (nrhs == 2) {
            unsigned long initLen = (unsigned long)mxGetNumberOfElements(prhs[1]);
            if (initLen == 1) {
                /* M&N's default initializer (see genrand_int32) is
                 * 5489UL, make zero do the same thing as that.
                 */
                unsigned long initVal = (unsigned long)mxGetScalar(prhs[1]);
                if (initVal == 0) initVal = 5489UL;
                init_genrand(initVal);
            } else if (mxIsDouble(prhs[1])) {
                if (initLen <= N) {
                    double *q = (double*)mxGetData(prhs[1]);
                    unsigned long init[N], *p = init;
                    for (int i=initLen; i; i--) *p++ = (unsigned long) *q++;
                    init_by_array(init, initLen);
                } else {
                    mexErrMsgIdAndTxt("twister:InvalidInitLen", "Initializer J must have fewer than 625 elements.");
                }
            } else if (mxIsUint32(prhs[1])) {
                unsigned long stateLen = (unsigned long)mxGetNumberOfElements(prhs[1]);
                unsigned long *state = (unsigned long*)mxGetData(prhs[1]);
                if ((stateLen == N+1) && (state[N] <= N)) {
                    unsigned long *q = state, *p = mt;
                    for (int i=N; i; i--) *p++ = *q++;
                    mti = (int) *q++;
                } else {
                    mexErrMsgIdAndTxt("twister:InvalidStateLen", "Invalid state vector S.");
                }
            } else {
                mexErrMsgIdAndTxt("twister:InvalidInitOrState", "Second input must be an initializer or a state vector.");
            }
        }

    } else {
        int nelem, localDims[10];
        int *dims = localDims;
        int ndim = (nrhs == 1) ? mxGetNumberOfElements(prhs[0]) : nrhs;

        if (ndim > 10) {
            dims = (int *)mxCalloc(ndim, sizeof(int));
        }

        /* Individual size args given.
         */
        if (nrhs > 1) {
            int *p = dims;
            nelem = 1;
            for (int i=0; i<ndim; i++) {
                if ((!mxIsDouble(prhs[i])) || (mxGetNumberOfElements(prhs[i])!=1) || mxIsComplex(prhs[i])) {
                    errMsgNum = 101; goto cleanup;
                }
                nelem *= *p++ = (int)mxGetScalar(prhs[i]);
            }

        /* Size vector given.
         */
        } else { /* nrhs == 1, nrhs==0 has already been weeded out */
            if ((!mxIsDouble(prhs[0])) || (mxGetNumberOfElements(prhs[0])<1) ||  mxIsComplex(prhs[0])) {
                errMsgNum = 102; goto cleanup;

            /* Single size given, we'll return a square matrix.
             */
            } else if (ndim == 1) {
                ndim = 2;
                int n = (int)mxGetScalar(prhs[0]);
                dims[0] = n; dims[1] = n;
                nelem = n*n;

            /* Size vector.
             */
            } else {
                int *p = dims;
                double *q = (double*)mxGetData(prhs[0]);
                nelem = 1;
                for (int i=ndim; i; i--) nelem *= *p++ = (int)*q++;
            }
        }

        for (int i=0; i<ndim; i++) {
            if ((dims[i] < 0) || dims[i] > INT_MAX) {
                errMsgNum = 103; goto cleanup;
            }
        }

        /* Create the output matrix, get a pointer to its data, and fill it
         * in with random values.
         */
        {
        plhs[0] = mxCreateNumericArray(ndim,dims,mxDOUBLE_CLASS,mxREAL);
        double *r = (double*) mxGetData(plhs[0]);
        for (int i=nelem; i; i--,r++) *r = genrand_res53();
        }

cleanup:
        if (dims != localDims) mxFree((void *)dims);
        if (errMsgNum) {
            mexErrMsgIdAndTxt("twister:InvalidSize", "Invalid output size.");
        }
    }
}
