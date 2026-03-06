// turok - Settings persistence
// Comprehensive settings exposing Xenia/ReXGlue runtime CVars

#pragma once

#include <string>
#include <filesystem>

struct TurokSettings {
    // [gfx]
    std::string render_path = "rov";  // "rov" or "rtv"
    int draw_resolution_scale_x = 1;
    int draw_resolution_scale_y = 1;
    bool fullscreen = false;
    bool vsync = true;

    // [gpu]
    bool trace_gpu_stream = false;
    std::string trace_gpu_prefix = "";
    int query_occlusion_fake_sample_count = 1000;
    bool native_stencil_value_output = true;
    bool gamma_render_target_as_unorm16 = true;
    bool force_convert_line_loops_to_strips = false;
    bool force_convert_quad_lists_to_triangle_lists = false;
    bool force_convert_triangle_fans_to_lists = false;
    int primitive_processor_cache_min_indices = 0;
    int texture_cache_memory_limit_render_to = 24;
    int texture_cache_memory_limit_soft = 384;
    int texture_cache_memory_limit_hard = 768;
    int texture_cache_memory_limit_soft_lifetime = 30;
    bool mrt_edram_used_range_clamp_to_min = false;
    bool execute_unclipped_draw_vs_on_cpu_for_psi_render_backend = false;

    // [gpu_shader]
    // (shader-specific options can be added as discovered)

    // [cpu_runtime]
    bool protect_zero_page = true;
    bool ignore_undefined_externs = true;

    // [kernel]
    // (kernel options)

    // [memory]
    // (memory options)

    // [audio]
    bool mute = false;

    // [game]
    bool full_game = true;

    // [controls]
    std::string controller_1 = "auto";
    bool connected_2 = false;
    bool connected_3 = false;
    bool connected_4 = false;

    // [debug]
    bool show_fps = true;
    bool show_console = false;

    // [log]
    std::string log_level = "info";
    bool log_verbose = false;
};

// Load settings from TOML file. Returns defaults if file doesn't exist.
TurokSettings LoadSettings(const std::filesystem::path& path);

// Save settings to TOML file.
void SaveSettings(const std::filesystem::path& path, const TurokSettings& settings);
