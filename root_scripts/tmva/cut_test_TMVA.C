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
#include <TStyle.h>

#include <TMVA/Reader.h>
#include <TMVA/Factory.h>
#include <TMVA/DataLoader.h>
#include <TMVA/DataSetInfo.h>
#include <TMVA/Config.h>
#include <TMVA/MethodDL.h>
#include <TMVA/Tools.h>

#include <ROOT/TTreeProcessorMT.hxx>
#include <ROOT/TThreadedObject.hxx>

static const bool hasGEM = true;
static const bool plotOnlyGEM = false;

const float wp = 0.6;


bool base_cut(
	bool  reco_isGEM,
	bool  reco_isTracker,
	bool  reco_isGlobal,
	int   nHitsMu,
	int   nHitsTracker,
	int   nHitsPix,
	float reco_mu_localChi2,
	float reco_mu_normChi2,
	float reco_mu_pt,
	float reco_mu_eta,
	float reco_mu_dxy,
	float reco_mu_dz,
	float nPV,
	bool  reco_mu_highPurity,
	int   reco_mu_nMatches,
	bool  reco_muGEMquality
) {
	if (!reco_isGEM) {
		return false;
	}
	if (reco_mu_eta < 1.6 || reco_mu_eta > 2.8) {
		return false;
	}
	if (reco_mu_nMatches == 0) {
		return false;
	}
	return true;
}


