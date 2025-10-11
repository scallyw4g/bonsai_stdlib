#ifndef BONSAI_SHADER_PATH
#define BONSAI_SHADER_PATH "shaders/"
#endif

#ifndef STDLIB_SHADER_PATH
#define STDLIB_SHADER_PATH "external/bonsai_stdlib/shaders/"
#endif


#include <bonsai_stdlib/src/perlin.cpp>

#include <bonsai_stdlib/src/assert.cpp>
#include <bonsai_stdlib/src/memory_arena.cpp>
#include <bonsai_stdlib/src/vector.cpp>
#include <bonsai_stdlib/src/debug_print.cpp>  // TODO(Jesse, id: 91, tags: cleanup, metaprogramming, format_counted_string): Jettison this .. Can it be metaprogrammed?
#include <bonsai_stdlib/src/counted_string.cpp>
#include <bonsai_stdlib/src/sort.cpp>
#include <bonsai_stdlib/src/rect.cpp>
#include <bonsai_stdlib/src/primitive_containers.cpp>
#include <bonsai_stdlib/src/platform.cpp>
#include <bonsai_stdlib/src/thread.cpp>
#include <bonsai_stdlib/src/string_builder.cpp>
#include <bonsai_stdlib/src/ansi_stream.cpp>
#include <bonsai_stdlib/src/binary_parser.cpp>
#include <bonsai_stdlib/src/colors.cpp>
#include <bonsai_stdlib/src/bitmap.cpp>
#include <bonsai_stdlib/src/matrix.cpp>
#include <bonsai_stdlib/src/heap_memory.cpp>
#include <bonsai_stdlib/src/xml.cpp>
#include <bonsai_stdlib/src/file.cpp>
#include <bonsai_stdlib/src/filesystem.cpp>

// NOTE(Jesse): Must match defines in header.glsl
#define VERTEX_POSITION_LAYOUT_LOCATION    0

// NOTE(Jesse): Normals and UVs are mutually exclusive, so we use slot 1 for either/or;
// We never draw 3D geometry with UVs and 2D UI doesn't need normals
#define VERTEX_NORMAL_LAYOUT_LOCATION      1
#define VERTEX_UV_LAYOUT_LOCATION          1

#define VERTEX_COLOR_LAYOUT_LOCATION       2
#define VERTEX_TRANS_EMISS_LAYOUT_LOCATION 3


#include <bonsai_stdlib/src/gl.cpp>
#include <bonsai_stdlib/src/texture.cpp>
#include <bonsai_stdlib/src/shader.cpp>
#include <bonsai_stdlib/src/2d_render_utils.cpp>
#include <bonsai_stdlib/src/ui/gl.cpp>

#include <bonsai_stdlib/src/gpu_mapped_buffer.cpp>

#include <bonsai_stdlib/src/to_string.cpp>

#include <bonsai_stdlib/src/texture_cursor.cpp>
#include <bonsai_stdlib/src/ui/interactable.cpp>
#include <bonsai_stdlib/src/ui/ui.cpp>
#include <bonsai_stdlib/src/debug_ui.cpp>

#include <bonsai_stdlib/src/c_token.cpp>
#include <bonsai_stdlib/src/c_parser.cpp>

#include <bonsai_stdlib/src/initialize.cpp>

#if BONSAI_DEBUG_SYSTEM_API
#include <bonsai_debug/debug.cpp>
#endif
