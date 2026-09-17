#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/utils/file.hpp>
#include <Geode/utils/async.hpp>
#include "TTRL.hpp"
#include "GameplayProbe.hpp"
#include "WindowAnalyzer.hpp"
#include "WindowOverlay.hpp"
#include <string>

using namespace geode::prelude;
using namespace cocos2d;

namespace {

class $modify(WindowAnalyzerPlayLayer, PlayLayer) {
    struct Fields {
        twa::WindowOverlay* overlay = nullptr;
        twa::GeometryDashProbe probe;
        bool attached = false;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects))
            return false;

        auto overlay = twa::WindowOverlay::create();
        if (overlay) {
            this->addChild(overlay, 9999);
            m_fields->overlay = overlay;
            m_fields->attached = true;

            overlay->setOnAnalyzeClicked([this]() {
                this->onAnalyzeReplay();
            });
        }

        return true;
    }

    void onAnalyzeReplay() {
        auto overlay = m_fields->overlay;
        if (!overlay) return;

        overlay->setStatus("Opening file picker...");

        using namespace geode::utils::file;
        this->retain();
        async::spawn(
            pick(FilePickMode::OpenFile, FilePickOptions{}),
            [this](Result<std::optional<std::filesystem::path>> result) {
                this->handlePickResult(result);
                this->release();
            }
        );
    }

    void handlePickResult(Result<std::optional<std::filesystem::path>> const& result) {
        auto overlay = m_fields->overlay;
        if (!overlay) return;

        if (result.isErr()) {
            overlay->setStatus("File picker error: " + result.unwrapErr());
            return;
        }

        auto opt = result.unwrap();
        if (!opt.has_value()) {
            overlay->setStatus("Cancelled.");
            return;
        }

        auto path = opt.value();
        if (path.extension() != ".ttrl") {
            overlay->setStatus("Please select a .ttrl file.");
            return;
        }

        auto parsed = twa::TTRLReader::read(path);
        if (!parsed.ok) {
            overlay->setStatus("Failed to read TTRL: " + parsed.error);
            return;
        }

        if (parsed.timeline.size() == 0) {
            overlay->setStatus("Header OK, but events aren't decoded for this TTRL revision yet.");
            overlay->setWindows({});
            return;
        }

        overlay->setStatus("Analyzing " + std::to_string(parsed.timeline.size()) + " inputs...");

        twa::WindowAnalyzer analyzer;
        auto windows = analyzer.analyze(parsed.timeline, m_fields->probe);

        overlay->setStatus("Done. Note: gameplay probe isn't implemented yet, so windows show as 0 for now.");
        overlay->setWindows(windows);
    }

    void onQuit() {
        if (m_fields->overlay) {
            m_fields->overlay->removeFromParentAndCleanup(true);
            m_fields->overlay = nullptr;
        }
        PlayLayer::onQuit();
    }
};

}

$execute {
    log::info("Toasty Window Analyzer v0.2.0 loaded.");
}
