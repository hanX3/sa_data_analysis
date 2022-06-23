#include "cube.h"

//
cube::cube(char *cube_name, char *tab_name, float e_max)
{
  int nclook, lmin, lmax;
  char cube_file_name[256], scr_file_name[256];
  char tab_file_name[256];
  int i;

  //read tab file,get the ch information
  strcpy(tab_file_name,tab_name);
  if(!strstr(tab_file_name,".tab")){
    strcat(tab_file_name,".tab");
  }
  if(read_tab(tab_file_name, &nclook, &lmin, &lmax, luch) == 1){
    printf("open tab file %s error !!!\n", tab_file_name);
    exit(-1);
  }
  //get input largest ch
  cube_adc_max = nclook;
  cube_e_max = e_max;
  cube_e2ch = 1.0*cube_adc_max/cube_e_max;
  if(lmax > RW_MAXCH) {
     printf("the cube ch is  too large (curr_ch:%d,max = %d).\n", lmax, RW_MAXCH);
     exit(-1);
  }
  //get cube largest ch;
  cube_length = lmax;

  printf("cube_adc_max %d, cube_e_max %f, lmin %d, lmax %d\n", cube_adc_max, cube_e_max, lmin, lmax);

  //init the lookup table,map adc ch into cube channels,
  for(i=0;i<8;i++){
    lumx[i] = 0;
    lumy[i] = 0;
    lumz[i] = i/4;
  }
  for(i=8;i<cube_length;i++){
    lumx[i] = (i/8)*2;
    lumy[i] = lumy[i-8] + lumx[i];
    lumz[i] = lumz[i-8] + lumy[i];
  }

  /* open cube and scratch */
  char *ptr;
  if((ptr=strstr(cube_name,".cub"))){
    *ptr='\0';
  }
  strcpy(cube_file_name, cube_name);
  strcat(cube_file_name, ".cub");
  strcpy(cube_cube_filename, cube_file_name);
  if(!(cube_cube_file = open_3cube(cube_file_name, cube_length))){
    printf("create the cube file %s error!\n", cube_file_name);
    exit(-1);
  }
  else{
    printf("create the cube file %s .\n", cube_file_name);
  }

  //open scratch file 
  strcpy(scr_file_name, cube_name);
  strcat(scr_file_name, ".scr");
  //allocate 500MB for scratch file
  cube_scr_size = RW_SCR_SIZE;
  if(!(cube_scr_file = open_scr(scr_file_name, cube_scr_size))){
    fclose(cube_cube_file);
    printf("create the scratch file %s error!\n", scr_file_name);
    exit(-1);
   }
  else{
    printf("create the scratch file %s .\n", scr_file_name);
  }

  /* clear buf1 and buf2 */
  for(i=0;i<RW_LB1;i++){
    nbuf1[i] = 0;
  }
  for(i=0;i<RW_LB2;i++){
    nbuf2[i] = 0;
    scrptr[i] = 0;
  }
  srand(time(NULL));
  printf("now start create cube\n");
  printf("maxium of adc ch is %d, the cube length is %d\n", cube_adc_max, cube_length);
}

//
cube::~cube()
{

}

/***************************************************************************
open_3cube: returns file descriptor on success or NULL on failure
name - cube file name on disk
length - number of channels on cube axis
*/
FILE *cube::open_3cube(char *name, int length)
{
  FILE  *file;
  int i, nummc;
  FHead3D head;
  Record3D rec;

  if(!name){
    return NULL;
  }
  if(!(file = fopen(name,"w+"))){
    printf("Cannot open cube file %s\n", name);
    return NULL;
  }
  rewind(file);

  //calculate the number of minicubes and initialize cube file
  nummc = (length + 7)/8;
  nummc = nummc*(nummc+1)*(nummc+2)/3; /* minicubes in cube */
  cube_num_mc = nummc;
  cube_num_recs = (nummc + 4087)/4088; /* note, an empty minicube is 1 byte */
  //file head 1024byte
  strncpy(head.id, "Incub8r3/Pro4d  ", 16);
  head.numch = length;
  head.bpc = 4;
  head.cps = 6;
  head.numrecs = cube_num_recs;
  memset(head.resv, 0, 992);

  //write file header
  rewind(file);
  if(!fwrite(&head, 1024, 1, file)){
    printf("write file head error\n");
    fclose(file);
    return NULL;
  }
  /* initialize cube record*/
  for(i=0;i<4088;i++){
    rec.d[i] = 0;
  }
  /* loop through records */
  for(i=0;i<cube_num_recs;i++){
    rec.h.minmc = i*4088;
    rec.h.nummc = 4088;
    rec.h.offset = 8;
    if(!fwrite(&rec, 4096, 1, file)){
      printf("write file record error\n");
      fclose(file);
      return NULL;
    }
  }

  fflush(file);
  return file;
}

