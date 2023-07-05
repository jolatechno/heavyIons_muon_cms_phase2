#pragma once

const Int_t kNCentralityBins = 200;

const Double_t binTable[kNCentralityBins + 1] = {0, 12.0726, 12.9775, 13.7646, 14.6009, 15.4167, 16.2718, 17.1496, 18.0357, 18.933, 19.8565, 20.8485, 21.8614, 22.9305, 23.9966, 25.0743, 26.2168, 27.3867, 28.5948, 29.892, 31.1614, 32.5169, 33.96, 35.4747, 37.0106, 38.5893, 40.2316, 41.9823, 43.8428, 45.8105, 47.8386, 50.0019, 52.2309, 54.4736, 56.8414, 59.323, 61.9717, 64.7627, 67.6805, 70.6497, 73.7457, 76.9925, 80.1679, 83.6124, 87.1841, 90.8269, 94.685, 98.676, 102.906, 107.15, 111.736, 116.464, 121.338, 126.239, 131.41, 136.801, 142.322, 148.098, 154.006, 160.177, 166.526, 172.901, 179.823, 186.767, 194.153, 201.624, 209.203, 217.071, 225.281, 233.668, 242.615, 251.737, 261.417, 270.915, 280.69, 290.442, 301.228, 312.217, 322.842, 334.313, 346.203, 358.49, 370.721, 383.106, 396.469, 409.893, 423.605, 437.406, 451.505, 465.856, 481.029, 496.263, 512.126, 528.386, 544.806, 561.834, 579.189, 596.839, 614.239, 632.817, 651.443, 670.052, 689.384, 709.959, 729.154, 749.497, 770.253, 792.303, 813.733, 835.81, 858.891, 881.69, 906.947, 931.019, 955.307, 981.046, 1007.22, 1033.61, 1059.87, 1086.71, 1114.7, 1142.3, 1169.1, 1198.04, 1227.5, 1258.53, 1289.54, 1320.27, 1351.44, 1383.87, 1417.43, 1451.81, 1486.47, 1519.98, 1555.82, 1591.49, 1628.13, 1664.54, 1701.71, 1739.11, 1778.24, 1816.52, 1855.62, 1895.81, 1938.61, 1981.62, 2023.59, 2067.99, 2111.18, 2155.79, 2201.01, 2247.7, 2295.59, 2343.33, 2392.58, 2443.16, 2494.46, 2545.55, 2599.57, 2653.17, 2706.87, 2762.46, 2819.43, 2876.81, 2934.32, 2993.26, 3054.02, 3114.36, 3177.81, 3241.03, 3303.68, 3369.72, 3436.8, 3504.28, 3576.17, 3649.11, 3719.79, 3791.38, 3866.09, 3941.29, 4017.09, 4096.05, 4177.37, 4259.28, 4344.66, 4431.01, 4522.42, 4614.38, 4708.27, 4805.19, 4901.19, 5001.78, 5105.13, 5206.38, 5312.61, 5421.24, 5536.04, 5659.64, 5797.53, 5975.82, 7029.37};

Int_t getHiBinFromhiHF(const Double_t hiHF) {
	Int_t binPos = -1;
	for (int i = 0; i < kNCentralityBins; ++i) {
		if (hiHF >= binTable[i] && hiHF < binTable[i + 1]) {
			binPos = i;
			break;
		}
	}

	binPos = kNCentralityBins - 1 - binPos;

	return (Int_t)(kNCentralityBins * ((Double_t)binPos) / ((Double_t)kNCentralityBins));
}

