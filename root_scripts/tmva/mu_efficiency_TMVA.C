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

#include <TMVA/Reader.h>
#include <TMVA/Factory.h>
#include <TMVA/DataLoader.h>
#include <TMVA/DataSetInfo.h>
#include <TMVA/Config.h>
#include <TMVA/MethodDL.h>
#include <TMVA/Tools.h>

#include "../utilities/embeding_weight.hpp"
#include "../utilities/error_util.hpp"

#include "utils/TMVA_cuts.hpp"

bool hasGEM = true;
bool plotOnlyGEM = true;
float wp = 0.4;

const bool embeding = false, binning = true, hasNtracks = false;

std::vector<float> percentiles = {0.9, 1};


TCanvas* mu_efficiency_from_name(const char* filename) {
	uint etaBins = 60, ptBins = 60;
	float etaMin = 0, etaMax = 4;
	float ptMin  = 0, ptMax  = 6;
	int max_n_event = -1;


	int nthreads = 1; //std::thread::hardware_concurrency();
	std::cout << "using " << nthreads << " threads" << std::endl;
	ROOT::EnableImplicitMT(nthreads);



    TMVA::Reader *reader = new TMVA::Reader("!Color:!Silent");
    const char *weightfile = "dataloader/weights/TMVAClassification_BDTG.weights.xml";

/*int*/ float reco_mu_isGEM_, reco_isTracker_, reco_isGlobal_, reco_mu_highPurity_;
/*int*/ float nHitsMu_, nHitsTracker_, nHitsPix_;
	float reco_mu_localChi2_, reco_mu_normChi2_, reco_mu_pt_, reco_mu_eta_;
	float reco_mu_dxy_, reco_mu_dz_, nPV_;
/*int*/ float reco_mu_nMatches_, reco_muGEMquality_;

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
    	{"Reco_mu_nMuValHits",  &nHitsMu_},
    	{"Reco_mu_nTrkHits",    &nHitsTracker_},
    	{"Reco_mu_nPixValHits", &nHitsPix_},
    	{"Reco_mu_localChi2",   &reco_mu_localChi2_},
	 // {"Reco_mu_normChi2",    &reco_mu_normChi2_},
    	{"Reco_mu_pt",          &reco_mu_pt_},
    	{"Reco_mu_eta",         &reco_mu_eta_},
    	{"Reco_mu_dxy",         &reco_mu_dxy_},
    	{"Reco_mu_dz",          &reco_mu_dz_},
    	{"nPV",                 &nPV_},
	 // {"Reco_mu_highPurity",  &reco_mu_highPurity_},
	    {"Reco_mu_nMatches",    &reco_mu_nMatches_} // ,
	 // {"Reco_muGEMquality",   &reco_muGEMquality_}
    };
    for (auto var_name : var_names) {
        reader->AddVariable(var_name, float_variables[var_name]);
    }

    std::cout << "Read model..." << std::endl;
    reader->BookMVA("BDTG", weightfile);
    std::cout << "... Done" << std::endl;



  	// read file
	std::cout << "openning \"" << filename << "\"..." << std::endl;
	ROOT::TTreeProcessorMT tp(filename, "hionia/myTree");

	ROOT::TThreadedObject<std::vector<int>> nTracks;
	std::vector<int> limits_file = {};

	if (binning) {
		std::cout << "reading file nTracks..." << std::endl;
		if (hasNtracks) {
			tp.Process(readField<short>("Ntracks", nTracks, max_n_event, nthreads));
		} else {
			tp.Process(readField<int>("NTotalTracks", nTracks, max_n_event, nthreads));
		}

		std::cout << "processing file nTracks..." << std::endl;
		processFunctionPercentile(percentiles, limits_file, nTracks);
	}



	ROOT::TThreadedObject<TH2F> ptEtaGen   ("", "", etaBins, etaMin, etaMax, ptBins, ptMin, ptMax);
	ROOT::TThreadedObject<TH2F> ptEtaTrack ("", "", etaBins, etaMin, etaMax, ptBins, ptMin, ptMax);

	ROOT::TThreadedObject<TH2F> ptEtaEffTrack      ("eff", "Tracker muon reco. efficiency;muon |#eta|;muon p_{t}[GeV]", etaBins, etaMin, etaMax, ptBins, ptMin, ptMax);
	ROOT::TThreadedObject<TH2F> ptEtaFakeRateTrack ("fakerate", "Tracker muon fake rate;muon |#eta|;muon p_{t}[GeV]",   etaBins, etaMin, etaMax, ptBins, ptMin, ptMax);

    ROOT::TThreadedObject<TH2F> ptErrTrack  ("Muon chamber hit per muon",  "Tracker muon p_{t} resolution;muon |#eta|;muon p_{t}[GeV]", etaBins, etaMin, etaMax, ptBins, ptMin, ptMax); // "pt err",  "pt err;
	ROOT::TThreadedObject<TH2F> etaErrTrack ("Tracker hit per muon",       "Tracker muon #eta resolution;muon |#eta|;muon p_{t}[GeV]",  etaBins, etaMin, etaMax, ptBins, ptMin, ptMax); // "eta err", "eta err;


	auto readerFunction = [&](TTreeReader &myReader) {
		// Create a TTreeReader for the tree, for instance by passing the
		// TTree's name and the TDirectory / TFile it is in.
		TTreeReaderValue<short> Ntracks     (myReader, hasNtracks ? "Ntracks"     : "nPV");
		TTreeReaderValue<int>   NTotalTracks(myReader, hasNtracks ? "Gen_mu_size" : "NTotalTracks");

		TTreeReaderValue<float> hiHF(myReader, embeding ? "SumET_HF" : "Gen_weight");

		TTreeReaderValue<int>          gen_mu_size (myReader, "Gen_mu_size");
		TTreeReaderValue<int>          reco_mu_size(myReader, "Reco_mu_size");
		TTreeReaderArray<short>        reco_mu_idx (myReader, "Gen_mu_whichRec");
		TTreeReaderArray<short>        gen_mu_idx  (myReader, "Reco_mu_whichGen");
		TTreeReaderValue<TClonesArray> gen_mu_4mom (myReader, "Gen_mu_4mom");
		TTreeReaderValue<TClonesArray> reco_mu_4mom(myReader, "Reco_mu_4mom");

		TTreeReaderArray<bool>  reco_isTracker    (myReader, "Reco_mu_isTracker"); //"Reco_mu_isGlobal");		
		TTreeReaderArray<bool>  reco_mu_isGEM     (myReader, hasGEM ? "Reco_mu_isGEM" : "Reco_mu_isTracker");
		TTreeReaderArray<bool>  reco_isGlobal     (myReader, "Reco_mu_isGlobal");
		TTreeReaderArray<int>   nHitsMu           (myReader, "Reco_mu_nMuValHits");
		TTreeReaderArray<int>   nHitsTracker      (myReader, "Reco_mu_nTrkHits");
		TTreeReaderArray<int>   nHitsPix          (myReader, "Reco_mu_nPixValHits");
		TTreeReaderArray<float> reco_mu_localChi2 (myReader, "Reco_mu_localChi2");
		TTreeReaderArray<float> reco_mu_normChi2  (myReader, "Reco_mu_normChi2");
		TTreeReaderArray<float> reco_mu_dxy       (myReader, "Reco_mu_dxy");
		TTreeReaderArray<float> reco_mu_dz        (myReader, "Reco_mu_dz");
		TTreeReaderValue<short> nPV               (myReader, "nPV");
		TTreeReaderArray<bool>  reco_mu_highPurity(myReader, "Reco_mu_highPurity");
		TTreeReaderArray<int>   reco_mu_nMatches  (myReader, "Reco_mu_nMatches");
		TTreeReaderArray<bool>  reco_muGEMquality (myReader, "Reco_muGEMquality");

		auto myPtEtaGen            = ptEtaGen.Get();

		auto myPtEtaTrack          = ptEtaTrack.Get();
		auto myPtEtaEffTrack       = ptEtaEffTrack.Get();
		auto myPtEtaFakeRateTrack  = ptEtaFakeRateTrack.Get();
		auto myPtErrTrack          = ptErrTrack.Get();
		auto myEtaErrTrack         = etaErrTrack.Get();

		for (int j = 0; myReader.Next() && (j < max_n_event/nthreads || max_n_event <= 0); j++) {
			float weigth = embeding ? findNcoll(getHiBinFromhiHF(*hiHF)) : 1.f;

			if (binning) {
				if (hasNtracks) {
					if (limits_file[0] > *Ntracks      ||      *Ntracks > limits_file[1]) {
						continue;
					}
				} else {
					if (limits_file[0] > *NTotalTracks || *NTotalTracks > limits_file[1]) {
						continue;
					}
				}
			}


			for (short i = 0; i < *gen_mu_size; ++i) {
		    	short  reco_idx  = reco_mu_idx[i];
		    	bool   isPositiv = reco_idx != -1;

		    	TLorentzVector* mom4 = (TLorentzVector*)gen_mu_4mom->At(i);
		    	float           pt   = mom4->Pt();
		    	float           eta  = abs(mom4->Eta());

		    	myPtEtaGen->Fill(eta, pt, weigth);
			}

			for (int i = 0; i < *reco_mu_size; ++i) {
				short gen_idx   = gen_mu_idx[i];
		    	bool  isPositiv = gen_idx >= 0 && gen_idx < *gen_mu_size;

		    	TLorentzVector* mom4 = (TLorentzVector*)reco_mu_4mom->At(i);
		    	float           pt   = mom4->Pt();
		    	float           eta  = abs(mom4->Eta());

		    	if (pass_TMVA_domain_cut(
					hasGEM ? reco_mu_isGEM[i] : false,
					reco_isTracker    [i],
					reco_isGlobal     [i],
					nHitsMu           [i],
					nHitsTracker      [i],
					nHitsPix          [i],
					reco_mu_localChi2 [i],
					reco_mu_normChi2  [i],
					pt,
					eta,
					reco_mu_dxy       [i],
					reco_mu_dz        [i],
				   *nPV,
					reco_mu_highPurity[i],
					reco_mu_nMatches  [i],
					reco_muGEMquality [i]
				)) {
			    	if (!pass_TMVA_pre_cut(
						hasGEM ? reco_mu_isGEM[i] : false,
						reco_isTracker    [i],
						reco_isGlobal     [i],
						nHitsMu           [i],
						nHitsTracker      [i],
						nHitsPix          [i],
						reco_mu_localChi2 [i],
						reco_mu_normChi2  [i],
						pt,
						eta,
						reco_mu_dxy       [i],
						reco_mu_dz        [i],
					   *nPV,
						reco_mu_highPurity[i],
						reco_mu_nMatches  [i],
						reco_muGEMquality [i]
					)) {
			    		continue;
					}

					nHitsMu_           = nHitsMu          [i];
					nHitsTracker_      = nHitsTracker     [i];
					nHitsPix_          = nHitsPix         [i];
					reco_mu_localChi2_ = reco_mu_localChi2[i];
					reco_mu_pt_        = pt;
					reco_mu_eta_       = eta;
					reco_mu_dxy_       = reco_mu_dxy      [i];
					reco_mu_dz_        = reco_mu_dz       [i];
					nPV_               = *nPV;
					reco_mu_nMatches_  = reco_mu_nMatches [i];

					float proba = reader->EvaluateMVA("BDTG");
					if (proba < wp) {
						continue;
					}
				} else if (plotOnlyGEM) {
					continue;
				}
		    	

		    	myPtEtaTrack->Fill(eta, pt, weigth);

		    	if (isPositiv) {
		    		      mom4   = (TLorentzVector*)reco_mu_4mom->At(gen_idx);
		    		float genPt  = mom4->Pt();
		    		float genEta = abs(mom4->Eta());

		    		myPtEtaEffTrack->Fill(eta, pt, weigth);

			    	myPtErrTrack ->Fill(eta, pt, abs(genPt  - pt) /genPt);
			    	myEtaErrTrack->Fill(eta, pt, abs(genEta - eta)/genEta);
		    	} else {
		    		myPtEtaFakeRateTrack->Fill(eta, pt, weigth);
		    	}
		    }
		}
	};
	

	std::cout << std::endl << "reading file..." << std::endl;
	tp.Process(readerFunction);


	auto ptEtaGenMerged           = ptEtaGen.Merge();

	auto ptEtaTrackMerged         = ptEtaTrack.Merge();
	auto ptEtaEffTrackMerged      = ptEtaEffTrack.Merge();
	auto ptEtaFakeRateTrackMerged = ptEtaFakeRateTrack.Merge();
	auto ptErrTrackMerged         = ptErrTrack.Merge();
	auto etaErrTrackMerged        = etaErrTrack.Merge();

	std::cout << std::endl << "process..." << std::endl;

	ptEtaEffTrackMerged ->Divide(ptEtaGenMerged.get());

	ptEtaFakeRateTrackMerged ->Divide(ptEtaTrackMerged.get());
	zero_normalize(ptEtaFakeRateTrackMerged, ptEtaTrackMerged);


	get_error_self(ptErrTrackMerged);
	get_error_self(etaErrTrackMerged);




	TCanvas* canva = new TCanvas("Tracker muons", "", 1000, 1000);
    canva->Divide(2, 2);
	canva->SetFillStyle(1001);
	canva->SetFillColor(kWhite);


	canva->cd(1);
	ptEtaEffTrackMerged->SetStats(kFALSE);
	ptEtaEffTrackMerged->DrawCopy("COLZ");
	canva->cd(3);
	ptEtaFakeRateTrackMerged->SetStats(kFALSE);
	ptEtaFakeRateTrackMerged->DrawCopy("COLZ");

	canva->cd(2);
	gPad->SetLogz();
	ptErrTrackMerged->SetStats(kFALSE);
	ptErrTrackMerged->DrawCopy("COLZ");
	canva->cd(4);
	gPad->SetLogz();
	etaErrTrackMerged->SetStats(kFALSE);
	etaErrTrackMerged->DrawCopy("COLZ");

	canva->Update();


	return canva;
}

void mu_efficiency() {
	char filename[100];

	// request user input
  	std::cout << "filename: ";
  	scanf("%s", filename);

  	auto canva = mu_efficiency_from_name(filename);
  	canva->SaveAs("../../figures/mu_eff_GEM_TMVA.png");
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

  	auto canva = mu_efficiency_from_name(filename);
  	canva->SaveAs("../../figures/mu_eff_GEM_TMVA.png");

	return 0;
}