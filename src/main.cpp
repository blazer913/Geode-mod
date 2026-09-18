// Frame Window Macro
//  - records / replays inputs by physics step
//  - noclip + speedhack
//  - measures how many frames earlier/later every input can be placed
//    and draws that number in the level at the spot of the click
//
// Frame window definition used here:
//   For input i, shift ONLY that input by s frames (all others stay as recorded)
//   and replay with noclip forced on. The shift "passes" if no NEW hazard hit
//   happens between the shifted input and (next input of the same player +
//   lookahead frames). left/right = furthest passing shift in each direction,
//   window = left + right + 1.

#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/UILayer.hpp>
#include <Geode/modify/CCScheduler.hpp>
#include <Geode/ui/Notification.hpp>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <functional>

#include "macro.hpp"

using namespace geode::prelude;

// ---------------------------------------------------------------- helpers

namespace {
	bool optBool(char const* k) { return Mod::get()->getSettingValue<bool>(k); }
	double optFloat(char const* k) { return Mod::get()->getSettingValue<double>(k); }
	int optInt(char const* k) { return static_cast<int>(Mod::get()->getSettingValue<int64_t>(k)); }

	void toast(std::string const& msg, NotificationIcon icon) {
		Notification::create(msg, icon)->show();
	}

	ccColor3B windowColor(int w) {
		if (w < 0) return ccColor3B{170, 170, 170}; // not analysed
		if (w <= 1) return ccColor3B{255, 70, 70};  // 0 = macro fails, 1 = frame perfect
		if (w <= 3) return ccColor3B{255, 150, 50};
		if (w <= 6) return ccColor3B{255, 230, 70};
		return ccColor3B{90, 255, 110};
	}
}

// ------------------------------------------------------ in-level indicators

class FWIndicators : public CCNode {
	struct Item {
		float x;
		CCNode* node;
	};
	std::vector<Item> m_items;

public:
	static FWIndicators* create(FWMacro const& macro) {
		auto ret = new FWIndicators();
		if (ret && ret->setup(macro)) {
			ret->autorelease();
			return ret;
		}
		CC_SAFE_DELETE(ret);
		return nullptr;
	}

	bool setup(FWMacro const& macro) {
		if (!CCNode::init()) return false;

		float sc = static_cast<float>(optFloat("indicator-scale"));
		auto dots = CCDrawNode::create();
		this->addChild(dots);

		for (auto const& in : macro.inputs) {
			auto c3 = windowColor(in.window);
			ccColor4F c4 = {c3.r / 255.f, c3.g / 255.f, c3.b / 255.f, 1.f};
			CCPoint p = {in.x, in.y};

			// press = filled dot, release = ring
			dots->drawDot(p, 5.f * sc, c4);
			if (!in.down) dots->drawDot(p, 2.8f * sc, ccColor4F{0.05f, 0.05f, 0.05f, 1.f});

			auto holder = CCNode::create();
			holder->setPosition(p);

			std::string txt = in.window >= 0 ? std::to_string(in.window) : std::string("?");
			auto big = CCLabelBMFont::create(txt.c_str(), "bigFont.fnt");
			big->setScale(0.32f * sc);
			big->setColor(c3);
			big->setPosition({0.f, 13.f * sc});
			holder->addChild(big);

			if (in.window >= 0) {
				std::string sub = fmt::format("-{} +{}", in.left, in.right);
				auto subLabel = CCLabelBMFont::create(sub.c_str(), "chatFont.fnt");
				subLabel->setScale(0.42f * sc);
				subLabel->setOpacity(210);
				subLabel->setPosition({0.f, -9.f * sc});
				holder->addChild(subLabel);
			}

			this->addChild(holder, 1);
			m_items.push_back({in.x, holder});
		}

		this->scheduleUpdate();
		return true;
	}

	// only draw labels near the player so huge macros stay cheap
	void update(float) override {
		auto pl = PlayLayer::get();
		if (!pl || !pl->m_player1) return;
		float px = pl->m_player1->getPositionX();
		for (auto& it : m_items) {
			it.node->setVisible(it.x > px - 260.f && it.x < px + 520.f);
		}
	}
};

