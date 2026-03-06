// turok - Settings persistence implementation

#include "settings.h"

#include <toml++/toml.hpp>
#include <fstream>

TurokSettings LoadSettings(const std::filesystem::path& path) {
    TurokSettings s;
    if (!std::filesystem::exists(path)) return s;

    try {
        auto tbl = toml::parse_file(path.string());

        // [gfx]
        s.render_path = tbl["gfx"]["render_path"].value_or(s.render_path);
        s.draw_resolution_scale_x = tbl["gfx"]["draw_resolution_scale_x"].value_or(s.draw_resolution_scale_x);
        s.draw_resolution_scale_y = tbl["gfx"]["draw_resolution_scale_y"].value_or(s.draw_resolution_scale_y);
        s.fullscreen = tbl["gfx"]["fullscreen"].value_or(s.fullscreen);
        s.vsync = tbl["gfx"]["vsync"].value_or(s.vsync);

        // [gpu]
        s.trace_gpu_stream = tbl["gpu"]["trace_gpu_stream"].value_or(s.trace_gpu_stream);
        s.trace_gpu_prefix = tbl["gpu"]["trace_gpu_prefix"].value_or(s.trace_gpu_prefix);
        s.query_occlusion_fake_sample_count = tbl["gpu"]["query_occlusion_fake_sample_count"].value_or(s.query_occlusion_fake_sample_count);
        s.native_stencil_value_output = tbl["gpu"]["native_stencil_value_output"].value_or(s.native_stencil_value_output);
        s.gamma_render_target_as_unorm16 = tbl["gpu"]["gamma_render_target_as_unorm16"].value_or(s.gamma_render_target_as_unorm16);
        s.force_convert_line_loops_to_strips = tbl["gpu"]["force_convert_line_loops_to_strips"].value_or(s.force_convert_line_loops_to_strips);
        s.force_convert_quad_lists_to_triangle_lists = tbl["gpu"]["force_convert_quad_lists_to_triangle_lists"].value_or(s.force_convert_quad_lists_to_triangle_lists);
        s.force_convert_triangle_fans_to_lists = tbl["gpu"]["force_convert_triangle_fans_to_lists"].value_or(s.force_convert_triangle_fans_to_lists);
        s.primitive_processor_cache_min_indices = tbl["gpu"]["primitive_processor_cache_min_indices"].value_or(s.primitive_processor_cache_min_indices);
        s.texture_cache_memory_limit_render_to = tbl["gpu"]["texture_cache_memory_limit_render_to"].value_or(s.texture_cache_memory_limit_render_to);
        s.texture_cache_memory_limit_soft = tbl["gpu"]["texture_cache_memory_limit_soft"].value_or(s.texture_cache_memory_limit_soft);
        s.texture_cache_memory_limit_hard = tbl["gpu"]["texture_cache_memory_limit_hard"].value_or(s.texture_cache_memory_limit_hard);
        s.texture_cache_memory_limit_soft_lifetime = tbl["gpu"]["texture_cache_memory_limit_soft_lifetime"].value_or(s.texture_cache_memory_limit_soft_lifetime);
        s.mrt_edram_used_range_clamp_to_min = tbl["gpu"]["mrt_edram_used_range_clamp_to_min"].value_or(s.mrt_edram_used_range_clamp_to_min);
        s.execute_unclipped_draw_vs_on_cpu_for_psi_render_backend = tbl["gpu"]["execute_unclipped_draw_vs_on_cpu_for_psi_render_backend"].value_or(s.execute_unclipped_draw_vs_on_cpu_for_psi_render_backend);

        // [cpu_runtime]
        s.protect_zero_page = tbl["cpu_runtime"]["protect_zero_page"].value_or(s.protect_zero_page);
        s.ignore_undefined_externs = tbl["cpu_runtime"]["ignore_undefined_externs"].value_or(s.ignore_undefined_externs);

        // [audio]
        s.mute = tbl["audio"]["mute"].value_or(s.mute);

        // [game]
        s.full_game = tbl["game"]["full_game"].value_or(s.full_game);

        // [controls]
        s.controller_1 = tbl["controls"]["controller_1"].value_or(s.controller_1);
        s.connected_2 = tbl["controls"]["connected_2"].value_or(s.connected_2);
        s.connected_3 = tbl["controls"]["connected_3"].value_or(s.connected_3);
        s.connected_4 = tbl["controls"]["connected_4"].value_or(s.connected_4);

        // [debug]
        s.show_fps = tbl["debug"]["show_fps"].value_or(s.show_fps);
        s.show_console = tbl["debug"]["show_console"].value_or(s.show_console);

        // [log]
        s.log_level = tbl["log"]["log_level"].value_or(s.log_level);
        s.log_verbose = tbl["log"]["log_verbose"].value_or(s.log_verbose);
    } catch (const toml::parse_error&) {
        // Parse error: return defaults
    }

    return s;
}

