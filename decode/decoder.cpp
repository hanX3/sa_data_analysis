#include "decoder.h"

decoder::decoder()
{
  index = 0;
  evthead = 0;
  evtsize = 0;

  clover = -1;
  crystal = -1;

  adc = 0;
  cfd = 0;
  ts = 0;

  madc[0] = 0;
  mcfd[0] = 0;
  mts[0]  = 0;
}

decoder::~decoder()
{

}

bool decoder::getblock(unsigned short (&blockin)[32768])
{
  if(sizeof(blockin)/sizeof(unsigned short) != 32768){
    cout << "wrong block size : " << sizeof(blockin)/sizeof(unsigned short) << endl;
    return false;
  }

  block = blockin;
  index = BLOCKHEADSIZE;

  return true;
}

bool decoder::getnexteventheader()
{
  evthead = *(block+index);
  evtsize = *(block+index+1)/2-2;

  if(evthead!=0xFFFF || evtsize==0x0000){
    //  cout << "end of block" << endl;
    return false;
  }

  index += 2; //  add evt header size
  return true;
}

bool decoder::decode()
{
  madc.clear();
  mcfd.clear();
  mts.clear();

  unsigned short vsnseq, vsn, vsnsign, simboltime;
  unsigned short vsnsimbol, simboltt;

  for(int j=0;j<evtsize;){
    vsnseq = *(block+index);
    vsn = vsnseq & 0x00FF;
    vsnsign = vsnseq & 0xFF00;
    simboltt= *(block+index+1);

    if((vsnsign!=0x4800) && (vsnsign!=0x8000) && (vsnsign!=0x5e00)){
      cout << "vsnsign:" << vsnsign << endl;
      cout << "decode wrong ..." << endl;
      return false;
    }

    if(vsnseq==0x5e5e && simboltt==0x5e5e){
      //  cout << "end of block decode ..." << endl;
      return false;
    }
    unsigned short id = 0;
    if((vsnsign==0x4800) && (vsn>0) && (vsn<=NPAR)){
      for(int k=0;k<4;++k){
        if(*(block+index+0+1+k) > 0){
          //  id = vsn + ((8*k)<<8);
          id = (vsn<<8) + 8*k;  //  for channel sort
          madc.insert({id, *(block+index+0+1+k)});
          mcfd.insert({id, *(block+index+4+1+k)});
        }
      }
      index += 10;
      j += 10;
    }

    if(vsnsign==0x8000 && simboltt==0x270F){
      simboltime=*(block+index);
      vsnsimbol=simboltime & 0x00FF;

      index += 2;
      j += 2;

      long long a, b, c;
      long long tt;
      unsigned short idtemp, idtemp1, idtemp2;
      for(int k=0;k<vsnsimbol;){
        idtemp = *(block+index);
        idtemp1 = idtemp & 0xFF00;
        idtemp2 = idtemp & 0x00FF;
        idtemp = (idtemp2<<8) + (idtemp1>>8);
        //  cout << "idtemp:" << idtemp << endl;
        //  cout << "idtemp1:" << idtemp1 << endl;
        //  cout << "idtemp2:" << idtemp2 << endl;
        //  cout << "idtemp:" << idtemp << endl;

        a = *(block+index+1);
        b = *(block+index+2);
        c = *(block+index+3);
        tt = 10*((a<<32) + (b<<16) + c);
        mts.insert({idtemp, tt});
        index += 4;
        j += 4;
        k += 4;
      }
    }
  }

  return true;
}

void decoder::printevent()
{
  cout << "print event " << endl;
  cout << "event head: " << evthead << endl;
  cout << "event size: " << evtsize << endl;

  map<unsigned short, unsigned short>::iterator itmadc = madc.begin();
  map<unsigned short, unsigned short>::iterator itmcfd = mcfd.begin();
  map<unsigned short, long long>::iterator itmts = mts.begin();

  while(itmadc != madc.end()){
    cout << "adc: " << itmadc->first << " " << itmadc->second << endl;
    itmadc++;
  }
  while(itmcfd != mcfd.end()){
    cout << "tdc: " << itmcfd->first << " " << itmcfd->second << endl;
    itmcfd++;
  }
  while(itmts != mts.end()){
    cout << "ts: " << itmts->first << " " << itmts->second << endl;
    itmts++;
  }
}
