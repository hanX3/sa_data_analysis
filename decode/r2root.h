#ifndef _R2ROOT_H_
#define _R2ROOT_H_

#include "decoder.h"

#include "TString.h"
#include "TROOT.h"
#include "TTree.h"
#include "TFile.h"
#include "TBenchmark.h"
#include "TH1.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <unistd.h>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class r2root
{
public:
  r2root(TString rawfilepathh, TString rootfilepathh, TString filenamee, int runnumberr);
  virtual ~r2root();

  void process();

private:
  void clearopt();

private:
  decoder *rawdec;

  TFile *file;
  TTree *t;
  TBenchmark *benchmark;
  int runnumber;
  TString rawfilepath;
  TString filename;

  map<unsigned short, unsigned short> madcc;
  map<unsigned short, unsigned short> mcfdd;
  map<unsigned short, long long> mtss;

  Short_t multi;
  Short_t ch[4*NPAR];
  Short_t clover[4*NPAR];
  Short_t crystal[4*NPAR];

  UShort_t adc[4*NPAR];
  Long64_t timestamp[4*NPAR];
};

#endif
