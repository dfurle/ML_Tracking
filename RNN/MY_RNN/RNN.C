

void RNN() {

  int ntime = 10;
  int ninput = 30;
  int batchSize = 100;
  int maxepochs = 20;

  int nTotEvts = 10000; // total events to be generated for signal or background

  TMVA::PyMethodBase::PyInitialize();

  auto inputFile = TFile::Open("time_data_t10_d30.root");
  TString outfileName("data_RNN_CPU.root");
  TFile* outputFile = TFile::Open(outfileName, "RECREATE");

  // TMVA::Factory* factory = new TMVA::Factory("TMVAClassification",
  //                                            outputFile,
  //                                            "!V:!Silent:Color:DrawProgressBar:Transformations=None:!Correlations:"
  //                                            "AnalysisType=Classification:ModelPersistence");

  TMVA::Factory* factory = new TMVA::Factory("TMVAClassificationM",
                                             outputFile,
                                             "!V:!Silent:Color:DrawProgressBar:Transformations=None:!Correlations:"
                                             "AnalysisType=Classification:ModelPersistence");

  TMVA::DataLoader* dataloader = new TMVA::DataLoader("dataset");
  // dataloader->AddVariable("var");

  TTree* signalTree = (TTree*)inputFile->Get("sgn");
  TTree* background = (TTree*)inputFile->Get("bkg");


  TFile inputFile2("events.root");
  TTreeReader myReader("mTree",&inputFile2);
  TTreeReaderValue<std::vector<std::vector<double>>> myVectorX(myReader,"x");
  TTreeReaderValue<std::vector<std::vector<double>>> myVectorZ(myReader,"z");
  std::vector<std::vector<double>> b_x;  
  std::vector<std::vector<double>> b_z;

  unsigned int evtCounter = 0;
  while (myReader.Next()){
    cout << "Event " << evtCounter++ << endl;
    for (auto&& value : *myVectorX){
      for (auto&& value : value){
        cout << "x: " << value << " ";
      }
      cout << endl;
    }
    for (auto&& value : *myVectorZ){
      for (auto&& a : value){
        cout << "z: " << a << " ";
      }
      cout << endl;
    }
  }


  const int nvar = ninput * ntime;

  /// add variables - use new AddVariablesArray function
  for (auto i = 0; i < ntime; i++) {
    dataloader->AddVariablesArray(Form("vars_time%d", i), ninput);
  }

  dataloader->AddSignalTree(signalTree, 1.0);
  dataloader->AddBackgroundTree(background, 1.0);

  // check given input
  auto& datainfo = dataloader->GetDataSetInfo();
  auto vars = datainfo.GetListOfVariables();
  std::cout << "number of variables is " << vars.size() << std::endl;
  for (auto& v : vars){
    std::cout << v << std::endl;
  }
    
  std::cout << std::endl;

  int nTrainSig = 0.8 * nTotEvts;
  int nTrainBkg = 0.8 * nTotEvts;

  // auto a = TMVA::Interface::Keras<float>("model.h5");

  // build the string options for DataLoader::PrepareTrainingAndTestTree
  TString prepareOptions = TString::Format("nTrain_Signal=%d:nTrain_Background=%d:SplitMode=Random:SplitSeed=100:NormMode=NumEvents:!V:!CalcCorrelations", nTrainSig, nTrainBkg);

  // Apply additional cuts on the signal and background samples (can be different)
  TCut mycuts = ""; // for example: TCut mycuts = "abs(var1)<0.5 && abs(var2-0.5)<1";
  TCut mycutb = "";

  dataloader->PrepareTrainingAndTestTree(mycuts, mycutb, prepareOptions);

  TString inputLayoutString = TString::Format("InputLayout=%d|%d", ntime, ninput);

  /// Define RNN layer layout
  ///  it should be   LayerType (RNN or LSTM or GRU) |  number of units | number of inputs | time steps | remember output (typically no=0 | return full sequence
  TString rnnLayout = TString::Format("LSTM|10|%d|%d|0|1", ninput, ntime);

  /// add after RNN a reshape layer (needed top flatten the output) and a dense layer with 64 units and a last one
  /// Note the last layer is linear because  when using Crossentropy a Sigmoid is applied already
  TString layoutString = TString("Layout=") + rnnLayout + TString(",RESHAPE|FLAT,DENSE|64|TANH,LINEAR");

  /// Defining Training strategies. Different training strings can be concatenate. Use however only one
  TString trainingString1 = TString::Format("LearningRate=1e-3,Momentum=0.0,Repetitions=1,"
                                            "ConvergenceSteps=5,BatchSize=%d,TestRepetitions=1,"
                                            "WeightDecay=1e-2,Regularization=None,MaxEpochs=%d,"
                                            "Optimizer=ADAM,DropConfig=0.0+0.+0.+0.",
                                            batchSize,
                                            maxepochs);

  TString trainingStrategyString("TrainingStrategy=");
  trainingStrategyString += trainingString1; // + "|" + trainingString2

  /// Define the full RNN Noption string adding the final options for all network
  TString rnnOptions("!H:V:ErrorStrategy=CROSSENTROPY:VarTransform=None:"
                     "WeightInitialization=XAVIERUNIFORM:ValidationSize=0.2:RandomSeed=1234");

  rnnOptions.Append(":");
  rnnOptions.Append(inputLayoutString);
  rnnOptions.Append(":");
  rnnOptions.Append(layoutString);
  rnnOptions.Append(":");
  rnnOptions.Append(trainingStrategyString);
  rnnOptions.Append(":");
  rnnOptions.Append("Architecture=CPU");

  TString rnnName = "TMVA_LSTM";
  factory->BookMethod(dataloader, TMVA::Types::kDL, rnnName, rnnOptions);

  printf("finished booking\n");

  TString modelName = TString::Format("model.h5");
  TString trainedModelName = TString::Format("model_trained.h5");






  if(gSystem->AccessPathName(modelName)){
    printf("AAAAAAA MODELLLLLL\n");
  } else {
    // book PyKeras method only if Keras model could be created
    printf("AAAA LOADING MODELLLL\n");
    Info("TMVA_RNN_Classification", "Booking Keras LSTM model");
    factory->BookMethod(dataloader, TMVA::Types::kPyKeras, "PyKeras_LSTM",
                        TString::Format("!H:!V:VarTransform=None:FilenameModel=%s:"
                                        "FilenameTrainedModel=%s:GpuOptions=allow_growth=True:"
                                        "NumEpochs=%d:BatchSize=%d",
                                        modelName.Data(), trainedModelName.Data(), maxepochs, batchSize));
  }



  // factory->BookMethod(dataloader,
  //                     TMVA::Types::kBDT,
  //                     "BDTG",
  //                     "!H:!V:NTrees=100:MinNodeSize=2.5%:BoostType=Grad:Shrinkage=0.10:UseBaggedBoost:"
  //                     "BaggedSampleFraction=0.5:nCuts=20:"
  //                     "MaxDepth=2");

  factory->TrainAllMethods();
}