// ------------------------------------------------------------- controller

enum class Mode { Idle, Recording, Replaying, Analyzing };

struct Ctrl {
	static Ctrl& get() {
		static Ctrl s;
		return s;
	}

	Mode mode = Mode::Idle;
	FWMacro macro;
	std::filesystem::path path;

	int frame = 0;          // physics step counter (reset on every level reset)
	size_t nextIdx = 0;     // next input to inject
	bool injecting = false; // true while WE call handleButton
	bool pendingReset = false;
	int loggedDeaths = 0;   // hazard hits ignored by noclip

	// ---- analysis state
	enum class Phase { Baseline, Sweep };
	Phase phase = Phase::Baseline;
	std::vector<FWInput> playlist;      // macro with the shifted inputs applied
	std::vector<int> prv, nxt, segEnd;  // same-player neighbours + end of test window
	std::vector<int> bound[2];          // [0] = later (+), [1] = earlier (-)
	std::vector<char> baseFail;
	std::vector<std::vector<int>> passes;
	size_t pass = 0;
	int dir = 1;
	int k = 0;
	std::vector<int> alive;             // inputs shifted in the current run
	std::vector<int> baseDeaths, runDeaths;
	int runEnd = 0;
	bool runDone = false;
	int runsDone = 0;
	int W = 10, L = 30;

	// ------------------------------------------------------ level lifecycle

	void onEnterLevel(PlayLayer* pl) {
		mode = Mode::Idle;
		frame = 0;
		nextIdx = 0;
		pendingReset = false;
		loggedDeaths = 0;
		macro.clear();

		std::string name = pl->m_level->m_levelName;
		for (auto& ch : name) {
			if (!std::isalnum(static_cast<unsigned char>(ch))) ch = '_';
		}
		int id = pl->m_level->m_levelID.value();
		path = Mod::get()->getSaveDir() / "macros" / fmt::format("{}_{}.fwm", id, name);
		macro.load(path);
		refreshIndicators(pl);
	}

	void onReset() {
		frame = 0;
		nextIdx = 0;
		if (mode == Mode::Recording) macro.inputs.clear();
	}

	void refreshIndicators(PlayLayer* pl) {
		if (!pl || !pl->m_objectLayer) return;
		if (auto old = pl->m_objectLayer->getChildByID("fw-indicators"_spr)) {
			old->removeFromParent();
		}
		if (!optBool("show-indicators") || macro.inputs.empty()) return;
		if (auto layer = FWIndicators::create(macro)) {
			layer->setID("fw-indicators"_spr);
			pl->m_objectLayer->addChild(layer, 1000);
		}
	}

	// Runs from PlayLayer::update, i.e. outside the physics step, so it is
	// safe to reset the level from here.
	void tick(PlayLayer* pl) {
		if (pendingReset) {
			pendingReset = false;
			pl->resetLevelFromStart();
			return;
		}
		if (mode != Mode::Analyzing) return;
		if (!runDone && frame > runEnd) runDone = true;
		if (runDone) {
			runDone = false;
			finishRun(pl);
		}
	}

	// ------------------------------------------------------- record / replay

	void record(GJBaseGameLayer* gl, bool down, int button, bool p1) {
		FWInput in;
		in.frame = frame;
		in.down = down;
		in.p2 = !p1;
		in.button = button;
		auto plr = p1 ? gl->m_player1 : gl->m_player2;
		if (plr) {
			in.x = plr->getPositionX();
			in.y = plr->getPositionY();
		}
		macro.inputs.push_back(in);
	}

	void inject(GJBaseGameLayer* gl) {
		auto& list = (mode == Mode::Analyzing) ? playlist : macro.inputs;
		while (nextIdx < list.size() && list[nextIdx].frame <= frame) {
			auto const& in = list[nextIdx++];
			injecting = true;
			gl->handleButton(in.down, in.button, !in.p2);
			injecting = false;
		}
	}

