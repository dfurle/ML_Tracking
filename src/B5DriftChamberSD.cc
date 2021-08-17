//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
/// \file B5DriftChamberSD.cc
/// \brief Implementation of the B5DriftChamber class

#include "B5DriftChamberSD.hh"
#include "B5DriftChamberHit.hh"

#include "G4HCofThisEvent.hh"
#include "G4TouchableHistory.hh"
#include "G4Track.hh"
#include "G4Step.hh"
#include "G4SDManager.hh"
#include "G4ios.hh"
#include <stdio.h>


#include "g4analysis.hh"
#include "G4RunManager.hh"

static FILE* myfile;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B5DriftChamberSD::B5DriftChamberSD(G4String name)
: G4VSensitiveDetector(name), 
  fHitsCollection(nullptr), fHCID(-1)
{
  collectionName.insert("driftChamberColl");
  if(strcmp(this->GetName().c_str(),"chamber1") == 0){
    // myfile = (FILE *) fopen("data.txt","w");
    myfile = (FILE *) fopen("data.csv","w");
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B5DriftChamberSD::~B5DriftChamberSD()
{
  printf("destructing\n");
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void B5DriftChamberSD::Initialize(G4HCofThisEvent* hce){

  fHitsCollection 
    = new B5DriftChamberHitsCollection(SensitiveDetectorName,collectionName[0]);

  if (fHCID<0) { 
     fHCID = G4SDManager::GetSDMpointer()->GetCollectionID(fHitsCollection); 
  }
  hce->AddHitsCollection(fHCID,fHitsCollection);
  // printf("initalize1 %s\n",this->GetName().c_str());
  hits.clear();
  momentum_h = 0;

  auto analysisManager = G4AnalysisManager::Instance();

  HistA = analysisManager->GetH1Id("HistA");
  HistB = analysisManager->GetH1Id("HistB");
  HistAB = analysisManager->GetH2Id("HistAB");
  // printf("init: %s\n",this->GetName().c_str());
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......


G4bool B5DriftChamberSD::ProcessHits(G4Step* step, G4TouchableHistory*){
  auto charge = step->GetTrack()->GetDefinition()->GetPDGCharge();
  if (charge==0.) return true;
  
  auto preStepPoint = step->GetPreStepPoint();

  // step->GetTrack()[0].Get 

  auto touchable = step->GetPreStepPoint()->GetTouchable();
  auto motherPhysical = touchable->GetVolume(1); // mother
  auto copyNo = motherPhysical->GetCopyNo();
  auto a = step->GetTrack()[0].GetPosition().x();
  auto b = step->GetTrack()[0].GetPosition().y();
  auto c = step->GetTrack()[0].GetPosition().z();
  auto angle_a = step->GetTrack()[0].GetMomentumDirection().x();
  auto angle_b = step->GetTrack()[0].GetMomentumDirection().y();
  auto angle_c = step->GetTrack()[0].GetMomentumDirection().z();
  // printf("name: %s\n",this->GetName().c_str());
  // printf("  pos: %f %f %f\n",a,b,c);
  // printf("  ang: %f %f %f\n",angle_a,angle_b,angle_c);
  auto worldPos = preStepPoint->GetPosition();
  auto momentum = preStepPoint->GetMomentum().mag();
  
  auto hit_id = preStepPoint->GetTouchableHandle()->GetCopyNumber(1);
  auto hit_name = preStepPoint->GetTouchableHandle()->GetVolume()->GetName();
  // if(strcmp(hit_name.c_str(),"wirePlane1Physical") == 0)
  // if(strcmp(this->GetName().c_str(),"chamber1") == 0){
    // printf("name:%s|id:%d world pos: %8.2f %6.2f %7.2f | p=%4.2f\n",hit_name.c_str(),hit_id, worldPos.x(),worldPos.y(),worldPos.z(),momentum/CLHEP::GeV);
  momentum_h = momentum/CLHEP::GeV;
  hits.push_back(worldPos);
  // }

  auto localPos 
    = touchable->GetHistory()->GetTopTransform().TransformPoint(worldPos);
  
  auto hit = new B5DriftChamberHit(copyNo);
  hit->SetWorldPos(worldPos);
  hit->SetLocalPos(localPos);
  hit->SetTime(preStepPoint->GetGlobalTime());
  hit->SetMomentum(momentum_h);

  if(strcmp(this->GetName().c_str(),"chamberF") == 0){
    double x_m = preStepPoint->GetMomentum().x();
    double z_m = preStepPoint->GetMomentum().z();
    // double angle = atan2(z_m,x_m);
    double angle = atan2(x_m,z_m);
    hit->SetInitAngle(angle);
    printf("setting init angle %frad, %fdeg\n",angle,angle*(180/M_PI));
  } else {
    hit->SetInitAngle(0);
  }


  
  fHitsCollection->insert(hit);
  
  return true;
}


void B5DriftChamberSD::EndOfEvent(G4HCofThisEvent*HCE){
  // printf(":%s:\n",this->GetName().c_str());
  // printf("cur:");
  // for(int z = 0; z < hits.size(); z++){
  //   printf("  %f,%f,%f\n",hits[z].x(),hits[z].y(),hits[z].z());
  // }
  // printf("\n");
  // printf("hits size:%d\n",hits.size());

  if(strcmp(this->GetName().c_str(),"chamberF") == 0){
    // printf("fin\n");
    return;
  }

  auto analysisManager = G4AnalysisManager::Instance();

  std::vector<double> angles;
  int ctr = 0;
  for(int i = 0; i < hits.size() - 1; i++){
    for(int j = i+1; j < hits.size(); j++){
      auto diff_x = hits[i].x() - hits[j].x();
      auto diff_z = hits[i].z() - hits[j].z();
      // auto angle = diff_z/diff_x;
      auto angle = atan2(diff_z,diff_x);
      auto b = hits[i].z() - (diff_z/diff_x) * hits[i].x();
      analysisManager->FillH1(HistA,angle);
      analysisManager->FillH1(HistB,b);
      analysisManager->FillH2(HistAB,angle,b);
      // 300, -1.8, -1.4
      float gap = 0.4 / 300.;
      float diff = angle + 1.8;
      float id_f = diff / gap;
      // printf("loc id: %f = %d\n",id_f,int(id_f));
      // printf("id: %d, %d = %d\n",i,j,ctr++);
      // printf("  a: %f\n",angle);
      // printf("  b: %f\n",b);
    }
  }

  // printf("fin\n");


  // fprintf((FILE *)myfile,"%.2f",current[0]);
  // for(int z = 1; z < current.size(); z+=3){
  //   // fprintf((FILE *)myfile," %8.2f %6.2f %8.2f",current[z],current[z+1],current[z+2]);
  //   fprintf(myfile,",%.2f,%.2f,%.2f",current[z],current[z+1],current[z+2]);
  // }
  // fprintf(myfile,"\n");
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