TCanvas* cut_test_from_name(const char* filename) {
    TMVA::Reader *reader = new TMVA::Reader("!Color:!Silent");
    const char *weightfile = "dataloader/weights/TMVAClassification_BDTG.weights.xml";


	uint ptBins = 60;
	float ptMin  = 0,   ptMax   = 6;
	int max_n_event = -1;


	/* int nthreads = std::thread::hardware_concurrency();
	std::cout << "using " << nthreads << " threads" << std::endl;
	ROOT::EnableImplicitMT(nthreads); */


	TH1F gen        ("",                                    "",                                   ptBins, ptMin, ptMax);

	TH1F reco       ("",                                    "",                                   ptBins, ptMin, ptMax);
	TH1F recoTrue   ("Reconstruction efficiency baseline",  "baseline;Pt (GeV);reco efficiency",  ptBins, ptMin, ptMax);
	TH1F recoFake   ("Fake rate baseline",                  "baseline;Pt (GeV);fake rate",        ptBins, ptMin, ptMax);
	
	TH1F recoCut    ("",                                    "",                                   ptBins, ptMin, ptMax);
	TH1F recoTrueCut("Reconstruction efficiency after cut", "after cut;Pt (GeV);reco efficiency", ptBins, ptMin, ptMax);
	TH1F recoFakeCut("Fake rate after cut",                 "after cut ;Pt (GeV);fake rate",      ptBins, ptMin, ptMax);



	std::cout << "Read file..." << std::endl;
	
  	TFile *myFile    = TFile::Open(filename);
    TTree *read_tree = (TTree*)myFile->Get("muTree_test");
    TTree *gen_tree  = (TTree*)myFile->Get("genTree_test");

	std::cout << "Set branches adress..." << std::endl;

	float gen_mu_pt, gen_mu_eta;
	gen_tree->SetBranchAddress("Gen_mu_pt",  &gen_mu_pt);
	gen_tree->SetBranchAddress("Gen_mu_eta", &gen_mu_eta);

	// create new branch
/*int*/ float reco_mu_isFake;
	read_tree->SetBranchAddress("Reco_mu_isFake", &reco_mu_isFake);

/*int*/ float reco_mu_isGEM, reco_isTracker, reco_isGlobal, reco_mu_highPurity;
/*int*/ float nHitsMu, nHitsTracker, nHitsPix;
	float reco_mu_localChi2, reco_mu_normChi2, reco_mu_pt, reco_mu_eta;
	float reco_mu_dxy, reco_mu_dz, nPV;
	read_tree->SetBranchAddress("Reco_mu_isGEM",       &reco_mu_isGEM);
	read_tree->SetBranchAddress("Reco_mu_isTracker",   &reco_isTracker);
	read_tree->SetBranchAddress("Reco_mu_isGlobal",    &reco_isGlobal);
	read_tree->SetBranchAddress("Reco_mu_nMuValHits",  &nHitsMu);
	read_tree->SetBranchAddress("Reco_mu_nTrkHits",    &nHitsTracker);
	read_tree->SetBranchAddress("Reco_mu_nPixValHits", &nHitsPix);
	read_tree->SetBranchAddress("Reco_mu_localChi2",   &reco_mu_localChi2);
	read_tree->SetBranchAddress("Reco_mu_normChi2",    &reco_mu_normChi2);
	read_tree->SetBranchAddress("Reco_mu_pt",          &reco_mu_pt);
	read_tree->SetBranchAddress("Reco_mu_eta",         &reco_mu_eta);
	read_tree->SetBranchAddress("Reco_mu_dxy",         &reco_mu_dxy);
	read_tree->SetBranchAddress("Reco_mu_dz",          &reco_mu_dz);
	read_tree->SetBranchAddress("nPV",                 &nPV);
	read_tree->SetBranchAddress("Reco_mu_highPurity",  &reco_mu_highPurity);

/*int*/ float reco_mu_nMatches, reco_muGEMquality;
	read_tree->SetBranchAddress("Reco_mu_nMatches",   &reco_mu_nMatches);
	read_tree->SetBranchAddress("Reco_muGEMquality",  &reco_muGEMquality);


    std::cout << "Add variables..." << std::endl;

    std::vector<TString> var_names = {
    	"Reco_mu_nMuValHits",
    	"Reco_mu_nTrkHits",
    	"Reco_mu_nPixValHits",
    	"Reco_mu_localChi2",
	 // "Reco_mu_normChi2",
    	"Reco_mu_pt",
    	"Reco_mu_eta",
    	"Reco_mu_dxy",
    	"Reco_mu_dz",
    	"nPV",
	 // "Reco_mu_highPurity",
	    "Reco_mu_nMatches" // ,
	 // "Reco_muGEMquality"
    };
    std::map<TString, float*> float_variables = {
    	{"Reco_mu_nMuValHits",  &nHitsMu},
    	{"Reco_mu_nTrkHits",    &nHitsTracker},
    	{"Reco_mu_nPixValHits", &nHitsPix},
    	{"Reco_mu_localChi2",   &reco_mu_localChi2},
	 // {"Reco_mu_normChi2",    &reco_mu_normChi2},
    	{"Reco_mu_pt",          &reco_mu_pt},
    	{"Reco_mu_eta",         &reco_mu_eta},
    	{"Reco_mu_dxy",         &reco_mu_dxy},
    	{"Reco_mu_dz",          &reco_mu_dz},
    	{"nPV",                 &nPV},
	 // {"Reco_mu_highPurity",  &reco_mu_highPurity},
	    {"Reco_mu_nMatches",    &reco_mu_nMatches} // ,
	 // {"Reco_muGEMquality",   &reco_muGEMquality}
    };
    for (auto var_name : var_names) {
        reader->AddVariable(var_name, float_variables[var_name]);
    }

    std::cout << "Read model..." << std::endl;
    reader->BookMVA("BDTG", weightfile);
    std::cout << "... Done" << std::endl;

    size_t n_true = 0, n_true_cut = 0, n_false = 0, n_false_cut = 0, n_gen = 0;

    Int_t n_entries = gen_tree->GetEntries();
	for (int iEntry = 0; gen_tree->LoadTree(iEntry) >= 0; ++iEntry) {
	    gen_tree->GetEntry(iEntry);
		
		n_gen++;
		gen.Fill(gen_mu_pt);
	}

	n_entries = read_tree->GetEntries();
	for (int iEntry = 0; read_tree->LoadTree(iEntry) >= 0; ++iEntry) {
	    read_tree->GetEntry(iEntry);

	   	if (iEntry%(n_entries/100*5) == 0) {
	   		std::cout << "\t" << iEntry/(n_entries/100) << "\% complited..." << std::endl;
			std::cout << "\t\tadvancment: " << n_gen << "  " << n_false << "/" << n_true << " -> " << n_false_cut << "/" << n_true_cut << std::endl;
	   	}

	   	if (reco_mu_eta < 1.6 || reco_mu_eta > 2.8) {
	   		continue;
	   	}

		if (!plotOnlyGEM || base_cut(
				reco_mu_isGEM,
				reco_isTracker,
				reco_isGlobal,
				nHitsMu,
				nHitsTracker,
				nHitsPix,
				reco_mu_localChi2,
				reco_mu_normChi2,
				reco_mu_pt,
				reco_mu_eta,
				reco_mu_dxy,
				reco_mu_dz,
			    nPV,
				reco_mu_highPurity,
				reco_mu_nMatches,
				reco_muGEMquality
			))
		{
			reco.Fill(reco_mu_pt);
			if (reco_mu_isFake) {
				n_false++;
				recoFake.Fill(reco_mu_pt);
			} else {
				n_true++;
				recoTrue.Fill(reco_mu_pt);
			}
		}

		if (reco_mu_isGEM) {
				if (base_cut(
					reco_mu_isGEM,
					reco_isTracker,
					reco_isGlobal,
					nHitsMu,
					nHitsTracker,
					nHitsPix,
					reco_mu_localChi2,
					reco_mu_normChi2,
					reco_mu_pt,
					reco_mu_eta,
					reco_mu_dxy,
					reco_mu_dz,
				    nPV,
					reco_mu_highPurity,
					reco_mu_nMatches,
					reco_muGEMquality
				))
			{
				float proba = reader->EvaluateMVA("BDTG");
			    bool  pass = proba > wp;

			    if (pass) {
					recoCut.Fill(reco_mu_pt);
					if (reco_mu_isFake) {
						n_false_cut++;
						recoFakeCut.Fill(reco_mu_pt);
					} else {
						n_true_cut++;
						recoTrueCut.Fill(reco_mu_pt);
					}
				}
			}
		} else if (!plotOnlyGEM) {
			recoCut.Fill(reco_mu_pt);
			if (reco_mu_isFake) {
				n_false_cut++;
				recoFakeCut.Fill(reco_mu_pt);
			} else {
				n_true_cut++;
				recoTrueCut.Fill(reco_mu_pt);
			}
		}
	}

	std::cout << n_gen << "  " << n_false << "/" << n_true << " -> " << n_false_cut << "/" << n_true_cut << std::endl;


	TCanvas* canva = new TCanvas("Tracker muons", "", 1000, 1000);
    canva->Divide(2, 1);
	canva->SetFillStyle(1001);
	canva->SetFillColor(kWhite);

	recoFake.Divide(&reco);
	recoFakeCut.Divide(&recoCut);

	recoTrue.Divide(&gen);
	recoTrueCut.Divide(&gen);


	canva->cd(1);
	gStyle->SetOptTitle(0);
	recoTrue.SetStats(kFALSE);
	recoTrue.SetLineColor(kRed);
	recoTrue.DrawCopy("cp");
	recoTrueCut.SetStats(kFALSE);
	recoTrueCut.SetLineColor(kBlue);
	recoTrueCut.DrawCopy("cp same");
	gPad->BuildLegend();
	canva->cd(2);
	gStyle->SetOptTitle(0);
	recoFake.SetStats(kFALSE);
	recoFake.SetLineColor(kRed);
	recoFake.DrawCopy("cp");
	recoFakeCut.SetStats(kFALSE);
	recoFakeCut.SetLineColor(kBlue);
	recoFakeCut.DrawCopy("cp same");
	gPad->BuildLegend();

	canva->Update();


	return canva;
}

void cut_test() {
	char filename[100];

	// request user input
  	std::cout << "filename: ";
  	scanf("%s", filename);

  	TCanvas* canva = cut_test_from_name(filename);
  	canva->SaveAs("../../figures/cut_test.png");
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

	TCanvas* canva = cut_test_from_name(filename);
  	canva->SaveAs("../../figures/cut_test.png");

	return 0;
}