	void releaseAll(PlayLayer* pl) {
		injecting = true;
		pl->handleButton(false, 1, true);
		pl->handleButton(false, 1, false);
		injecting = false;
	}

	void onWouldDie() {
		if (mode == Mode::Analyzing) {
			if (runDeaths.empty() || runDeaths.back() != frame) runDeaths.push_back(frame);
		} else {
			loggedDeaths++;
		}
	}

	void finishRecording(PlayLayer* pl) {
		mode = Mode::Idle;
		if (macro.inputs.empty()) return;
		if (macro.save(path)) {
			toast(fmt::format("Saved {} inputs", macro.inputs.size()), NotificationIcon::Success);
		} else {
			toast("Couldn't save macro", NotificationIcon::Error);
		}
		refreshIndicators(pl);
	}

	void stopAll(PlayLayer* pl) {
		Mode old = mode;
		if (old == Mode::Recording) finishRecording(pl);
		mode = Mode::Idle;
		if (old == Mode::Analyzing) { // discard partial results
			macro.load(path);
			refreshIndicators(pl);
			pendingReset = true;
		}
		if (old == Mode::Replaying) releaseAll(pl);
	}

	void toggleRecord(PlayLayer* pl) {
		if (mode == Mode::Recording) return stopAll(pl);
		if (mode != Mode::Idle) stopAll(pl);
		macro.clear();
		refreshIndicators(pl);
		mode = Mode::Recording;
		pendingReset = true;
	}

	void toggleReplay(PlayLayer* pl) {
		if (mode == Mode::Replaying) return stopAll(pl);
		if (mode != Mode::Idle) stopAll(pl);
		if (macro.inputs.empty()) return toast("No macro recorded for this level", NotificationIcon::Warning);
		mode = Mode::Replaying;
		pendingReset = true;
	}

	void toggleAnalyze(PlayLayer* pl) {
		if (mode == Mode::Analyzing) return stopAll(pl);
		if (mode != Mode::Idle) stopAll(pl);
		if (macro.inputs.empty()) return toast("No macro recorded for this level", NotificationIcon::Warning);
		mode = Mode::Analyzing;
		anBegin();
	}

	void clearMacro(PlayLayer* pl) {
		stopAll(pl);
		macro.clear();
		std::error_code ec;
		std::filesystem::remove(path, ec);
		refreshIndicators(pl);
	}

	// -------------------------------------------------------------- analysis

	void anBegin() {
		W = std::max(1, optInt("max-window"));
		L = std::max(5, optInt("lookahead"));
		auto& in = macro.inputs;
		size_t n = in.size();

		prv.assign(n, -1);
		nxt.assign(n, -1);
		segEnd.assign(n, 0);
		int last[2] = {-1, -1};
		for (size_t i = 0; i < n; i++) {
			int p = in[i].p2 ? 1 : 0;
			prv[i] = last[p];
			if (last[p] >= 0) nxt[last[p]] = static_cast<int>(i);
			last[p] = static_cast<int>(i);
			in[i].window = -1;
			in[i].left = in[i].right = 0;
		}
		for (size_t i = 0; i < n; i++) {
			int nf = nxt[i] >= 0 ? in[nxt[i]].frame : in[i].frame + L;
			segEnd[i] = nf + L;
		}
		bound[0].assign(n, 0);
		bound[1].assign(n, 0);
		baseFail.assign(n, 0);
		passes.clear();
		pass = 0;
		dir = 1;
		k = 0;
		alive.clear();
		phase = Phase::Baseline;
		runsDone = 0;
		startRun(0, 0);
	}

	bool validShift(int i, int s) const {
		auto const& in = macro.inputs;
		int nf = in[i].frame + s;
		if (nf < 0) return false;
		if (prv[i] >= 0 && nf <= in[prv[i]].frame) return false;
		if (nxt[i] >= 0 && nf >= in[nxt[i]].frame) return false;
		return true;
	}

