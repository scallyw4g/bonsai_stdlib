
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
#include <bonsai_stdlib/src/gl.h>
#include <bonsai_stdlib/src/platform_struct.h>
#include <bonsai_stdlib/src/heap_allocator.h>
#include <bonsai_stdlib/src/stream.h>
#include <bonsai_stdlib/src/counted_string.h>
#include <bonsai_stdlib/src/console_io.h>
#include <bonsai_stdlib/src/line.h>
#include <bonsai_stdlib/src/quaternion.h>
#include <bonsai_stdlib/src/string_builder.h>
#include <bonsai_stdlib/src/ansi_stream.h>
#include <bonsai_stdlib/src/matrix.h>
#include <bonsai_stdlib/src/rect.h>
#include <bonsai_stdlib/src/ray.h>
#include <bonsai_stdlib/src/xml.h>

/* #include <bonsai_debug/src/debug.h> */


struct light;
struct camera;

// TODO(Jesse, id: 83, tags: metaprogramming, immediate): d_union-ify this
struct shader_uniform
{
  shader_uniform_type Type;
  union {
    texture *Texture;
    m4 *M4;
    v3 *V3;
    s32 *S32;
    u32 *U32;
    r32 *R32;
    camera *Camera;
    light *Light;
    void *Data;
  };

  s32 ID;
  const char *Name;
  shader_uniform *Next;
};

struct shader
{
  u32 ID;
  shader_uniform *FirstUniform;
};

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

struct untextured_3d_geometry_buffer
{
  v3 *Verts;
  v4 *Colors;
  v3 *Normals;

  // TODO(Jesse): The fuck are these doing as 32bit?!
  u32 End;
  u32 At;

  u64 Timestamp;
};

link_internal void
DeepCopy(untextured_3d_geometry_buffer *Src, untextured_3d_geometry_buffer *Dest)
{
  umm Count = Src->At;
  Assert(Dest->End >= Count);

  CopyMemory((u8*)Src->Verts,   (u8*)Dest->Verts,   Count*sizeof(v3));
  CopyMemory((u8*)Src->Colors,  (u8*)Dest->Colors,  Count*sizeof(v4));
  CopyMemory((u8*)Src->Normals, (u8*)Dest->Normals, Count*sizeof(v3));

  Dest->At = u32(Count);
  Dest->Timestamp = Src->Timestamp;
}

struct gpu_mapped_element_buffer
{
  u32 VertexHandle;
  u32 NormalHandle;
  u32 ColorHandle;

  untextured_3d_geometry_buffer Buffer;
};

struct framebuffer
{
  u32 ID;
  u32 Attachments;
};

struct render_entity_to_texture_group
{
  // For the GameGeo
  camera *Camera;
  framebuffer GameGeoFBO;
  shader GameGeoShader;
  m4 ViewProjection;
  gpu_mapped_element_buffer GameGeo;
  /* shader DebugGameGeoTextureShader; */
};

#include <bonsai_stdlib/src/2d.h>
#include <bonsai_stdlib/src/interactable.h>
