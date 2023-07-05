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

#include "../utilities/embeding_weight.hpp"
#include "../utilities/error_util.hpp"
const bool embeding = false, binning = true, hasNtracks = false;
const bool trackerIncludeGEM = true, trackerGEMonly = true, hasGEM = true;

std::vector<float> percentiles = {0.9, 1};


std::pair<TCanvas*, TCanvas*> mu_efficiency_from_name(const char* filename) {
	uint etaBins = 60, ptBins = 60;
	float etaMin = 0, etaMax = 4;
	float ptMin  = 0, ptMax  = 6;
	int max_n_event = -1;


	int nthreads = std::thread::hardware_concurrency();
	std::cout << "using " << nthreads << " threads" << std::endl;
	ROOT::EnableImplicitMT(nthreads);



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
	ROOT::TThreadedObject<TH2F> ptEtaGlobal("", "", etaBins, etaMin, etaMax, ptBins, ptMin, ptMax);

	ROOT::TThreadedObject<TH2F> ptEtaEffTrack      ("eff", "Tracker muon reco. efficiency;muon |#eta|;muon p_{t}[GeV]", etaBins, etaMin, etaMax, ptBins, ptMin, ptMax);
	ROOT::TThreadedObject<TH2F> ptEtaEffGlobal     ("eff", "Global muon reco. efficiency;muon |#eta|;muon p_{t}[GeV]",  etaBins, etaMin, etaMax, ptBins, ptMin, ptMax);
	ROOT::TThreadedObject<TH2F> ptEtaFakeRateTrack ("fakerate", "Tracker muon fake rate;muon |#eta|;muon p_{t}[GeV]",   etaBins, etaMin, etaMax, ptBins, ptMin, ptMax);
	ROOT::TThreadedObject<TH2F> ptEtaFakeRateGlobal("fakerate", "Global muon fake rate;muon |#eta|;muon p_{t}[GeV]",    etaBins, etaMin, etaMax, ptBins, ptMin, ptMax);

    ROOT::TThreadedObject<TH2F> ptErrTrack  ("Muon chamber hit per muon",  "Tracker muon p_{t} resolution;muon |#eta|;muon p_{t}[GeV]", etaBins, etaMin, etaMax, ptBins, ptMin, ptMax); // "pt err",  "pt err;
    ROOT::TThreadedObject<TH2F> ptErrGlobal ("Muon chamber hit per muon",  "Global muon p_{t} resolution;muon |#eta|;muon p_{t}[GeV]",  etaBins, etaMin, etaMax, ptBins, ptMin, ptMax); // "pt err",  "pt err;
    ROOT::TThreadedObject<TH2F> etaErrTrack ("Tracker hit per muon",       "Tracker muon #eta resolution;muon |#eta|;muon p_{t}[GeV]",  etaBins, etaMin, etaMax, ptBins, ptMin, ptMax); // "eta err", "eta err;
    ROOT::TThreadedObject<TH2F> etaErrGlobal("Tracker hit per muon",       "Global muon #eta resolution;muon |#eta|;muon p_{t}[GeV]",   etaBins, etaMin, etaMax, ptBins, ptMin, ptMax); // "eta err", "eta err;


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

		TTreeReaderArray<bool> reco_mu_isGEM (myReader, hasGEM ? "Reco_mu_isGEM" : "Reco_mu_isTracker");
		TTreeReaderArray<bool> reco_isTracker(myReader, "Reco_mu_isTracker");
		TTreeReaderArray<bool> reco_isGlobal (myReader, "Reco_mu_isGlobal");
		TTreeReaderArray<int>  nHitsMu       (myReader, "Reco_mu_nMuValHits");
		TTreeReaderArray<int>  nHitsTracker  (myReader, "Reco_mu_nTrkHits");

		auto myPtEtaGen            = ptEtaGen.Get();

		auto myPtEtaTrack          = ptEtaTrack.Get();
		auto myPtEtaEffTrack       = ptEtaEffTrack.Get();
		auto myPtEtaFakeRateTrack  = ptEtaFakeRateTrack.Get();
		auto myPtErrTrack          = ptErrTrack.Get();
		auto myEtaErrTrack         = etaErrTrack.Get();

		auto myPtEtaGlobal         = ptEtaGlobal.Get();
		auto myPtEtaEffGlobal      = ptEtaEffGlobal.Get();
		auto myPtEtaFakeRateGlobal = ptEtaFakeRateGlobal.Get();
		auto myPtErrGlobal         = ptErrGlobal.Get();
		auto myEtaErrGlobal        = etaErrGlobal.Get();

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

		    	if (isPositiv) {
		    		      mom4   = (TLorentzVector*)reco_mu_4mom->At(reco_idx);
		    		float recPt  = mom4->Pt();
		    		float recEta = abs(mom4->Eta());

		    		if ((reco_isTracker[reco_idx] && !trackerGEMonly) ||
		    			(reco_mu_isGEM [reco_idx] && trackerIncludeGEM))
		    		{
		    			myPtEtaEffTrack->Fill(eta, pt, weigth);

			    		myPtErrTrack ->Fill(eta, pt, abs(pt  - recPt) /pt);
			    		myEtaErrTrack->Fill(eta, pt, abs(eta - recEta)/eta);
		    		}
		    		if (reco_isGlobal[reco_idx]) {
		    			myPtEtaEffGlobal->Fill(eta, pt, weigth);

			    		myPtErrGlobal ->Fill(eta, pt, abs(pt  - recPt) /pt);
			    		myEtaErrGlobal->Fill(eta, pt, abs(eta - recEta)/eta);
		    		}
		    	}
			}

			for (int i = 0; i < *reco_mu_size; ++i) {
				short gen_idx   = gen_mu_idx[i];
		    	bool  isPositiv = gen_idx >= 0 && gen_idx < *gen_mu_size;

		    	TLorentzVector* mom4 = (TLorentzVector*)reco_mu_4mom->At(i);
		    	float           pt   = mom4->Pt();
		    	float           eta  = abs(mom4->Eta());

		    	
		    	if ((reco_isTracker[i] && !trackerGEMonly) ||
		    		(reco_mu_isGEM [i] && trackerIncludeGEM))
		    	{
		    		myPtEtaTrack->Fill(eta, pt, weigth);
		    		if (!isPositiv)
		    			myPtEtaFakeRateTrack->Fill(eta, pt, weigth);
		    	}
		    	if(reco_isGlobal[i]) {
		    		myPtEtaGlobal->Fill(eta, pt, weigth);
		    		if (!isPositiv)
		    			myPtEtaFakeRateGlobal->Fill(eta, pt, weigth);
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

	auto ptEtaGlobalMerged         = ptEtaGlobal.Merge();
	auto ptEtaEffGlobalMerged      = ptEtaEffGlobal.Merge();
	auto ptEtaFakeRateGlobalMerged = ptEtaFakeRateGlobal.Merge();
	auto ptErrGlobalMerged         = ptErrGlobal.Merge();
	auto etaErrGlobalMerged        = etaErrGlobal.Merge();

	std::cout << std::endl << "process..." << std::endl;

	ptEtaEffTrackMerged ->Divide(ptEtaGenMerged.get());
	ptEtaEffGlobalMerged->Divide(ptEtaGenMerged.get());

	ptEtaFakeRateTrackMerged ->Divide(ptEtaTrackMerged.get());
	ptEtaFakeRateGlobalMerged->Divide(ptEtaGlobalMerged.get());


	get_error_self(ptErrTrackMerged);
	get_error_self(ptErrGlobalMerged);
	get_error_self(etaErrTrackMerged);
	get_error_self(etaErrGlobalMerged);




	TCanvas* trackerCanvas = new TCanvas("Tracker muons", "", 1000, 1000);
    trackerCanvas->Divide(2, 2);
	trackerCanvas->SetFillStyle(1001);
	trackerCanvas->SetFillColor(kWhite);


	trackerCanvas->cd(1);
	ptEtaEffTrackMerged->SetStats(kFALSE);
	ptEtaEffTrackMerged->DrawCopy("COLZ");
	trackerCanvas->cd(3);
	ptEtaFakeRateTrackMerged->SetStats(kFALSE);
	ptEtaFakeRateTrackMerged->DrawCopy("COLZ");

	trackerCanvas->cd(2);
	gPad->SetLogz();
	ptErrTrackMerged->SetStats(kFALSE);
	ptErrTrackMerged->DrawCopy("COLZ");
	trackerCanvas->cd(4);
	gPad->SetLogz();
	etaErrTrackMerged->SetStats(kFALSE);
	etaErrTrackMerged->DrawCopy("COLZ");

	trackerCanvas->Update();




	TCanvas* globalCanvas = new TCanvas("Global muons", "", 1000, 1000);
    globalCanvas->Divide(2, 2);
	globalCanvas->SetFillStyle(1001);
	globalCanvas->SetFillColor(kWhite);


	globalCanvas->cd(1);
	ptEtaEffGlobalMerged->SetStats(kFALSE);
	ptEtaEffGlobalMerged->DrawCopy("COLZ");
	globalCanvas->cd(3);
	ptEtaFakeRateGlobalMerged->SetStats(kFALSE);
	ptEtaFakeRateGlobalMerged->DrawCopy("COLZ");

	globalCanvas->cd(2);
	gPad->SetLogz();
	ptErrGlobalMerged->SetStats(kFALSE);
	ptErrGlobalMerged->DrawCopy("COLZ");
	globalCanvas->cd(4);
	gPad->SetLogz();
	etaErrGlobalMerged->SetStats(kFALSE);
	etaErrGlobalMerged->DrawCopy("COLZ");

	globalCanvas->Update();




	return std::pair(trackerCanvas, globalCanvas);
}

void mu_efficiency() {
	char filename[100];

	// request user input
  	std::cout << "filename: ";
  	scanf("%s", filename);

  	auto [trackerCanvas, globalCanvas] = mu_efficiency_from_name(filename);
  	trackerCanvas->SaveAs("../../figures/mu_eff_tracker.png");
  	globalCanvas ->SaveAs("../../figures/mu_eff_global.png");
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

	auto [trackerCanvas, globalCanvas] = mu_efficiency_from_name(filename);
  	trackerCanvas->SaveAs("../../figures/mu_eff_tracker.png");
  	globalCanvas ->SaveAs("../../figures/mu_eff_global.png");

	return 0;
}