	void startRun(int sdir, int sk) {
		playlist = macro.inputs;
		int end = 0;
		if (phase == Phase::Baseline) {
			end = macro.lastFrame() + L + 2;
		} else {
			for (int i : alive) {
				playlist[i].frame += sdir * sk;
				end = std::max(end, segEnd[i] + 2);
			}
		}
		std::stable_sort(playlist.begin(), playlist.end(), [](FWInput const& a, FWInput const& b) {
			return a.frame < b.frame;
		});
		runEnd = end;
		runDeaths.clear();
		runDone = false;
		nextIdx = 0;
		pendingReset = true;
		runsDone++;
	}

	void finishRun(PlayLayer* pl) {
		std::sort(runDeaths.begin(), runDeaths.end());
		auto& in = macro.inputs;

		if (phase == Phase::Baseline) {
			baseDeaths = runDeaths;

			// blame every baseline death on the latest input at or before it
			std::vector<int> frames;
			frames.reserve(in.size());
			for (auto const& x : in) frames.push_back(x.frame);
			for (int d : baseDeaths) {
				auto it = std::upper_bound(frames.begin(), frames.end(), d);
				int idx = static_cast<int>(it - frames.begin()) - 1;
				if (idx >= 0) baseFail[idx] = 1;
			}

			std::vector<int> testable;
			for (size_t i = 0; i < in.size(); i++) {
				if (baseFail[i]) {
					in[i].window = 0;
					in[i].left = in[i].right = 0;
				} else {
					testable.push_back(static_cast<int>(i));
				}
			}

			// pack inputs whose test windows can't overlap into the same pass so
			// several inputs are measured per replay
			std::vector<int> passEnd;
			for (int i : testable) {
				bool placed = false;
				for (size_t p = 0; p < passes.size(); p++) {
					if (in[i].frame - W > passEnd[p] + W) {
						passes[p].push_back(i);
						passEnd[p] = segEnd[i];
						placed = true;
						break;
					}
				}
				if (!placed) {
					passes.push_back({i});
					passEnd.push_back(segEnd[i]);
				}
			}

			phase = Phase::Sweep;
			pass = 0;
			dir = 1;
			k = 0;
			advanceSweep(pl);
			return;
		}

		// sweep result: which shifted inputs survived?
		int shift = dir * k;
		std::vector<int> keep;
		for (int i : alive) {
			int s0 = std::min(in[i].frame, in[i].frame + shift);
			bool fail = false;
			for (auto it = std::lower_bound(runDeaths.begin(), runDeaths.end(), s0);
				 it != runDeaths.end() && *it <= segEnd[i]; ++it) {
				if (!std::binary_search(baseDeaths.begin(), baseDeaths.end(), *it)) {
					fail = true;
					break;
				}
			}
			if (!fail) {
				bound[dir > 0 ? 0 : 1][i] = k;
				keep.push_back(i);
			}
		}
		alive = keep;
		k++;
		advanceSweep(pl);
	}

	void advanceSweep(PlayLayer* pl) {
		for (;;) {
			if (k == 0) {
				if (pass >= passes.size()) return finishAnalysis(pl);
				alive = passes[pass];
				k = 1;
			}
			std::vector<int> keep;
			for (int i : alive) {
				if (k <= W && validShift(i, dir * k)) keep.push_back(i);
			}
			alive = keep;
			if (alive.empty()) {
				k = 0;
				if (dir == 1) {
					dir = -1;
				} else {
					dir = 1;
					pass++;
				}
				continue;
			}
			return startRun(dir, k);
		}
	}

	void finishAnalysis(PlayLayer* pl) {
		auto& in = macro.inputs;
		for (size_t i = 0; i < in.size(); i++) {
			if (baseFail[i]) continue;
			in[i].left = bound[1][i];
			in[i].right = bound[0][i];
			in[i].window = in[i].left + in[i].right + 1;
		}
		macro.save(path);
		mode = Mode::Idle;
		refreshIndicators(pl);
		toast(fmt::format("Frame windows done ({} replays)", runsDone), NotificationIcon::Success);
		pendingReset = true;
	}

