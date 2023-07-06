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

#include <ROOT/TTreeProcessorMT.hxx>
#include <ROOT/TThreadedObject.hxx>

#include "utils/TMVA_cuts.hpp"

static const int   Max_mu_size     = 10000;
static const bool  hasGEM          = true;
static const float test_proportion = 0.4;


void tmva_pre_processing_from_file(const char* filename) {
	int nthreads = std::thread::hardware_concurrency();
	std::cout << "using " << nthreads << " threads" << std::endl;
	ROOT::EnableImplicitMT(nthreads);


	std::cout << "openning \"" << filename << "\"..." << std::endl;

  	// read file
  	std::unique_ptr<TFile> myFile(TFile::Open(filename, "READ") );
  	if (!myFile || myFile->IsZombie()) {
  		std::cerr << "Error opening file \"" << filename << "\" !" << std::endl;
  		exit(-1);
	}

	// read file subdirectory
	std::unique_ptr<TDirectoryFile> myFileDir(myFile->Get<TDirectoryFile>("hionia"));

	// read tree
	std::unique_ptr<TTree> read_tree(myFileDir->Get<TTree>("myTree"));
	// std::unique_ptr<TTree> write_tree = std::make_unique<TTree>("T", "learning data");

	TFile* output = TFile::Open("tmva_file.root", "RECREATE");
	TTree* write_tree_train = new TTree("muTree_train",    "Tree containing all muons for TMVA training");
	TTree* write_tree_test  = new TTree("muTree_test",     "Tree containing all muons for TMVA testing");
	TTree* fake_tree_train  = new TTree("fakeTree_train",  "Background tree containing fake muons for TMVA training");
	TTree* fake_tree_test   = new TTree("fakeTree_test",   "Background tree containing fake muons for TMVA testing");
	TTree* true_tree_train  = new TTree("trueTree_train",  "Signal tree containing true muons for TMVA training");
	TTree* true_tree_test   = new TTree("trueTree_test",   "Signal tree containing true muons for TMVA testing");
	TTree* gen_tree_train   = new TTree("genTree_train",   "Tree containing generated muons for training");
	TTree* gen_tree_test    = new TTree("genTree_test",    "Tree containing generated muons for testing");


/*int*/ float write_reco_mu_isGEM, write_reco_isTracker, write_reco_isGlobal, write_reco_mu_highPurity;
/*int*/ float write_nHitsMu, write_nHitsTracker, write_nHitsPix;
	float write_reco_mu_localChi2, write_reco_mu_normChi2, write_reco_mu_pt, write_reco_mu_eta;
	float write_reco_mu_dxy, write_reco_mu_dz, write_nPV;
/*int*/ float write_reco_mu_nMatches, write_reco_muGEMquality;
	float write_gen_weight;

	std::vector<std::tuple<const char*, float*, const char*>> ptr_map = {
		{"Reco_mu_isGEM",       &write_reco_mu_isGEM,      "Reco_mu_isGEM/F" /*O*/},
		{"Reco_mu_isTracker",   &write_reco_isTracker,     "Reco_mu_isTracker/F" /*O*/},
		{"Reco_mu_isGlobal",    &write_reco_isGlobal,      "Reco_mu_isGlobal/F" /*O*/},
		{"Reco_mu_nMuValHits",  &write_nHitsMu,            "Reco_mu_nMuValHits/F" /*I*/},
		{"Reco_mu_nTrkHits",    &write_nHitsTracker,       "Reco_mu_nTrkHits/F" /*I*/},
		{"Reco_mu_nPixValHits", &write_nHitsPix,           "Reco_mu_nPixValHits/F" /*I*/},
		{"Reco_mu_localChi2",   &write_reco_mu_localChi2,  "Reco_mu_localChi2/F"},
		{"Reco_mu_normChi2",    &write_reco_mu_normChi2,   "Reco_mu_normChi2/F"},
		{"Reco_mu_pt",          &write_reco_mu_pt,         "Reco_mu_pt/F"},
		{"Reco_mu_eta",         &write_reco_mu_eta,        "Reco_mu_eta/F"},
		{"Reco_mu_dxy",         &write_reco_mu_dxy,        "Reco_mu_dxy/F"},
		{"Reco_mu_dz",          &write_reco_mu_dz,         "Reco_mu_dz/F"},
		{"nPV",                 &write_nPV,                "nPV/F" /*I*/},

		{"Reco_mu_highPurity",  &write_reco_mu_highPurity, "Reco_mu_highPurity/F" /*O*/},
		{"Reco_mu_nMatches",    &write_reco_mu_nMatches,   "Reco_mu_nMatches/F" /*I*/},
		{"Reco_muGEMquality",   &write_reco_muGEMquality,  "Reco_muGEMquality/F" /*O*/},

		{"Gen_weight",          &write_gen_weight,         "Gen_weight/F"}
	};


	// create new branch
/*int*/ float write_reco_mu_isFake;
	write_tree_train->Branch("Reco_mu_isFake", &write_reco_mu_isFake, "Reco_mu_isFake/F" /*I*/);
	write_tree_test ->Branch("Reco_mu_isFake", &write_reco_mu_isFake, "Reco_mu_isFake/F" /*I*/);

	for (auto [name1, ptr, name2] : ptr_map) {
		write_tree_train->Branch(name1, ptr, name2);
		write_tree_test ->Branch(name1, ptr, name2);
		fake_tree_train ->Branch(name1, ptr, name2);
		fake_tree_test  ->Branch(name1, ptr, name2);
		true_tree_train ->Branch(name1, ptr, name2);
		true_tree_test  ->Branch(name1, ptr, name2);
	}

	/* -----------------------------------------
	--------------------------------------------
	----------------------------------------- */

	
	float gen_mu_pt, gen_mu_eta, gen_mu_weight;

	gen_tree_train->Branch("Gen_mu_pt",  &gen_mu_pt,     "Gen_mu_pt/F");
	gen_tree_train->Branch("Gen_mu_eta", &gen_mu_eta,    "Gen_mu_eta/F");
	gen_tree_train->Branch("Gen_weight", &gen_mu_weight, "Gen_weight/F");

	gen_tree_test->Branch("Gen_mu_pt",  &gen_mu_pt,     "Gen_mu_pt/F");
	gen_tree_test->Branch("Gen_mu_eta", &gen_mu_eta,    "Gen_mu_eta/F");
	gen_tree_test->Branch("Gen_weight", &gen_mu_weight, "Gen_weight/F");



	/* -----------------------------------------
	--------------------------------------------
	----------------------------------------- */


	// read one branch
	int   gen_mu_size, reco_mu_size;
	short gen_mu_idx[Max_mu_size];
	read_tree->SetBranchAddress("Gen_mu_size",      &gen_mu_size);
	read_tree->SetBranchAddress("Reco_mu_size",     &reco_mu_size);
	read_tree->SetBranchAddress("Reco_mu_whichGen", &gen_mu_idx);

	bool  reco_mu_isGEM[Max_mu_size], reco_isTracker[Max_mu_size], reco_isGlobal[Max_mu_size], reco_mu_highPurity[Max_mu_size];
	int   nHitsMu[Max_mu_size], nHitsTracker[Max_mu_size], nHitsPix[Max_mu_size];
	float reco_mu_localChi2[Max_mu_size], reco_mu_normChi2[Max_mu_size];
	float reco_mu_dxy[Max_mu_size], reco_mu_dz[Max_mu_size];
	Short_t nPV;
	TClonesArray *reco_4mom = new TClonesArray("TLorentzVector", Max_mu_size);
	read_tree->SetBranchAddress(hasGEM ? "Reco_mu_isGEM" : "Reco_mu_isTracker", &reco_mu_isGEM);
	read_tree->SetBranchAddress("Reco_mu_isTracker",                            &reco_isTracker);
	read_tree->SetBranchAddress("Reco_mu_isGlobal",                             &reco_isGlobal);
	read_tree->SetBranchAddress("Reco_mu_nMuValHits",                           &nHitsMu);
	read_tree->SetBranchAddress("Reco_mu_nTrkHits",                             &nHitsTracker);
	read_tree->SetBranchAddress("Reco_mu_nPixValHits",                          &nHitsPix);
	read_tree->SetBranchAddress("Reco_mu_localChi2",                            &reco_mu_localChi2);
	read_tree->SetBranchAddress("Reco_mu_normChi2",                             &reco_mu_normChi2);
	read_tree->SetBranchAddress("Reco_mu_4mom",                                 &reco_4mom);
	read_tree->SetBranchAddress("Reco_mu_dxy",                                  &reco_mu_dxy);
	read_tree->SetBranchAddress("Reco_mu_dz",                                   &reco_mu_dz);
	read_tree->SetBranchAddress("nPV",                                          &nPV);
	read_tree->SetBranchAddress("Reco_mu_highPurity",                           &reco_mu_highPurity);

	int  reco_mu_nMatches[Max_mu_size];
	bool reco_muGEMquality[Max_mu_size];
	read_tree->SetBranchAddress("Reco_mu_nMatches",  &reco_mu_nMatches);
	read_tree->SetBranchAddress("Reco_muGEMquality", &reco_muGEMquality);

	float read_gen_weight;
	short reco_mu_idx[Max_mu_size];
	TClonesArray *gen_mu_4mom = new TClonesArray("TLorentzVector", Max_mu_size);
	read_tree->SetBranchAddress("Gen_mu_whichRec", &reco_mu_idx);
	read_tree->SetBranchAddress("Gen_mu_4mom",     &gen_mu_4mom);
	read_tree->SetBranchAddress("Gen_weight",      &read_gen_weight);
	

	/* -----------------------------------------
	--------------------------------------------
	----------------------------------------- */


	std::cout << "Computing and filling \"Reco_mu_isFake\"..." << std::endl;

	size_t n_write_train = 0, n_write_test = 0;
	size_t n_fake_train  = 0, n_fake_test  = 0;
	size_t n_true_train  = 0, n_true_test  = 0;
	size_t n_gen_train   = 0, n_gen_test   = 0;

	Int_t n_entries = read_tree->GetEntries();
	for (int iEntry = 0; read_tree->LoadTree(iEntry) >= 0; ++iEntry) {
	    read_tree->GetEntry(iEntry);

	   	if (iEntry%(n_entries/100*5) == 0) {
	   		std::cout << "\t" << iEntry/(n_entries/100) << "\% complited..." << std::endl;
	   	}
	    
		for (int i = 0; i < gen_mu_size; ++i) {
		    TLorentzVector* mom4 = (TLorentzVector*)gen_mu_4mom->At(i);

			float randF  = static_cast<float>(std::rand())/(static_cast<float>(RAND_MAX));
			bool  isTest = randF < test_proportion;


			if (isTest) { gen_tree_test->GetEntry(n_gen_test++);   }
			else        { gen_tree_train->GetEntry(n_gen_train++); }

			gen_mu_pt     = mom4->Pt();
			gen_mu_eta    = mom4->Eta();
	    	gen_mu_weight = read_gen_weight;

			if (isTest) { gen_tree_test->Fill();  }
			else        { gen_tree_train->Fill(); }
		}

	    for (int i = 0; i < reco_mu_size; ++i) {
		    TLorentzVector* mom4 = (TLorentzVector*)reco_4mom->At(i);
			short gen_idx        = gen_mu_idx[i];

			float randF  = static_cast<float>(std::rand())/(static_cast<float>(RAND_MAX));
			bool  isTest = randF < test_proportion;

	    	bool  isTrueMuon = gen_idx >= 0 && gen_idx < gen_mu_size;
	   		bool cut         = pass_TMVA_pre_cut(
				hasGEM ?
					reco_mu_isGEM [i] : false,
				reco_isTracker    [i],
				reco_isGlobal     [i],
				nHitsMu           [i],
				nHitsTracker      [i],
				nHitsPix          [i],
				reco_mu_localChi2 [i],
				reco_mu_normChi2  [i],
				mom4->Pt(),
				abs(mom4->Eta()),
				reco_mu_dxy       [i],
				reco_mu_dz        [i],
				nPV,
				reco_mu_highPurity[i],
				reco_mu_nMatches  [i],
				reco_muGEMquality [i]
			);


			if (isTest) { write_tree_test->GetEntry(n_write_test++);   }
			else        { write_tree_train->GetEntry(n_write_train++); }
	   		if (cut) {
	   			if (isTrueMuon) {
					if (isTest) { true_tree_test->GetEntry(n_true_test++);   }
					else        { true_tree_train->GetEntry(n_true_train++); }
	   			} else {
					if (isTest) { fake_tree_test->GetEntry(n_fake_test++);   }
					else        { fake_tree_train->GetEntry(n_fake_train++); }
	   			}
	   		}

	    	write_reco_mu_isFake     = !isTrueMuon;
	    	write_reco_mu_isGEM      = hasGEM ? reco_mu_isGEM[i] : false;
	    	write_reco_isTracker     = reco_isTracker    [i];
	    	write_reco_isGlobal      = reco_isGlobal     [i];
	    	write_nHitsMu            = nHitsMu           [i];
	    	write_nHitsPix           = nHitsPix          [i];
	    	write_nHitsTracker       = nHitsTracker      [i];
	    	write_reco_mu_localChi2  = reco_mu_localChi2 [i];
	    	write_reco_mu_normChi2   = reco_mu_normChi2  [i];
	    	write_reco_mu_dxy        = reco_mu_dxy       [i];
			write_reco_mu_dz         = reco_mu_dz        [i];
			write_nPV                = nPV;

			write_reco_mu_highPurity = reco_mu_highPurity[i];
			write_reco_mu_nMatches   = reco_mu_nMatches  [i];
			write_reco_muGEMquality  = reco_muGEMquality [i];

	    	write_reco_mu_pt  =     mom4->Pt();
	    	write_reco_mu_eta = abs(mom4->Eta());

	    	write_gen_weight = read_gen_weight;

			if (isTest) { write_tree_test->Fill();  }
			else        { write_tree_train->Fill(); }
	   		if (cut) {
	   			if (isTrueMuon) {
					if (isTest) { true_tree_test->Fill();  }
					else        { true_tree_train->Fill(); }
	   			} else {
					if (isTest) { fake_tree_test->Fill();  }
					else        { fake_tree_train->Fill(); }
	   			}
	   		}
	    }
	}

	std::cout << "... Done !" << std::endl;
	std::cout << "Writting ..." << std::endl;

	write_tree_train->Write();
	write_tree_test ->Write();
	fake_tree_train ->Write();
	fake_tree_test  ->Write();
	true_tree_train ->Write();
	true_tree_test  ->Write();
	gen_tree_train  ->Write();
	gen_tree_test   ->Write();
	output->WriteObject(write_tree_train, "muTree_train");
	output->WriteObject(write_tree_test,  "muTree_test");
	output->WriteObject(fake_tree_train,  "fakeTree_train");
	output->WriteObject(fake_tree_test,   "fakeTree_test");
	output->WriteObject(true_tree_train,  "trueTree_train");
	output->WriteObject(true_tree_test,   "trueTree_test");
	output->WriteObject(gen_tree_train,   "genTree_train");
	output->WriteObject(gen_tree_test,    "genTree_test");

	std::cout << "... Done !" << std::endl;

	gen_tree_train->Print();
	std::cout << std::endl;
	gen_tree_test->Print();
	std::cout << std::endl << std::endl;

	write_tree_train->Print();
	std::cout << std::endl;
	write_tree_test->Print();
	std::cout << std::endl << std::endl;

	fake_tree_train->Print();
	std::cout << std::endl;
	fake_tree_test->Print();
	std::cout << std::endl << std::endl;

	true_tree_train->Print();
	std::cout << std::endl;
	true_tree_test->Print();

	output->Close();
}

void tmva_pre_processing() {
	char filename[100];

	// request user input
  	std::cout << "filename: ";
  	scanf("%s", filename);

  	tmva_pre_processing_from_file(filename);
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

  	tmva_pre_processing_from_file(filename);

	return 0;
}