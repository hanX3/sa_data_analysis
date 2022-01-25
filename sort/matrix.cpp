//  write by wangjg@imp

#include "matrix.h"

Matrix::Matrix(char* matrix_name,float E)
{
    char str_tmp[256];
    strcpy(str_tmp,matrix_name);
    if(!matrix_name )
    {
        printf("matrix name error\n");
        exit(-1);
    }
    if(strstr(matrix_name,".m4b") || strstr(matrix_name,".spn"))
    {
        byte_4=1;
        printf("the matrix will be creat with 4byte format\n");
    }
    else
    {
        byte_4=0;
        if(!strstr(matrix_name,".mat"))
            strcat(str_tmp,".mat");
        printf("the matrix will be creat with 2byte format\n");
    }
    if(!(C_MatrixFile=fopen(str_tmp,"wb")))
    {
        fclose(C_MatrixFile);
        printf("file %s error\n",matrix_name);
        exit(-1);
    }
    C_Emax=E;
    C_E2CH_rate=1.0*MATRIX_DIMENSION/C_Emax;
    C_max_in_ch=MATRIX_DIMENSION;
    if(byte_4==0)
        memset(matrix_2b,0,sizeof(unsigned short)*MATRIX_DIMENSION*MATRIX_DIMENSION);
    else
        memset(matrix_4b,0,sizeof(unsigned int)*MATRIX_DIMENSION*MATRIX_DIMENSION);
    srand((long)time(NULL));
}

//matrix
void Matrix::Fill(float ex,float ey)
{
    if(byte_4==0)
    {
        matrix_2b[E2CH(ex)][E2CH(ey)]++;
        if(matrix_2b[E2CH(ex)][E2CH(ey)]>256*256)
        {
            printf("the coin data exceed the 2byte(2^16)!!!,\n\
                    plz change the mat name with xxx.m4b in construc function\n");
            exit(-1);
        }
    }
    else
        matrix_4b[E2CH(ex)][E2CH(ey)]++;
}
//for symmetry matrix
void Matrix::Fill(int mul,float *elist)
{
    int mulmax=100;
    mulmax=(mul<mulmax?mul:mulmax);
    for(int i=0;i<mulmax-1;i++)
        for(int j=i+1;j<mulmax;j++)
        {
            Fill(elist[i],elist[j]);
            Fill(elist[j],elist[i]);
        }
}

//E convert to matrix ch
inline int Matrix::E2CH(float E)
{
    int ch=E*C_E2CH_rate+1.0*rand()/RAND_MAX;
    if(ch>=C_max_in_ch)
        ch=C_max_in_ch-1;
    else if(ch<0)
        ch=0;
    return ch;
}
int Matrix::ReadEle(int xch,int ych)
{
    if(byte_4==0)
        return matrix_2b[xch][ych];
    else
        return matrix_4b[xch][ych];
}

void Matrix::Save()
{
    if(byte_4==0)
        fwrite(matrix_2b,sizeof(unsigned short),MATRIX_DIMENSION*MATRIX_DIMENSION,C_MatrixFile);
    else
        fwrite(matrix_4b,sizeof(unsigned int),MATRIX_DIMENSION*MATRIX_DIMENSION,C_MatrixFile);
    fclose(C_MatrixFile);
}