// return the mean Ncoll for a given centrality bin (running from 0 to 200!!!)
Float_t findNcoll(Int_t hiBin) {
	const Float_t Ncoll[kNCentralityBins] = {1915.28, 1883.37, 1867.16, 1832.88, 1787.73, 1750.82, 1705.53, 1675.23, 1622.55, 1579.44, 1532.36, 1504.61, 1460.66, 1432.14, 1391.85, 1359.29, 1326.07, 1298.05, 1261.81, 1233.17, 1211.46, 1177.28, 1153.38, 1125.23, 1099.13, 1074.48, 1044.45, 1025.31, 994.60, 974.68, 950.58, 926.55, 908.18, 888.27, 865.08, 844.94, 821.74, 800.17, 782.19, 765.30, 745.00, 731.14, 713.44, 689.41, 672.54, 659.63, 643.05, 623.34, 607.19, 591.25, 581.11, 564.52, 549.44, 537.63, 519.57, 508.89, 494.72, 479.95, 468.30, 452.87, 443.78, 435.44, 418.83, 409.59, 398.56, 386.30, 374.37, 365.13, 354.42, 342.57, 337.09, 325.37, 316.98, 308.55, 297.66, 289.17, 278.47, 271.44, 264.29, 255.97, 248.99, 239.60, 234.54, 227.77, 219.03, 213.20, 204.66, 198.37, 192.89, 184.95, 181.27, 174.62, 169.96, 161.32, 157.70, 152.44, 147.70, 141.20, 136.18, 132.62, 128.49, 123.55, 119.19, 114.39, 110.32, 106.38, 102.50, 99.24, 96.16, 91.78, 89.12, 85.50, 81.57, 78.51, 76.14, 72.55, 69.93, 67.10, 64.48, 62.41, 59.51, 57.70, 55.07, 53.20, 50.41, 48.14, 46.47, 44.84, 42.66, 40.80, 39.37, 37.54, 35.95, 34.47, 32.76, 31.26, 30.08, 28.52, 27.58, 26.26, 25.32, 23.95, 22.95, 22.00, 20.77, 19.92, 18.93, 18.06, 17.31, 16.20, 15.86, 15.27, 14.34, 13.79, 13.17, 12.44, 11.79, 11.12, 10.61, 10.10, 10.07, 9.67, 8.98, 8.70, 8.22, 7.71, 7.22, 6.81, 6.56, 6.12, 6.54, 6.11, 5.88, 5.42, 5.13, 4.86, 4.72, 4.32, 4.05, 3.82, 4.34, 4.11, 3.87, 3.59, 3.34, 3.14, 2.84, 2.67, 2.51, 2.30, 2.42, 2.24, 2.18, 2.07, 2.03, 1.97, 1.91, 1.88, 1.85, 1.80};
	return Ncoll[hiBin];
};

template<typename T, typename T2>
auto readField(const char *field_name, ROOT::TThreadedObject<std::vector<T2>> &fieldStat, int max_n_event=-1, int nthreads=1) {
	return [&, field_name=field_name,  max_n_event=max_n_event, nthreads=nthreads](TTreeReader &myReader) {
		TTreeReaderValue<T> field(myReader, field_name);
		auto myFieldStat = fieldStat.Get();

		for (int j = 0; myReader.Next() && (j < max_n_event/nthreads || max_n_event <= 0); j++) {
			myFieldStat->push_back(*field);
		}
	};
}

template<typename T1, typename T2, typename T3>
auto processFunctionPercentile(const std::vector<T1> &percentiles, std::vector<T2> &limits, ROOT::TThreadedObject<std::vector<T3>> &fieldStat) {
	auto mergeFunction = [&](std::shared_ptr<std::vector<int>> target, std::vector<std::shared_ptr<std::vector<int>>> &objs) {
		int i = 0;
		for (auto &obj : objs) {
			if (obj != nullptr) {
				target->insert(target->begin(), obj->begin(), obj->end());
			}
		}
	};

	auto   nTracksMerged = fieldStat.Merge(mergeFunction);
	size_t n_events      = nTracksMerged->size();

	auto previous_cat_begin = nTracksMerged->begin();
	for (float percentile : percentiles) {
		if (percentile <= 0.f) {
			limits.push_back(0);
		} else if (percentile >= 1.f) {
			limits.push_back(std::numeric_limits<int>::max());
		} else {
			auto next_cat_begin = nTracksMerged->begin() + n_events*percentile;
			std::nth_element(previous_cat_begin, next_cat_begin, nTracksMerged->end());
			previous_cat_begin = next_cat_begin;

			limits.push_back(*previous_cat_begin);
		}
	}
}