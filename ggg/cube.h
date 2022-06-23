#ifndef _CUBE_H
#define _CUBE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>

#define RW_MAXFILES 9999         /* Max files per tape to read */
#define RW_MAXRECORDS 1000000000 /* Max number of records per tape to read */
#define RW_MAXCH 4000            /* Maximum channels on cube axis */
                                 /*    (must be multiple of 8)    */

#define RW_LB1 163744            /* length required for buf1, depends on RW_MAXCH */
                                 /* RW_LB1 > x(x+1)(x+2)/(6*128),                 */
                                 /* where x = RW_MAXCH/8                          */

#define RW_DB1 90                /* depth of buf1 */
#define RW_LB2 5117               /* length required for buf2, depends on RW_LB1   */
                                 /* RW_LB2 >= RW_LB1/RW_CHNK_S                    */

#define RW_DB2 180               /* depth of buf2 not to exceed RW_SCR_RECL       */
                                 /* byte records in scr file                      */

#define RW_SCR_RECL 32768        /* Record length of scr file                     */
                                 /* must be >= RW_DB2*2*(RW_DB1+1) + 4            */

#define RW_CHNK_S  32            /* size of update chunk, in units of 1/4 MB      */
#define RW_MAXMULT 40
#define RW_SCR_SIZE 2000          /* size of scratch flie                          */

/*
  a = RW_MAXCH/8                 = 175
  b = a*(a+1)*(a+2)/6            = 908600
  c = ceil(b/128)                = 7099
  RW_LB2 = ceil(c/RW_CHNK_S)     = 222
  RW_LB1 = RW_LB2*RW_CHNK_S      = 7104
  RW_DB1 = floor((RW_SCR_RECL-4)/(2*(RW_DB2+1)))

  RW_LB1 > x(x+1)(x+2)/(6*128), where x = RW_MAXCH/8
  RW_LB2 >= RW_LB1/RW_CHNK_S
*/

typedef struct {
  char id[16];            /* "Incub8r3/Pro4d  " */
  int  numch;             /* number of channels on axis */
  int  bpc;               /* bytes per channel, = 4 */
  int  cps;               /* 1/cps symmetry compression, = 6 */
  int  numrecs;           /* number of 4kB data records in the file */
  char resv[992];         /* FUTURE flags */
} FHead3D;

typedef struct {
  int minmc;          /* start minicube number, starts at 0 */
  short nummc;        /* number of minicubes stored in here */
  short offset;       /* offset in bytes to first full minicube */
} RHead3D;

typedef struct {
  RHead3D h;
  unsigned char d[4088];  /* the bit compressed data */
} Record3D;               /* see the compression alg for details */


/* look up minicube addr */
#define LUMC(x,y,z) lumx[x] + lumy[y] + lumz[z]
/* look up addr in minicube */
#define LUIMC(x,y,z) (x&7) + ((y&7)<<3) + ((z&3)<<6)
#define MCLEN(mcptr) (mcptr[0] + (mcptr[0] >= 7*32 ? mcptr[1]*32+2 : 1))

//class define
class cube
{
public:
  cube(char *cube_name, char *tab_file_name, float e_max);
  virtual ~cube();

private:
  char cube_cube_filename[256];
  FILE *cube_cube_file;
  FILE *cube_scr_file;
  int  cube_length;     /* length of axis on cube */
  int  cube_num_mc;      /* number of minicubes */
  int  cube_scr_size;
  int  cube_num_scr;     /* number of records written to scrfile */
  int  cube_adc_max;
  float cube_e_max;
  float cube_e2ch;
  int  cube_num_recs;    /* current number of records in cube file */
  short luch[16384];    /* look-up table, maps ADC channels to cube channels */
  int lumx[RW_MAXCH];   /* look-up table, maps 3d ch to linear minicube */
  int lumy[RW_MAXCH];
  int lumz[RW_MAXCH];

  /* increment buffers */
  unsigned short buf1[RW_LB1][RW_DB1];
  unsigned short buf2[RW_LB2][RW_DB2][RW_DB1+1];
  int nbuf1[RW_LB1];       /* number in each row of buf1 */
  int nbuf2[RW_LB2];       /* number in each row of buf2 */
  int scrptr[RW_LB2];      /* last record # written to scr file */
  int dbinfo[10];
  /* for each row of buf2              */

public:
  void fill(int gemult, int *elist);
  void fill(int gemult,float *elist);
  void fill(float ex, float ey, float ez);
  inline int e2ch(float e);
  void save();
  FILE *open_3cube (char *name, int length);
  FILE *open_scr (char *name, int length);
  void setup_replay();
  int read_tab(char *lnam, int *nclook, int *lmin, int *lmax, short *lut_ret);
  int compress3d(unsigned int *in, unsigned char *out);
  void decompress3d(unsigned char in[1024], unsigned int out[256]); 
  void dbuf2(int addr8b);
};


#endif
