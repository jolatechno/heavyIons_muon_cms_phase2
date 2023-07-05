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
#include <TLegend.h>

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

TCanvas* QQ_mass_resolution_comparison_from_name(const char* filename1, const char* filename2) {
	float J_psi_m = 3.096916, J_psi_dm = 5e-1;

	float mass_width = 0.3;
	uint massBins = 50;
	int max_n_event = -1;
	
	float massMin = J_psi_m - mass_width/2, massMax = J_psi_m + mass_width/2;

	float QQrapidityCutLow = 0/*1.6*/, QQrapidityCutHigh = 1/*2.4*/;
	float minVtxProb = 0.01;

	float muPtCutLow = 0, muPtCutHigh = 9;
	float muEtaCutLow = 0, muEtaCutHigh = 9;

	float xSize = 0.3, ySize = 0.1;
	float xBegin = 0.12;
	float yBegin = 0.12, ySpace = 0.03;

	ROOT::TThreadedObject<TH1F> massHist1("mass", "J/#Psi reconstructed invariant mass;J/#Psi mass;\\#Ocurences", massBins, massMin, massMax);
	ROOT::TThreadedObject<TH1F> massHist2("mass", "J/#Psi reconstructed invariant mass;J/#Psi mass;\\#Ocurences", massBins, massMin, massMax);

	int nthreads = std::thread::hardware_concurrency();
	std::cout << "using " << nthreads << " threads" << std::endl;
	ROOT::EnableImplicitMT(nthreads);


	auto readerFunction = [&](ROOT::TThreadedObject<TH1F> &massHist) {
		return [&](TTreeReader &myReader) {
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
				    	      mom4 = (TLorentzVector*)reco_mu_4mom->At(reco_idx);
				    	float pt_i = mom4->Pt(), eta_i = abs(mom4->Rapidity());
				    	if (pt_i  < muPtCutLow  || pt_i  > muPtCutHigh ||
				    		eta_i < muEtaCutLow || eta_i > muEtaCutHigh)
				    		continue;
				    	      mom4 = (TLorentzVector*)reco_mu_4mom->At(reco_jdx);
				    	float pt_j = mom4->Pt(), eta_j = abs(mom4->Rapidity());
				    	if (pt_j  < muPtCutLow  || pt_j  > muPtCutHigh ||
				    		eta_j < muEtaCutLow || eta_j > muEtaCutHigh)
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
	};


	TCanvas* c1 = new TCanvas("er QQ", "", 1000, 1000);

	RooRealVar x("x", "x", (massMin + massMax)/2.f, massMin, massMax);
	RooPlot *frame = x.frame();
	frame->SetTitle ("J/#Psi reconstructed invariant mass resolution");
	frame->SetXTitle("J/#Psi mass");
	frame->SetYTitle("\\#Ocurences");


	auto plotFunction = [&](ROOT::TThreadedObject<TH1F> &massHist, const char* legend, float ybegin, Color_t color) {
		auto massHistMerged = massHist.Merge();
		massHistMerged->Scale(1.f/massHistMerged->Integral());

		// fit
		// Create a binned dataset that imports contents of TH1 and associates its contents to observable 'x'
		RooDataHist dh(legend, "dh", x, Import(*massHistMerged.get()));
		// declare variables
		RooRealVar mean("mean", "mean of gaussian", J_psi_m, J_psi_m - J_psi_dm, J_psi_m + J_psi_dm);
		RooRealVar sigma("sigma", "width of gaussian", 2e-2, 1e-5, mass_width/2);
		// declare weird variables
		RooRealVar alphaL("alphaL", "", 2.f, 1e-1, 1e1);
		RooRealVar alphaR("alphaR", "", 2.f, 1e-1, 1e1);
		RooRealVar nL("nL", "", 2.f, 1e-1, 5e1);
		RooRealVar nR("nR", "", 2.f, 1e-1, 5e1);
		// Build gaussian pdf in terms of x,mean and sigma 	
		RooCrystalBall gauss("gauss", "gaussian PDF", x, mean, sigma, alphaL, alphaR, nL, nR);
	 // RooRealVar     sigmaR("sigma", "width of gaussian", 2e-2, 1e-5, mass_width/2);
	 // RooCrystalBall gauss("gauss", "gaussian PDF", x, mean, sigma, sigmaR, alphaL, alphaR, nL, nR);
	 // RooCrystalBall gauss("gauss", "gaussian PDF", x, mean, sigma, alphaL, nL);
	 // RooCBShape     gauss("gauss", "gaussian PDF", x, mean, sigma, alphaL, nL);

		// plot
		dh.plotOn(frame, MarkerColor(color));
		gauss.fitTo(dh);
		gauss.plotOn(frame, LineColor(color));

		// legend
		std::string mLegend = "m: "         + std::to_string(mean.getValV())   + " #pm "      + std::to_string(mean.getError());
		std::string sLegend = "#sigma: "    + std::to_string(sigma.getValV() ) + " #pm "      + std::to_string(sigma.getError());
	 // std::string sLegend = "#sigmaL: "   + std::to_string(sigma.getValV() ) + " #sigmaR: " + std::to_string(sigmaR.getValV());
		std::string cLegend = "#chi^{2}: "  + std::to_string(frame->chiSquare());
		std::string nLegend = "nL: "        + std::to_string(nL.getValV())     + " nR: "      + std::to_string(nR.getValV());
		std::string aLegend = "alphaL: "    + std::to_string(alphaL.getValV()) + " alphaR: "  + std::to_string(alphaR.getValV());
		//plot
		TLegend *leg = new TLegend(xBegin, ybegin - ySize, xBegin + xSize, ybegin);
		leg->SetHeader(legend, "C");
		leg->AddEntry ((TObject*)0, mLegend.c_str(), "");
		leg->AddEntry ((TObject*)0, sLegend.c_str(), "");
		leg->AddEntry ((TObject*)0, cLegend.c_str(), "");
		leg->AddEntry ((TObject*)0, nLegend.c_str(), "");
		leg->AddEntry ((TObject*)0, aLegend.c_str(), "");

		return leg;
	};

	// read file
	std::cout << "openning \"" << filename1 << "\" and \"" << filename2 << "\"..." << std::endl;
	ROOT::TTreeProcessorMT tp1(filename1, "hionia/myTree");
	ROOT::TTreeProcessorMT tp2(filename2, "hionia/myTree");
	
	std::cout << "reading file 1..." << std::endl;
	tp1.Process(readerFunction(massHist1));

	std::cout << "processing file 1..." << std::endl;
	auto *leg1 = plotFunction(massHist1, "phase 2", 1 - yBegin, kRed);

	std::cout << "reading file 2..." << std::endl;
	tp2.Process(readerFunction(massHist2));

	std::cout << "processing file 2..." << std::endl;
	auto *leg2 = plotFunction(massHist2, "run 3", 1 - (yBegin + ySize + ySpace), kBlue);
	
	frame->Draw();
	leg1->Draw("same");
	leg2->Draw("same");

	return c1;
}

void QQ_mass_resolution_comparison() {
	char filename1[100], filename2[100];

	// request user input
  	std::cout << "filename1: ";
  	scanf("%s", filename1);
  	std::cout << "filename2: ";
  	scanf("%s", filename2);

  	auto *c1 = QQ_mass_resolution_comparison_from_name(filename1, filename2);
  	c1->SaveAs("../../figures/QQ_massRes_comparison.png");
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

	auto *c1 = QQ_mass_resolution_comparison_from_name(filename1, filename2);
  	c1->SaveAs("../../figures/QQ_massRes_comparison.png");

	return 0;
}