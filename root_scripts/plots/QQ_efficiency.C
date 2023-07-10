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
bool embeding = false;

TCanvas* QQ_efficiency_from_name(const char* filename) {
	uint etaBins = 60, ptBins = 60;
	float etaMin = 0, etaMax = 4;
	float ptMin  = 0, ptMax  = 6;
	int max_n_event = -1;

	float QQrapidityCutLow = 1.6, QQrapidityCutHigh = 2.4;
	float minVtxProb = 0.01;

	int nthreads = std::thread::hardware_concurrency();
	std::cout << "using " << nthreads << " threads" << std::endl;
	ROOT::EnableImplicitMT(nthreads);


	ROOT::TThreadedObject<TH2F> ptEtaGen("", "", etaBins, etaMin, etaMax, ptBins, ptMin, ptMax);
	ROOT::TThreadedObject<TH2F> ptEta   ("", "", etaBins, etaMin, etaMax, ptBins, ptMin, ptMax);

	ROOT::TThreadedObject<TH2F> ptEtaEff     ("eff", "J/#Psi reco. efficiency;J/#Psi |#eta|;J/#Psi p_{t}[GeV]", etaBins, etaMin, etaMax, ptBins, ptMin, ptMax);
	ROOT::TThreadedObject<TH2F> ptEtaFakeRate("fakerate",   "J/#Psi fake rate;J/#Psi |#eta|;J/#Psi p_{t}[GeV]", etaBins, etaMin, etaMax, ptBins, ptMin, ptMax);

    ROOT::TThreadedObject<TH2F> ptErr ("rapidy relative error",  "rapidy relative error;J/#Psi |#eta|;J/#Psi p_{t}[GeV]",  etaBins, etaMin, etaMax, ptBins, ptMin, ptMax); // "pt err",  "pt err;
    ROOT::TThreadedObject<TH2F> etaErr("eta relative error",     "eta relative error;J/#Psi |#eta|;J/#Psi p_{t}[GeV]",     etaBins, etaMin, etaMax, ptBins, ptMin, ptMax); // "eta err", "eta err;


	float sizeGen = 0, sizeReco = 0, truePositiv = 0;
	auto readerFunction = [&](TTreeReader &myReader) {
		// Create a TTreeReader for the tree, for instance by passing the
		// TTree's name and the TDirectory / TFile it is in.
		TTreeReaderValue<float> hiHF(myReader, embeding ? "SumET_HF" : "Gen_weight");

		TTreeReaderValue<int>          gen_QQ_size (myReader, "Gen_QQ_size");
		TTreeReaderValue<int>          reco_QQ_size(myReader, "Reco_QQ_size");
		TTreeReaderArray<short>        reco_QQ_idx (myReader, "Gen_QQ_whichRec");
		TTreeReaderArray<short>        gen_QQ_idx  (myReader, "Reco_QQ_whichGen");
		TTreeReaderValue<TClonesArray> gen_QQ_4mom (myReader, "Gen_QQ_4mom");
		TTreeReaderValue<TClonesArray> reco_QQ_4mom(myReader, "Reco_QQ_4mom");
		TTreeReaderArray<short>        reco_QQ_sign(myReader, "Reco_QQ_sign");
		TTreeReaderArray<float>        reco_QQ_VtxProb(myReader, "Reco_QQ_VtxProb");

		TTreeReaderArray<short> reco_mu_idx(myReader, "Reco_QQ_mupl_idx");
		TTreeReaderArray<short> reco_mu_jdx(myReader, "Reco_QQ_mumi_idx");

		TTreeReaderArray<short> gen_mu_idx(myReader, "Gen_QQ_mupl_idx");
		TTreeReaderArray<short> gen_mu_jdx(myReader, "Gen_QQ_mumi_idx");

		TTreeReaderValue<TClonesArray> reco_mu_4mom  (myReader, "Reco_mu_4mom");
		TTreeReaderArray<short>        mu_whichgen   (myReader, "Reco_mu_whichGen");
		TTreeReaderArray<bool>         is_soft_cut   (myReader, "Reco_mu_isSoftCutBased"); //"Reco_mu_isSoftCutBased"); //"Reco_mu_isGlobal);" //"Reco_mu_isTracker");
		TTreeReaderArray<bool>         reco_isTracker(myReader, "Reco_mu_isTracker"); //"Reco_mu_isGlobal");
		TTreeReaderArray<short>        reco_mu_charge(myReader, "Reco_mu_charge");

		auto myPtEtaGen      = ptEtaGen.Get();
		auto myPtEta         = ptEta.Get();
		auto myPtEtaEff      = ptEtaEff.Get();
		auto myPtEtaFakeRate = ptEtaFakeRate.Get();
		auto myPtErr         = ptErr.Get();
		auto myEtaErr        = etaErr.Get();

		for (int j = 0; myReader.Next() && (j < max_n_event/nthreads || max_n_event <= 0); j++) {
			float weigth = embeding ? findNcoll(getHiBinFromhiHF(*hiHF)) : 1.f;

 			sizeGen  += weigth* *gen_QQ_size;
			sizeReco += weigth* *reco_QQ_size;

			for (short i = 0; i < *gen_QQ_size; ++i) {
		    	short  QQreco_idx  = reco_QQ_idx[i];
		    	bool   isPositiv = QQreco_idx >= 0 && QQreco_idx < *reco_QQ_size;
		    	truePositiv     += weigth*isPositiv;

		    	TLorentzVector* mom4 = (TLorentzVector*)gen_QQ_4mom->At(i);
		    	float           pt   = mom4->Pt();
		    	float           eta  = abs(mom4->Rapidity());

		    	myPtEtaGen->Fill(eta, pt, weigth);
		    }

			for (int QQreco_idx = 0; QQreco_idx < *reco_QQ_size; ++QQreco_idx) {
				short i         = gen_QQ_idx[QQreco_idx];
		    	bool  isPositiv = i >= 0 && i < *gen_QQ_size;

		    	TLorentzVector* mom4   = (TLorentzVector*)reco_QQ_4mom->At(QQreco_idx);
		    	float           recPt  = mom4->Pt();
		    	float           recEta = mom4->Rapidity();

		    	int reco_idx = reco_mu_idx[QQreco_idx], reco_jdx = reco_mu_jdx[QQreco_idx];
			    int gen_idx = gen_mu_idx[i], gen_jdx = gen_mu_jdx[i];

			    if (reco_mu_charge[reco_idx]*reco_mu_charge[reco_jdx] >= 0) {
			    	continue;
			    }

	    		// conditions on muons
		    	/*if ((reco_idx != gen_idx || reco_jdx != gen_jdx) &&
		    		(reco_jdx != gen_idx || reco_idx != gen_jdx))
		    		continue;
	    		if (!is_soft_cut[reco_idx] || !is_soft_cut[reco_jdx])
		    		continue;
		    	if (!reco_isTracker[reco_idx] || !reco_isTracker[reco_jdx])
		    		continue;
		    	if ((mu_whichgen[reco_idx] != gen_idx || mu_whichgen[reco_jdx] != gen_jdx) &&
		    		(mu_whichgen[reco_jdx] != gen_idx || mu_whichgen[reco_idx] != gen_jdx))
		    		continue;

		    	// conditions on J/Psi
		    	if (recEta < QQrapidityCutLow || recEta > QQrapidityCutHigh)
		    		continue;
		    	if (reco_QQ_sign[QQreco_idx] != 0)
		    		continue;
		    	if (reco_QQ_VtxProb[QQreco_idx] < minVtxProb)
		    		continue;*/
		    	

		    	myPtEta->Fill(recEta, recPt, weigth);

		    	if (isPositiv) {
			    	      mom4 = (TLorentzVector*)gen_QQ_4mom->At(i);
		    		float pt   = mom4->Pt();
		    		float eta  = abs(mom4->Rapidity());


	    			myPtEtaEff->Fill(recEta, recPt, weigth);

		    		myPtErr ->Fill(eta, pt, abs(pt  - recPt) /pt);
		    		myEtaErr->Fill(eta, pt, abs(eta - recEta)/eta);
		    	} else {
		    		myPtEtaFakeRate->Fill(recEta, recPt, weigth);
		    	}
		    }
		}
	};


  	// read file
	std::cout << "openning \"" << filename << "\"..." << std::endl;
	ROOT::TTreeProcessorMT tp(filename, "hionia/myTree");
	tp.Process(readerFunction);



	auto ptEtaGenMerged      = ptEtaGen.Merge();

	auto ptEtaMerged         = ptEta.Merge();
	auto ptEtaEffMerged      = ptEtaEff.Merge();
	auto ptEtaFakeRateMerged = ptEtaFakeRate.Merge();
	auto ptErrMerged         = ptErr.Merge();
	auto etaErrMerged        = etaErr.Merge();



	std::cout << std::endl << std::endl << "total number of generated particule is "               << sizeGen     << std::endl;
	std::cout <<                           "total number of reconstructed particule is "           << sizeReco    << std::endl;
	std::cout <<                           "total number of correctly reconstructed particule is " << truePositiv << std::endl;

	float reco_eff   =    truePositiv/sizeGen;
	float fake_rate = 1 - truePositiv/sizeReco;

	std::cout << std::endl << "Reconstruction effeciency is " << reco_eff << std::endl;
	std::cout <<              "Fake positive rate is "       << fake_rate << std::endl;





	ptErrMerged->Divide(ptEtaEffMerged.get());
	etaErrMerged->Divide(ptEtaEffMerged.get());
	ptEtaEffMerged->Divide(ptEtaGenMerged.get());
	ptEtaFakeRateMerged->Divide(ptEtaMerged.get());




	TCanvas* c1 = new TCanvas("er QQ","",1000,1000);
    c1->Divide(2, 2);
	c1->SetFillStyle(1001);
	c1->SetFillColor(kWhite);


	c1->cd(1);
	ptEtaEffMerged->SetStats(kFALSE);
	ptEtaEffMerged->DrawCopy("COLZ");
	c1->cd(2);
	ptEtaFakeRateMerged->SetStats(kFALSE);
	ptEtaFakeRateMerged->DrawCopy("COLZ");

	c1->cd(3);
	gPad->SetLogz();
	ptErrMerged->SetStats(kFALSE);
	ptErrMerged->DrawCopy("COLZ");
	c1->cd(4);
	gPad->SetLogz();
	etaErrMerged->SetStats(kFALSE);
	etaErrMerged->DrawCopy("COLZ");

	c1->Update();




	return c1;
}

void QQ_efficiency() {
	char filename[100];

	// request user input
  	std::cout << "filename: ";
  	scanf("%s", filename);

  	auto *c1 = QQ_efficiency_from_name(filename);
  	c1->SaveAs("../../figures/QQ_eff.png");
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

	auto *c1 = QQ_efficiency_from_name(filename);
  	c1->SaveAs("../../figures/QQ_eff.png");

	return 0;
}