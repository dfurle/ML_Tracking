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
    // graph[i]->GetXaxis()->SetTitle("X Pos");
    // graph[i]->GetYaxis()->SetTitle("Z Pos");
    graph[i]->GetXaxis()->SetTitle("Z Pos");
    graph[i]->GetYaxis()->SetTitle("X Pos");
    // graph[i]->SetMinimum(-1500);
    // graph[i]->SetMaximum(1500);
    graph[i]->SetMinimum(-1000);
    graph[i]->SetMaximum(1000);
    // graph[i]->GetHistogram()->SetMinimum(-1000);
    // graph[i]->GetHistogram()->SetMaximum(1000);
    // graph[i]->GetXaxis()->SetRangeUser(-1000,1000);
    // graph[i]->GetXaxis()->SetRangeUser(0.2,0.4);
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


    h_a->Reset();
    h_b->Reset();
    h_ab->Reset();
    for(int i = 0; i < 3; i++){
      while(graph[i]->GetN() > 0){
        graph[i]->RemovePoint(0);
      }
    }



    position_x.clear();
    position_z.clear();

    for(int i = 0; i < 302; i++){
      id_array[i].clear();
    }

    std::vector<double> vpos_x;
    std::vector<double> vpos_z;

    for (Long64_t gentry=jentry; gentry<jentry+3;gentry++) {
      fChain->GetEntry(gentry);
      printf(":%f\n",InitAngle);
      momentum.push_back(InitAngle);
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
        if(bin_n >= 0 && bin_n < 302){
          std::vector<int> vec = id_array[bin_n];
          // std::vector<int>::iterator i_it = std::find(vec.begin(),vec.end(),i);
          // std::vector<int>::iterator j_it = std::find(vec.begin(),vec.end(),j);
          auto i_it = std::find(vec.begin(),vec.end(),i);
          auto j_it = std::find(vec.begin(),vec.end(),j);
          if(i_it == vec.end()){
            id_array[bin_n].push_back(i);
            // printf("added id: %d %d\n",bin_n,i);
          } else {
            // printf("found: %d@%ld\n",*i_it,i_it-vec.begin());
          }
          if(j_it == vec.end()){
            id_array[bin_n].push_back(j);
            // printf("added id: %d %d\n",bin_n,j);
          } else {
            // printf("found: %d@%ld\n",*j_it,j_it-vec.begin());
          }
        } else {
          printf("size invalid!! id=%d, a=%f\n",bin_n,angle);
        }
      }
    }

    // printf("----------\n");
    // printf("|id_array|\n");
    // printf("----------\n");
    // for(int i = 0 ; i < 302; i++){
    //   printf("bin:%d->%lu\n",i,id_array[i].size());
    //   for(int j = 0; j < id_array[i].size(); j++){
    //     printf(" id:%d->%d\n",j,id_array[i][j]);
    //   }
    // }
    // printf("\n\n");

    std::vector<std::pair<int,double>> angles;

    int counter = h_a->FindFirstBinAbove(5, 1, 1, 300);

    while(counter < 301){
      int size = h_a->GetBinContent(counter);
      // printf("val: %d\n",size);
      int nIDs = id_array[counter].size();
      // printf("size: %d\n",nIDs);

      if(size >= 14 && size <= 16){
        printf("size between 14 and 16 %d\n",nIDs);
        std::vector<double> posx;
        std::vector<double> posz;
        for(int i = 0; i < nIDs; i++){
          // printf(" %d->%d\n",i,id_array[counter][i]);
          posx.push_back(vpos_x[id_array[counter][i]]);
          posz.push_back(vpos_z[id_array[counter][i]]);
        }
        position_x.push_back(posx);
        position_z.push_back(posz);

        double value = h_a->GetBinCenter(counter);
        angles.push_back(std::pair<int,double>(counter,value));
      } else if (size >= 6) {
        // printf("counter: %d\n",counter);
        double left   = h_a->GetBinCenter(counter-1);
        double center = h_a->GetBinCenter(counter);
        double right  = h_a->GetBinCenter(counter+1);
        int sleft   = h_a->GetBinContent(counter-1);
        int scenter = h_a->GetBinContent(counter);
        int sright  = h_a->GetBinContent(counter+1);
        printf("size above 6: l:%d c:%d s:%d\n",sleft, scenter, sright);
        if(sright > scenter || sleft > scenter){
          int next_counter = h_a->FindFirstBinAbove(5, 1, counter+1, 300);
          if(next_counter == -1)
            break;
          else
            counter = next_counter;
          continue;
        }
        std::vector<double> posx;
        std::vector<double> posz;
        for(int c = -1; c <= 1; c++){
          for(int i = 0; i < id_array[counter+c].size(); i++){
            // printf(" %d->%d\n",i,id_array[counter+c][i]);
            posx.push_back(vpos_x[id_array[counter+c][i]]);
            posz.push_back(vpos_z[id_array[counter+c][i]]);
          }
        }
        position_x.push_back(posx);
        position_z.push_back(posz);

        double value = (left*sleft + center*scenter + right*sright)/(sleft+scenter+sright);
        angles.push_back(std::pair<int,double>(counter,value));
      }

      int next_counter = h_a->FindFirstBinAbove(5, 1, counter+1, 300);
      if(next_counter == -1)
        break;
      else
        counter = next_counter;
    }

    // printf("positionx:\n");
    // for(int a = 0; a < position_x.size(); a++){
    //   printf("id:%d\n",a);
    //   for(int i = 0; i < position_x[a].size(); i++){
    //     printf(" i:%d->%f,%f\n",i,position_x[a][i],position_z[a][i]);
    //   }
    // }

    printf("tracks found at:\n");
    for(int a = 0; a < angles.size() && a < position_x.size(); a++){
      printf(" bin:%d with angle %f\n",angles[a].first,angles[a].second);
      for(int i = 0; i < position_x[a].size(); i++){
        // graph[a]->SetPoint(graph[a]->GetN(), position[a][i]["x"],position[a][i]["z"]);
        graph[a]->SetPoint(graph[a]->GetN(), position_z[a][i],position_x[a][i]);
      }
    }


    c1->cd(1);
    h_a->Draw();
    c1->cd(2);
    h_b->Draw();
    c1->cd(3);
    h_ab->Draw("colz");

    for(int i = 0; i < 3; i++){
      ffit[i]->Clear();
      graph[i]->Fit(ffit[i], "+rob=0.75");
      c1->cd(4+i);

      graph[i]->SetMinimum(-1000);
      graph[i]->SetMaximum(1000);
      
      graph[i]->Draw("A*");
      ffit[i]->Draw("same");
    }
    
    c1->Modified();
    c1->Update();


    // for(int i = 0; i < 3; i++){
    //   Double_t p1_1 = ffit1->GetParameter(1);
    //   Double_t p0_1 = ffit1->GetParameter(0);
    // }

    printf("\n");

    mTree->Fill();
    myFile->Write();

    for(long i = 0; i < 1000; i++) {
      gSystem->ProcessEvents();
      usleep(10000);
    }
  }
}
