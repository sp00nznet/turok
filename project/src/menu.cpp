// turok - Menu bar and config dialogs implementation
// Xenia-style comprehensive settings panel

#include "menu.h"
#include "settings.h"

#include <rex/ui/menu_item.h>
#include <rex/ui/window.h>
#include <rex/ui/windowed_app.h>
#include <rex/ui/imgui_dialog.h>
#include <rex/ui/imgui_drawer.h>
#include <rex/runtime.h>
#include <rex/cvar.h>
#include <rex/graphics/flags.h>

#include <imgui.h>

#include <fstream>
#include <string>

using namespace rex::ui;

// ============================================================================
// Unified Settings Dialog - Xenia-style sidebar + content panels
// ============================================================================

class SettingsDialog : public ImGuiDialog {
public:
    SettingsDialog(ImGuiDrawer* drawer, WindowedAppContext* app_context,
                   Window* window, TurokSettings* settings,
                   const std::filesystem::path& settings_path,
                   std::function<void()> on_done)
        : ImGuiDialog(drawer), app_context_(app_context), window_(window),
          settings_(settings), settings_path_(settings_path),
          on_done_(std::move(on_done)) {
        // Copy settings for editing
        edit_ = *settings;
    }

protected:
    void OnDraw(ImGuiIO& io) override {
        (void)io;
        ImGui::SetNextWindowSize(ImVec2(700, 520), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoCollapse)) {
            ImGui::End();
            return;
        }

        // Left sidebar
        ImGui::BeginChild("##sidebar", ImVec2(140, -40), true);
        const char* tabs[] = {
            "Audio", "Clock", "GPU", "GPU/D3D12", "GPU/Shader",
            "Input", "Kernel", "Keybinds", "Log", "Memory",
            "Runtime", "UI/D3D12", "UI/Presenter", "UI/Window", "Debug"
        };
        for (int i = 0; i < 15; i++) {
            if (ImGui::Selectable(tabs[i], selected_tab_ == i))
                selected_tab_ = i;
        }
        ImGui::EndChild();

        ImGui::SameLine();

        // Right content panel
        ImGui::BeginChild("##content", ImVec2(0, -40), true);
        switch (selected_tab_) {
        case 0: DrawAudio(); break;
        case 1: DrawClock(); break;
        case 2: DrawGPU(); break;
        case 3: DrawGPUD3D12(); break;
        case 4: DrawGPUShader(); break;
        case 5: DrawInput(); break;
        case 6: DrawKernel(); break;
        case 7: DrawKeybinds(); break;
        case 8: DrawLog(); break;
        case 9: DrawMemory(); break;
        case 10: DrawRuntime(); break;
        case 11: DrawUID3D12(); break;
        case 12: DrawUIPresenter(); break;
        case 13: DrawUIWindow(); break;
        case 14: DrawDebug(); break;
        }
        ImGui::EndChild();

        // Bottom buttons
        ImGui::Separator();
        float avail = ImGui::GetContentRegionAvail().x;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + avail - 260);

        if (ImGui::Button("Save to config", ImVec2(120, 0))) {
            *settings_ = edit_;
            SaveSettings(settings_path_, *settings_);
            ApplyLiveCVars();
            Close();
            if (on_done_) on_done_();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            Close();
            if (on_done_) on_done_();
        }

        ImGui::End();
    }