/***************************************************************************
open_scr: returns file descriptor on success or NULL on failure
name - file name on disk
length - size on Megabytes of scr file
*/
FILE *cube::open_scr(char *name, int length)
{
  FILE *file;
  int b[256], i;

  if(!name || length < 1 || !(file=fopen(name,"w+"))){
    printf("something wrong during creat scratch file.\n");
    return NULL;
  }
  rewind(file);
  for(i=0;i<length*1024;i++) /* writing to make sure we have space */
    if(!fwrite(b, 1024, 1, file)){
      printf("Could not allocate scratch space on disk (%dM).\n",length);
      fclose(file);
      return NULL;
    }
  cube_num_scr = 0;
  rewind(file);
  printf("open scratch file  ...Done.\n");
  return file;
}

//save cube into file
void cube::save()
{
  int            numchunks, chunknum;
  Record3D       recIn, recOut;
  int            j, k, mc, mcl;
  unsigned char  *mcptrIn, *mcptrOut, cbuf[2048];
  int            minmc, maxmc;
  int            recnumIn, recnumOut, nbytes;
  int            addr13b, addr8b, addr21b;
  int            nmcperc = RW_CHNK_S*256;
  unsigned short inbuf[RW_DB2+1][RW_DB1+1];
  FHead3D        filehead;
  unsigned int   *chunk;
  FILE           *OutFile;
  char           filname[512];

  printf("start malloc chunk ...\n");
  while(!(chunk=(unsigned int *)malloc(RW_CHNK_S*256*1024))){
    printf(" chunk malloc failed...\n"
           "  ... please free up some memory and press return.\n");
    fgets(filname, 256, stdin);
  }

  /* open new output file for incremented copy of cube */
  strcpy(filname, cube_cube_filename);
  strcat(filname, ".tmp-increment-copy");
  if(!(OutFile = fopen(filname,"w+"))){
    printf("Cannot open new output cube file. %s\n",filname);
    exit(-1);
  }
  else{
    printf("open new output cube file. %s\n",filname);
  }

  fseek(cube_cube_file, 0, SEEK_SET);
  if(!fread(&filehead, 1024, 1, cube_cube_file)){
    printf("Cannot read file header, aborting.\n");
    exit(-1);
  }
  fseek(OutFile, 0, SEEK_SET);
  if(!fwrite (&filehead, 1024, 1, OutFile)){
    printf("Cannot write file header, aborting.\n");
    exit(-1);
  }

  /* a chunk is RW_CHNK_S/16 Mchannels, nmcperc minicubes */
  numchunks = (cube_num_mc+nmcperc-1)/nmcperc;

  printf("\n cube_nummc %d, nmcperc %d\n", cube_num_mc, nmcperc);
  printf("total %d numchunks \n", numchunks);
  printf("Updating cube from scratch file\n");

  /* read in first record */
  recnumIn = 0;
  fseek(cube_cube_file, 1024, SEEK_SET);
  if(!fread(&recIn, 4096, 1, cube_cube_file)){
    printf(" Corrupted cube, aborting.\n");
    exit(-1);
  }
  mcptrIn = recIn.d;

  /* init the first output record */
  recnumOut = 0;
  recOut.h.minmc = recIn.h.minmc;
  recOut.h.offset = 8;
  mcptrOut = recOut.d;

  /* loop through all the chunks in the file */
  for(chunknum=0; chunknum<numchunks; chunknum++){
    minmc = chunknum*nmcperc;
    maxmc = minmc+nmcperc-1;
    if(maxmc>cube_num_mc-1)
      maxmc = cube_num_mc - 1;
    printf("chunknum %d, minmc %d, maxmc %d\n", chunknum, minmc, maxmc);
    printf("chunk %d, recs %d %d\n", chunknum, recnumIn, recnumOut);
    fflush(stdout);

    /* loop through all the minicubes in the chunk */
    for(mc=minmc; mc<=maxmc; mc++){
      dbinfo[2]=mc;
      if(mc > recIn.h.minmc+recIn.h.nummc-1){
        /* next compressed minicube starts in the next input record */
        if(!fread(&recIn, 4096, 1, cube_cube_file)){
          printf("Corrupted cube, aborting.\n");
          exit(-1);
        }
        recnumIn++;
        mcptrIn = recIn.d + recIn.h.offset - 8;
        /* at this point our minicube should be at the start of the record */
        if(recIn.h.minmc != mc){
          printf("Severe ERROR 1 - fatal!\n");
          printf("rec: %d  mc: %d  should be %d\n", recnumIn, mc, recIn.h.minmc);
          exit(-1);
        }
      }
      else if(mcptrIn > recIn.d + 4088){
        printf("Severe ERROR 2 - fatal!\n");
        printf("rec: %d  mc: %d  should be > %d\n", recnumIn, mc, recIn.h.minmc + recIn.h.nummc - 1);
        exit(-1);
      }
      mcl=MCLEN(mcptrIn);
      if(mcptrIn + mcl > recIn.d + 4088){
        /* compressed minicube spills over into the next input record */
        nbytes = mcptrIn + mcl - (recIn.d + 4088);
        memcpy (cbuf, mcptrIn, mcl-nbytes);
        if(!fread (&recIn, 4096, 1, cube_cube_file)){
           printf("Corrupted cube, aborting...\n");
           exit(-1);
        }
        if(nbytes != (recIn.h.offset-8)){
          printf("Severe ERROR 3 - fatal!\n");
          printf("rec, offset, nbytes, mcl, mcptr: %d %d %d %d %ld\n"
                 "mc minmc nummc: %d %d %d\n", recnumIn+1, recIn.h.offset, nbytes ,mcl,
                 (long int) (mcptrIn-recIn.d+8), mc, recIn.h.minmc, recIn.h.nummc);
          exit(-1);
        }
        if(recIn.h.minmc != mc+1){
          printf("Severe ERROR 4 - fatal!\n");
          printf("rec: %d  mc: %d  should be %d\n", recnumIn+1, mc+1, recIn.h.minmc);
          exit(-1);
        }
        recnumIn++;
        memcpy (&cbuf[mcl-nbytes], recIn.d, nbytes);
        decompress3d (cbuf, &chunk[(mc-minmc)<<8]);
        mcptrIn = recIn.d + recIn.h.offset - 8;
      }
      else{
        decompress3d (mcptrIn, &chunk[(mc-minmc)<<8]);
        mcptrIn += mcl;
      }
    }

    /* increment the chunk from the buffers */
    addr8b = chunknum;

    /* first empty the corresponding parts of buf1 */
    for(addr13b=addr8b*RW_CHNK_S;addr13b<(addr8b+1)*RW_CHNK_S; addr13b++){
      addr21b = (addr13b - addr8b*RW_CHNK_S)<<16;
      for(j=0;j<nbuf1[addr13b];j++)
        chunk[addr21b+buf1[addr13b][j]]++;
      nbuf1[addr13b] = 0;
    }

    /* next empty the corresponding parts of buf2 */
    for(j=0;j<nbuf2[addr8b];j++){
      addr21b = buf2[addr8b][j][RW_DB1]<<16;
      for(k=0; k<RW_DB1; k++)
        chunk[addr21b+buf2[addr8b][j][k]]++;
    }
    nbuf2[addr8b] = 0;

    /* increment the chunk from the scratch file */
    while(scrptr[addr8b]>0){
      fseek(cube_scr_file, ((long)scrptr[addr8b]-1)*RW_SCR_RECL, SEEK_SET);
      printf("scrptr[addr8b] %d\n", scrptr[addr8b]);
      if(!fread(inbuf, RW_SCR_RECL, 1, cube_scr_file)){
        printf("Could not read scr file.\n");
        exit(-1);
      }
      for(j=0;j<RW_DB2;j++){
        addr21b = inbuf[j][RW_DB1]<<16;
        for(k=0; k<RW_DB1; k++)  chunk[addr21b+inbuf[j][k]]++;
      }
      memcpy(&(scrptr[addr8b]), &inbuf[RW_DB2][0], 4);
    }

    /* recompress and rewrite the chunk */
    /* loop through all the minicubes in the chunk */
    for(mc=minmc; mc<=maxmc; mc++){
      mcl = compress3d (&chunk[(mc-minmc)<<8], cbuf);
      if(mcptrOut + mcl > recOut.d + 4088){
        /* the minicube spills over the end of the output record */
        if(mcptrOut + 2 > recOut.d + 4088){
          /* need at least first 2 bytes of minicube in current record */
          /* so move whole minicube to next record */
          recOut.h.nummc = mc - recOut.h.minmc;
          if(!fwrite(&recOut, 4096, 1, OutFile)){
            printf("Cannot write cube, aborting.\n");
            exit(-1);
          }
          recOut.h.minmc = mc;
          recOut.h.offset = 8;
          memcpy (recOut.d, cbuf, mcl);
          mcptrOut = recOut.d + recOut.h.offset - 8 + mcl;
        }
        else{
          /* move only part of minicube to next record */
          nbytes = mcptrOut + mcl - (recOut.d + 4088);
          memcpy (mcptrOut, cbuf, mcl-nbytes);
          recOut.h.nummc = mc - recOut.h.minmc + 1;
          if(!fwrite(&recOut, 4096, 1, OutFile)){
            printf("Cannot write cube, aborting.\n");
            exit(-1);
          }
          recOut.h.minmc = mc + 1;
          recOut.h.offset = nbytes + 8;
          memcpy(recOut.d, cbuf+mcl-nbytes, nbytes);
          mcptrOut = recOut.d + recOut.h.offset - 8;
        }
        recnumOut++;
      }
      else{
        memcpy (mcptrOut, cbuf, mcl);
        mcptrOut += mcl;
      }
    }
  } /* end of loop through chunks in the file */

  /* write out the last record */
  recOut.h.nummc = cube_num_mc - recOut.h.minmc;
  if(!fwrite(&recOut, 4096, 1, OutFile)){
    printf("Cannot write cube, aborting...\n");
    exit(-1);
  }
  recnumOut++;

  fseek(OutFile, 0, SEEK_SET);
  if (!fread (&filehead, 1024, 1, OutFile)) {
      printf(" Corrupted file header, aborting...\n");
      exit(-1);
  }
  filehead.numrecs = recnumOut;
  fseek(OutFile, 0, SEEK_SET);
  if(!fwrite (&filehead, 1024, 1, OutFile)){
    printf("Cannot write file header, aborting.\n");
    exit(-1);
  }
  fflush(OutFile);

  free(chunk);
  fclose(cube_cube_file);
  cube_cube_file = OutFile;
  if(rename(filname, cube_cube_filename)){
    printf("Cannot rename file, aborting...\n");
    exit(-1);
  }
  printf("\n   ...Done updating cube: There are now %d records.\n",  recnumOut);
}

