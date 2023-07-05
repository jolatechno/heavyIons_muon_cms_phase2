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

/*
// make mu_comparison_efficiency && ./mu_comparison_efficiency.out ../test_files/embeded_phase2.root ../test_files/embeded_run3.root
const bool embeding = false;
const std::string label1 = "phase 2", label2 = "run 3";
const bool binning1 = true, binning2 = true;
const bool hasNtracks1 = false, hasNtracks2 = true;
/* */

/* */
// make mu_comparison_efficiency && ./mu_comparison_efficiency.out ../test_files/pp_phase2.root ../test_files/pp_run3.root
const bool embeding = false;
const std::string label1 = "phase 2", label2 = "run 3";
const bool binning1 = true, binning2 = true;
const bool hasNtracks1 = true, hasNtracks2 = true;
/* */

/*
// make mu_comparison_efficiency && ./mu_comparison_efficiency.out ../test_files/pp_run3.root ../test_files/embeded_run3.root
const bool embeding = false;
const std::string label1 = "run 3 p-p", label2 = "run 3 embeded";
const bool binning1 = false, binning2 = true;
const bool hasNtracks1 = true, hasNtracks2 = true;
/* */

/*
// make mu_comparison_efficiency && ./mu_comparison_efficiency.out ../test_files/pp_phase2.root ../test_files/embeded_phase2.root
const bool embeding = false;
const std::string label1 = "phase2 p-p", label2 = "phase2 embeded";
const bool binning1 = false, binning2 = true;
const bool hasNtracks1 = true, hasNtracks2 = false;
/* */


std::vector<float> percentiles = {0, 0.1, 0.9, 1};