private:
    void CvarCheckbox(const char* label, bool* val, const char* tag = "[live]") {
        ImGui::Checkbox(label, val);
        if (tag) {
            ImGui::SameLine();
            ImGui::TextDisabled("%s", tag);
        }
    }

    void CvarInt(const char* label, int* val, int step = 1, const char* tag = "[live]") {
        ImGui::SetNextItemWidth(100);
        ImGui::InputInt(label, val, step);
        if (tag) {
            ImGui::SameLine();
            ImGui::TextDisabled("%s", tag);
        }
    }

    void CvarString(const char* label, std::string* val, const char* tag = "[live]") {
        char buf[256] = {};
        std::strncpy(buf, val->c_str(), sizeof(buf) - 1);
        ImGui::SetNextItemWidth(200);
        if (ImGui::InputText(label, buf, sizeof(buf))) {
            *val = buf;
        }
        if (tag) {
            ImGui::SameLine();
            ImGui::TextDisabled("%s", tag);
        }
    }

    // --- Panel implementations ---

    void DrawAudio() {
        ImGui::Text("Audio");
        ImGui::Separator();
        CvarCheckbox("Mute", &edit_.mute, "[live]");
        ImGui::TextDisabled("XMA audio decoding is handled by the ReXGlue runtime.");
    }

    void DrawClock() {
        ImGui::Text("Clock");
        ImGui::Separator();
        ImGui::TextDisabled("Timebase scaling is handled automatically.");
        ImGui::TextDisabled("Xbox 360 timebase: 49.875 MHz");
        ImGui::TextDisabled("Host TSC is scaled to match via __rdtsc() override.");
    }

    void DrawGPU() {
        ImGui::Text("GPU");
        ImGui::Separator();

        CvarCheckbox("fullscreen", &edit_.fullscreen, "[live]");
        CvarString("trace_gpu_prefix", &edit_.trace_gpu_prefix, "[live]");
        CvarCheckbox("trace_gpu_stream", &edit_.trace_gpu_stream, "[live]");
        CvarCheckbox("vsync", &edit_.vsync, "[live]");

        ImGui::Spacing();
        CvarInt("query_occlusion_fake_sample_count", &edit_.query_occlusion_fake_sample_count, 100, "");
        CvarCheckbox("native_stencil_value_output", &edit_.native_stencil_value_output, "[live]");
        CvarCheckbox("gamma_render_target_as_unorm16", &edit_.gamma_render_target_as_unorm16, "[live]");

        ImGui::Spacing();
        ImGui::Text("Primitive Conversion:");
        CvarCheckbox("force_convert_line_loops_to_strips", &edit_.force_convert_line_loops_to_strips, "[live]");
        CvarCheckbox("force_convert_quad_lists_to_triangle_lists", &edit_.force_convert_quad_lists_to_triangle_lists, "[live]");
        CvarCheckbox("force_convert_triangle_fans_to_lists", &edit_.force_convert_triangle_fans_to_lists, "[live]");

        ImGui::Spacing();
        CvarInt("primitive_processor_cache_min_indices", &edit_.primitive_processor_cache_min_indices, 1, "");

        ImGui::Spacing();
        ImGui::Text("Texture Cache Memory Limits (MB):");
        CvarInt("texture_cache_memory_limit_render_to", &edit_.texture_cache_memory_limit_render_to, 8, "[restart]");
        CvarInt("texture_cache_memory_limit_soft", &edit_.texture_cache_memory_limit_soft, 64, "[restart]");
        CvarInt("texture_cache_memory_limit_hard", &edit_.texture_cache_memory_limit_hard, 64, "[restart]");
        CvarInt("texture_cache_memory_limit_soft_lifetime", &edit_.texture_cache_memory_limit_soft_lifetime, 5, "");

        ImGui::Spacing();
        CvarInt("draw_resolution_scale_x", &edit_.draw_resolution_scale_x, 1, "[restart]");
        CvarInt("draw_resolution_scale_y", &edit_.draw_resolution_scale_y, 1, "[restart]");

        ImGui::Spacing();
        CvarCheckbox("mrt_edram_used_range_clamp_to_min", &edit_.mrt_edram_used_range_clamp_to_min, "[live]");
        CvarCheckbox("execute_unclipped_draw_vs_on_cpu_for_psi_render_backend",
                      &edit_.execute_unclipped_draw_vs_on_cpu_for_psi_render_backend, "[live]");
    }

    void DrawGPUD3D12() {
        ImGui::Text("GPU / D3D12");
        ImGui::Separator();

        ImGui::Text("Render Path:");
        ImGui::SameLine(160);
        ImGui::SetNextItemWidth(180);
        int idx = (edit_.render_path == "rtv") ? 1 : 0;
        const char* items[] = {"ROV (Recommended)", "RTV"};
        if (ImGui::Combo("##render_path", &idx, items, 2)) {
            edit_.render_path = (idx == 0) ? "rov" : "rtv";
        }
        ImGui::TextDisabled("Render path change requires restart.");
    }

    void DrawGPUShader() {
        ImGui::Text("GPU / Shader");
        ImGui::Separator();
        ImGui::TextDisabled("Shader translation is automatic via the D3D12 backend.");
        ImGui::TextDisabled("Xbox 360 shaders are converted to DXBC at runtime.");
    }

    void DrawInput() {
        ImGui::Text("Input");
        ImGui::Separator();
        ImGui::Text("Player 1 Controller:");
        ImGui::SameLine(200);
        ImGui::SetNextItemWidth(180);
        char buf[64] = {};
        std::strncpy(buf, edit_.controller_1.c_str(), sizeof(buf) - 1);
        if (ImGui::InputText("##ctrl1", buf, sizeof(buf))) {
            edit_.controller_1 = buf;
        }

        ImGui::Spacing();
        ImGui::Text("Multiplayer Slots:");
        CvarCheckbox("Player 2 connected", &edit_.connected_2, "[live]");
        CvarCheckbox("Player 3 connected", &edit_.connected_3, "[live]");
        CvarCheckbox("Player 4 connected", &edit_.connected_4, "[live]");

        ImGui::Spacing();
        ImGui::TextDisabled("Keyboard: WASD=D-pad, Z/J=A, X/K=B, C/L=X, V=Y");
        ImGui::TextDisabled("Enter=Start, Esc=Back, Q/E=LB/RB, 1/3=LT/RT");
        ImGui::TextDisabled("XInput controllers are merged with keyboard input.");
    }

    void DrawKeybinds() {
        ImGui::Text("Keybinds");
        ImGui::Separator();

        ImGui::TextDisabled("Default keybinds (hardcoded):");
        ImGui::Spacing();

        if (ImGui::BeginTable("##keybinds", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH)) {
            ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, 160);
            ImGui::TableSetupColumn("Key", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();

            const char* binds[][2] = {
                {"D-pad Up", "W / Up Arrow"},
                {"D-pad Down", "S / Down Arrow"},
                {"D-pad Left", "A / Left Arrow"},
                {"D-pad Right", "D / Right Arrow"},
                {"A (Confirm)", "Z / J"},
                {"B (Cancel)", "X / K"},
                {"X (Special)", "C / L"},
                {"Y", "V"},
                {"Start", "Enter"},
                {"Back", "Backspace / Esc"},
                {"LB", "Q"},
                {"RB", "E"},
                {"LT", "1"},
                {"RT", "3"},
                {"Fullscreen", "F11"},
            };
            for (auto& b : binds) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn(); ImGui::Text("%s", b[0]);
                ImGui::TableNextColumn(); ImGui::Text("%s", b[1]);
            }
            ImGui::EndTable();
        }
    }

    void DrawRuntime() {
        ImGui::Text("CPU Runtime");
        ImGui::Separator();
        CvarCheckbox("protect_zero_page", &edit_.protect_zero_page, "[restart]");
        CvarCheckbox("ignore_undefined_externs", &edit_.ignore_undefined_externs, "[restart]");
        ImGui::Spacing();
        ImGui::TextDisabled("Static recompilation - all PPC code is pre-translated to x86-64.");
        ImGui::TextDisabled("No JIT or interpreter is used at runtime.");
    }

    void DrawKernel() {
        ImGui::Text("Kernel");
        ImGui::Separator();
        CvarCheckbox("Unlock full game (bypass license check)", &edit_.full_game, "[restart]");
    }

    void DrawMemory() {
        ImGui::Text("Memory");
        ImGui::Separator();
        ImGui::TextDisabled("Memory configuration is handled automatically.");
        ImGui::TextDisabled("Guest memory: 512 MB address space with demand paging.");
    }

    void DrawLog() {
        ImGui::Text("Logging");
        ImGui::Separator();

        ImGui::Text("Log Level:");
        ImGui::SameLine(120);
        ImGui::SetNextItemWidth(150);
        const char* levels[] = {"error", "warning", "info", "debug", "trace"};
        int level_idx = 2; // default: info
        for (int i = 0; i < 5; i++) {
            if (edit_.log_level == levels[i]) { level_idx = i; break; }
        }
        if (ImGui::Combo("##log_level", &level_idx, levels, 5)) {
            edit_.log_level = levels[level_idx];
        }

        CvarCheckbox("Verbose logging", &edit_.log_verbose, "[restart]");
    }

    void DrawUID3D12() {
        ImGui::Text("UI / D3D12");
        ImGui::Separator();
        ImGui::TextDisabled("D3D12 UI integration settings.");
        ImGui::TextDisabled("ImGui overlay renders via the D3D12 immediate drawer.");
    }

    void DrawUIPresenter() {
        ImGui::Text("UI / Presenter");
        ImGui::Separator();
        CvarCheckbox("vsync", &edit_.vsync, "[live]");
        ImGui::Spacing();
        ImGui::TextDisabled("The presenter manages swap chain and frame pacing.");
        ImGui::TextDisabled("VdSwap frame limiter uses QPC spin-loop for precise 16.667ms timing.");
    }

    void DrawUIWindow() {
        ImGui::Text("UI / Window");
        ImGui::Separator();
        CvarCheckbox("fullscreen", &edit_.fullscreen, "[live]");
        ImGui::TextDisabled("F11 toggles fullscreen at any time.");
    }

    void DrawDebug() {
        ImGui::Text("Debug");
        ImGui::Separator();
        CvarCheckbox("Show FPS overlay", &edit_.show_fps, "[live]");
        CvarCheckbox("Show debug console", &edit_.show_console, "[live]");
    }

    void ApplyLiveCVars() {
        // Apply CVars that take effect immediately (tagged [live])
        REXCVAR_SET(vsync, edit_.vsync);
        REXCVAR_SET(trace_gpu_stream, edit_.trace_gpu_stream);

        // Fullscreen
        if (edit_.fullscreen != settings_->fullscreen && window_ && app_context_) {
            bool fs = edit_.fullscreen;
            auto* w = window_;
            app_context_->CallInUIThreadDeferred([w, fs]() {
                w->SetFullscreen(fs);
            });
        }
    }

    WindowedAppContext* app_context_;
    Window* window_;
    TurokSettings* settings_;
    std::filesystem::path settings_path_;
    std::function<void()> on_done_;
    TurokSettings edit_;
    int selected_tab_ = 1; // default to GPU tab
};