void cube::dbuf2(int addr8b)
{
  char outbuf[RW_SCR_RECL];
  memcpy(outbuf, &(buf2[addr8b][0][0]), RW_DB2*2*(RW_DB1+1));
  memcpy(outbuf+RW_DB2*2*(RW_DB1+1), &(scrptr[addr8b]), 4);

  if(!fwrite(outbuf, RW_SCR_RECL, 1, cube_scr_file)) {
    printf(" Could not write to scr file.. fatal.\n");
    exit(-1);
  }
  cube_num_scr++;
  scrptr[addr8b] = cube_num_scr;

  if(cube_num_scr*RW_SCR_RECL >= cube_scr_size*1024*1024){
    /* scr full, inc cub */
    save();
    cube_num_scr = 0;
    rewind(cube_scr_file);
  }
}

//
inline int cube::e2ch(float e)
{
  int ch = cube_e2ch*e +1.0*rand()/RAND_MAX;
  //printf("cube_e2ch %f, e %f\n", cube_e2ch, e);
  //printf("ch %d\n", ch);
  if(ch >= cube_adc_max){
    ch = cube_adc_max-1;
  }else if(ch<0){
    ch = 0;
  }

  //printf("ch %d\n\n", ch);
  return ch;
}

//fill()
void cube::fill(float ex,float ey,float ez)
{
  int mul = 3;
  int elist[3];
  elist[0] = e2ch(ex);
  elist[1] = e2ch(ey);
  elist[2] = e2ch(ez);
  fill(mul, elist);
}

