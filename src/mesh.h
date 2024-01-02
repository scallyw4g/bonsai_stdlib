struct vertex_material
{
  v3 Color;
  f32 Transparency;
  f32 Emission;
};

typedef vertex_material matl;

poof(gen_constructor(vertex_material))
#include <generated/gen_constructor_vertex_material.h>

inline void
FillArray(vertex_material Color, vertex_material *Dest, s32 Count)
{
  for (s32 Index = 0; Index < Count; ++Index) { Dest[Index] = Color; }
}

struct untextured_3d_geometry_buffer
{
  v3 *Verts;
  v3 *Normals;
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

typedef untextured_3d_geometry_buffer geo_u3d;
typedef untextured_3d_geometry_buffer* geo_u3d_ptr;


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
          Result.Verts   = Src->Verts + ReservationAt;
          Result.Normals = Src->Normals + ReservationAt;
          Result.Mat     = Src->Mat + ReservationAt;

          Result.End = ElementsToReserve;

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

link_internal void
DeepCopy(untextured_3d_geometry_buffer *Src, untextured_3d_geometry_buffer *Dest)
{
  umm Count = Src->At;
  Assert(Dest->End >= Count);

  CopyMemory((u8*)Src->Verts,   (u8*)Dest->Verts,   Count*sizeof(v3));
  CopyMemory((u8*)Src->Normals, (u8*)Dest->Normals, Count*sizeof(v3));
  CopyMemory((u8*)Src->Mat,     (u8*)Dest->Mat,     Count*sizeof(vertex_material));
  /* CopyMemory((u8*)Src->Colors,  (u8*)Dest->Colors,  Count*sizeof(v3)); */
  /* CopyMemory((u8*)Src->TransEmiss, (u8*)Dest->TransEmiss, Count*sizeof(v2)); */

  Dest->At = u32(Count);
  Dest->Timestamp = Src->Timestamp;
}
