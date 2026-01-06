/* #define BONSAI_NO_AVX */

#define MAIN_THREAD_ThreadLocal_ThreadIndex (0)
#define RENDER_THREAD_ThreadLocal_ThreadIndex (1)

// NOTE(Jesse): This is strictly present to hack around some deficiencies in
// poof.  It should be removed in the future.
#include <bonsai_stdlib/src/poof_crutches.h>

#include <bonsai_stdlib/src/poof_functions.h>
#include <bonsai_stdlib/src/primitives.h>
#include <bonsai_stdlib/src/assert.h>

struct bonsai_stdlib;
link_internal bonsai_stdlib * GetStdlib();

struct debug_state;
link_internal debug_state * GetDebugState();


#include <bonsai_stdlib/src/simd.h>
#include <bonsai_stdlib/src/simd_sse.h>

#ifndef BONSAI_NO_AVX
#include <bonsai_stdlib/src/simd_avx2.h>
#include <bonsai_stdlib/src/avx2_v3.h>
#endif

#include <bonsai_stdlib/src/console_macros.h>
#include <bonsai_stdlib/src/globals.h>
#include <bonsai_stdlib/src/platform.h>
#include <bonsai_stdlib/src/bit_tricks.h>
#include <bonsai_stdlib/src/atomic.h>
#include <bonsai_stdlib/src/thread.h>

#include <bonsai_stdlib/src/costable.h>
#include <bonsai_stdlib/src/arccostable.h>

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
#ifndef BONSAI_NO_AVX
#include <bonsai_stdlib/src/perlin.h>
#endif
#include <bonsai_stdlib/src/simplex.h>
#include <bonsai_stdlib/src/input.h>
#include <bonsai_stdlib/src/mutex.h>
#include <bonsai_stdlib/src/work_queue.h>

#include <bonsai_stdlib/src/mesh.h>
#include <bonsai_stdlib/src/gpu_mapped_buffer.h>
#include <bonsai_stdlib/src/framebuffer.h>
#include <bonsai_stdlib/src/render.h>

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
#include <bonsai_stdlib/src/binary_parser.h>
#include <bonsai_stdlib/src/rect.h>
#include <bonsai_stdlib/src/xml.h>
#include <bonsai_stdlib/src/ui/interactable.h>
#include <bonsai_stdlib/src/ui/ui.h>

#include <bonsai_stdlib/src/c_token.h>
#include <bonsai_stdlib/src/c_parser.h>

#include <bonsai_stdlib/src/debug_ui.h>



#include <bonsai_debug/debug.h>
#include <bonsai_debug/src/api.h>

#define UNPACK_STDLIB(Stdlib) \
  os *Os         = &(Stdlib)->Os; \
  platform *Plat = &(Stdlib)->Plat;


struct bonsai_stdlib
poof(@do_editor_ui)
{
                  os  Os;
            platform  Plat;
     application_api  AppApi;
              opengl  GL;

  // NOTE(Jesse): If we've setup a thread pool we allocate ThreadStates (and
  // set ThreadLocal_ThreadIndex for each thread), but if we just want a
  // single-threaded program (or want to access a thread_local_state before
  // we've initialized stdlib) we don't have to bother with that.. there's just
  // a sentinal here.
  thread_local_state *ThreadStates;
  thread_local_state  DefaultThreadState;

  hot_reloadable_file ShaderHeaderFile;
          ansi_stream ShaderHeaderCode;

  //
  // Debug
  //

  // TODO(Jesse): Move into debug_state?
  texture_block_array AllTextures;
  shader_ptr_block_array AllShaders;

#if BONSAI_DEBUG_SYSTEM_API
  debug_state DebugState;
#else
  // NOTE(Jesse): This is a crutch for the UI .. barf ..
  void *DebugState;
#endif
};

global_variable bonsai_stdlib *Global_Stdlib;

link_internal opengl *
GetGL()
{
  return &Global_Stdlib->GL;
}

link_internal bonsai_stdlib *
GetStdlib()
{
  return Global_Stdlib;
}

link_internal debug_state *
GetDebugState()
{
  debug_state *Result = 0;
#if BONSAI_DEBUG_SYSTEM_API
  bonsai_stdlib *Stdlib = GetStdlib();
  if (Stdlib && Stdlib->DebugState.Initialized)
  {
    Result = &Stdlib->DebugState;
  }
#endif
  return Result;
}

