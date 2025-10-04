struct vertex_material
{
  u16 ColorIndex;

  // NOTE(Jesse): Must come before Emission because the VertexAttribIPointer is set up with this members' offset
  // @vertex_attrib_I_pointer_transparency_offsetof
  u8 Transparency;
  u8 Emission;
};
CAssert(sizeof(vertex_material) == 4);
/* CAssert(OffsetOf(ColorIndex, vertex_material) == 0); */
/* CAssert(OffsetOf(Transparency, vertex_material) == 2); */
/* CAssert(OffsetOf(Emission, vertex_material) == 3); */

// NOTE(Jesse): Must match defines in header.glsl
#define RENDERER_MAX_LIGHT_EMISSION_VALUE (5.f)

link_internal vertex_material
VertexMaterial( u16 ColorIndex , f32 Transparency , f32 Emission  )
{
  Transparency = Clamp01(Transparency);
  Emission = Clamp(0.f, Emission, RENDERER_MAX_LIGHT_EMISSION_VALUE);

  vertex_material Reuslt = {
    .ColorIndex = ColorIndex,
    .Transparency = u8(Transparency*255.f),
    .Emission = u8(Emission*(255.f/RENDERER_MAX_LIGHT_EMISSION_VALUE))
  };
  return Reuslt;
}


typedef vertex_material matl;

poof(gen_constructor(vertex_material))
#include <generated/gen_constructor_vertex_material.h>

inline void
FillArray(vertex_material Color, vertex_material *Dest, s32 Count)
{
  for (s32 Index = 0; Index < Count; ++Index) { Dest[Index] = Color; }
}

enum data_type
{
  DataType_Undefinded = 0,
  /* DataType_f32        = 1, */
  DataType_v3         = 2,
  DataType_v3_u8      = 3,
};

global_variable
u32 DataTypeToElementSize[] = {
  0,  // DataType_Undefinded,
  4,  // DataType_f32,
  12, // DataType_v3,
  3,  // DataType_v3_u8,
};


#include <bonsai_stdlib/src/geometry_buffer.h>

struct geometry_buffer_stub_v3
{
               v3 *Verts;
               v3 *Normals;
  vertex_material *Mat;
};

struct geometry_buffer_stub_v3_u8
{
            v3_u8 *Verts;
            v3_u8 *Normals;
  vertex_material *Mat;
};

struct untextured_3d_geometry_buffer
{
  data_type Type;

  void *Next; poof(@ui_skip @no_serialize)

             void *Verts;
             void *Normals;
  vertex_material *Mat;

  // NOTE(Jesse): We're never going to have more than 4bln vertices, so these
  // can be 32 bits.  They can realistaiclly probably be 24 bits, but that's
  // definitely a waste of time right now.
  u32 End;
  u32 At;

  // NOTE(Jesse): This keeps track of what buffer the current reservation buffer came from.
  untextured_3d_geometry_buffer *Parent; poof(@no_serialize)
  u32 BufferNeedsToGrow;                 poof(@no_serialize)
  u64 Timestamp;                         poof(@no_serialize)
};

typedef untextured_3d_geometry_buffer  geo_u3d;
typedef untextured_3d_geometry_buffer* geo_u3d_ptr;


link_internal geometry_buffer_stub_v3_u8
GetBufferStub_v3_u8(untextured_3d_geometry_buffer *Dest)
{
  Assert(Dest->Type == DataType_v3_u8);
  geometry_buffer_stub_v3_u8 Result = {
    .Verts = Cast(v3_u8*, Dest->Verts),
    .Normals = Cast(v3_u8*, Dest->Normals),
    .Mat  = Dest->Mat,
  };
  return Result;
}

link_internal geometry_buffer_stub_v3
GetBufferStub_v3(untextured_3d_geometry_buffer *Dest)
{
  Assert(Dest->Type == DataType_v3);
  geometry_buffer_stub_v3 Result = {
    .Verts = Cast(v3*, Dest->Verts),
    .Normals = Cast(v3*, Dest->Normals),
    .Mat  = Dest->Mat,
  };
  return Result;
}