//fill()
void cube::fill(int gemult, float *ey)
{
  int elist[100];
  int mul= (gemult<100 ? gemult : 100);
  for(int i=0;i<mul;i++){
    elist[i] = e2ch(ey[i]);
  }
  fill(mul, elist);
}

//fill()  
void cube::fill(int gemult, int *elist)
{
  int mc_addr, inmc_addr, addr13b, addr16b, addr8b;
  int ex, ey, ez;
  int i,j,k,l,tmp;
  unsigned long numincr = 0, numevents = 0;

  if(gemult < 3) return;

  /* convert from ADC to cube channel numbers */
  j = 0;
  for(i=0;i<gemult;i++){
    if(elist[i]>=0 && elist[i]<cube_adc_max && luch[elist[i]]>0)
      elist[j++] = luch[elist[i]]-1;
  }
  //luch ch and multi
  gemult = j;
  if(gemult<3)  return;
  /* order elist */
  for(i=gemult-1;i>0;i--){
    if(elist[i] < elist[i-1]){
      tmp = elist[i];
      elist[i] = elist[i-1];
      elist[i-1] = tmp;
      j = i;
      while(j<gemult-1 && elist[j]>elist[j+1]){
        tmp = elist[j]; 
        elist[j] = elist[j+1]; 
        elist[j+1] = tmp;
        j++;
      }
    }
  }

  /* loop through all possible combinations */
  for(l=2;l<gemult;l++){
    ez = elist[l];
    for(k=1;k<l;k++){
      ey = elist[k];
      for(j=0;j<k;j++){
        ex = elist[j];
        numincr++;
        /* linear address (LUMC first 21bits, LUIMC last 8bits) */
        mc_addr = LUMC (ex,ey,ez);
        inmc_addr = LUIMC (ex,ey,ez);
        addr13b = mc_addr/256;
        addr16b = (mc_addr&255)*256+inmc_addr;
        buf1[addr13b][nbuf1[addr13b]++] = addr16b;
        /* dump buf1 to buf2 */
        if(nbuf1[addr13b] == RW_DB1){
          addr8b = addr13b / RW_CHNK_S;
          memcpy(&(buf2[addr8b][nbuf2[addr8b]][0]), &(buf1[addr13b][0]), RW_DB1*2); /* copy whole row into buf2 */
          buf2[addr8b][nbuf2[addr8b]++][RW_DB1] = (addr13b-(addr8b*RW_CHNK_S));
          nbuf1[addr13b] = 0;
          if(nbuf2[addr8b] == RW_DB2){
            dbuf2(addr8b);  /* dump buf2 to scratch file */
            nbuf2[addr8b] = 0;
          }
        }
      }
    }
  }
}

