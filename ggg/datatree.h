//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Mon Mar 28 13:53:03 2022 by ROOT version 6.24/02
// from TTree tr_3d/3-fold data
// found on file: R1888_3d.root
//////////////////////////////////////////////////////////

#ifndef datatree_h
#define datatree_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

#include <iostream>

// Header file for the classes stored in the TTree if any.

class datatree {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

// Fixed size dimensions of array or collections stored in the TTree if any.

   // Declaration of leaf types
   Float_t        energy0;
   Float_t        energy1;
   Float_t        energy2;

   // List of branches
   TBranch        *b_energy0;   //!
   TBranch        *b_energy1;   //!
   TBranch        *b_energy2;   //!

   datatree(TTree *tree=0);
   virtual ~datatree();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop();
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);

   Long64_t GetEntries();
};

#endif

#ifdef datatree_cxx
datatree::datatree(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      std::cout << "!!! wrong tree!!!" << std::endl;
      return;
   }
   Init(tree);
}

datatree::~datatree()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t datatree::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t datatree::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
   if (!fChain) return -5;
   Long64_t centry = fChain->LoadTree(entry);
   if (centry < 0) return centry;
   if (fChain->GetTreeNumber() != fCurrent) {
      fCurrent = fChain->GetTreeNumber();
      Notify();
   }
   return centry;
}

void datatree::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("energy0", &energy0, &b_energy0);
   fChain->SetBranchAddress("energy1", &energy1, &b_energy1);
   fChain->SetBranchAddress("energy2", &energy2, &b_energy2);
   Notify();
}

Bool_t datatree::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void datatree::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t datatree::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
Long64_t datatree::GetEntries()
{
  return fChain->GetEntriesFast();
}
#endif // #ifdef datatree_cxx
