#pragma once

void get_error_self(std::shared_ptr<TH2F> hist) {
	Int_t n = hist->GetNcells();
	for(Int_t i = 0; i < n; i++) 
		if (hist->GetBinContent(i) != 0) {
			hist->SetBinContent(i, hist->GetBinError(i));
		}
}

void zero_normalize(std::shared_ptr<TH2F> hist, const std::shared_ptr<TH2F> hist_ref, float zero_val=1e-7) {
	Int_t n = hist->GetNcells();
	for(Int_t i = 0; i < n; i++) 
		if (hist->GetBinContent(i) == 0 && hist_ref->GetBinContent(i) != 0) {
			hist->SetBinContent(i, zero_val);
		}
}

void pin_hist(std::shared_ptr<TH2F> hist, float min_, float max_) {
	Int_t n = hist->GetNcells();
	for(Int_t i = 0; i < n; i++) 
		hist->SetBinContent(i, std::min(max_, std::max(min_, (float)hist->GetBinContent(i))));
}