void SaveSettings(const std::filesystem::path& path, const TurokSettings& s) {
    std::ofstream f(path);
    if (!f) return;

    f << "[gfx]\n";
    f << "render_path = " << toml::value<std::string>(s.render_path) << "\n";
    f << "draw_resolution_scale_x = " << s.draw_resolution_scale_x << "\n";
    f << "draw_resolution_scale_y = " << s.draw_resolution_scale_y << "\n";
    f << "fullscreen = " << (s.fullscreen ? "true" : "false") << "\n";
    f << "vsync = " << (s.vsync ? "true" : "false") << "\n";
    f << "\n";

    f << "[gpu]\n";
    f << "trace_gpu_stream = " << (s.trace_gpu_stream ? "true" : "false") << "\n";
    f << "trace_gpu_prefix = " << toml::value<std::string>(s.trace_gpu_prefix) << "\n";
    f << "query_occlusion_fake_sample_count = " << s.query_occlusion_fake_sample_count << "\n";
    f << "native_stencil_value_output = " << (s.native_stencil_value_output ? "true" : "false") << "\n";
    f << "gamma_render_target_as_unorm16 = " << (s.gamma_render_target_as_unorm16 ? "true" : "false") << "\n";
    f << "force_convert_line_loops_to_strips = " << (s.force_convert_line_loops_to_strips ? "true" : "false") << "\n";
    f << "force_convert_quad_lists_to_triangle_lists = " << (s.force_convert_quad_lists_to_triangle_lists ? "true" : "false") << "\n";
    f << "force_convert_triangle_fans_to_lists = " << (s.force_convert_triangle_fans_to_lists ? "true" : "false") << "\n";
    f << "primitive_processor_cache_min_indices = " << s.primitive_processor_cache_min_indices << "\n";
    f << "texture_cache_memory_limit_render_to = " << s.texture_cache_memory_limit_render_to << "\n";
    f << "texture_cache_memory_limit_soft = " << s.texture_cache_memory_limit_soft << "\n";
    f << "texture_cache_memory_limit_hard = " << s.texture_cache_memory_limit_hard << "\n";
    f << "texture_cache_memory_limit_soft_lifetime = " << s.texture_cache_memory_limit_soft_lifetime << "\n";
    f << "mrt_edram_used_range_clamp_to_min = " << (s.mrt_edram_used_range_clamp_to_min ? "true" : "false") << "\n";
    f << "execute_unclipped_draw_vs_on_cpu_for_psi_render_backend = " << (s.execute_unclipped_draw_vs_on_cpu_for_psi_render_backend ? "true" : "false") << "\n";
    f << "\n";

    f << "[cpu_runtime]\n";
    f << "protect_zero_page = " << (s.protect_zero_page ? "true" : "false") << "\n";
    f << "ignore_undefined_externs = " << (s.ignore_undefined_externs ? "true" : "false") << "\n";
    f << "\n";

    f << "[audio]\n";
    f << "mute = " << (s.mute ? "true" : "false") << "\n";
    f << "\n";

    f << "[game]\n";
    f << "full_game = " << (s.full_game ? "true" : "false") << "\n";
    f << "\n";

    f << "[controls]\n";
    f << "controller_1 = " << toml::value<std::string>(s.controller_1) << "\n";
    f << "connected_2 = " << (s.connected_2 ? "true" : "false") << "\n";
    f << "connected_3 = " << (s.connected_3 ? "true" : "false") << "\n";
    f << "connected_4 = " << (s.connected_4 ? "true" : "false") << "\n";
    f << "\n";

    f << "[debug]\n";
    f << "show_fps = " << (s.show_fps ? "true" : "false") << "\n";
    f << "show_console = " << (s.show_console ? "true" : "false") << "\n";
    f << "\n";

    f << "[log]\n";
    f << "log_level = " << toml::value<std::string>(s.log_level) << "\n";
    f << "log_verbose = " << (s.log_verbose ? "true" : "false") << "\n";
}
