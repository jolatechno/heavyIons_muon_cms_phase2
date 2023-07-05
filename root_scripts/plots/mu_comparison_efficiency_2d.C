#include <iostream>
#include <thread>
#include <utility>
#include <string>

#include <THStack.h>
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

#include <ROOT/TTreeProcessorMT.hxx>
#include <ROOT/TThreadedObject.hxx>

#include "../utilities/embeding_weight.hpp"
#include "../utilities/error_util.hpp"
const bool embeding = false;

std::pair<TCanvas*, TCanvas*> mu_comparison_efficiency_2d_from_name(const char* filename1, const char* filename2) {
	uint etaBins = 60, ptBins = 60;
	float etaMin = 0, etaMax = 4;
	float ptMin  = 0, ptMax  = 6;
	int max_n_event = -1;


	int nthreads = std::thread::hardware_concurrency();
	std::cout << "using " << nthreads << " threads" << std::endl;
	ROOT::EnableImplicitMT(nthreads);


	ROOT::TThreadedObject<TH2F> ptEtaGen1   ("", "", etaBins, etaMin, etaMax, ptBins, ptMin, ptMax);
	ROOT::TThreadedObject<TH2F> ptEtaTrack1 ("", "", etaBins, etaMin, etaMax, ptBins, ptMin, ptMax);
	ROOT::TThreadedObject<TH2F> ptEtaGlobal1("", "", etaBins, etaMin, etaMax, ptBins, ptMin, ptMax);

	ROOT::TThreadedObject<TH2F> ptEtaEffTrack1      ("eff", "Tracker muon reco. efficiency;muon |#eta|;muon p_{t}[GeV]", etaBins, etaMin, etaMax, ptBins, ptMin, ptMax);
	ROOT::TThreadedObject<TH2F> ptEtaEffGlobal1     ("eff", "Global muon reco. efficiency;muon |#eta|;muon p_{t}[GeV]",  etaBins, etaMin, etaMax, ptBins, ptMin, ptMax);
	ROOT::TThreadedObject<TH2F> ptEtaFakeRateTrack1 ("fakerate", "Tracker muon fake rate;muon |#eta|;muon p_{t}[GeV]",   etaBins, etaMin, etaMax, ptBins, ptMin, ptMax);
	ROOT::TThreadedObject<TH2F> ptEtaFakeRateGlobal1("fakerate", "Global muon fake rate;muon |#eta|;muon p_{t}[GeV]",    etaBins, etaMin, etaMax, ptBins, ptMin, ptMax);

    ROOT::TThreadedObject<TH2F> ptErrTrack1  ("Muon chamber hit per muon",  "Tracker muon p_{t} resolution;muon |#eta|;muon p_{t}[GeV]", etaBins, etaMin, etaMax, ptBins, ptMin, ptMax); // "pt err",  "pt err;
    ROOT::TThreadedObject<TH2F> ptErrGlobal1 ("Muon chamber hit per muon",  "Global muon p_{t} resolution;muon |#eta|;muon p_{t}[GeV]",  etaBins, etaMin, etaMax, ptBins, ptMin, ptMax); // "pt err",  "pt err;
    ROOT::TThreadedObject<TH2F> etaErrTrack1 ("Tracker hit per muon",       "Tracker muon #eta resolution;muon |#eta|;muon p_{t}[GeV]",  etaBins, etaMin, etaMax, ptBins, ptMin, ptMax); // "eta err", "eta err;
    ROOT::TThreadedObject<TH2F> etaErrGlobal1("Tracker hit per muon",       "Global muon #eta resolution;muon |#eta|;muon p_{t}[GeV]",   etaBins, etaMin, etaMax, ptBins, ptMin, ptMax); // "eta err", "eta err;


    ROOT::TThreadedObject<TH2F> ptEtaGen2   ("", "", etaBins, etaMin, etaMax, ptBins, ptMin, ptMax);
	ROOT::TThreadedObject<TH2F> ptEtaTrack2 ("", "", etaBins, etaMin, etaMax, ptBins, ptMin, ptMax);
	ROOT::TThreadedObject<TH2F> ptEtaGlobal2("", "", etaBins, etaMin, etaMax, ptBins, ptMin, ptMax);

	ROOT::TThreadedObject<TH2F> ptEtaEffTrack2      ("eff", "Tracker muon reco. efficiency;muon |#eta|;muon p_{t}[GeV]", etaBins, etaMin, etaMax, ptBins, ptMin, ptMax);
	ROOT::TThreadedObject<TH2F> ptEtaEffGlobal2     ("eff", "Global muon reco. efficiency;muon |#eta|;muon p_{t}[GeV]",  etaBins, etaMin, etaMax, ptBins, ptMin, ptMax);
	ROOT::TThreadedObject<TH2F> ptEtaFakeRateTrack2 ("fakerate", "Tracker muon fake rate;muon |#eta|;muon p_{t}[GeV]",   etaBins, etaMin, etaMax, ptBins, ptMin, ptMax);
	ROOT::TThreadedObject<TH2F> ptEtaFakeRateGlobal2("fakerate", "Global muon fake rate;muon |#eta|;muon p_{t}[GeV]",    etaBins, etaMin, etaMax, ptBins, ptMin, ptMax);

    ROOT::TThreadedObject<TH2F> ptErrTrack2  ("Muon chamber hit per muon",  "Tracker muon p_{t} resolution;muon |#eta|;muon p_{t}[GeV]", etaBins, etaMin, etaMax, ptBins, ptMin, ptMax); // "pt err",  "pt err;
    ROOT::TThreadedObject<TH2F> ptErrGlobal2 ("Muon chamber hit per muon",  "Global muon p_{t} resolution;muon |#eta|;muon p_{t}[GeV]",  etaBins, etaMin, etaMax, ptBins, ptMin, ptMax); // "pt err",  "pt err;
    ROOT::TThreadedObject<TH2F> etaErrTrack2 ("Tracker hit per muon",       "Tracker muon #eta resolution;muon |#eta|;muon p_{t}[GeV]",  etaBins, etaMin, etaMax, ptBins, ptMin, ptMax); // "eta err", "eta err;
    ROOT::TThreadedObject<TH2F> etaErrGlobal2("Tracker hit per muon",       "Global muon #eta resolution;muon |#eta|;muon p_{t}[GeV]",   etaBins, etaMin, etaMax, ptBins, ptMin, ptMax); // "eta err", "eta err;


	auto readerFunction = [&](int file) {
		return [&, file=file](TTreeReader &myReader) {
			// Create a TTreeReader for the tree, for instance by passing the
			// TTree's name and the TDirectory / TFile it is in.
			TTreeReaderValue<float> hiHF(myReader, embeding ? "SumET_HF" : "Gen_weight");

			TTreeReaderValue<int>          gen_mu_size (myReader, "Gen_mu_size");
			TTreeReaderValue<int>          reco_mu_size(myReader, "Reco_mu_size");
			TTreeReaderArray<short>        reco_mu_idx (myReader, "Gen_mu_whichRec");
			TTreeReaderArray<short>        gen_mu_idx  (myReader, "Reco_mu_whichGen");
			TTreeReaderValue<TClonesArray> gen_mu_4mom (myReader, "Gen_mu_4mom");
			TTreeReaderValue<TClonesArray> reco_mu_4mom(myReader, "Reco_mu_4mom");

			TTreeReaderArray<bool> reco_isTracker(myReader, "Reco_mu_isTracker");
			TTreeReaderArray<bool> reco_isGlobal (myReader, "Reco_mu_isGlobal");
			TTreeReaderArray<int>  nHitsMu       (myReader, "Reco_mu_nMuValHits");
			TTreeReaderArray<int>  nHitsTracker  (myReader, "Reco_mu_nTrkHits");

			auto myPtEtaGen = file == 1 ? ptEtaGen1.Get() : ptEtaGen2.Get();

			auto myPtEtaTrack          = file == 1 ? ptEtaTrack1.Get()         : ptEtaTrack2.Get();
			auto myPtEtaEffTrack       = file == 1 ? ptEtaEffTrack1.Get()      : ptEtaEffTrack2.Get();
			auto myPtEtaFakeRateTrack  = file == 1 ? ptEtaFakeRateTrack1.Get() : ptEtaFakeRateTrack2.Get();
			auto myPtErrTrack          = file == 1 ? ptErrTrack1.Get()         : ptErrTrack2.Get();
			auto myEtaErrTrack         = file == 1 ? etaErrTrack1.Get()        : etaErrTrack2.Get();

			auto myPtEtaGlobal         = file == 1 ? ptEtaGlobal1.Get()         : ptEtaGlobal2.Get();
			auto myPtEtaEffGlobal      = file == 1 ? ptEtaEffGlobal1.Get()      : ptEtaEffGlobal2.Get();
			auto myPtEtaFakeRateGlobal = file == 1 ? ptEtaFakeRateGlobal1.Get() : ptEtaFakeRateGlobal2.Get();
			auto myPtErrGlobal         = file == 1 ? ptErrGlobal1.Get()         : ptErrGlobal2.Get();
			auto myEtaErrGlobal        = file == 1 ? etaErrGlobal1.Get()        : etaErrGlobal2.Get();

			for (int j = 0; myReader.Next() && (j < max_n_event/nthreads || max_n_event <= 0); j++) {
				float weigth = embeding ? findNcoll(getHiBinFromhiHF(*hiHF)) : 1.f;

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

			    		if (reco_isTracker[reco_idx]) {
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

			    	
			    	if(reco_isTracker[i]) {
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
	};

	auto processFunction = [&](int file) {
		auto ptEtaGenMerged = file == 1 ? ptEtaGen1.Merge() : ptEtaGen2.Merge();

		auto ptEtaTrackMerged         = file == 1 ? ptEtaTrack1.Merge()         : ptEtaTrack2.Merge();
		auto ptEtaEffTrackMerged      = file == 1 ? ptEtaEffTrack1.Merge()      : ptEtaEffTrack2.Merge();
		auto ptEtaFakeRateTrackMerged = file == 1 ? ptEtaFakeRateTrack1.Merge() : ptEtaFakeRateTrack2.Merge();
		auto ptErrTrackMerged         = file == 1 ? ptErrTrack1.Merge()         : ptErrTrack2.Merge();
		auto etaErrTrackMerged        = file == 1 ? etaErrTrack1.Merge()        : etaErrTrack2.Merge();

		auto ptEtaGlobalMerged         = file == 1 ? ptEtaGlobal1.Merge()         : ptEtaGlobal2.Merge();
		auto ptEtaEffGlobalMerged      = file == 1 ? ptEtaEffGlobal1.Merge()      : ptEtaEffGlobal2.Merge();
		auto ptEtaFakeRateGlobalMerged = file == 1 ? ptEtaFakeRateGlobal1.Merge() : ptEtaFakeRateGlobal2.Merge();
		auto ptErrGlobalMerged         = file == 1 ? ptErrGlobal1.Merge()         : ptErrGlobal2.Merge();
		auto etaErrGlobalMerged        = file == 1 ? etaErrGlobal1.Merge()        : etaErrGlobal2.Merge();

		get_error_self(ptErrTrackMerged);
		get_error_self(ptErrGlobalMerged);
		get_error_self(etaErrTrackMerged);
		get_error_self(etaErrGlobalMerged);

		auto divide = [&](std::shared_ptr<TH2F> &hist1, std::shared_ptr<TH2F> &hist2) {
			Int_t n = hist1->GetNcells();
			for(Int_t i = 0; i < n; i++) {
				float hist1_value = hist1->GetBinContent(i);
				float hist2_value = hist2->GetBinContent(i);

				if (hist2_value != 0 && hist1_value != 0) {
		    		hist1->SetBinContent(i, hist1_value/hist2_value);
				}
			}
		};

		divide(ptEtaEffTrackMerged, ptEtaGenMerged);
		divide(ptEtaEffGlobalMerged, ptEtaGenMerged);

		divide(ptEtaFakeRateTrackMerged, ptEtaTrackMerged);
		divide(ptEtaFakeRateGlobalMerged, ptEtaGlobalMerged);
	};


	// read file
	std::cout << "openning \"" << filename1 << "\" and \"" << filename2 << "\"..." << std::endl;
	ROOT::TTreeProcessorMT tp1(filename1, "hionia/myTree");
	ROOT::TTreeProcessorMT tp2(filename2, "hionia/myTree");

  	std::cout << "reading file 1..." << std::endl;
	tp1.Process(readerFunction(1));

	std::cout << "processing file 1..." << std::endl;
	processFunction(1);

	std::cout << "reading file 2..." << std::endl;
	tp2.Process(readerFunction(2));

	std::cout << "processing file 2..." << std::endl;
	processFunction(2);


	auto getRelativeError = [&](std::shared_ptr<TH2F> hist1, std::shared_ptr<TH2F> hist2, float min_value, float max_value) {
		float zero_value = 1e-5;

		Int_t n = hist1->GetNcells();
		for(Int_t i = 0; i < n; i++) {
			float hist1_value = hist1->GetBinContent(i);
			float hist2_value = hist2->GetBinContent(i);

			if (hist1_value != 0) {
	    		hist1->SetBinContent(i, 
	    			std::min(
	    				max_value,
	    			std::max(
	    				min_value,
	    				(hist1_value - hist2_value)/hist1_value
	    			)));
			} else if (hist2_value != 0) {
				hist1->SetBinContent(i, zero_value);
			}
		}

		return hist1;
	};

	auto ptEtaEffTrackComp       = getRelativeError(ptEtaEffTrack1.Get(),       ptEtaEffTrack2.Get(),       1e-3f, 0.5f);
	auto ptEtaEffGlobalComp      = getRelativeError(ptEtaEffGlobal1.Get(),      ptEtaEffGlobal2.Get(),      1e-3f, 0.5f);
	auto ptEtaFakeRateTrackComp  = getRelativeError(ptEtaFakeRateTrack2.Get(),  ptEtaFakeRateTrack1.Get(),  1e-3f, 1.f);
	auto ptEtaFakeRateGlobalComp = getRelativeError(ptEtaFakeRateGlobal2.Get(), ptEtaFakeRateGlobal1.Get(), 1e-3f, 1.f);

	auto ptErrTrackComp   = getRelativeError(ptErrTrack2.Get(),   ptErrTrack1.Get(),   1e-3f, 1.f);
    auto ptErrGlobalComp  = getRelativeError(ptErrGlobal2.Get(),  ptErrGlobal1.Get(),  1e-3f, 1.f);
    auto etaErrTrackComp  = getRelativeError(etaErrTrack2.Get(),  etaErrTrack1.Get(),  1e-3f, 1.f);
    auto etaErrGlobalComp = getRelativeError(etaErrGlobal2.Get(), etaErrGlobal1.Get(), 1e-3f, 1.f);


	TCanvas* trackerCanvas = new TCanvas("Tracker muons", "", 1000, 1000);
    trackerCanvas->Divide(2, 2);
	trackerCanvas->SetFillStyle(1001);
	trackerCanvas->SetFillColor(kWhite);


	trackerCanvas->cd(1);
	// gPad->SetLogz();
	ptEtaEffTrackComp->SetStats(kFALSE);
	ptEtaEffTrackComp->DrawCopy("COLZ");
	trackerCanvas->cd(3);
	// gPad->SetLogz();
	ptEtaFakeRateTrackComp->SetStats(kFALSE);
	ptEtaFakeRateTrackComp->DrawCopy("COLZ");

	trackerCanvas->cd(2);
	// gPad->SetLogz();
	ptErrTrackComp->SetStats(kFALSE);
	ptErrTrackComp->DrawCopy("COLZ");
	trackerCanvas->cd(4);
	// gPad->SetLogz();
	etaErrTrackComp->SetStats(kFALSE);
	etaErrTrackComp->DrawCopy("COLZ");

	trackerCanvas->Update();




	TCanvas* globalCanvas = new TCanvas("Global muons", "", 1000, 1000);
    globalCanvas->Divide(2, 2);
	globalCanvas->SetFillStyle(1001);
	globalCanvas->SetFillColor(kWhite);


	globalCanvas->cd(1);
	// gPad->SetLogz();
	ptEtaEffGlobalComp->SetStats(kFALSE);
	ptEtaEffGlobalComp->DrawCopy("COLZ");
	globalCanvas->cd(3);
	// gPad->SetLogz();
	ptEtaFakeRateGlobalComp->SetStats(kFALSE);
	ptEtaFakeRateGlobalComp->DrawCopy("COLZ");

	globalCanvas->cd(2);
	// gPad->SetLogz();
	ptErrGlobalComp->SetStats(kFALSE);
	ptErrGlobalComp->DrawCopy("COLZ");
	globalCanvas->cd(4);
	// gPad->SetLogz();
	etaErrGlobalComp->SetStats(kFALSE);
	etaErrGlobalComp->DrawCopy("COLZ");

	globalCanvas->Update();




	return std::pair(trackerCanvas, globalCanvas);
}

void mu_efficiency() {
	char filename1[100], filename2[100];

	// request user input
  	std::cout << "filename1: ";
  	scanf("%s", filename1);
  	std::cout << "filename2: ";
  	scanf("%s", filename2);

  	auto [trackerCanvas, globalCanvas] = mu_comparison_efficiency_2d_from_name(filename1, filename2);
  	trackerCanvas->SaveAs("../../figures/mu_comparison_eff_2d_tracker.png");
  	globalCanvas ->SaveAs("../../figures/mu_comparison_eff_2d_global.png");
}

int main(int argc, char** argv) {
	if (argc < 3) {
		std::cerr << "Not enough filename given !" << std::endl;
  		exit(-1);
	} else if (argc > 3) {
		std::cerr << "Too many arguments !" << std::endl;
  		exit(-1);
	}

	char *filename1 = argv[1];
	char *filename2 = argv[2];

	auto [trackerCanvas, globalCanvas] = mu_comparison_efficiency_2d_from_name(filename1, filename2);
  	trackerCanvas->SaveAs("../../figures/mu_comparison_eff_2d_tracker.png");
  	globalCanvas ->SaveAs("../../figures/mu_comparison_eff_2d_global.png");

	return 0;
}