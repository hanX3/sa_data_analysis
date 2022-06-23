
#include "datatree.h"
#include "cube.h"
#include <TFile.h>
#include <TTree.h>

#include <iostream>

using std::cout; using std::endl;

int main()
{
  char cube_name[1024] = "118I.cub";
  char tab_name[1024] = "118I.tab";
  
  cube *cu = new cube(cube_name, tab_name, 2048);

  TFile *file_in = TFile::Open("./3d.root");
  TTree *tr_in = (TTree*)file_in->Get("tr_3d");
  datatree *dat = new datatree(tr_in);

  Long64_t entries = dat->GetEntries();
  for(Long64_t i=0;i<entries;i++){
    dat->GetEntry(i);

    if(dat->energy0>=(2048-1.) || dat->energy1>=(2048-1.) || dat->energy2>=(2048-1.))
      continue;
    cu->fill(dat->energy0, dat->energy1, dat->energy2);
  }

  cu->save();

  return 0;
}

