#pragma once

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>

// One recorded input (a press or a release).
struct FWInput {
	int frame = 0;        // physics step the input was applied on
	bool down = true;     // press (true) or release (false)
	bool p2 = false;      // player 2?
	int button = 1;       // 1 = jump, 2/3 = platformer left/right
	float x = 0.f;        // player position when the input happened
	float y = 0.f;

	// Frame-window results. window < 0 means "not analysed yet".
	// window == 0 means the macro itself fails at this input.
	int window = -1;      // total frames the input can land on (left + right + 1)
	int left = 0;         // frames it can be moved earlier
	int right = 0;        // frames it can be moved later
};

struct FWMacro {
	std::vector<FWInput> inputs;

	void clear() { inputs.clear(); }
	int lastFrame() const { return inputs.empty() ? 0 : inputs.back().frame; }

	bool save(std::filesystem::path const& path) const {
		std::error_code ec;
		std::filesystem::create_directories(path.parent_path(), ec);
		std::ofstream f(path, std::ios::trunc);
		if (!f) return false;
		f << std::setprecision(9);
		f << "FWM1 " << inputs.size() << '\n';
		for (auto const& i : inputs) {
			f << i.frame << ' ' << (i.down ? 1 : 0) << ' ' << (i.p2 ? 1 : 0) << ' '
			  << i.button << ' ' << i.x << ' ' << i.y << ' '
			  << i.window << ' ' << i.left << ' ' << i.right << '\n';
		}
		return static_cast<bool>(f);
	}

	bool load(std::filesystem::path const& path) {
		inputs.clear();
		std::ifstream f(path);
		if (!f) return false;

		std::string magic;
		size_t n = 0;
		if (!(f >> magic >> n) || magic != "FWM1") return false;

		inputs.reserve(n);
		for (size_t k = 0; k < n; k++) {
			FWInput i;
			int d = 1, p = 0;
			if (!(f >> i.frame >> d >> p >> i.button >> i.x >> i.y >> i.window >> i.left >> i.right)) break;
			i.down = d != 0;
			i.p2 = p != 0;
			inputs.push_back(i);
		}
		std::stable_sort(inputs.begin(), inputs.end(), [](FWInput const& a, FWInput const& b) {
			return a.frame < b.frame;
		});
		return !inputs.empty();
	}
};