untextured_3d_geometry_buffer
ReserveBufferSpace(untextured_3d_geometry_buffer *Src, u32 ElementsToReserve)
{
  untextured_3d_geometry_buffer Result = {};
  /* TIMED_FUNCTION(); */
  if (ElementsToReserve)
  {
    // NOTE(Jesse): During testing I'm going to limit the chain to 1 link.
    // @single_parent_chain_link_untextured_3d
    Assert(Src->Parent == False);

    /* auto Parent = Src; */
    /* while (Src->Parent) Parent = Src->Parent; */


    for (;;)
    {
      umm ReservationAt = Src->At;
      umm ReservationRequest = ReservationAt + ElementsToReserve;
      if (ReservationRequest < Src->End)
      {
        if ( AtomicCompareExchange(&Src->At, (u32)ReservationRequest, (u32)ReservationAt) )
        {
          // TODO(Jesse): Make this switch
          switch(Src->Type)
          {
            InvalidCase(DataType_Undefinded);
            case DataType_v3:
            {
              Result.Verts   = Cast(void*, (Cast(v3*, Src->Verts) + ReservationAt));
              Result.Normals = Cast(void*, (Cast(v3*, Src->Normals) + ReservationAt));
            } break;
            case DataType_v3_u8:
            {
              Result.Verts   = Cast(void*, (Cast(v3_u8*, Src->Verts) + ReservationAt));
              Result.Normals = Cast(void*, (Cast(v3_u8*, Src->Normals) + ReservationAt));
            } break;
          }

          Result.Mat = Src->Mat + ReservationAt;
          Result.End = ElementsToReserve;
          Result.Type = Src->Type;

          Result.Parent = Src;

          break;
        }
        else
        {
          continue;
        }
      }
      else
      {
        Warn("Failed to reserve buffer space");
        Src->BufferNeedsToGrow += ElementsToReserve;
        break;
      }
    }
  }

  return Result;
}

#if 0
link_internal void
DeepCopy(world_chunk_geometry_buffer *Src, untextured_3d_geometry_buffer *Dest)
{
  NotImplemented;
}

link_internal void
DeepCopy(world_chunk_geometry_buffer *Src, world_chunk_geometry_buffer *Dest)
{
  umm Count = Src->At;
  Assert(Dest->End >= Count);

  CopyMemory((u8*)Src->Verts,   (u8*)Dest->Verts,   Count*sizeof(v3_u8));
  CopyMemory((u8*)Src->Normals, (u8*)Dest->Normals, Count*sizeof(v3_u8));
  CopyMemory((u8*)Src->Mat,     (u8*)Dest->Mat,     Count*sizeof(vertex_material));
  /* CopyMemory((u8*)Src->Colors,  (u8*)Dest->Colors,  Count*sizeof(v3)); */
  /* CopyMemory((u8*)Src->TransEmiss, (u8*)Dest->TransEmiss, Count*sizeof(v2)); */

  Dest->At = u32(Count);
  Dest->Timestamp = Src->Timestamp;
}
#endif

link_internal void
DeepCopy(untextured_3d_geometry_buffer *Src, untextured_3d_geometry_buffer *Dest)
{
  umm Count = Src->At;
  Assert(Dest->End >= Count);

  switch (Src->Type)
  {
    InvalidCase(DataType_Undefinded);

    case DataType_v3:
    {
      CopyMemory((u8*)Src->Verts,   (u8*)Dest->Verts,   Count*sizeof(v3));
      CopyMemory((u8*)Src->Normals, (u8*)Dest->Normals, Count*sizeof(v3));
      CopyMemory((u8*)Src->Mat,     (u8*)Dest->Mat,     Count*sizeof(vertex_material));
    } break;

    case DataType_v3_u8:
    {
      CopyMemory((u8*)Src->Verts,   (u8*)Dest->Verts,   Count*sizeof(v3_u8));
      CopyMemory((u8*)Src->Normals, (u8*)Dest->Normals, Count*sizeof(v3_u8));
      CopyMemory((u8*)Src->Mat,     (u8*)Dest->Mat,     Count*sizeof(vertex_material));
    } break;
  }

  /* CopyMemory((u8*)Src->Colors,  (u8*)Dest->Colors,  Count*sizeof(v3)); */
  /* CopyMemory((u8*)Src->TransEmiss, (u8*)Dest->TransEmiss, Count*sizeof(v2)); */

  Dest->At = u32(Count);
  Dest->Timestamp = Src->Timestamp;
  Dest->Type = Src->Type;
}
