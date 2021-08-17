#define b5_cxx
#include "b5.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>

void b5::Loop()
{

  TCanvas* c1 = new TCanvas("c1","Canvas");
  // c1->SetCanvasSize(600,900);
  c1->SetWindowSize(600,900);
  c1->Divide(2, 3);
  double pos_x[3][6];
  double pos_z[3][6];
  std::array<std::vector<int>,300> id_array;
  TF1* ffit1 = new TF1("ffit1", "pol1", -600, -300);
  TF1* ffit2 = new TF1("ffit1", "pol1", -600, -300);
  TF1* ffit3 = new TF1("ffit1", "pol1", -600, -300);

  //   In a ROOT session, you can do:
  //      root> .L b5.C
  //      root> b5 t
  //      root> t.GetEntry(12); // Fill t data members with entry number 12
  //      root> t.Show();       // Show values of entry 12
  //      root> t.Show(16);     // Read and show values of entry 16
  //      root> t.Loop();       // Loop on all entries
  //

  //     This is the loop skeleton where:
  //    jentry is the global entry number in the chain
  //    ientry is the entry number in the current Tree
  //  Note that the argument to GetEntry must be:
  //    jentry for TChain::GetEntry
  //    ientry for TTree::GetEntry and TBranch::GetEntry
  //
  //       To read only selected branches, Insert statements like:
  // METHOD1:
  //    fChain->SetBranchStatus("*",0);  // disable all branches
  //    fChain->SetBranchStatus("branchname",1);  // activate branchname
  // METHOD2: replace line
  //    fChain->GetEntry(jentry);       //read all branches
  //by  b_branchname->GetEntry(ientry); //read only this branch
  if (fChain == 0) return;

  Long64_t nentries = fChain->GetEntriesFast();

  Long64_t nbytes = 0, nb = 0;
  for (Long64_t jentry=0; jentry<nentries-3;jentry++) {
    printf("size: %lld\n",nentries);
    // Long64_t ientry = LoadTree(jentry);
    // if (ientry < 0) break;
    // nb = fChain->GetEntry(jentry);   nbytes += nb;
    // if (Cut(ientry) < 0) continue;
    printf("\n\nLoop: %lld\n",jentry);

    TGraph* graph1;
    TGraph* graph2;
    TGraph* graph3;

    h_a->Reset();
    h_b->Reset();
    h_ab->Reset();

    for(int i = 0; i < 300; i++){
      id_array[i].clear();
    }

    std::vector<double> vpos_x;
    std::vector<double> vpos_z;

    for (Long64_t gentry=jentry; gentry<jentry+3;gentry++) {
      fChain->GetEntry(gentry);
      for(int i = 0; i < PositionX->size(); i++){
        vpos_x.push_back(PositionX->at(i));
        vpos_z.push_back(PositionZ->at(i));
        // maybe sort them by position so its harder for model to learn?
      }
    }
    for(int i = 0; i < vpos_x.size()-1 && i < vpos_z.size()-1; i++){
      for(int j = i+1; j < vpos_x.size() && j < vpos_z.size(); j++){
        auto diff_x = vpos_x[i] - vpos_x[j];
        auto diff_z = vpos_z[i] - vpos_z[j];
        auto angle = atan2(diff_z,diff_x);
        auto b = vpos_z[i] - (diff_z/diff_x) * vpos_x[i];

        h_a->Fill(angle);
        h_b->Fill(b);
        h_ab->Fill(angle,b);

        int bin_n = h_a->FindBin(angle);
        if(bin_n >= 0 && bin_n < 300){
          std::vector<int> vec = id_array[bin_n];
          auto i_it = std::find(vec.begin(),vec.end(),i);
          auto j_it = std::find(vec.begin(),vec.end(),j);
          if(i_it != vec.end()){
            id_array[bin_n].push_back(i);
            printf("added id: %d %d\n",bin_n,i);
          }
          if(j_it != vec.end()){
            id_array[bin_n].push_back(j);
            printf("added id: %d %d\n",bin_n,j);
          }
        } else {
          printf("size invalid!! id=%d, a=%f\n",bin_n,angle);
        }
      }
    }

    std::vector<std::pair<int,double>> angles;

    for(int a = 0; a < 300; a++){
      // printf("nbin: %d = %f\n",a,h_a->GetBinCenter(a));
      for(int z = 0; z < id_array[a].size(); z++){
        // printf(" id: %d\n",id_array[a][z]);
      }
      int scenter = id_array[a].size();
      if(scenter >= 14 && scenter <= 16){
        double value = h_a->GetBinCenter(a);
        // printf("track at angle %f\n",value);
        angles.push_back(std::pair<int,double>(a,value));
      }else if(scenter >= 6){
        double left   = h_a->GetBinCenter(a-1);
        double center = h_a->GetBinCenter(a);
        double right  = h_a->GetBinCenter(a+1);
        int sleft   = id_array[a-1].size();
        int sright  = id_array[a+1].size();
        // if(sleft > (15/2 - 1))
        if(sright > scenter){
          continue;
        }
        if(sleft > scenter){
          continue;
        }
        double value = (left*sleft + center*scenter + right*sright)/(sleft+scenter+sright);
        // printf("track at angle %f\n",value);
        angles.push_back(std::pair<int,double>(a,value));

      }
    }
    printf("\n");

    printf("tracks found at:\n");
    for(int a = 0; a < angles.size(); a++){
      printf(" bin:%d with angle %f\n",angles[a].first,angles[a].second);
    }









    int ctr = 0;
    int point = 0;
    for (Long64_t gentry=jentry; gentry<jentry+3;gentry++) {
      // Long64_t ientry = LoadTree(gentry);
      // if (ientry < 0) break;
      nb = fChain->GetEntry(gentry);
      nbytes += nb;

      int inner_ctr = 0;
      // for(int i = 0; i < PositionX->size() - 1; i++, point++){
      //   for(int j = i+1; j < PositionX->size(); j++, ctr++, inner_ctr++){
      //     auto diff_x = PositionX->at(i) - PositionX->at(j);
      //     auto diff_z = PositionZ->at(i) - PositionZ->at(j);
      //     // auto angle = diff_z/diff_x;
      //     auto angle = atan2(diff_z,diff_x);
      //     auto b = PositionZ->at(i) - (diff_z/diff_x) * PositionX->at(i);
      //     // h_a->Fill(angle);
      //     // h_b->Fill(b);
      //     // h_ab->Fill(angle,b);
          
      //     // int bin_n = h_a->FindBin(angle);
      //     // if(bin_n >= 0 && bin_n < 300){
      //     //   id_array[bin_n].push_back(point);
      //     //   // id_array[bin_n].push_back(ctr);
      //     //   printf("added id: %d %d\n",bin_n,point);
      //     // }

      //     // printf("id: %d,%d = %02d,%02d\n",i,j,inner_ctr,ctr);
      //     // printf("id: %d, %d = %d , %d\n",i,j,inner_ctr,ctr);
      //     // printf("  a: %f\n",angle);
      //     // printf("  b: %f\n",b);
      //   }
      // }

      for(int i = 0; i < PositionX->size(); i++){
        pos_x[gentry-jentry][i] = PositionX->at(i);
        pos_z[gentry-jentry][i] = PositionZ->at(i);
        printf("id: %d\n",i);
        printf("  x: %f\n",pos_x[gentry-jentry][i]);
        printf("  z: %f\n",pos_z[gentry-jentry][i]);
      }
    }
    graph1 = new TGraph(6,pos_x[0],pos_z[0]);
    graph2 = new TGraph(6,pos_x[1],pos_z[1]);
    graph3 = new TGraph(6,pos_x[2],pos_z[2]);
    ffit1->SetLineColor(kBlue);
    ffit2->SetLineColor(kBlue);
    ffit3->SetLineColor(kBlue);
    graph1->Fit(ffit1, "+rob=0.75");
    graph2->Fit(ffit2, "+rob=0.75");
    graph3->Fit(ffit3, "+rob=0.75");
    Double_t p1_1 = ffit1->GetParameter(1);
    Double_t p0_1 = ffit1->GetParameter(0);
    Double_t p1_2 = ffit2->GetParameter(1);
    Double_t p0_2 = ffit2->GetParameter(0);
    Double_t p1_3 = ffit3->GetParameter(1);
    Double_t p0_3 = ffit3->GetParameter(0);

    c1->cd(1);
    h_a->Draw();
    c1->cd(2);
    h_b->Draw();
    c1->cd(3);
    h_ab->Draw("colz");

    c1->cd(4);
    graph1->Draw("A*");
    graph1->GetXaxis()->SetTitle("X Pos");
    graph1->GetYaxis()->SetTitle("Z Pos");
    ffit1->Draw("same");

    c1->cd(5);
    graph2->Draw("A*");
    graph2->GetXaxis()->SetTitle("X Pos");
    graph2->GetYaxis()->SetTitle("Z Pos");
    ffit2->Draw("same");

    c1->cd(6);
    graph3->Draw("A*");
    graph3->GetXaxis()->SetTitle("X Pos");
    graph3->GetYaxis()->SetTitle("Z Pos");
    ffit3->Draw("same");
    
    c1->Modified();
    c1->Update();

    printf("\n");

    std::vector<std::pair<int,double>> angles;

    for(int a = 0; a < 300; a++){
      // printf("nbin: %d = %f\n",a,h_a->GetBinCenter(a));
      for(int z = 0; z < id_array[a].size(); z++){
        // printf(" id: %d\n",id_array[a][z]);
      }
      int scenter = id_array[a].size();
      if(scenter >= 14 && scenter <= 16){
        double value = h_a->GetBinCenter(a);
        // printf("track at angle %f\n",value);
        angles.push_back(std::pair<int,double>(a,value));
      }else if(scenter >= 6){
        double left   = h_a->GetBinCenter(a-1);
        double center = h_a->GetBinCenter(a);
        double right  = h_a->GetBinCenter(a+1);
        int sleft   = id_array[a-1].size();
        int sright  = id_array[a+1].size();
        // if(sleft > (15/2 - 1))
        if(sright > scenter){
          continue;
        }
        if(sleft > scenter){
          continue;
        }
        double value = (left*sleft + center*scenter + right*sright)/(sleft+scenter+sright);
        // printf("track at angle %f\n",value);
        angles.push_back(std::pair<int,double>(a,value));

      }
    }
    printf("\n");

    printf("tracks found at:\n");
    for(int a = 0; a < angles.size(); a++){
      printf(" bin:%d with angle %f\n",angles[a].first,angles[a].second);
    }


    for(long i = 0; i < 1000; i++) {
      gSystem->ProcessEvents();
      usleep(10000);
    }
    delete graph1;
    delete graph2;
    delete graph3;
  }
}
