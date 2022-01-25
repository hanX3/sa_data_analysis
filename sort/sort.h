#ifndef SORT_H
#define SORT_H

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <numeric>  //  accumulate method

#include "string.h"
#include "set.h"

#include "TFile.h"
#include "TH2F.h"
#include "TTree.h"
#include "TString.h"
#include "TROOT.h"
#include "TBenchmark.h"

#include "matrix.h"
#include "cube.h"

using std::cout;
using std::endl;
using std::vector;
using std::map;
using std::accumulate;

class sort
{
public:
  sort(TString rawfilepathh, TString outfilepathh, TString filenamee, int runnumberstart, int runnumberstopp);
  virtual ~sort();
  int lep_runnum;
  bool openfile(int num);
  void process();
private:
  bool getcalipar(int runnumberr);
  void addbackV1();
  void addbackV2();
  void addbackV3();
  void addbackV4();
  void addbackV5();
  void mkmatV1();  //  no windows
  void mkmatV2();  //  no windows
  Double_t cali(Short_t chh, UShort_t adcc);

private:
  TString rawfilepath;
  TString outfilepath;
  TString filename;
  int runnumberstart;
  int runnumberstop;

  Matrix *mat;
  Cube *cub;

private:
  Double_t p0[4*NPAR]; //
  Double_t p1[4*NPAR]; //
  Double_t p2[4*NPAR]; //

  TFile *filein;
  TTree *treein;

  Long64_t totalentry;
  TBranch  *b_multi;   //!
  TBranch  *b_ch;   //!
  TBranch  *b_clover;   //!
  TBranch  *b_crystal;   //!
  TBranch  *b_adc;   //!
  TBranch  *b_timestamp;   //!

  Short_t  multi;
  Short_t  ch[4*NPAR];   //[multi]
  Short_t  clover[4*NPAR];   //[multi]
  Short_t  crystal[4*NPAR];   //[multi]
  UShort_t adc[4*NPAR];   //[multi]
  Long64_t timestamp[4*NPAR];   //[multi]

  Float_t energy[4*NPAR];
  Short_t multi_coin;
  Float_t energy_coin[4*NPAR];
  Long64_t count2d;
  Long64_t count3d;
  Long64_t fire2cry_total;
  Long64_t fire2cry_normal;
  Long64_t fire2cry_dia;
  Long64_t fire2cry_sep;  //2 no dia crystal fired large than COINWINDOW
  Long64_t fire3cry_total;
  Long64_t fire3cry_con1; //fire in COINWINDOW
  Long64_t fire3cry_con2; //0+1 < COINWINDOW, 1+2 < COINWINDOW, 0+2 > COINWINDOW
  Long64_t fire3cry_con3;
  Long64_t fire3cry_con4;
  Long64_t fire4cry_total;

  Short_t multi_ab;
  Short_t clover_ab[NPAR];
  Long64_t timestamp_ab[NPAR];
  Float_t energy_ab[NPAR];
  Short_t multi_ab_coin;
  Short_t clover_ab_coin[NPAR];
  Float_t energy_ab_coin[NPAR];

  TFile *fileout;
  TTree *treeout;
  TH2F *h2all;
  TH2F *h2all_ab;
  TH2F *h2ado_fd;
  TH2F *h2ado_90;
  TH2F *h2pol_tran;
  TH2F *h2pol_vert;

  TBenchmark *benchmark;
};


#endif
