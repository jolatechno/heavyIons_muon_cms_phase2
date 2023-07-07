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

#include <TMVA/Reader.h>
#include <TMVA/Factory.h>
#include <TMVA/DataLoader.h>
#include <TMVA/DataSetInfo.h>
#include <TMVA/Config.h>
#include <TMVA/MethodDL.h>
#include <TMVA/Tools.h>

#include <ROOT/TTreeProcessorMT.hxx>
#include <ROOT/TThreadedObject.hxx>

#include "../utilities/embeding_weight.hpp"
#include "utils/TMVA_cuts.hpp"
bool embeding = false;
bool hasGEM = true;
bool plotOnlyGEM = false;
float wp = 0.6;

TCanvas* QQ_mass_resolution_from_name(const char* filename) {
	float massMin = 2.6, massMax = 3.4;
	uint massBins = 100;
	int max_n_event = 50000;

	float xSize = 0.3, ySize = 0.15;
	float xBegin = 0.12, yBegin = 0.87;

	ROOT::TThreadedObject<TH1F> massHist("mass", "J/#Psi reconstructed invariant mass;J/#Psi mass;\\#Ocurences", massBins, massMin, massMax);

	float QQrapidityCutLow = 1.6, QQrapidityCutHigh = 2.4;
	float minVtxProb = 0.01;

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
		TTreeReaderArray<short>        reco_mu_charge(myReader, "Reco_mu_charge");

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

		for (int j = 0; myReader.Next() && (j < max_n_event/nthreads || max_n_event <= 0); j++) {
			float weigth = embeding ? findNcoll(getHiBinFromhiHF(*hiHF)) : 1.f;

			for (int QQreco_idx = 0; QQreco_idx < *reco_QQ_size; ++QQreco_idx) {
				short i         = gen_QQ_idx[QQreco_idx];
		    	bool  isPositiv = true; // i >= 0 && i < *gen_QQ_size;

		    	TLorentzVector* mom4    = (TLorentzVector*)reco_QQ_4mom->At(QQreco_idx);
		    	float           recPt   = mom4->Pt();
		    	float           recEta  = abs(mom4->Rapidity());
		    	float           recMass = mom4->M();
		    	
		    	if (isPositiv) {
		    		int reco_idx = reco_mu_idx[QQreco_idx], reco_jdx = reco_mu_jdx[QQreco_idx];

			    	if (reco_mu_charge[reco_idx]*reco_mu_charge[reco_jdx] >= 0) {
			    		continue;
			    	}

			    	      mom4         = (TLorentzVector*)reco_mu_4mom->At(reco_idx);
		    		float reco_mu_pti  = mom4->Pt();
		    		float reco_mu_etai = abs(mom4->Rapidity());

		    		      mom4         = (TLorentzVector*)reco_mu_4mom->At(reco_jdx);
		    		float reco_mu_ptj  = mom4->Pt();
		    		float reco_mu_etaj = abs(mom4->Rapidity());

		    		// conditions on muons
		    		if (pass_TMVA_domain_cut(
						hasGEM ? reco_mu_isGEM[reco_idx] : false,
						reco_isTracker    [reco_idx],
						reco_isGlobal     [reco_idx],
						nHitsMu           [reco_idx],
						nHitsTracker      [reco_idx],
						nHitsPix          [reco_idx],
						reco_mu_localChi2 [reco_idx],
						reco_mu_normChi2  [reco_idx],
						reco_mu_pti,
						reco_mu_etai,
						reco_mu_dxy       [reco_idx],
						reco_mu_dz        [reco_idx],
					   *nPV,
						reco_mu_highPurity[reco_idx],
						reco_mu_nMatches  [reco_idx],
						reco_muGEMquality [reco_idx]
					)) {
				    	if (!pass_TMVA_pre_cut(
							hasGEM ? reco_mu_isGEM[reco_idx] : false,
							reco_isTracker    [reco_idx],
							reco_isGlobal     [reco_idx],
							nHitsMu           [reco_idx],
							nHitsTracker      [reco_idx],
							nHitsPix          [reco_idx],
							reco_mu_localChi2 [reco_idx],
							reco_mu_normChi2  [reco_idx],
							reco_mu_pti,
							reco_mu_etai,
							reco_mu_dxy       [reco_idx],
							reco_mu_dz        [reco_idx],
						   *nPV,
							reco_mu_highPurity[reco_idx],
							reco_mu_nMatches  [reco_idx],
							reco_muGEMquality [reco_idx]
						)) {
				    		continue;
						}

						nHitsMu_           = nHitsMu          [reco_idx];
						nHitsTracker_      = nHitsTracker     [reco_idx];
						nHitsPix_          = nHitsPix         [reco_idx];
						reco_mu_localChi2_ = reco_mu_localChi2[reco_idx];
						reco_mu_pt_        = reco_mu_pti;
						reco_mu_eta_       = reco_mu_etai;
						reco_mu_dxy_       = reco_mu_dxy      [reco_idx];
						reco_mu_dz_        = reco_mu_dz       [reco_idx];
						nPV_               = *nPV;
						reco_mu_nMatches_  = reco_mu_nMatches [reco_idx];

						float proba = reader->EvaluateMVA("BDTG");
						if (proba < wp) {
							continue;
						}
					} else if (plotOnlyGEM) {
						continue;
					}

		    		if (pass_TMVA_domain_cut(
						hasGEM ? reco_mu_isGEM[reco_idx] : false,
						reco_isTracker    [reco_idx],
						reco_isGlobal     [reco_idx],
						nHitsMu           [reco_idx],
						nHitsTracker      [reco_idx],
						nHitsPix          [reco_idx],
						reco_mu_localChi2 [reco_idx],
						reco_mu_normChi2  [reco_idx],
						reco_mu_pti,
						reco_mu_etai,
						reco_mu_dxy       [reco_idx],
						reco_mu_dz        [reco_idx],
					   *nPV,
						reco_mu_highPurity[reco_idx],
						reco_mu_nMatches  [reco_idx],
						reco_muGEMquality [reco_idx]
					)) {
						if (!pass_TMVA_pre_cut(
							hasGEM ? reco_mu_isGEM[reco_jdx] : false,
							reco_isTracker    [reco_jdx],
							reco_isGlobal     [reco_jdx],
							nHitsMu           [reco_jdx],
							nHitsTracker      [reco_jdx],
							nHitsPix          [reco_jdx],
							reco_mu_localChi2 [reco_jdx],
							reco_mu_normChi2  [reco_jdx],
							reco_mu_pti,
							reco_mu_etai,
							reco_mu_dxy       [reco_jdx],
							reco_mu_dz        [reco_jdx],
						   *nPV,
							reco_mu_highPurity[reco_jdx],
							reco_mu_nMatches  [reco_jdx],
							reco_muGEMquality [reco_jdx]
						)) {
				    		continue;
						}

						nHitsMu_           = nHitsMu          [reco_jdx];
						nHitsTracker_      = nHitsTracker     [reco_jdx];
						nHitsPix_          = nHitsPix         [reco_jdx];
						reco_mu_localChi2_ = reco_mu_localChi2[reco_jdx];
						reco_mu_pt_        = reco_mu_ptj;
						reco_mu_eta_       = reco_mu_etaj;
						reco_mu_dxy_       = reco_mu_dxy      [reco_jdx];
						reco_mu_dz_        = reco_mu_dz       [reco_jdx];
						nPV_               = *nPV;
						reco_mu_nMatches_  = reco_mu_nMatches [reco_jdx];

						float proba = reader->EvaluateMVA("BDTG");
						if (proba < wp) {
							continue;
						}
					} else if (plotOnlyGEM) {
						continue;
					}

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

	RooPlot *frame = x.frame(Title("Imported TH1 with Poisson error bars"));
	dh.plotOn(frame);
	gauss.fitTo(dh);
	gauss.plotOn(frame);


	TCanvas* c1 = new TCanvas("er QQ", "", 1000, 1000);

	// massHistMerged->SetStats(kFALSE);
	// massHistMerged->DrawCopy("COLZ");
	frame->Draw();

	// legend
	std::string mLegend = "m: "         + std::to_string(mean.getValV())   + " #pm "      + std::to_string(mean.getError());
	std::string sLegend = "#sigma: "    + std::to_string(sigma.getValV() ) + " #pm "      + std::to_string(sigma.getError());
 // std::string sLegend = "#sigmaL: "   + std::to_string(sigma.getValV() ) + " #sigmaR: " + std::to_string(sigmaR.getValV());
	std::string cLegend = "#chi^{2}: "  + std::to_string(frame->chiSquare());
	std::string nLegend = "nL: "        + std::to_string(nL.getValV())     + " nR: "      + std::to_string(nR.getValV());
	std::string aLegend = "alphaL: "    + std::to_string(alphaL.getValV()) + " alphaR: "  + std::to_string(alphaR.getValV());
	//plot
	TLegend *leg = new TLegend(xBegin, yBegin - ySize, xBegin + xSize, yBegin);
	leg->SetHeader("fit", "C");
	leg->AddEntry ((TObject*)0, mLegend.c_str(), "");
	leg->AddEntry ((TObject*)0, sLegend.c_str(), "");
	leg->AddEntry ((TObject*)0, cLegend.c_str(), "");
	leg->AddEntry ((TObject*)0, nLegend.c_str(), "");
	leg->AddEntry ((TObject*)0, aLegend.c_str(), "");

	leg->Draw("same");

	return c1;
}

void QQ_mass_resolution() {
	char filename[100];

	// request user input
  	std::cout << "filename: ";
  	scanf("%s", filename);

  	auto *c1 = QQ_mass_resolution_from_name(filename);
  	c1->SaveAs("../../figures/QQ_massRes_TMVA.png");
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
  	c1->SaveAs("../../figures/QQ_massRes_TMVA.png");

	return 0;
}