	std::string analysisStatus() const {
		if (phase == Phase::Baseline) return fmt::format("ANALYZING | baseline | f{}", frame);
		return fmt::format("ANALYZING | pass {}/{} | {}{}f | replay {}",
			std::min(pass + 1, passes.size()), passes.size(), dir > 0 ? "+" : "-", k, runsDone);
	}
};

// ------------------------------------------------------------------- HUD

class FWHud : public CCNode {
	CCLabelBMFont* m_label = nullptr;

	static void addBtn(CCMenu* menu, char const* text, float x, float y, std::function<void()> cb) {
		auto spr = ButtonSprite::create(text);
		spr->setScale(0.45f);
		auto btn = CCMenuItemExt::createSpriteExtra(spr, [cb](CCMenuItemSpriteExtra*) { cb(); });
		btn->setPosition({x, y});
		menu->addChild(btn);
	}

	static void stepSpeed(int d) {
		static const double steps[] = {0.1, 0.25, 0.5, 0.75, 1.0, 1.25, 1.5, 2.0, 3.0, 4.0, 8.0};
		constexpr int N = sizeof(steps) / sizeof(steps[0]);
		double cur = optFloat("speed");
		int idx = 0;
		double best = 1e9;
		for (int i = 0; i < N; i++) {
			if (std::fabs(steps[i] - cur) < best) {
				best = std::fabs(steps[i] - cur);
				idx = i;
			}
		}
		idx = std::clamp(idx + d, 0, N - 1);
		Mod::get()->setSettingValue<double>("speed", steps[idx]);
	}

public:
	static FWHud* create() {
		auto ret = new FWHud();
		if (ret && ret->setup()) {
			ret->autorelease();
			return ret;
		}
		CC_SAFE_DELETE(ret);
		return nullptr;
	}

	bool setup() {
		if (!CCNode::init()) return false;
		auto win = CCDirector::sharedDirector()->getWinSize();

		m_label = CCLabelBMFont::create("", "chatFont.fnt");
		m_label->setAnchorPoint({0.f, 1.f});
		m_label->setPosition({6.f, win.height - 6.f});
		m_label->setScale(0.55f);
		m_label->setOpacity(170);
		this->addChild(m_label);

		auto menu = CCMenu::create();
		menu->setPosition({0.f, 0.f});
		this->addChild(menu);

		float y1 = win.height - 46.f;
		float y2 = win.height - 68.f;
		addBtn(menu, "REC", 26.f, y1, [] { if (auto pl = PlayLayer::get()) Ctrl::get().toggleRecord(pl); });
		addBtn(menu, "PLAY", 62.f, y1, [] { if (auto pl = PlayLayer::get()) Ctrl::get().toggleReplay(pl); });
		addBtn(menu, "ANALYZE", 110.f, y1, [] { if (auto pl = PlayLayer::get()) Ctrl::get().toggleAnalyze(pl); });
		addBtn(menu, "CLEAR", 160.f, y1, [] { if (auto pl = PlayLayer::get()) Ctrl::get().clearMacro(pl); });
		addBtn(menu, "NOCLIP", 36.f, y2, [] {
			Mod::get()->setSettingValue<bool>("noclip", !optBool("noclip"));
		});
		addBtn(menu, "-", 82.f, y2, [] { stepSpeed(-1); });
		addBtn(menu, "+", 102.f, y2, [] { stepSpeed(+1); });

		this->scheduleUpdate();
		return true;
	}

	void update(float) override {
		auto& c = Ctrl::get();
		std::string top;
		switch (c.mode) {
			case Mode::Idle:
				top = fmt::format("IDLE | {} inputs", c.macro.inputs.size());
				break;
			case Mode::Recording:
				top = fmt::format("REC | frame {} | {} inputs", c.frame, c.macro.inputs.size());
				break;
			case Mode::Replaying:
				top = fmt::format("REPLAY | frame {} | {}/{}", c.frame, c.nextIdx, c.macro.inputs.size());
				break;
			case Mode::Analyzing:
				top = c.analysisStatus();
				break;
		}
		std::string text = fmt::format("{}\nnoclip {} | {:.2f}x | hazard hits ignored {}",
			top, optBool("noclip") ? "ON" : "OFF", optFloat("speed"), c.loggedDeaths);
		m_label->setString(text.c_str());
	}
};

