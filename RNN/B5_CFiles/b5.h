//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Tue Aug  3 12:25:28 2021 by ROOT version 6.22/06
// from TTree B5/Hits
// found on file: B5.root
//////////////////////////////////////////////////////////

#ifndef b5_h
#define b5_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.
#include "vector"

class b5 {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

// Fixed size dimensions of array or collections stored in the TTree if any.

   // Declaration of leaf types
   Int_t           Dc1Hits;
   Int_t           Dc2Hits;
   Double_t        ECEnergy;
   Double_t        HCEnergy;
   Double_t        Time1;
   Double_t        Time2;
   vector<double>  *ECEnergyVector;
   vector<double>  *HCEnergyVector;
   vector<double>  *PositionX;
   vector<double>  *PositionY;
   vector<double>  *PositionZ;
   Double_t        Momentum;
   Double_t        InitAngle;

   // List of branches
   TBranch        *b_Dc1Hits;   //!
   TBranch        *b_Dc2Hits;   //!
   TBranch        *b_ECEnergy;   //!
   TBranch        *b_HCEnergy;   //!
   TBranch        *b_Time1;   //!
   TBranch        *b_Time2;   //!
   TBranch        *b_ECEnergyVector;   //!
   TBranch        *b_HCEnergyVector;   //!
   TBranch        *b_PositionX;   //!
   TBranch        *b_PositionY;   //!
   TBranch        *b_PositionZ;   //!
   TBranch        *b_Momentum;   //!
   TBranch        *b_InitAngle;   //!

    // TH1D* h_x;
    // TH1D* h_y;
    // TH1D* h_r;
    // TH3D* h_xyr;

    TH1D* h_a;
    TH1D* h_b;
    TH2D* h_ab;



   b5(TTree *tree=0);
   virtual ~b5();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop();
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
};

#endif

#ifdef b5_cxx
b5::b5(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("B5.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("B5.root");
      }
      f->GetObject("B5",tree);

   }
   Init(tree);

  // h_a = new TH1D("h_a","Angle Of Thing",300,-1.8,-1.4);
  // h_a = new TH1D("h_a","Angle Of Thing",300,-1.8,-1.);
  h_a = new TH1D("h_a","Angle Of Thing",300,-0.1,0.1);
  // h_b = new TH1D("h_b","B intersection",300,-9000,2000);
  h_b = new TH1D("h_b","B intersection",300,-1000,1000);
  // h_ab = new TH2D("h_ab","AB 2d hist",20,-1.8,-1.,20,-9000,2000);
  h_ab = new TH2D("h_ab","AB 2d hist",20,-0.1,0.1,20,-200,200);


  // h_x = new TH1D("h_x","x coords",20,-20000, -1000);
  // h_y = new TH1D("h_y","y coords",20,-4000,-1000);
  // h_r = new TH1D("h_r","radii",300,5000,20000);
  // h_xyr = new TH3D("h_xyr","all combined",20,-20000,-1000, 20,-4000,-1000, 20,5000,20000);
}

b5::~b5()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t b5::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t b5::LoadTree(Long64_t entry)
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

void b5::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set object pointer
   ECEnergyVector = 0;
   HCEnergyVector = 0;
   PositionX = 0;
   PositionY = 0;
   PositionZ = 0;
   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("Dc1Hits", &Dc1Hits, &b_Dc1Hits);
   fChain->SetBranchAddress("Dc2Hits", &Dc2Hits, &b_Dc2Hits);
   fChain->SetBranchAddress("ECEnergy", &ECEnergy, &b_ECEnergy);
   fChain->SetBranchAddress("HCEnergy", &HCEnergy, &b_HCEnergy);
   fChain->SetBranchAddress("Time1", &Time1, &b_Time1);
   fChain->SetBranchAddress("Time2", &Time2, &b_Time2);
   fChain->SetBranchAddress("ECEnergyVector", &ECEnergyVector, &b_ECEnergyVector);
   fChain->SetBranchAddress("HCEnergyVector", &HCEnergyVector, &b_HCEnergyVector);
   fChain->SetBranchAddress("PositionX", &PositionX, &b_PositionX);
   fChain->SetBranchAddress("PositionY", &PositionY, &b_PositionY);
   fChain->SetBranchAddress("PositionZ", &PositionZ, &b_PositionZ);
   fChain->SetBranchAddress("Momentum", &Momentum, &b_Momentum);
   fChain->SetBranchAddress("InitAngle", &InitAngle, &b_InitAngle);
   Notify();
}

Bool_t b5::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void b5::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t b5::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
#endif // #ifdef b5_cxx
