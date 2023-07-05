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

#include <RooRealVar.h>
#include <RooDataSet.h>
#include <RooDataHist.h>
#include <RooCrystalBall.h>
#include <RooCBShape.h>
#include <RooPlot.h>
using namespace RooFit;

#include <ROOT/TTreeProcessorMT.hxx>
#include <ROOT/TThreadedObject.hxx>

#include "../utilities/embeding_weight.hpp"
bool embeding = false;

TCanvas* QQ_mass_resolution_from_name(const char* filename) {
	float massMin = 2.6, massMax = 3.4;
	uint massBins = 100;
	int max_n_event = -1;

	ROOT::TThreadedObject<TH1F> massHist("mass", "J/#Psi reconstructed invariant mass;J/#Psi mass;\\#Ocurences", massBins, massMin, massMax);

	float QQrapidityCutLow = 1.6, QQrapidityCutHigh = 2.4;
	float minVtxProb = 0.01;

	int nthreads = std::thread::hardware_concurrency();
	std::cout << "using " << nthreads << " threads" << std::endl;
	ROOT::EnableImplicitMT(nthreads);


	auto readerFunction = [&](TTreeReader &myReader) {
		// Create a TTreeReader for the tree, for instance by passing the
		// TTree's name and the TDirectory / TFile it is in.
		TTreeReaderValue<float> hiHF(myReader, embeding ? "SumET_HF" : "Gen_weight");

		TTreeReaderValue<int>          gen_QQ_size    (myReader, "Gen_QQ_size");
		TTreeReaderValue<int>          reco_QQ_size   (myReader, "Reco_QQ_size");
		TTreeReaderArray<short>        reco_QQ_idx    (myReader, "Gen_QQ_whichRec");
		TTreeReaderArray<short>        gen_QQ_idx     (myReader, "Reco_QQ_whichGen");
		TTreeReaderValue<TClonesArray> gen_QQ_4mom    (myReader, "Gen_QQ_4mom");
		TTreeReaderValue<TClonesArray> reco_QQ_4mom   (myReader, "Reco_QQ_4mom");
		TTreeReaderArray<short>        reco_QQ_sign   (myReader, "Reco_QQ_sign");
		TTreeReaderArray<float>        reco_QQ_VtxProb(myReader, "Reco_QQ_VtxProb");

		TTreeReaderArray<short> reco_mu_idx(myReader, "Reco_QQ_mupl_idx");
		TTreeReaderArray<short> reco_mu_jdx(myReader, "Reco_QQ_mumi_idx");

		TTreeReaderArray<short> gen_mu_idx(myReader, "Gen_QQ_mupl_idx");
		TTreeReaderArray<short> gen_mu_jdx(myReader, "Gen_QQ_mumi_idx");

		TTreeReaderValue<TClonesArray> reco_mu_4mom  (myReader, "Reco_mu_4mom");
		TTreeReaderArray<short>        mu_whichgen   (myReader, "Reco_mu_whichGen");
		TTreeReaderArray<bool>         is_soft_cut   (myReader, "Reco_mu_isSoftCutBased"); //"Reco_mu_isSoftCutBased"); //"Reco_mu_isGlobal);" //"Reco_mu_isTracker");
		TTreeReaderArray<bool>         reco_isTracker(myReader, "Reco_mu_isTracker"); //"Reco_mu_isGlobal");

		for (int j = 0; myReader.Next() && (j < max_n_event/nthreads || max_n_event <= 0); j++) {
			float weigth = embeding ? findNcoll(getHiBinFromhiHF(*hiHF)) : 1.f;

			for (int QQreco_idx = 0; QQreco_idx < *reco_QQ_size; ++QQreco_idx) {
				short i         = gen_QQ_idx[QQreco_idx];
		    	bool  isPositiv = i >= 0 && i < *gen_QQ_size;

		    	TLorentzVector* mom4    = (TLorentzVector*)reco_QQ_4mom->At(QQreco_idx);
		    	float           recPt   = mom4->Pt();
		    	float           recEta  = abs(mom4->Rapidity());
		    	float           recMass = mom4->M();
		    	
		    	if (isPositiv) {
		    		int reco_idx = reco_mu_idx[QQreco_idx], reco_jdx = reco_mu_jdx[QQreco_idx];
			    	int gen_idx = gen_mu_idx[i], gen_jdx = gen_mu_jdx[i];

			    	      mom4 = (TLorentzVector*)reco_QQ_4mom->At(QQreco_idx);
		    		float pt   = mom4->Pt();
		    		float eta  = abs(mom4->Rapidity());

		    		// conditions on muons
			    	if ((reco_idx != gen_idx || reco_jdx != gen_jdx) &&
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
			    		continue;

			    	massHist->Fill(recMass, weigth);
		    	}
		    }
		}
	};


  	// read file
	std::cout << "openning \"" << filename << "\"..." << std::endl;
	ROOT::TTreeProcessorMT tp(filename, "hionia/myTree");
	tp.Process(readerFunction);

	auto massHistMerged = massHist.Merge();
	massHistMerged->Scale(1.f/massHistMerged->Integral());

	// fit
	RooRealVar x("x", "x", (massMin + massMax)/2.f, massMin, massMax);
	// Create a binned dataset that imports contents of TH1 and associates its contents to observable 'x'
	RooDataHist dh("dh", "dh", x, Import(*massHistMerged.get()));
	// declare variables
	RooRealVar mean("mean", "mean of gaussian", (massMin + massMax)/2.f, massMin, massMax);
	RooRealVar sigma("sigma", "width of gaussian", 0.1, 0.001, 10);
	// declare weird variables
	RooRealVar alpha("alpha", "", 1.f, -1000.f, 1000.f);
	RooRealVar n("n", "", 1.f, -1000.f, 1000.f);
	// Build gaussian pdf in terms of x,mean and sigma
	RooCBShape gauss("gauss", "gaussian PDF", x, mean, sigma, alpha, n);

	RooPlot *frame = x.frame(Title("Imported TH1 with Poisson error bars"));
	dh.plotOn(frame);
	gauss.fitTo(dh);
	gauss.plotOn(frame);


	TCanvas* c1 = new TCanvas("er QQ", "", 1000, 1000);

	// massHistMerged->SetStats(kFALSE);
	// massHistMerged->DrawCopy("COLZ");
	frame->Draw();

	return c1;
}

void QQ_mass_resolution() {
	char filename[100];

	// request user input
  	std::cout << "filename: ";
  	scanf("%s", filename);

  	auto *c1 = QQ_mass_resolution_from_name(filename);
  	c1->SaveAs("../../figures/QQ_massRes.png");
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

	auto *c1 = QQ_mass_resolution_from_name(filename);
  	c1->SaveAs("../../figures/QQ_massRes.png");

	return 0;
}