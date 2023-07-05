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
#include <TProfile.h>
#include <TLegend.h>

#include <RooRealVar.h>
#include <RooDataSet.h>
#include <RooDataHist.h>
#include <RooCrystalBall.h>
#include <RooCBShape.h>
#include <RooPlot.h>
#include <RooExpPoly.h>
using namespace RooFit;

#include <ROOT/TTreeProcessorMT.hxx>
#include <ROOT/TThreadedObject.hxx>

#include "../utilities/embeding_weight.hpp"
bool embeding = true;

std::pair<TCanvas*, TCanvas*> centrality_from_name(const char* filename) {
	uint centralityBins = 100, YBins = 100;
	float YMin = 0, YMax = 5000;
	float centralityMax = 200;
	int max_n_event = -1;


	int nthreads = std::thread::hardware_concurrency();
	std::cout << "using " << nthreads << " threads" << std::endl;
	ROOT::EnableImplicitMT(nthreads);

	ROOT::TThreadedObject<TH2F> correlationPlot("eff", "centrality vs Ntracks;centrality;Ntracks", centralityBins, 0.f, 1.f, YBins, YMin, YMax);

	float sizeGen = 0, sizeReco = 0, truePositiv = 0;
	auto readerFunction = [&](TTreeReader &myReader) {
		TTreeReaderValue<int>   Centrality(myReader, "Centrality");
		TTreeReaderValue<short> Y         (myReader, "Ntracks");

		auto myCorrelationPlot = correlationPlot.Get();

		for (int j = 0; myReader.Next() && (j < max_n_event/nthreads || max_n_event <= 0); j++) {
			float centrality = *Centrality/centralityMax;
			float y = *Y;

			myCorrelationPlot->Fill(centrality, y);
		}
	};


  	// read file
	std::cout << "openning \"" << filename << "\"..." << std::endl;
	ROOT::TTreeProcessorMT tp(filename, "hionia/myTree");
	tp.Process(readerFunction);
	
	auto correlationPlotMerged = correlationPlot.Merge();

	float bin_width = 1.f/centralityBins;
	correlationPlotMerged->Scale(1.f/correlationPlotMerged->Integral("width")/bin_width);

	TCanvas* canva2d = new TCanvas("2D Centrality correaltion", "", 1000, 1000);
	correlationPlotMerged->SetStats(kFALSE);
	correlationPlotMerged->DrawCopy("COLZ");



	auto correlationPlot1D = correlationPlotMerged->ProfileX("");

	RooRealVar x("x", "x", 0.f, 1.f);
	RooDataHist y("y", "y", x, Import(*correlationPlot1D));

	RooRealVar a("a", "", 1.f, -1000.f, 1000.f);
	RooRealVar b("b", "", 1.f, -1000.f, 1000.f);
	RooRealVar c("a", "", 1.f, -1000.f, 1000.f);
	RooExpPoly pol("pol", "pol", x, RooArgList(a, b, c), 0);

	RooPlot *frame = x.frame(Title("Imported TH1 with Poisson error bars"));
	y.plotOn(frame);
	pol.fitTo(y);
	pol.plotOn(frame);

	TCanvas* canva1d = new TCanvas("1D Centrality correaltion", "", 1000, 1000);
 // correlationPlot1D->SetStats(kFALSE);
 // correlationPlot1D->DrawCopy("COLZ");
	frame->Draw();

	// legend
	std::string aLegend = "a: " + std::to_string(a.getValV())   + " #pm " + std::to_string(a.getError());
	std::string bLegend = "b: " + std::to_string(b.getValV())   + " #pm " + std::to_string(b.getError());
 // std::string cLegend = "c: " + std::to_string(c.getValV())   + " #pm " + std::to_string(c.getError());
	//plot
	TLegend *leg = new TLegend(0.6,0.7,0.85,0.85);
 // leg->SetHeader("exp(a + b.x + c.x^2)", "C");
	leg->SetHeader("exp(a + b.x)", "C");
	leg->AddEntry ((TObject*)0, aLegend.c_str(), "");
	leg->AddEntry ((TObject*)0, bLegend.c_str(), "");
 // leg->AddEntry ((TObject*)0, cLegend.c_str(), "");
	leg->Draw();


	return std::pair(canva2d, canva1d);
}

void centrality() {
	char filename[100];

	// request user input
  	std::cout << "filename: ";
  	scanf("%s", filename);

  	auto [canva2d, canva1d] = centrality_from_name(filename);
  	canva2d->SaveAs("../../figures/centrality_correlation.png");
  	canva1d->SaveAs("../../figures/centrality_correlation_fit.png");
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

  	auto [canva2d, canva1d] = centrality_from_name(filename);
  	canva2d->SaveAs("../../figures/centrality_correlation.png");
  	canva1d->SaveAs("../../figures/centrality_correlation_fit.png");

	return 0;
}