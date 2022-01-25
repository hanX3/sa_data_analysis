// write by wangjg@imp

#ifndef MATRIX_H
#define MATRIX_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>

#define MATRIX_DIMENSION 4096

//class define
class Matrix
{
    private:
        FILE *C_MatrixFile;
        int byte_4;
        unsigned short matrix_2b[MATRIX_DIMENSION][MATRIX_DIMENSION];
        unsigned int  matrix_4b[MATRIX_DIMENSION][MATRIX_DIMENSION];
        float C_Emax;
        float C_E2CH_rate;
        int C_max_in_ch;
    public:
        Matrix(char* matrix_name, float E);
        ~Matrix(){};
        void Fill(float ex,float ey);
        void Fill(int mul,float *elist);
        void Save();
        int ReadEle(int,int);
        inline int E2CH(float E);
};

#endif
