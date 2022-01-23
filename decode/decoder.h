#ifndef _decoder_h_
#define _decoder_h_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <iostream>
#include <map>

using std::cout;
using std::endl;
using std::map;

#define BLOCKHEADSIZE 12 // unit 2bytes
#define NPAR 12 // clover number maxiumn
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class decoder
{
public:
  decoder();
  virtual ~decoder();

  void clearall();

  bool getblock(unsigned short (&blockin)[32768]);
  void printevent();
  bool getnexteventheader();
  bool decode();

  inline map<unsigned short, unsigned short> getmadc() {return madc;}
  inline map<unsigned short, unsigned short> getmcfd() {return mcfd;}
  inline map<unsigned short, long long> getmts() {return mts;}

private:
  //  bool decode();

  unsigned short *block;
  unsigned short index;    // index of block
  unsigned short evthead;
  unsigned short evtsize;

  unsigned short clover;  // clover  id #0-#11
  unsigned short crystal; // crystal id #0-#3

  unsigned short adc;
  unsigned short cfd;     // CFD time
  long long      ts;      // timestamp of the fire

  map<unsigned short, unsigned short> madc;
  map<unsigned short, unsigned short> mcfd;
  map<unsigned short, long long> mts;

};

#endif
