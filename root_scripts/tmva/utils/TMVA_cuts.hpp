#pragma once

bool pass_TMVA_domain_cut(
	bool  reco_isGEM,
	bool  reco_isTracker,
	bool  reco_isGlobal,
	int   nHitsMu,
	int   nHitsTracker,
	int   nHitsPix,
	float reco_mu_localChi2,
	float reco_mu_normChi2,
	float reco_mu_pt,
	float reco_mu_eta,
	float reco_mu_dxy,
	float reco_mu_dz,
	float nPV,
	bool  reco_mu_highPurity,
	int   reco_mu_nMatches,
	bool  reco_muGEMquality
) {
	return reco_isGEM || reco_isTracker;
}

bool pass_TMVA_pre_cut(
	bool  reco_isGEM,
	bool  reco_isTracker,
	bool  reco_isGlobal,
	int   nHitsMu,
	int   nHitsTracker,
	int   nHitsPix,
	float reco_mu_localChi2,
	float reco_mu_normChi2,
	float reco_mu_pt,
	float reco_mu_eta,
	float reco_mu_dxy,
	float reco_mu_dz,
	float nPV,
	bool  reco_mu_highPurity,
	int   reco_mu_nMatches,
	bool  reco_muGEMquality
) {
	bool pass = pass_TMVA_domain_cut(
		reco_isGEM,
		reco_isTracker,
		reco_isGlobal,
		nHitsMu,
		nHitsTracker,
		nHitsPix,
		reco_mu_localChi2,
		reco_mu_normChi2,
		reco_mu_pt,
		reco_mu_eta,
		reco_mu_dxy,
		reco_mu_dz,
		nPV,
		reco_mu_highPurity,
		reco_mu_nMatches,
		reco_muGEMquality
	);

	pass = pass && reco_mu_nMatches > 0;
	if (reco_isGEM) {
		pass = pass && (reco_mu_eta > 1.6 && reco_mu_eta < 2.8);
	}
	if (nHitsTracker) {
		pass = pass && (reco_mu_eta > 1.2 && reco_mu_eta < 2.4);
	}
	return pass;
}