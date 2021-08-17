#define b5_cxx
#include "b5.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>

void b5::Loop()
{
  TFile *myFile = new TFile("events.root","RECREATE");
  // TNtuple *ntuple = new TNtuple("ntuple","Events","x:y");
  TTree* mTree = new TTree("mTree","My Tree");
  TCanvas* c1 = new TCanvas("c1","Canvas");
  // c1->SetCanvasSize(600,900);
  c1->SetWindowSize(1200,800);
  c1->Divide(3, 2);
  // std::vector<std::vector<std::map<std::string,double>>> position;
  std::vector<std::vector<double>> position_x;
  std::vector<std::vector<double>> position_z;
  std::vector<double> momentum;
  std::array<std::vector<int>,302> id_array;

  mTree->Branch("x",&position_x);
  mTree->Branch("z",&position_z);
  mTree->Branch("momentum",&momentum);

  TGraph* graph[3];
  for(int i = 0; i < 3; i++){
    graph[i] = new TGraph();
    graph[i]->GetXaxis()->SetTitle("Z Pos");
    graph[i]->GetYaxis()->SetTitle("X Pos");
    graph[i]->SetMinimum(-1000);
    graph[i]->SetMaximum(1000);
  }


  // Int_t colors[] = {0, 1, 2, 3, 4, 5, 20}; // #colors >= #levels - 1
  // gStyle->SetPalette((sizeof(colors)/sizeof(Int_t)), colors);
  // #levels <= #colors + 1 (notes: +-1.79e308 = +-DBL_MAX; +1.17e-38 = +FLT_MIN)
  // Double_t levels[] = {-1.79e308, 1.17e-38, 0.90, 0.95, 1.00, 1.05, 1.10, 1.79e308};
  // h_ab->SetContour((sizeof(levels)/sizeof(Double_t)), levels);

  TF1* ffit[3];
  for(int i = 0; i < 3; i++){
    ffit[i] = new TF1("ffit1", "pol1");
    ffit[i]->SetLineColor(kBlue);
  }
  if (fChain == 0) return;

  Long64_t nentries = fChain->GetEntriesFast();

  Long64_t nbytes = 0, nb = 0;
  // for (Long64_t jentry=0; jentry<nentries-3;jentry++) {
  for (Long64_t jentry=0; jentry<nentries-1;jentry++) {
  // for (Long64_t jentry=0; jentry<5;jentry++) {
  // for (Long64_t jentry=0; jentry<4;jentry++) {
    printf("size: %lld\n",nentries);
    printf("\n\nLoop: %lld\n",jentry);
    // h_a->Reset();
    // h_b->Reset();
    // h_ab->Reset();
    // for(int i = 0; i < 3; i++){
    //   while(graph[i]->GetN() > 0){
    //     graph[i]->RemovePoint(0);
    //   }
    // }
    position_x.clear();
    position_z.clear();
    momentum.clear();

    for(int i = 0; i < 302; i++){
      id_array[i].clear();
    }

    std::vector<double> vpos_x;
    std::vector<double> vpos_z;
    

    // for (Long64_t gentry=jentry; gentry<jentry+3;gentry++) {
    for (Long64_t gentry=jentry; gentry<jentry+1;gentry++) {
      fChain->GetEntry(gentry);
      printf(":%f\n",InitAngle);
      // momentum.push_back(InitAngle);
      for(int i = 0; i < PositionX->size(); i++){
        vpos_x.push_back(PositionX->at(i));
        vpos_z.push_back(PositionZ->at(i));
        // maybe sort them by position so its harder for model to learn?
      }
    }

    for(int i = 0; i < vpos_x.size()-1 && i < vpos_z.size()-1; i++){
      for(int j = i+1; j < vpos_x.size() && j < vpos_z.size(); j++){
        auto diff_x = vpos_x[j] - vpos_x[i];
        auto diff_z = vpos_z[j] - vpos_z[i];
        auto angle = atan2(diff_x,diff_z);
        auto b = vpos_x[i] - (diff_x/diff_z) * vpos_z[i];

        h_a->Fill(angle);
        h_b->Fill(b);
        h_ab->Fill(angle,b);
      }
    }

    c1->cd(1);
    h_a->Draw();
    c1->cd(2);
    h_b->Draw();
    c1->cd(3);
    h_ab->Draw("colz");

    c1->Modified();
    c1->Update();


    // for(long i = 0; i < 1000; i++) {
    //   gSystem->ProcessEvents();
    //   // usleep(10000);
    //   // usleep(5000);
    //   usleep(100);
    // }
  }
}
