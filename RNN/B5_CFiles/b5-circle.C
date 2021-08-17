#define b5_cxx
#include "b5.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>


bool line_params(double c1[2], double c2[2], double line[2]){
  if(c1[0]-c2[0] == 0 || c1[1]-c2[1] == 0)
    return false;
  auto slope = (c1[1]-c2[1])/(c1[0]-c2[0]);
  auto mid_x = abs(c1[0]-c2[0])/2+min(c1[0],c2[0]);
  auto mid_y = abs(c1[1]-c2[1])/2+min(c1[1],c2[1]);
  auto intercept = (1/slope*mid_x) + mid_y;
  line[0] = (-1/slope);
  line[1] = intercept;
  return true;
}


void b5::Loop(){
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
  std::vector<double> initialAngle;
  std::vector<double> chi_a;
  std::array<std::vector<int>,302> id_array;

  mTree->Branch("x",&position_x);
  mTree->Branch("z",&position_z);
  mTree->Branch("initialAngle",&initialAngle);
  mTree->Branch("chi_a",&chi_a);

  h_xyr->GetXaxis()->SetTitle("X Axis");
  h_xyr->GetYaxis()->SetTitle("Y Axis");
  h_xyr->GetZaxis()->SetTitle("Z Axis");

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
    // ffit[i] = new TF1("ffit1", "pol1");
    ffit[i] = new TF1("ffit1", "pol2");
    ffit[i]->SetLineColor(kBlue);
  }
  if (fChain == 0) return;

  Long64_t nentries = fChain->GetEntriesFast();

  Long64_t nbytes = 0, nb = 0;
  // for (Long64_t jentry=0; jentry<nentries-3;jentry++) {
  // for (Long64_t jentry=0; jentry<nentries-1;jentry++) {
  for (Long64_t jentry=0; jentry<20;jentry++) {
  // for (Long64_t jentry=1; jentry<2;jentry++) {
    printf("size: %lld\n",nentries);
    printf("\n\nLoop: %lld\n",jentry);
    if(jentry == 66){
      continue;
    }
    h_x->Reset();
    h_y->Reset();
    h_r->Reset();
    h_xyr->Reset();
    for(int i = 0; i < 3; i++){
      while(graph[i]->GetN() > 0){
        graph[i]->RemovePoint(0);
      }
    }
    position_x.clear();
    position_z.clear();
    initialAngle.clear();
    chi_a.clear();

    for(int i = 0; i < 302; i++){
      id_array[i].clear();
    }

    std::vector<double> vpos_x;
    std::vector<double> vpos_z;
    

    for (Long64_t gentry=jentry; gentry<jentry+3;gentry++) {
    // for (Long64_t gentry=jentry; gentry<jentry+1;gentry++) {
      fChain->GetEntry(gentry);
      printf(":%f\n",InitAngle);
      initialAngle.push_back(InitAngle);
      for(int i = 0; i < PositionX->size(); i++){
        vpos_x.push_back(PositionX->at(i));
        vpos_z.push_back(PositionZ->at(i));
        // maybe sort them by position so its harder for model to learn?
      }
    }

    printf("\n\npoints size: %d\n",vpos_x.size());

    for(int i = 0; i < vpos_x.size()-2; i++){
      for(int j = i+1; j < vpos_x.size()-1; j++){
        for(int k = j+1; k < vpos_x.size(); k++){
          double coords[3][2];
          coords[0][0] = vpos_x[i];
          coords[0][1] = vpos_z[i];
          coords[1][0] = vpos_x[j];
          coords[1][1] = vpos_z[j];
          coords[2][0] = vpos_x[k];
          coords[2][1] = vpos_z[k];

          double line1[2];
          double line2[2];
          if(!line_params(coords[0],coords[1],line1))
            continue;
          
          if(!line_params(coords[0],coords[2],line2))
            continue;

          // printf("line params: %f,%f | %f,%f\n",line1[0],line1[1],line2[0],line2[1]);

          double point_x = (line2[1]-line1[1])/(line1[0]-line2[0]);
          double point_y = line1[0] * point_x + line1[1];
          double rsq = ((point_x-coords[0][0])*(point_x-coords[0][0])) + ((point_y-coords[0][1])*(point_y-coords[0][1]));
          if(rsq <= 0)
            continue;
          double radius = sqrt(rsq);

          // printf("cir: %f,%f = %f\n",point_x,point_y,radius);

          if(point_y>-1000){
            continue;
          }
          if(point_x>-1000){
            continue;
          }

          h_x->Fill(point_x);
          h_y->Fill(point_y);
          // h_r->Fill(1./radius);
          h_r->Fill(radius);
          h_xyr->Fill(point_x,point_y,radius);

          if(radius>12000){
            printf("point: %f, %f = %f\n",point_x,point_y,radius);
          }

          // int bin_n = h_r->FindBin(1./radius);
          int bin_n = h_r->FindBin(radius);
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
            printf("size invalid!! id=%d, a=%f\n",bin_n,radius);
          }
        }
      }
    }

    printf("----------\n");
    printf("|id_array|\n");
    printf("----------\n");
    for(int i = 0 ; i < 302; i++){
      if(id_array[i].size() > 0){
        printf("bin:%d->%lu\n",i,id_array[i].size());
        for(int j = 0; j < id_array[i].size(); j++){
          printf(" id:%d->%d\n",j,id_array[i][j]);
        }
      }
    }
    printf("\n\n");

    std::vector<std::pair<int,double>> radii;

    int counter = h_r->FindFirstBinAbove(5, 1, 1, 300);

    while(counter < 301){
      int size = h_r->GetBinContent(counter);
      // printf("val: %d\n",size);
      int nIDs = id_array[counter].size();
      // printf("size: %d\n",nIDs);

      if(size >= 14 && size <= 16){
        printf("size between 14 and 16 @%d %d\n",counter,nIDs);
        std::vector<double> posx;
        std::vector<double> posz;
        for(int i = 0; i < nIDs; i++){
          // printf(" %d->%d\n",i,id_array[counter][i]);
          posx.push_back(vpos_x[id_array[counter][i]]);
          posz.push_back(vpos_z[id_array[counter][i]]);
        }
        position_x.push_back(posx);
        position_z.push_back(posz);

        double value = h_r->GetBinCenter(counter);
        radii.push_back(std::pair<int,double>(counter,value));
      } else if (size >= 6) {
        // printf("counter: %d\n",counter);
        double left   = h_r->GetBinCenter(counter-1);
        double center = h_r->GetBinCenter(counter);
        double right  = h_r->GetBinCenter(counter+1);
        int sleft   = h_r->GetBinContent(counter-1);
        int scenter = h_r->GetBinContent(counter);
        int sright  = h_r->GetBinContent(counter+1);
        if(sright > scenter || sleft > scenter){
          int next_counter = h_r->FindFirstBinAbove(5, 1, counter+1, 300);
          if(next_counter == -1)
            break;
          else
            counter = next_counter;
          continue;
        }
        printf("size above 6 @%d: l:%d c:%d s:%d\n",counter, sleft, scenter, sright);
        std::vector<double> posx;
        std::vector<double> posz;
        std::vector<int> ids_added;
        for(int c = -1; c <= 1; c++){
          for(int i = 0; i < id_array[counter+c].size(); i++){
            // printf(" %d->%d\n",i,id_array[counter+c][i]);
            auto it = std::find(ids_added.begin(),ids_added.end(),id_array[counter+c][i]);
            if(it == ids_added.end()){
              ids_added.push_back(id_array[counter+c][i]);
              // printf(" :%d %d\n",i,id_array[counter+c][i]);
              posx.push_back(vpos_x[id_array[counter+c][i]]);
              posz.push_back(vpos_z[id_array[counter+c][i]]);
            }
          }
        }
        position_x.push_back(posx);
        position_z.push_back(posz);

        double value = (left*sleft + center*scenter + right*sright)/(sleft+scenter+sright);
        radii.push_back(std::pair<int,double>(counter,value));
      }

      int next_counter = h_r->FindFirstBinAbove(5, 1, counter+1, 300);
      if(next_counter == -1)
        break;
      else
        counter = next_counter;
    }

    printf("position:\n");
    for(int a = 0; a < position_x.size(); a++){
      printf("id:%d\n",a);
      for(int i = 0; i < position_x[a].size(); i++){
        printf(" i:%d->%f,%f\n",i,position_x[a][i],position_z[a][i]);
      }
    }

    // printf("tracks found at:\n");
    // for(int a = 0; a < radii.size() && a < position_x.size(); a++){
    //   printf(" bin:%d with angle %f\n",radii[a].first,radii[a].second);
    //   for(int i = 0; i < position_x[a].size(); i++){
    //     // graph[a]->SetPoint(graph[a]->GetN(), position[a][i]["x"],position[a][i]["z"]);
    //     graph[a]->SetPoint(graph[a]->GetN(), position_z[a][i],position_x[a][i]);
    //   }
    // }


    for(int i = 0; i < 20; i++){
      for(int j = 0; j < 20; j++){
        for(int k = 0; k < 20; k++){
          if(h_xyr->GetBinContent(i,j,k) > 30){
            printf("bin: %d, %d, %d = %f\n",i,j,k,h_xyr->GetBinContent(i,j,k));
          }
        }
      }
    }







    c1->cd(1);
    h_x->Draw();
    c1->cd(2);
    h_y->Draw();
    c1->cd(3);
    h_r->Draw();
    c1->cd(4);
    h_xyr->Draw("lego2");
    // h_xyr->Draw("lego");

    // for(int i = 0; i < 3; i++){
    // for(int i = 0; i < 1; i++){
    //   // ffit[i]->Clear();
    //   graph[i]->Fit(ffit[i], "+rob=0.75");

    //   Double_t p1 = ffit[i]->GetParameter(1);
    //   Double_t p0 = ffit[i]->GetParameter(0);
    //   chi_a.push_back(p1);
    //   TFormula* f = ffit[i]->GetFormula();
    //   f->Print();

    //   // ffit[i]->Update();
    //   c1->cd(5+i);

    //   graph[i]->SetMinimum(-1000);
    //   graph[i]->SetMaximum(1000);
      
    //   // ffit[i]->Draw();
    //   graph[i]->Draw("A*");
    // }
    
    c1->Modified();
    c1->Update();


    // for(int i = 0; i < 3; i++){
    //   Double_t p1_1 = ffit1->GetParameter(1);
    //   Double_t p0_1 = ffit1->GetParameter(0);
    // }

    printf("\n");

    mTree->Fill();
    // myFile->Write();

    for(long i = 0; i < 1000; i++) {
    // for(long i = 0; i < 100; i++) {
      gSystem->ProcessEvents();
      // usleep(10000);
      // usleep(5000);
      usleep(1000);
      // usleep(10);
    }
  }
  myFile->Write();
}
