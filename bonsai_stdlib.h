
#define poof(...)

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
#include <bonsai_stdlib/src/math.h>
#include <bonsai_stdlib/src/file.h>
#include <bonsai_stdlib/src/filesystem.h>
#include <bonsai_stdlib/src/vector.h>
#include <bonsai_stdlib/src/matrix.h>
#include <bonsai_stdlib/src/colors.h>
#include <bonsai_stdlib/src/texture.h>
#include <bonsai_stdlib/src/shader.h>
#include <bonsai_stdlib/src/random.h>
#include <bonsai_stdlib/src/noise.h>
#include <bonsai_stdlib/src/perlin.h>
#include <bonsai_stdlib/src/input.h>
#include <bonsai_stdlib/src/mutex.h>
#include <bonsai_stdlib/src/work_queue.h>
#include <bonsai_stdlib/src/memory_arena.h>
#include <bonsai_stdlib/src/mesh.h>

#include <bonsai_stdlib/src/gl.h>
#include <bonsai_stdlib/src/ui/gl.h>

#include <bonsai_stdlib/src/platform_struct.h>
#include <bonsai_stdlib/src/heap_allocator.h>
#include <bonsai_stdlib/src/stream.h>
#include <bonsai_stdlib/src/counted_string.h>
#include <bonsai_stdlib/src/console_io.h>
#include <bonsai_stdlib/src/line.h>
#include <bonsai_stdlib/src/quaternion.h>
#include <bonsai_stdlib/src/string_builder.h>
#include <bonsai_stdlib/src/ansi_stream.h>
#include <bonsai_stdlib/src/rect.h>
#include <bonsai_stdlib/src/ray.h>
#include <bonsai_stdlib/src/xml.h>


// nocheckin Move this stuff to respective places they should live.
// 3d stuff is supposed to live in the engine

struct light;
struct camera;

struct textured_2d_geometry_buffer
{
  v3 *Verts;
  v3 *Colors;
  v3 *UVs;

  u32 End;
  u32 At;
};

struct untextured_2d_geometry_buffer
{
  v3 *Verts;
  v3 *Colors;

  u32 End;
  u32 At;
};

/* poof( */
/*   func compound_buffer(type_poof_symbol TypeName, type_poof_symbol Types ) */
/*   { */
/*     struct TypeName */
/*     { */
/*       Types */
      /* u32 At; */
      /* u32 End; */
    /* }; */
  /* } */
/* ) */
/* poof( compound_buffer( { untextured_3d_geometry_buffer }, { v3 *Verts; v4 *Colors; v3 *Normals; u64 Timestamp; })) */

#include <bonsai_stdlib/src/ui/interactable.h>
#include <bonsai_stdlib/src/ui/ui.h>