// ============================================================================
// MenuSystem implementation
// ============================================================================

struct MenuSystem::Impl {
    ImGuiDrawer* imgui_drawer;
    Window* window;
    WindowedAppContext* app_context;
    rex::Runtime* runtime;
    TurokSettings* settings;
    std::filesystem::path settings_path;
    std::function<void()> on_settings_changed;

    SettingsDialog* settings_dialog = nullptr;

    std::function<void()> MakeOnDone() {
        return [this]() {
            app_context->CallInUIThreadDeferred([this]() {
                settings_dialog = nullptr;
                if (on_settings_changed) on_settings_changed();
            });
        };
    }

    void ShowSettings() {
        if (settings_dialog) return;
        settings_dialog = new SettingsDialog(
            imgui_drawer, app_context, window,
            settings, settings_path, MakeOnDone());
    }

    void ShowAbout() {
        ImGuiDialog::ShowMessageBox(
            imgui_drawer,
            "About Turok",
            "Turok (Xbox 360) - Static Recompilation\n\n"
            "Built with ReXGlue SDK\n"
            "https://github.com/sp00nznet/turok");
    }
};

MenuSystem::MenuSystem(ImGuiDrawer* imgui_drawer, Window* window,
                       WindowedAppContext* app_context,
                       rex::Runtime* runtime,
                       TurokSettings* settings,
                       const std::filesystem::path& settings_path,
                       std::function<void()> on_settings_changed)
    : impl_(std::make_unique<Impl>()) {
    impl_->imgui_drawer = imgui_drawer;
    impl_->window = window;
    impl_->app_context = app_context;
    impl_->runtime = runtime;
    impl_->settings = settings;
    impl_->settings_path = settings_path;
    impl_->on_settings_changed = std::move(on_settings_changed);
}

MenuSystem::~MenuSystem() = default;

std::unique_ptr<MenuItem> MenuSystem::BuildMenuBar() {
    auto* ctx = impl_.get();

    auto root = MenuItem::Create(MenuItem::Type::kNormal);

    // --- File menu ---
    auto file_menu = MenuItem::Create(MenuItem::Type::kPopup, "File");
    file_menu->AddChild(MenuItem::Create(
        MenuItem::Type::kString, "Exit",
        [ctx]() { ctx->app_context->QuitFromUIThread(); }));
    root->AddChild(std::move(file_menu));

    // --- Config menu ---
    auto config_menu = MenuItem::Create(MenuItem::Type::kPopup, "Config");
    config_menu->AddChild(MenuItem::Create(
        MenuItem::Type::kString, "Settings...",
        [ctx]() { ctx->ShowSettings(); }));
    root->AddChild(std::move(config_menu));

    // --- Help menu ---
    auto help_menu = MenuItem::Create(MenuItem::Type::kPopup, "Help");
    help_menu->AddChild(MenuItem::Create(
        MenuItem::Type::kString, "About...",
        [ctx]() { ctx->ShowAbout(); }));
    root->AddChild(std::move(help_menu));

    return root;
}
