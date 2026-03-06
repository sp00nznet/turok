// turok - Menu bar and config dialogs
// Comprehensive Xenia-style settings panel with all GPU/CPU/Runtime CVars

#pragma once

#include <memory>
#include <functional>
#include <filesystem>

namespace rex {
class Runtime;
}

namespace rex::ui {
class MenuItem;
class ImGuiDrawer;
class Window;
class WindowedAppContext;
}

struct TurokSettings;

class MenuSystem {
public:
    MenuSystem(rex::ui::ImGuiDrawer* imgui_drawer,
               rex::ui::Window* window,
               rex::ui::WindowedAppContext* app_context,
               rex::Runtime* runtime,
               TurokSettings* settings,
               const std::filesystem::path& settings_path,
               std::function<void()> on_settings_changed);
    ~MenuSystem();

    std::unique_ptr<rex::ui::MenuItem> BuildMenuBar();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
