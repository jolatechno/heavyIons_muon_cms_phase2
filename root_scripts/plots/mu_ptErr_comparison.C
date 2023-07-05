#include <iostream>
#include <thread>
#include <utility>

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

TCanvas* mu_ptErr_comparison_from_name(const char* filename1, const char* filename2) {
	uint ptErrBins = 200;
	int max_n_event = -1;
	float ptErrMin = 1e-3, ptErrMax = 5e-2;

	float muPtCutLow = 0, muPtCutHigh = 9;
	float muEtaCutLow = 1.6, muEtaCutHigh = 2.4;

	ROOT::TThreadedObject<TH1F> ptErrHist1("ptRes1",   "Run 3 reco. mu pt resolution;mu ptErr/pt;\\#Ocurences", ptErrBins, ptErrMin, ptErrMax);
	ROOT::TThreadedObject<TH1F> ptErrHist2("ptRes2", "Phase 2 reco. mu pt resolution;mu ptErr/pt;\\#Ocurences", ptErrBins, ptErrMin, ptErrMax);

	int nthreads = std::thread::hardware_concurrency();
	std::cout << "using " << nthreads << " threads" << std::endl;
	ROOT::EnableImplicitMT(nthreads);

	auto readerFunction = [&](ROOT::TThreadedObject<TH1F> &ptErrHist) {
		return [&](TTreeReader &myReader) {
			// Create a TTreeReader for the tree, for instance by passing the
			// TTree's name and the TDirectory / TFile it is in.
			TTreeReaderValue<int>          reco_mu_size (myReader, "Reco_mu_size");
			TTreeReaderValue<TClonesArray> reco_mu_4mom (myReader, "Reco_mu_4mom");
			TTreeReaderArray<float>        reco_mu_ptErr(myReader, "Reco_mu_ptErr_inner");

			TTreeReaderValue<int>          gen_mu_size(myReader, "Gen_mu_size");
			TTreeReaderArray<short>        gen_mu_idx (myReader, "Reco_mu_whichGen");

			for (int j = 0; myReader.Next() && (j < max_n_event/nthreads || max_n_event <= 0); j++) {
				for (int i = 0; i < *reco_mu_size; ++i) {
			    	TLorentzVector* mom4    = (TLorentzVector*)reco_mu_4mom->At(i);
			    	float           recPt   = mom4->Pt();
			    	float           recEta  = abs(mom4->Eta());
			    	
			    	if (muEtaCutLow < recEta && recEta < muEtaCutHigh &&
			    		muPtCutLow  < recPt  && recPt  < muPtCutHigh)
			    	{
			    		if (gen_mu_idx[i] >= 0 && gen_mu_idx[i] < *gen_mu_size)
			    			ptErrHist->Fill(abs(reco_mu_ptErr[i]/recPt));
			    	}
			    }
			}
		};
	};


	TCanvas* c1 = new TCanvas("er QQ", "", 1000, 1000);
	c1->SetLogx();

	// read file
	std::cout << "openning \"" << filename1 << "\" and \"" << filename2 << "\"..." << std::endl;
	ROOT::TTreeProcessorMT tp1(filename1, "hionia/myTree");
	ROOT::TTreeProcessorMT tp2(filename2, "hionia/myTree");
	
	std::cout << "reading file 1..." << std::endl;
	tp1.Process(readerFunction(ptErrHist1));

	std::cout << "reading file 2..." << std::endl;
	tp2.Process(readerFunction(ptErrHist2));

	auto ptErrHistMerged1 = ptErrHist1.Merge();
	auto ptErrHistMerged2 = ptErrHist2.Merge();

	ptErrHistMerged1->Scale(1.f/ptErrHistMerged1->Integral());
	ptErrHistMerged2->Scale(1.f/ptErrHistMerged2->Integral());

	gStyle->SetOptTitle(1);
	ptErrHistMerged2->SetStats(kFALSE);
	ptErrHistMerged2->SetLineColor(kRed);
	ptErrHistMerged2->DrawCopy("hist");
	ptErrHistMerged1->SetStats(kFALSE);
	ptErrHistMerged1->SetLineColor(kBlue);
	ptErrHistMerged1->DrawCopy("hist same");
	gPad->BuildLegend();

	return c1;
}

void mu_ptErr_comparison() {
	char filename1[100], filename2[100];

	// request user input
  	std::cout << "filename1: ";
  	scanf("%s", filename1);
  	std::cout << "filename2: ";
  	scanf("%s", filename2);

  	auto *c1 = mu_ptErr_comparison_from_name(filename1, filename2);
  	c1->SaveAs("../../figures/reco_mu_ptErr.png");
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

	auto *c1 = mu_ptErr_comparison_from_name(filename1, filename2);
  	c1->SaveAs("../../figures/reco_mu_ptErr.png");

	return 0;
}