int cube::compress3d (unsigned int *in, unsigned char *out)
{
  static int sum_limits[] = { 16,32,64,128,
                              256,512,1024,2048,
                              4096,8192,17000,35000,
                              80000,160000,320000,640000 };

  static int bitmask[] = { 0,1,3,7,15,31,63,127,255,511,1023,2047,4095,8191,16383,32767 };
  int cube_length;
  int sum=0, i, j;
  int nplanes, noverflows, stmp, nbits;
  unsigned char *bitptr, *ovrptr;
  int test1, test2, test3;

  /* guess at depth of bit field */
  for(i=0;i<256;i+=16){
    sum += in[i];
  }

  i = 7;
  j = 8;
  while(j>1){
    j = j / 2;
    if(sum > sum_limits[i])
      i += j;
    else
      i -= j;
  }
  if(sum > sum_limits[i+1])
    nplanes = i + 1;
  else
    nplanes = i;

  while(1){
    test1 = test2 = test3 = 0;
    for(i=0;i<256;i++){
      test1 += in[i] >> nplanes;
      test2 += in[i] >> (nplanes+1);
      test3 += in[i] >> (nplanes+2);
    }
    if(test2 > 31){
      if(test3 > 31){
        nplanes += 3;
        continue;
      }
      nplanes += 2;
      noverflows = test3;
      break;
    }
    else if(test1 > 31){
      nplanes += 1;
      noverflows = test2;
      break;
    }
    noverflows = test1;
    break;
  }

  if(nplanes > 30)
    fprintf(stderr,"Expecting core dump...\n");

  /* insert length of compressed cube */
  if(nplanes < 7){
    out[0] = 32*nplanes + noverflows;
    bitptr = out+1;
    cube_length = 1;
  }
  else{
    out[0] = 224 + noverflows;
    out[1] = nplanes-7;
    bitptr = out+2;
    cube_length = 2;
  }
  ovrptr = bitptr + nplanes*32;
  cube_length += nplanes*32 + noverflows;

  /* now, compress */
  /* prepare bit planes and insert overflow bits... */
  while (nplanes>=8){
    for(i=0;i<256;i++){
      *bitptr++ = in[i]&bitmask[8];
      in[i] = in[i]>>8;
    }
    nplanes -= 8;
  }

  if(nplanes > 0){
    stmp = 0;
    nbits = 0;
    for(i=0;i<256;i++){
      /* insert nplanes number of bits */
      stmp = (stmp << nplanes) + (in[i] & bitmask[nplanes]);
      nbits += nplanes;
      if(nbits > 7){
        *bitptr++ = stmp >> (nbits - 8);
        nbits -= 8;
        stmp &= bitmask[nbits];
      }

      /* append overflows */
      noverflows = in[i] >> nplanes;
      for(j=0;j<noverflows;j++)
        *ovrptr++ = i;
    }
  }
  else{ /* just do overflows */
    for(i=0;i<256;i++){
      for(j=0;j<in[i];j++){
        *ovrptr++ = i;
      }
    }
  }
  return cube_length;
}

