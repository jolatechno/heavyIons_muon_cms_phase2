#include <iostream>
#include <thread>
#include <utility>

#include <TClass.h>
#include <TClonesArray.h>
#include <TDataMember.h>
#include <TDictionary.h>
#include <TTreeReaderValue.h>
#include <TSystem.h>
#include <TROOT.h>
#include <TCanvas.h>
#include <TH2.h>
#include <TFile.h>
#include <TClonesArray.h>
#include <TLorentzVector.h>
#include <TTreeReader.h>
#include <TTreeReaderArray.h>
#include <TTree.h>
#include <TTree.h>
 
#include <TMVA/Factory.h>
#include <TMVA/DataLoader.h>
#include <TMVA/DataSetInfo.h>
#include <TMVA/Config.h>
#include <TMVA/MethodDL.h>
#include <TMVA/Tools.h>

#include <ROOT/TTreeProcessorMT.hxx>
#include <ROOT/TThreadedObject.hxx>

void train_TMVA_from_filename(const char *fname)  {
    TMVA::Tools::Instance();

    // Choose methods
    std::map<std::string,int> Use;
    Use["BDTG"] = 1; // uses Gradient Boost

    std::cout << std::endl;
    std::cout << "==> Start TMVAClassification" << std::endl;

    TFile *input(0);
    if (!gSystem->AccessPathName(fname)) {
        input = TFile::Open(fname); // check if file in local directory exists
    }
    if (!input) {
        std::cout << "ERROR: could not open data file" << std::endl;
        exit(1);
    }
    std::cout << "--- TMVAClassification       : Using input file: " << input->GetName() << std::endl;


    TTree *signalTree_train     = (TTree*)input->Get("trueTree_train");
    TTree *signalTree_test      = (TTree*)input->Get("trueTree_test");
    TTree *backgroundTree_train = (TTree*)input->Get("fakeTree_train");
    TTree *backgroundTree_test  = (TTree*)input->Get("fakeTree_test");

    const char* outfileName = "tmva_trained.root" ;
    TFile* outputFile = TFile::Open(outfileName, "RECREATE");

    TMVA::Factory *factory = new TMVA::Factory("TMVAClassification", outputFile,
                                            "!V:!Silent:Color:!DrawProgressBar:Transformations=I;D;P;G,D:AnalysisType=Classification" );

    TMVA::DataLoader *dataloader=new TMVA::DataLoader("dataloader");
    dataloader->AddVariable("Reco_mu_nMuValHits", 'F' /*'I'*/);
    dataloader->AddVariable("Reco_mu_nTrkHits", 'F' /*'I'*/);
    dataloader->AddVariable("Reco_mu_nPixValHits", 'F' /*'I'*/);
    dataloader->AddVariable("Reco_mu_localChi2", 'F');
 // dataloader->AddVariable("Reco_mu_normChi2", 'F');
    dataloader->AddVariable("Reco_mu_pt", 'F');
    dataloader->AddVariable("Reco_mu_eta", 'F');
    dataloader->AddVariable("Reco_mu_dxy", 'F');
    dataloader->AddVariable("Reco_mu_dz", 'F');
    dataloader->AddVariable("nPV", 'F');
 // dataloader->AddVariable("Reco_mu_highPurity", 'F' /*'I'*/);
    dataloader->AddVariable("Reco_mu_nMatches", 'F' /*'I'*/);
 // dataloader->AddVariable("Reco_muGEMquality", 'F' /*'I'*/);

    Double_t signalWeight = 1.0;
    Double_t backgroundWeight = 1.0;

    dataloader->AddSignalTree    (signalTree_train,     signalWeight,     "Training");
    dataloader->AddSignalTree    (signalTree_test,      signalWeight,     "Test");
    dataloader->AddBackgroundTree(backgroundTree_train, backgroundWeight, "Training");
    dataloader->AddBackgroundTree(backgroundTree_test,  backgroundWeight, "Test");


    TCut mycuts = "";
    TCut mycutb = mycuts;


    Int_t n_sig_train = int(signalTree_train->GetEntries(mycuts));
    Int_t n_sig_test  = int(signalTree_test->GetEntries(mycuts));
    Int_t n_bkg_train = int(backgroundTree_train->GetEntries(mycuts));
    Int_t n_bkg_test  = int(backgroundTree_test->GetEntries(mycuts));

    std::stringstream text; 
    text << "nTrain_Signal=" << n_sig_train; 
    text << ":nTrain_Background=" << n_bkg_train;
    text << ":nTest_Signal=" << n_sig_test;
    text << ":nTest_Background=" << n_bkg_test;
    text << ":SplitMode=Random:NormMode=NumEvents:!V";

    dataloader->PrepareTrainingAndTestTree(mycuts, mycutb, text.str().c_str());

    // Book methods
    if (Use["BDTG"]) // Gradient Boost
        factory->BookMethod( dataloader, TMVA::Types::kBDT, "BDTG",
            "!H:!V:NTrees=1000:MinNodeSize=2.5%:BoostType=Grad:Shrinkage=0.10:UseBaggedBoost:BaggedSampleFraction=0.5:nCuts=20:MaxDepth=8" );

    // Train
    std::cout << std::endl << "=================================" << std::endl;
    std::cout << "============= Train =============" << std::endl;
    std::cout << "=================================" << std::endl << std::endl;;

    factory->TrainAllMethods();
    

    std::cout << std::endl << "=================================" << std::endl;
    std::cout << "============= Test  =============" << std::endl;
    std::cout << "=================================" << std::endl << std::endl;;

    factory->TestAllMethods();


    std::cout << std::endl << "=================================" << std::endl;
    std::cout << "============= Eval. =============" << std::endl;
    std::cout << "=================================" << std::endl << std::endl;;

    factory->EvaluateAllMethods();

    std::cout << "==> Wrote root file: " << outputFile->GetName() << std::endl;
    std::cout << "==> TMVAClassification is done!" << std::endl;

    outputFile->Close();

    delete factory;
    delete dataloader;
}

void train_TMVA() {
    char filename[100];

    // request user input
    std::cout << "filename: ";
    scanf("%s", filename);

    train_TMVA_from_filename(filename);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "No filename given !" << std::endl;
        exit(-1);
    } else if (argc > 2) {
        std::cerr << "Too many arguments !" << std::endl;
        exit(-1);
    }

    char *filename = argv[1];

    train_TMVA_from_filename(filename);

    return 0;
}