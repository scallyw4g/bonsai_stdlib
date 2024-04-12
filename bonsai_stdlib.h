

#define poof(...)

// NOTE(Jesse): This is strictly present to hack around some deficiencies in
// poof.  It should be removed in the future.
#include <bonsai_stdlib/src/poof_crutches.h>

#include <bonsai_stdlib/src/poof_functions.h>

#include <bonsai_stdlib/src/iterators.h>
#include <bonsai_stdlib/src/assert.h>
#include <bonsai_stdlib/src/primitives.h>
#include <bonsai_stdlib/src/console_macros.h>
#include <bonsai_stdlib/src/globals.h>
#include <bonsai_stdlib/src/platform.h>
#include <bonsai_stdlib/src/bit_tricks.h>
#include <bonsai_stdlib/src/atomic.h>
#include <bonsai_stdlib/src/thread.h>

#include <bonsai_stdlib/src/costable.h>
#include <bonsai_stdlib/src/arccostable.h>

#include <immintrin.h>
#include <xmmintrin.h>

#include <bonsai_stdlib/src/maff.h>
#include <bonsai_stdlib/src/memory_arena.h>
#include <bonsai_stdlib/src/primitive_containers.h>
#include <bonsai_stdlib/src/file.h>
#include <bonsai_stdlib/src/filesystem.h>
#include <bonsai_stdlib/src/vector.h>
#include <bonsai_stdlib/src/matrix.h>
#include <bonsai_stdlib/src/colors.h>

#include <bonsai_stdlib/src/gl.h>
#include <bonsai_stdlib/src/ui/gl.h>

#include <bonsai_stdlib/src/texture.h>
#include <bonsai_stdlib/src/shader.h>
#include <bonsai_stdlib/src/random.h>
#include <bonsai_stdlib/src/noise.h>
#include <bonsai_stdlib/src/perlin.h>
#include <bonsai_stdlib/src/simplex.h>
#include <bonsai_stdlib/src/input.h>
#include <bonsai_stdlib/src/mutex.h>
#include <bonsai_stdlib/src/work_queue.h>
#include <bonsai_stdlib/src/mesh.h>


struct gpu_element_buffer_handles
{
  // NOTE(Jesse): VertexHandle has to come first because it's the one that gets passed to GL.DeleteBuffers
  u32 VertexHandle;
  u32 NormalHandle;
  u32    MatHandle;

  u32 ElementCount;
  b32 Mapped;
};

// This is a buffer we ask for and directly copy into
struct gpu_mapped_element_buffer
{
  gpu_element_buffer_handles    Handles;
  untextured_3d_geometry_buffer Buffer;
};

struct framebuffer
{
  u32 ID;
  u32 Attachments;
};

struct texture_ptr_block_array;

struct render_entity_to_texture_group
{
  framebuffer                FBO;
  shader                     Shader;
  gpu_mapped_element_buffer  GeoBuffer;
  m4 ViewProjection;
};



#include <bonsai_stdlib/src/platform_struct.h>
#include <bonsai_stdlib/src/heap_allocator.h>
#include <bonsai_stdlib/src/stream.h>
#include <bonsai_stdlib/src/counted_string.h>
#include <bonsai_stdlib/src/sort.h>
#include <bonsai_stdlib/src/console_io.h>
#include <bonsai_stdlib/src/line.h>
#include <bonsai_stdlib/src/quaternion.h>
#include <bonsai_stdlib/src/string_builder.h>
#include <bonsai_stdlib/src/ansi_stream.h>
#include <bonsai_stdlib/src/rect.h>
#include <bonsai_stdlib/src/xml.h>
#include <bonsai_stdlib/src/geometry_buffer.h>
struct light;
struct camera;
#include <bonsai_stdlib/src/ui/interactable.h>
#include <bonsai_stdlib/src/ui/ui.h>

#include <bonsai_stdlib/src/c_token.h>
#include <bonsai_stdlib/src/c_parser.h>

#include <bonsai_stdlib/src/debug_ui.h>

#if BONSAI_DEBUG_SYSTEM_API
  #include <bonsai_debug/debug.h>
  #include <bonsai_debug/src/api.h>
#else
  struct bonsai_debug_system {};
  #define TIMED_FUNCTION(...)
  #define TIMED_NAMED_BLOCK(...)
  #define HISTOGRAM_FUNCTION(...)

  #define TIMED_BLOCK(...)
  #define END_BLOCK(...)

  #define DEBUG_VALUE(...)

  #define TIMED_MUTEX_WAITING(...)
  #define TIMED_MUTEX_AQUIRED(...)
  #define TIMED_MUTEX_RELEASED(...)

  #define DEBUG_FRAME_RECORD(...)
  #define DEBUG_FRAME_END(...)
  #define DEBUG_FRAME_BEGIN(...)

  #define WORKER_THREAD_WAIT_FOR_DEBUG_SYSTEM(...)
  #define MAIN_THREAD_ADVANCE_DEBUG_SYSTEM(...)
  #define WORKER_THREAD_ADVANCE_DEBUG_SYSTEM()

  #define DEBUG_CLEAR_META_RECORDS_FOR(...)
  #define DEBUG_TRACK_DRAW_CALL(...)
#endif

#define UNPACK_STDLIB(Stdlib) \
  os *Os         = &(Stdlib)->Os; \
  platform *Plat = &(Stdlib)->Plat;


struct bonsai_stdlib
{
  os Os;
  platform Plat;
  thread_local_state *ThreadStates;

  //
  // Debug
  //

  texture_block_array AllTextures;

  bonsai_debug_system DebugSystem;
};

link_weak bonsai_stdlib *GetStdlib();

