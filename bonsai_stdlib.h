
#define poof(...)

#include <bonsai_stdlib/headers/poof_functions.h>
#include <bonsai_stdlib/headers/iterators.h>
#include <bonsai_stdlib/headers/assert.h>
#include <bonsai_stdlib/headers/primitives.h>
#include <bonsai_stdlib/headers/console_macros.h>
#include <bonsai_stdlib/headers/globals.h>
#include <bonsai_stdlib/headers/platform.h>
#include <bonsai_stdlib/headers/bit_tricks.h>
#include <bonsai_stdlib/headers/atomic.h>
#include <bonsai_stdlib/headers/thread.h>
#include <bonsai_stdlib/headers/math.h>
#include <bonsai_stdlib/headers/file.h>
#include <bonsai_stdlib/headers/filesystem.h>
#include <bonsai_stdlib/headers/vector.h>
#include <bonsai_stdlib/headers/random.h>
#include <bonsai_stdlib/headers/perlin.h>
#include <bonsai_stdlib/headers/input.h>
#include <bonsai_stdlib/headers/mutex.h>
#include <bonsai_stdlib/headers/work_queue.h>
#include <bonsai_stdlib/headers/memory_arena.h>
#include <bonsai_stdlib/headers/gl.h>
#include <bonsai_stdlib/headers/platform_struct.h>
#include <bonsai_stdlib/headers/heap_allocator.h>
#include <bonsai_stdlib/headers/stream.h>
#include <bonsai_stdlib/headers/counted_string.h>
#include <bonsai_stdlib/headers/console_io.h>
#include <bonsai_stdlib/headers/line.h>
#include <bonsai_stdlib/headers/quaternion.h>
#include <bonsai_stdlib/headers/string_builder.h>
#include <bonsai_stdlib/headers/ansi_stream.h>
#include <bonsai_stdlib/headers/matrix.h>
#include <bonsai_stdlib/headers/rect.h>
#include <bonsai_stdlib/headers/ray.h>
#include <bonsai_stdlib/headers/xml.h>

/* #include <bonsai_debug/headers/debug.h> */


enum shader_uniform_type
{
  ShaderUniform_Undefined,
  ShaderUniform_M4,
  ShaderUniform_V3,
  ShaderUniform_S32,
  ShaderUniform_U32,
  ShaderUniform_R32,
  ShaderUniform_Texture,
  ShaderUniform_Light,
  ShaderUniform_Camera,
};

struct texture;
struct light;
struct camera;

// TODO(Jesse, id: 83, tags: metaprogramming, immediate): d_union-ify this
struct shader_uniform
{
  shader_uniform_type Type;
  union {
    texture *Texture;
    light *Light;
    m4 *M4;
    v3 *V3;
    s32 *S32;
    u32 *U32;
    r32 *R32;
    camera *Camera;
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

struct texture
{
  u32 ID;
  v2i Dim;
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

#include <bonsai_stdlib/headers/2d.h>
#include <bonsai_stdlib/headers/interactable.h>
