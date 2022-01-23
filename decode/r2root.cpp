#include "r2root.h"

#include <iostream>
#include <fstream>
#include <climits>
#include <cmath>
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......


r2root::r2root(TString rawfilepathh, TString rootfilepathh, TString filenamee, int runnumberr)
{
  runnumber = runnumberr;
  rawfilepath = rawfilepathh;
  filename = filenamee;
  benchmark = new TBenchmark;

  char tempfilename[1024];

  rawdec = new decoder();

  sprintf(tempfilename,"%s%s%04d.root",rootfilepathh.Data(),filenamee.Data(),runnumberr);
  file = new TFile(tempfilename,"RECREATE");
  t = new TTree("tree","iThemba data");

  t->Branch("multi",&multi,"multi/S");
  t->Branch("ch",ch,"ch[multi]/S");
  t->Branch("clover",clover,"clover[multi]/S");
  t->Branch("crystal",crystal,"crystal[multi]/S");
  t->Branch("adc",adc,"adc[multi]/s");
  t->Branch("timestamp",timestamp,"ts[multi]/L");
}

r2root::~r2root()
{
  delete rawdec;
}

void r2root::process()
{
  benchmark->Start("r2root");

  unsigned short cfd = 0;
  long long ts = 0;
  unsigned short id = 0;
  int nblock = 0;

  char datafile[1024];
  int fileno = 0;
  unsigned short block[32768];
  while(1){
    sprintf(datafile,"%s%s%d_%d",rawfilepath.Data(),filename.Data(),runnumber,fileno);
    if(access(datafile, F_OK) == -1)
      break;
    cout << "start docode " << datafile << endl;

    FILE *pFile;
    pFile = fopen(datafile, "rb");
    if(!pFile){
      cout << "open file error." << endl;
      return;
    }

    int jj=0;
    while(1){
      memset(block, 0, 32768);
      if(fread(block, sizeof(unsigned short), 32768, pFile) == 0){
        fclose(pFile);
        fileno++;
        break;
      }
      nblock++;
      if(nblock%(1024*8) == 0){
        cout << "process " << 64*nblock/1024 << "MB data." << endl;
      }
      if(!rawdec->getblock(block)){
        cout << "copy block wrong ..." << endl;
        break;
      }
      while(rawdec->getnexteventheader()){
        if(rawdec->decode()){
          clearopt();
          madcc.clear();
          mcfdd.clear();
          mtss.clear();
          id = 0;
          cfd = 0;
          ts = 0;

          if(jj<10){
            rawdec->printevent();
            jj++;
          }

          madcc = rawdec->getmadc();
          mcfdd = rawdec->getmcfd();
          mtss = rawdec->getmts();
          map<unsigned short, unsigned short>::iterator itmadc = madcc.begin();

          multi = madcc.size();
          for(int ii=0;ii<multi;ii++){
            id = itmadc->first;
            clover[ii] = ((id & 0xFF00)>>8) - 1;
            crystal[ii] = (id & 0x00FF)/8;
            ch[ii] = 4*clover[ii] + crystal[ii];

            adc[ii] = itmadc->second;
            cfd = mcfdd[id];
            ts = mtss[id];
            timestamp[ii] = ts-5+10*cfd/65536.;

            itmadc++;
          }
          t->Fill();
        }
      }
    } //  while(1)
  } //  while(1)

  file->cd();
  t->Write();
  file->Close();

  benchmark->Show("r2root");
}

void r2root::clearopt()
{
  multi = -1;
  memset(ch, -1, NPAR);
  memset(clover, -1, NPAR);
  memset(crystal, -1, NPAR);
  memset(adc, 0, NPAR);
  memset(timestamp, -1, NPAR);
}