std::vector<std::pair<TCanvas*, TCanvas*>> mu_comparison_efficiency_from_name(const char* filename1, const char* filename2) {
	int nthreads = std::thread::hardware_concurrency();
	std::cout << "using " << nthreads << " threads" << std::endl;
	ROOT::EnableImplicitMT(nthreads);


	uint etaBins = 60, ptBins = 60;
	float etaMin = 0, etaMax = 4;
	float ptMin  = 0, ptMax  = 6;
	int max_n_event = -1;

	float ptEtaCutLow = 0.f, ptEtaCutHigh = 2.4;
	float etaPtCutLow = 0.f, etaPtCutHigh = 6;


	// read file
	std::cout << "openning \"" << filename1 << "\" and \"" << filename2 << "\"..." << std::endl;
	ROOT::TTreeProcessorMT tp1(filename1, "hionia/myTree");
	ROOT::TTreeProcessorMT tp2(filename2, "hionia/myTree");
	
	ROOT::TThreadedObject<std::vector<int>> nTracks1, nTracks2;
	std::vector<int> limits_file1 = {}, limits_file2 = {};

	if (binning1) {
		std::cout << "reading file 1 nTracks..." << std::endl;
		if (hasNtracks1) {
			tp1.Process(readField<short>("Ntracks", nTracks1, max_n_event, nthreads));
		} else {
			tp1.Process(readField<int>("NTotalTracks", nTracks1, max_n_event, nthreads));
		}

		std::cout << "processing file 1 nTracks..." << std::endl;
		processFunctionPercentile(percentiles, limits_file1, nTracks1);
	}
	if (binning2) {
		std::cout << "reading file 2 nTracks..." << std::endl;
		if (hasNtracks2) {
			tp2.Process(readField<short>("Ntracks", nTracks2, max_n_event, nthreads));
		} else {
			tp2.Process(readField<int>("NTotalTracks", nTracks2, max_n_event, nthreads));
		}

		std::cout << "processing file 2 nTracks..." << std::endl;
		processFunctionPercentile(percentiles, limits_file2, nTracks2);
	}


	std::vector<std::pair<TCanvas*, TCanvas*>> canvas;

	for (int percentile_idx = 0; percentile_idx < percentiles.size() - 1; ++percentile_idx) {
		float percentile_begin = percentiles[percentile_idx];
		float percentile_end   = percentiles[percentile_idx + 1];
		std::cout << std::endl << "Ploting for percentile " << percentile_begin << "-" << percentile_end << std::endl;
		

		ROOT::TThreadedObject<TH1F> ptGen1   ("", "", ptBins, ptMin, ptMax);
		ROOT::TThreadedObject<TH1F> ptTrack1 ("", "", ptBins, ptMin, ptMax);
		ROOT::TThreadedObject<TH1F> ptGlobal1("", "", ptBins, ptMin, ptMax);
		
		ROOT::TThreadedObject<TH1F> etaGen1   ("", "", etaBins, etaMin, etaMax);
		ROOT::TThreadedObject<TH1F> etaTrack1 ("", "", etaBins, etaMin, etaMax);
		ROOT::TThreadedObject<TH1F> etaGlobal1("", "", etaBins, etaMin, etaMax);

		ROOT::TThreadedObject<TH1F> ptGen2   ("", "", ptBins, ptMin, ptMax);
		ROOT::TThreadedObject<TH1F> ptTrack2 ("", "", ptBins, ptMin, ptMax);
		ROOT::TThreadedObject<TH1F> ptGlobal2("", "", ptBins, ptMin, ptMax);

		ROOT::TThreadedObject<TH1F> etaGen2   ("", "", etaBins, etaMin, etaMax);
		ROOT::TThreadedObject<TH1F> etaTrack2 ("", "", etaBins, etaMin, etaMax);
		ROOT::TThreadedObject<TH1F> etaGlobal2("", "", etaBins, etaMin, etaMax);


		ROOT::TThreadedObject<TH1F> ptEffTrack1      ("eff",      (label1 + ";muon p_{t}[GeV];tracker muon reco. efficiency").c_str(), ptBins, ptMin, ptMax);
		ROOT::TThreadedObject<TH1F> ptEffGlobal1     ("eff",      (label1 + ";muon p_{t}[GeV];global muon reco. efficiency" ).c_str(), ptBins, ptMin, ptMax);
		ROOT::TThreadedObject<TH1F> ptFakeRateTrack1 ("fakerate", (label1 + ";muon p_{t}[GeV];tracker muon fake rate"       ).c_str(), ptBins, ptMin, ptMax);
		ROOT::TThreadedObject<TH1F> ptFakeRateGlobal1("fakerate", (label1 + ";muon p_{t}[GeV];global muon fake rate"        ).c_str(), ptBins, ptMin, ptMax);
		ROOT::TThreadedObject<TH1F> ptTrackHitTrack1 ("eff",      (label1 + ";muon p_{t}[GeV];tracker muon tracker hit"     ).c_str(), ptBins, ptMin, ptMax);
		ROOT::TThreadedObject<TH1F> ptTrackHitGlobal1("eff",      (label1 + ";muon p_{t}[GeV];global muon tracker hit"      ).c_str(), ptBins, ptMin, ptMax);
		ROOT::TThreadedObject<TH1F> ptMuCHitTrack1   ("eff",      (label1 + ";muon p_{t}[GeV];tracker muon muon chamber hit").c_str(), ptBins, ptMin, ptMax);
		ROOT::TThreadedObject<TH1F> ptMuCHitGlobal1  ("eff",      (label1 + ";muon p_{t}[GeV];global muon muon chamber hit" ).c_str(), ptBins, ptMin, ptMax);

		ROOT::TThreadedObject<TH1F> etaEffTrack1      ("eff",      (label1 + ";muon |#eta|;tracker muon reco. efficiency").c_str(), etaBins, etaMin, etaMax);
		ROOT::TThreadedObject<TH1F> etaEffGlobal1     ("eff",      (label1 + ";muon |#eta|;global muon reco. efficiency" ).c_str(), etaBins, etaMin, etaMax);
		ROOT::TThreadedObject<TH1F> etaFakeRateTrack1 ("fakerate", (label1 + ";muon |#eta|;tracker muon fake rate"       ).c_str(), etaBins, etaMin, etaMax);
		ROOT::TThreadedObject<TH1F> etaFakeRateGlobal1("fakerate", (label1 + ";muon |#eta|;global muon fake rate"        ).c_str(), etaBins, etaMin, etaMax);
		ROOT::TThreadedObject<TH1F> etaTrackHitTrack1 ("eff",      (label1 + ";muon |#eta|;tracker muon tracker hit"     ).c_str(), etaBins, etaMin, etaMax);
		ROOT::TThreadedObject<TH1F> etaTrackHitGlobal1("eff",      (label1 + ";muon |#eta|;global muon tracker hit"      ).c_str(), etaBins, etaMin, etaMax);
		ROOT::TThreadedObject<TH1F> etaMuCHitTrack1   ("eff",      (label1 + ";muon |#eta|;tracker muon muon chamber hit").c_str(), etaBins, etaMin, etaMax);
		ROOT::TThreadedObject<TH1F> etaMuCHitGlobal1  ("eff",      (label1 + ";muon |#eta|;global muon muon chamber hit" ).c_str(), etaBins, etaMin, etaMax);

		ROOT::TThreadedObject<TH1F> ptEffTrack2      ("eff",      (label2 + ";muon p_{t}[GeV];tracker muon reco. efficiency").c_str(), ptBins, ptMin, ptMax);
		ROOT::TThreadedObject<TH1F> ptEffGlobal2     ("eff",      (label2 + ";muon p_{t}[GeV];global muon reco. efficiency" ).c_str(), ptBins, ptMin, ptMax);
		ROOT::TThreadedObject<TH1F> ptFakeRateTrack2 ("fakerate", (label2 + ";muon p_{t}[GeV];tracker muon fake rate"       ).c_str(), ptBins, ptMin, ptMax);
		ROOT::TThreadedObject<TH1F> ptFakeRateGlobal2("fakerate", (label2 + ";muon p_{t}[GeV];global muon fake rate"        ).c_str(), ptBins, ptMin, ptMax);
		ROOT::TThreadedObject<TH1F> ptTrackHitTrack2 ("eff",      (label2 + ";muon p_{t}[GeV];tracker muon tracker hit"     ).c_str(), ptBins, ptMin, ptMax);
		ROOT::TThreadedObject<TH1F> ptTrackHitGlobal2("eff",      (label2 + ";muon p_{t}[GeV];global muon tracker hit"      ).c_str(), ptBins, ptMin, ptMax);
		ROOT::TThreadedObject<TH1F> ptMuCHitTrack2   ("eff",      (label2 + ";muon p_{t}[GeV];tracker muon muon chamber hit").c_str(), ptBins, ptMin, ptMax);
		ROOT::TThreadedObject<TH1F> ptMuCHitGlobal2  ("eff",      (label2 + ";muon p_{t}[GeV];global muon muon chamber hit" ).c_str(), ptBins, ptMin, ptMax);

		ROOT::TThreadedObject<TH1F> etaEffTrack2      ("eff",      (label2 + ";muon |#eta|;tracker muon reco. efficiency" ).c_str(), etaBins, etaMin, etaMax);
		ROOT::TThreadedObject<TH1F> etaEffGlobal2     ("eff",      (label2 + ";muon |#eta|;global muon reco. efficiency"  ).c_str(), etaBins, etaMin, etaMax);
		ROOT::TThreadedObject<TH1F> etaFakeRateTrack2 ("fakerate", (label2 + ";muon |#eta|;tracker muon fake rate"        ).c_str(), etaBins, etaMin, etaMax);
		ROOT::TThreadedObject<TH1F> etaFakeRateGlobal2("fakerate", (label2 + ";muon |#eta|;global muon fake rate"         ).c_str(), etaBins, etaMin, etaMax);
		ROOT::TThreadedObject<TH1F> etaTrackHitTrack2 ("eff",      (label2 + ";muon |#eta|;tracker muon tracker hit"      ).c_str(), etaBins, etaMin, etaMax);
		ROOT::TThreadedObject<TH1F> etaTrackHitGlobal2("eff",      (label2 + ";muon |#eta|;global muon tracker hit"       ).c_str(), etaBins, etaMin, etaMax);
		ROOT::TThreadedObject<TH1F> etaMuCHitTrack2   ("eff",      (label2 + ";muon |#eta|;ttracker muon muon chamber hit").c_str(), etaBins, etaMin, etaMax);
		ROOT::TThreadedObject<TH1F> etaMuCHitGlobal2  ("eff",      (label2 + ";muon |#eta|;global muon muon chamber hit"  ).c_str(), etaBins, etaMin, etaMax);

		auto readerFunction = [&](int file, int limitInf, int limitSup, bool nTracks_exists) {
			return [&, file=file, limitInf=limitInf, limitSup=limitSup, nTracks_exists=nTracks_exists](TTreeReader &myReader) {
				TTreeReaderValue<short> Ntracks     (myReader, nTracks_exists ? "Ntracks"     : "nPV");
				TTreeReaderValue<int>   NTotalTracks(myReader, nTracks_exists ? "Gen_mu_size" : "NTotalTracks");

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

				TTreeReaderArray<int>  nHitsMu     (myReader, "Reco_mu_nMuValHits");
				TTreeReaderArray<int>  nHitsTracker(myReader, "Reco_mu_nTrkHits");


				auto myPtGen             = file == 1 ? ptGen1.Get()             : ptGen2.Get();
				auto myEtaGen            = file == 1 ? etaGen1.Get()            : etaGen2.Get();

				auto myPtTrack           = file == 1 ? ptTrack1.Get()           : ptTrack2.Get();
				auto myPtGlobal          = file == 1 ? ptGlobal1.Get()          : ptGlobal2.Get();

				auto myEtaTrack          = file == 1 ? etaTrack1.Get()          : etaTrack2.Get();
				auto myEtaGlobal         = file == 1 ? etaGlobal1.Get()         : etaGlobal2.Get();

				auto myPtEffTrack        = file == 1 ? ptEffTrack1.Get()        : ptEffTrack2.Get();
				auto myPtEffGlobal       = file == 1 ? ptEffGlobal1.Get()       : ptEffGlobal2.Get();
				auto myPtFakeRateTrack   = file == 1 ? ptFakeRateTrack1.Get()   : ptFakeRateTrack2.Get();
				auto myPtFakeRateGlobal  = file == 1 ? ptFakeRateGlobal1.Get()  : ptFakeRateGlobal2.Get();
				auto myPtTrackHitTrack   = file == 1 ? ptTrackHitTrack1.Get()   : ptTrackHitTrack2.Get();
				auto myPtTrackHitGlobal  = file == 1 ? ptTrackHitGlobal1.Get()  : ptTrackHitGlobal2.Get();
				auto myPtMuCHitTrack     = file == 1 ? ptMuCHitTrack1.Get()     : ptMuCHitTrack2.Get();
				auto myPtMuCHitGlobal    = file == 1 ? ptMuCHitGlobal1.Get()    : ptMuCHitGlobal2.Get();

				auto myEtaEffTrack       = file == 1 ? etaEffTrack1.Get()       : etaEffTrack2.Get();
				auto myEtaEffGlobal      = file == 1 ? etaEffGlobal1.Get()      : etaEffGlobal2.Get();
				auto myEtaFakeRateTrack  = file == 1 ? etaFakeRateTrack1.Get()  : etaFakeRateTrack2.Get();
				auto myEtaFakeRateGlobal = file == 1 ? etaFakeRateGlobal1.Get() : etaFakeRateGlobal2.Get();
				auto myEtaTrackHitTrack  = file == 1 ? etaTrackHitTrack1.Get()  : etaTrackHitTrack2.Get();
				auto myEtaTrackHitGlobal = file == 1 ? etaTrackHitGlobal1.Get() : etaTrackHitGlobal2.Get();
				auto myEtaMuCHitTrack    = file == 1 ? etaMuCHitTrack1.Get()    : etaMuCHitTrack2.Get();
				auto myEtaMuCHitGlobal   = file == 1 ? etaMuCHitGlobal1.Get()   : etaMuCHitGlobal2.Get();

				for (int j = 0; myReader.Next() && (j < max_n_event/nthreads || max_n_event <= 0); j++) {
					if (nTracks_exists) {
						if (limitInf > *Ntracks      ||      *Ntracks > limitSup) {
							continue;
						}
					} else {
						if (limitInf > *NTotalTracks || *NTotalTracks > limitSup) {
							continue;
						}
					}

					float weigth = embeding ? findNcoll(getHiBinFromhiHF(*hiHF)) : 1.f;

					for (short i = 0; i < *gen_mu_size; ++i) {
				    	short  reco_idx  = reco_mu_idx[i];
				    	bool   isPositiv = reco_idx != -1;

				    	TLorentzVector* mom4 = (TLorentzVector*)gen_mu_4mom->At(i);
				    	float           pt   = mom4->Pt();
				    	float           eta  = abs(mom4->Eta());

				    	if (ptEtaCutLow < eta && eta < ptEtaCutHigh)
				    		myPtGen ->Fill(pt);
				    	if (etaPtCutLow < pt && pt < etaPtCutHigh)
				    		myEtaGen->Fill(eta);

				    	if (isPositiv) {
				    		      mom4   = (TLorentzVector*)reco_mu_4mom->At(reco_idx);
				    		float recPt  = mom4->Pt();
				    		float recEta = abs(mom4->Eta());

				    		if (reco_isTracker[reco_idx]) {
				    			if (ptEtaCutLow < eta && eta < ptEtaCutHigh) {
				    				myPtEffTrack->Fill(pt);

						    		myPtTrackHitTrack->Fill(pt, nHitsTracker[reco_idx]);
						    		myPtMuCHitTrack  ->Fill(pt, nHitsMu     [reco_idx]);
						    	}
				    			if (etaPtCutLow < pt && pt < etaPtCutHigh) {
				    				myEtaEffTrack->Fill(eta);

						    		myEtaTrackHitTrack->Fill(eta, nHitsTracker[reco_idx]);
						    		myEtaMuCHitTrack  ->Fill(eta, nHitsMu     [reco_idx]);
				    			}
				    		}
				    		if (reco_isGlobal[reco_idx]) {
				    			if (ptEtaCutLow < eta && eta < ptEtaCutHigh) {
				    				myPtEffGlobal->Fill(pt, 1.f);

						    		myPtTrackHitGlobal->Fill(pt, nHitsTracker[reco_idx]);
						    		myPtMuCHitGlobal  ->Fill(pt, nHitsMu     [reco_idx]);
				    			}
				    			if (etaPtCutLow < pt && pt < etaPtCutHigh) {
				    				myEtaEffGlobal->Fill(eta, 1.f);

						    		myEtaTrackHitGlobal->Fill(eta, nHitsTracker[reco_idx]);
						    		myEtaMuCHitGlobal  ->Fill(eta, nHitsMu     [reco_idx]);
				    			}
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
				    		if (ptEtaCutLow < eta && eta < ptEtaCutHigh)
				    			myPtTrack ->Fill(pt);
				    		if (etaPtCutLow < pt && pt < etaPtCutHigh)
				    			myEtaTrack->Fill(eta);

				    		if (!isPositiv) {
				    			if (ptEtaCutLow < eta && eta < ptEtaCutHigh)
					    			myPtFakeRateTrack ->Fill(pt);
				    			if (etaPtCutLow < pt && pt < etaPtCutHigh)
					    			myEtaFakeRateTrack->Fill(eta);
				    		}
				    	}
				    	if(reco_isGlobal[i]) {
				    		if (ptEtaCutLow < eta && eta < ptEtaCutHigh)
				    			myPtGlobal ->Fill(pt);
				    		if (etaPtCutLow < pt && pt < etaPtCutHigh)
				    			myEtaGlobal->Fill(eta);

				    		if (!isPositiv) {
				    			if (ptEtaCutLow < eta && eta < ptEtaCutHigh)
					    			myPtFakeRateGlobal ->Fill(pt);
				    			if (etaPtCutLow < pt && pt < etaPtCutHigh)
					    			myEtaFakeRateGlobal->Fill(eta);
				    		}
				    	}
				    }
				}
			};
		};


		auto processFunction = [&](int file) {
			auto myPtGen             = file == 1 ? ptGen1.Merge()             : ptGen2.Merge();
			auto myEtaGen            = file == 1 ? etaGen1.Merge()            : etaGen2.Merge();

			auto myPtTrack           = file == 1 ? ptTrack1.Merge()           : ptTrack2.Merge();
			auto myPtGlobal          = file == 1 ? ptGlobal1.Merge()          : ptGlobal2.Merge();

			auto myEtaTrack          = file == 1 ? etaTrack1.Merge()          : etaTrack2.Merge();
			auto myEtaGlobal         = file == 1 ? etaGlobal1.Merge()         : etaGlobal2.Merge();

			auto myPtEffTrack        = file == 1 ? ptEffTrack1.Merge()        : ptEffTrack2.Merge();
			auto myPtEffGlobal       = file == 1 ? ptEffGlobal1.Merge()       : ptEffGlobal2.Merge();
			auto myPtFakeRateTrack   = file == 1 ? ptFakeRateTrack1.Merge()   : ptFakeRateTrack2.Merge();
			auto myPtFakeRateGlobal  = file == 1 ? ptFakeRateGlobal1.Merge()  : ptFakeRateGlobal2.Merge();
			auto myPtTrackHitTrack   = file == 1 ? ptTrackHitTrack1.Merge()   : ptTrackHitTrack2.Merge();
			auto myPtTrackHitGlobal  = file == 1 ? ptTrackHitGlobal1.Merge()  : ptTrackHitGlobal2.Merge();
			auto myPtMuCHitTrack     = file == 1 ? ptMuCHitTrack1.Merge()     : ptMuCHitTrack2.Merge();
			auto myPtMuCHitGlobal    = file == 1 ? ptMuCHitGlobal1.Merge()    : ptMuCHitGlobal2.Merge();

			auto myEtaEffTrack       = file == 1 ? etaEffTrack1.Merge()       : etaEffTrack2.Merge();
			auto myEtaEffGlobal      = file == 1 ? etaEffGlobal1.Merge()      : etaEffGlobal2.Merge();
			auto myEtaFakeRateTrack  = file == 1 ? etaFakeRateTrack1.Merge()  : etaFakeRateTrack2.Merge();
			auto myEtaFakeRateGlobal = file == 1 ? etaFakeRateGlobal1.Merge() : etaFakeRateGlobal2.Merge();
			auto myEtaTrackHitTrack  = file == 1 ? etaTrackHitTrack1.Merge()  : etaTrackHitTrack2.Merge();
			auto myEtaTrackHitGlobal = file == 1 ? etaTrackHitGlobal1.Merge() : etaTrackHitGlobal2.Merge();
			auto myEtaMuCHitTrack    = file == 1 ? etaMuCHitTrack1.Merge()    : etaMuCHitTrack2.Merge();
			auto myEtaMuCHitGlobal   = file == 1 ? etaMuCHitGlobal1.Merge()   : etaMuCHitGlobal2.Merge();


			myPtTrackHitTrack ->Divide(myPtEffTrack.get());
			myPtTrackHitGlobal->Divide(myPtEffGlobal.get());
			myPtMuCHitTrack   ->Divide(myPtEffTrack.get());
			myPtMuCHitGlobal  ->Divide(myPtEffGlobal.get());

			myEtaTrackHitTrack ->Divide(myEtaEffTrack.get());
			myEtaTrackHitGlobal->Divide(myEtaEffGlobal.get());
			myEtaMuCHitTrack   ->Divide(myEtaEffTrack.get());
			myEtaMuCHitGlobal  ->Divide(myEtaEffGlobal.get());

			myPtEffTrack  ->Divide(myPtGen.get());
			myPtEffGlobal ->Divide(myPtGen.get());

			myEtaEffTrack ->Divide(myEtaGen.get());
			myEtaEffGlobal->Divide(myEtaGen.get());

			myPtFakeRateTrack  ->Divide(myPtTrack.get());
			myPtFakeRateGlobal ->Divide(myPtGlobal.get());

			myEtaFakeRateTrack ->Divide(myEtaTrack.get());
			myEtaFakeRateGlobal->Divide(myEtaGlobal.get());
		};


		auto drawFunction = [&](bool trackerOrGlobal, TCanvas *canva) {
			auto myPtEff1        = trackerOrGlobal ? ptEffTrack1.Get()       : ptEffGlobal1.Get();
			auto myPtEff2        = trackerOrGlobal ? ptEffTrack2.Get()       : ptEffGlobal2.Get();
			auto myPtFakeRate1   = trackerOrGlobal ? ptFakeRateTrack1.Get()  : ptFakeRateGlobal1.Get();
			auto myPtFakeRate2   = trackerOrGlobal ? ptFakeRateTrack2.Get()  : ptFakeRateGlobal2.Get();
			auto myPtTrackHit1   = trackerOrGlobal ? ptTrackHitTrack1.Get()  : ptTrackHitGlobal1.Get();
			auto myPtTrackHit2   = trackerOrGlobal ? ptTrackHitTrack2.Get()  : ptTrackHitGlobal2.Get();
			auto myPtMuCHit1     = trackerOrGlobal ? ptMuCHitTrack1.Get()    : ptMuCHitGlobal1.Get();
			auto myPtMuCHit2     = trackerOrGlobal ? ptMuCHitTrack2.Get()    : ptMuCHitGlobal2.Get();

			auto myEtaEff1       = trackerOrGlobal ? etaEffTrack1.Get()      : etaEffGlobal1.Get();
			auto myEtaEff2       = trackerOrGlobal ? etaEffTrack2.Get()      : etaEffGlobal2.Get();
			auto myEtaFakeRate1  = trackerOrGlobal ? etaFakeRateTrack1.Get() : etaFakeRateGlobal1.Get();
			auto myEtaFakeRate2  = trackerOrGlobal ? etaFakeRateTrack2.Get() : etaFakeRateGlobal2.Get();
			auto myEtaTrackHit1  = trackerOrGlobal ? etaTrackHitTrack1.Get() : etaTrackHitGlobal1.Get();
			auto myEtaTrackHit2  = trackerOrGlobal ? etaTrackHitTrack2.Get() : etaTrackHitGlobal2.Get();
			auto myEtaMuCHit1    = trackerOrGlobal ? etaMuCHitTrack1.Get()   : etaMuCHitGlobal1.Get();
			auto myEtaMuCHit2    = trackerOrGlobal ? etaMuCHitTrack2.Get()   : etaMuCHitGlobal2.Get();


			canva->cd(1);
			gStyle->SetOptTitle(0);
			myPtEff1->SetStats(kFALSE);
			myPtEff1->SetLineColor(kRed);
			myPtEff1->DrawCopy("cp");
			myPtEff2->SetStats(kFALSE);
			myPtEff2->SetLineColor(kBlue);
			myPtEff2->DrawCopy("cp same");
			gPad->BuildLegend();
			canva->cd(5);
			gStyle->SetOptTitle(0);
			myEtaEff1->SetStats(kFALSE);
			myEtaEff1->SetLineColor(kRed);
			myEtaEff1->DrawCopy("cp");
			myEtaEff2->SetStats(kFALSE);
			myEtaEff2->SetLineColor(kBlue);
			myEtaEff2->DrawCopy("cp same");
			gPad->BuildLegend();

			canva->cd(2);
			gStyle->SetOptTitle(0);
			myPtFakeRate1->SetStats(kFALSE);
			myPtFakeRate1->SetLineColor(kRed);
			myPtFakeRate1->DrawCopy("cp");
			myPtFakeRate2->SetStats(kFALSE);
			myPtFakeRate2->SetLineColor(kBlue);
			myPtFakeRate2->DrawCopy("cp same");
			gPad->BuildLegend();
			canva->cd(6);
			gStyle->SetOptTitle(0);
			myEtaFakeRate1->SetStats(kFALSE);
			myEtaFakeRate1->SetLineColor(kRed);
			myEtaFakeRate1->DrawCopy("cp");
			myEtaFakeRate2->SetStats(kFALSE);
			myEtaFakeRate2->SetLineColor(kBlue);
			myEtaFakeRate2->DrawCopy("cp same");
			gPad->BuildLegend();

			canva->cd(3);
			gStyle->SetOptTitle(0);
			myPtTrackHit1->SetStats(kFALSE);
			myPtTrackHit1->SetLineColor(kRed);
			myPtTrackHit1->DrawCopy("cp");
			myPtTrackHit2->SetStats(kFALSE);
			myPtTrackHit2->SetLineColor(kBlue);
			myPtTrackHit2->DrawCopy("cp same");
			gPad->BuildLegend();
			canva->cd(7);
			gStyle->SetOptTitle(0);
			myEtaTrackHit1->SetStats(kFALSE);
			myEtaTrackHit1->SetLineColor(kRed);
			myEtaTrackHit1->DrawCopy("cp");
			myEtaTrackHit2->SetStats(kFALSE);
			myEtaTrackHit2->SetLineColor(kBlue);
			myEtaTrackHit2->DrawCopy("cp same");
			gPad->BuildLegend();

			canva->cd(4);
			gStyle->SetOptTitle(0);
			myPtMuCHit1->SetStats(kFALSE);
			myPtMuCHit1->SetLineColor(kRed);
			myPtMuCHit1->DrawCopy("cp");
			myPtMuCHit2->SetStats(kFALSE);
			myPtMuCHit2->SetLineColor(kBlue);
			myPtMuCHit2->DrawCopy("cp same");
			gPad->BuildLegend();
			canva->cd(8);
			gStyle->SetOptTitle(0);
			myEtaMuCHit1->SetStats(kFALSE);
			myEtaMuCHit1->SetLineColor(kRed);
			myEtaMuCHit1->DrawCopy("cp");
			myEtaMuCHit2->SetStats(kFALSE);
			myEtaMuCHit2->SetLineColor(kBlue);
			myEtaMuCHit2->DrawCopy("cp same");
			gPad->BuildLegend();

			canva->Update();
		};
		
		std::cout << "reading file 1..." << std::endl;
		tp1.Process(readerFunction(1, 
			binning1 ? limits_file1[percentile_idx]     : 0, 
			binning1 ? limits_file1[percentile_idx + 1] : std::numeric_limits<int>::max(), 
			hasNtracks1));

		std::cout << "processing file 1..." << std::endl;
		processFunction(1);

		std::cout << "reading file 2..." << std::endl;
		tp2.Process(readerFunction(2, 
			binning2 ? limits_file2[percentile_idx]     : 0, 
			binning2 ? limits_file2[percentile_idx + 1] : std::numeric_limits<int>::max(), 
			hasNtracks2));

		std::cout << "processing file 2..." << std::endl;
		processFunction(2);


		TCanvas* trackerCanvas = new TCanvas(("Tracker muons-" + std::to_string(percentile_idx)).c_str(), "", 4000, 2000);
	    trackerCanvas->Divide(4, 2);
		trackerCanvas->SetFillStyle(1001);
		trackerCanvas->SetFillColor(kWhite);

		std::cout << "drawing tracker muon..." << std::endl;
		drawFunction(true, trackerCanvas);

		TCanvas* globalCanvas = new TCanvas(("Global muons-" + std::to_string(percentile_idx)).c_str(), "", 4000, 2000);
	    globalCanvas->Divide(4, 2);
		globalCanvas->SetFillStyle(1001);
		globalCanvas->SetFillColor(kWhite);

		std::cout << "drawing global muon..." << std::endl;
		drawFunction(false, globalCanvas);

		canvas.push_back(std::pair(trackerCanvas, globalCanvas));
	}

	return canvas;
}

void mu_comparison_efficiency() {
	char filename1[100], filename2[100];

	// request user input
  	std::cout << "filename1: ";
  	scanf("%s", filename1);
  	std::cout << "filename2: ";
  	scanf("%s", filename2);

	auto canvas = mu_comparison_efficiency_from_name(filename1, filename2);
	for (int percentile_idx = 0; percentile_idx < canvas.size(); ++percentile_idx) {
		int percentile_begin = std::round(100*percentiles[percentile_idx]);
		int percentile_end   = std::round(100*percentiles[percentile_idx + 1]);

		auto [trackerCanvas, globalCanvas] = canvas[percentile_idx];
	  	trackerCanvas->SaveAs(("../../figures/mu_comparison_eff_tracker_" + std::to_string(percentile_begin) + "-" + std::to_string(percentile_end) + ".png").c_str());
	  	globalCanvas ->SaveAs(("../../figures/mu_comparison_eff_global_"  + std::to_string(percentile_begin) + "-" + std::to_string(percentile_end) + ".png").c_str());
	}
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

	auto canvas = mu_comparison_efficiency_from_name(filename1, filename2);
	for (int percentile_idx = 0; percentile_idx < canvas.size(); ++percentile_idx) {
		int percentile_begin = std::round(100*percentiles[percentile_idx]);
		int percentile_end   = std::round(100*percentiles[percentile_idx + 1]);

		auto [trackerCanvas, globalCanvas] = canvas[percentile_idx];
	  	trackerCanvas->SaveAs(("../../figures/mu_comparison_eff_tracker_" + std::to_string(percentile_begin) + "-" + std::to_string(percentile_end) + ".png").c_str());
	  	globalCanvas ->SaveAs(("../../figures/mu_comparison_eff_global_"  + std::to_string(percentile_begin) + "-" + std::to_string(percentile_end) + ".png").c_str());
	}

	return 0;
}