void cube::decompress3d (unsigned char in[1024], unsigned int out[256])
{
  static int bitmask[] = { 0,1,3,7,15,31,63,127,255,511,1023,2047,4095,8191,16383,32767 };
  int nplanes, noverflows, nbits, savenplanes;
  unsigned char *bitptr;
  int i, j, t;

  nplanes = in[0] >> 5;
  noverflows = in[0] & 31;
  if(nplanes == 7){
    nplanes += in[1];
    bitptr = in+2;
  }
  else{
    bitptr = in+1;
  }
  /* printf("%d %d %d\n",nplanes,noverflows,*bitptr); */

  /* extract bit planes */
  savenplanes = nplanes;
  for(i=0; i<256; i++)
    out[i] = 0;
  j = 0;
  while(nplanes>=8){
    for(i=0; i<256; i++)
      out[i] += ((unsigned int)*bitptr++)<<j;
    nplanes -= 8;
    j += 8;
  }

  if(nplanes > 0){
    nbits = 0;
    for(i=0; i<256; i++){
      if(nbits+nplanes < 9){
        out[i] += ((*bitptr >> (8-nbits-nplanes)) & bitmask[nplanes])<<j;
        nbits += nplanes;
        if(nbits > 7){
          bitptr++;
          nbits -= 8;
        }
      }
      else{
        t = nplanes-8+nbits;
        out[i] += (((*bitptr & bitmask[8-nbits]) << t) + (*(bitptr+1) >> (8-t)))<<j;
        bitptr++;
        nbits = t;
      }
    }
  }

  /* extract overflows */
  for(i=0; i<noverflows; i++)
    out[*bitptr++] += 1 << savenplanes;
}

/***************************************************************************
   read_tab: returns 0 on success or 1 on failure
     lnam       - lookup table file name on disk
     nclook     - returned length (in chs) of lookup table
     lmin, lmax - returned min, max values in lookup table
     lut_ret    - returned lookup table values
*/
int cube::read_tab(char *lnam, int *nclook, int *lmin, int *lmax, short *lut_ret)
{
  printf("start read tab file ...\n");

  int   ibuf;
#ifdef VMS
  int   j;
  short *buf;
#endif
  FILE  *file;

  if(!(file=fopen(lnam,"r"))){
    printf("\007  ERROR -- Could not open %s for reading.\n",lnam);
    return 1;
  }
  fread(&ibuf, 4, 1, file); /* FORTRAN stuff */
#ifdef VMS
  buf = (short *) &ibuf;
  if(*buf != 14){
#else
  if(ibuf != 12){
#endif
    printf("\007  ERROR -- Bad record(1) in look-up file.\n");
    fclose(file);
    return 1;
  }
  if( (fread(nclook,4,1,file)) < 1 ||  /* Number of chs in lookup table */
      (fread(lmin,  4,1,file)) < 1 ||  /* Minimum value in lookup table */
      (fread(lmax,  4,1,file)) < 1 ) { /* Maximum value in lookup table */
  ERR:
    printf("\007  ERROR -- Unexpected end of look-up file.\n");
    fclose(file);
    return 1;
  }
  if(*nclook > 16384) *nclook = 16384;
  fread(&ibuf, 4, 1, file); /* FORTRAN stuff */

#ifdef VMS
  j   = *nclook;
  buf = lut_ret;
  while(j > 2042){
    if(fread(buf, 2042*2, 1, file) < 1) goto ERR;
    fread(&ibuf,4,1,file); /* FORTRAN stuff */
    buf += 2042; j -= 2042;
  }
  if(fread(buf, j*2, 1, file) < 1) goto ERR;
#else
  fread(&ibuf, 4, 1, file); /* FORTRAN stuff */
  if((fread(lut_ret, (*nclook)*2, 1, file)) < 1) goto ERR;
#endif
  fclose(file);
  return 0;
}