// ----------------------------------------------------------------- hooks

class $modify(FWUILayer, UILayer) {
	bool init(GJBaseGameLayer* gl) {
		if (!UILayer::init(gl)) return false;
		if (typeinfo_cast<PlayLayer*>(gl) && optBool("show-hud")) {
			if (auto hud = FWHud::create()) {
				hud->setID("fw-hud"_spr);
				this->addChild(hud, 100);
			}
		}
		return true;
	}
};

// speedhack: scale the time the whole game sees while a level is open
class $modify(FWScheduler, CCScheduler) {
	void update(float dt) {
		if (PlayLayer::get()) {
			double s = Ctrl::get().mode == Mode::Analyzing ? optFloat("analysis-speed") : optFloat("speed");
			if (s != 1.0) dt *= static_cast<float>(s);
		}
		CCScheduler::update(dt);
	}
};

class $modify(FWBase, GJBaseGameLayer) {
	// called once per physics step: this is our frame counter and injection point
	void processCommands(float dt) {
		auto pl = PlayLayer::get();
		if (!pl || static_cast<GJBaseGameLayer*>(pl) != static_cast<GJBaseGameLayer*>(this)) {
			GJBaseGameLayer::processCommands(dt);
			return;
		}
		auto& c = Ctrl::get();
		if (c.mode == Mode::Replaying || c.mode == Mode::Analyzing) c.inject(this);
		GJBaseGameLayer::processCommands(dt);
		c.frame++;
	}

	void handleButton(bool down, int button, bool isPlayer1) {
		auto pl = PlayLayer::get();
		bool inLevel = pl && static_cast<GJBaseGameLayer*>(pl) == static_cast<GJBaseGameLayer*>(this);
		auto& c = Ctrl::get();
		if (inLevel && !c.injecting) {
			// while a macro drives the player, ignore real input
			if (c.mode == Mode::Replaying || c.mode == Mode::Analyzing) return;
			if (c.mode == Mode::Recording) c.record(this, down, button, isPlayer1);
		}
		GJBaseGameLayer::handleButton(down, button, isPlayer1);
	}
};

class $modify(FWPlayLayer, PlayLayer) {
	void setupHasCompleted() {
		PlayLayer::setupHasCompleted();
		Ctrl::get().onEnterLevel(this);
	}

	void resetLevel() {
		PlayLayer::resetLevel();
		Ctrl::get().onReset();
	}

	void update(float dt) {
		PlayLayer::update(dt);
		Ctrl::get().tick(this);
	}

	// noclip (the anti-cheat spike is left alone on purpose)
	void destroyPlayer(PlayerObject* player, GameObject* object) {
		auto& c = Ctrl::get();
		if (object == m_anticheatSpike) {
			PlayLayer::destroyPlayer(player, object);
			return;
		}
		if (c.mode == Mode::Analyzing || optBool("noclip")) {
			c.onWouldDie();
			return;
		}
		PlayLayer::destroyPlayer(player, object);
	}

	// Safe mode: noclip / speedhack / macro-driven completions are never counted.
	void levelComplete() {
		auto& c = Ctrl::get();
		if (c.mode == Mode::Analyzing) {
			c.runDone = true;
			return;
		}
		if (c.mode == Mode::Recording) c.finishRecording(this);

		bool replay = c.mode == Mode::Replaying;
		if (replay) c.mode = Mode::Idle;

		if (replay || optBool("noclip") || optFloat("speed") != 1.0) {
			toast(replay ? "Replay finished (completion not submitted)"
			             : "Safe mode: cheated completions don't count",
				NotificationIcon::Warning);
			c.pendingReset = true;
			return;
		}
		PlayLayer::levelComplete();
	}
};
