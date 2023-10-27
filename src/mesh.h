
struct untextured_3d_geometry_buffer
{
  v3 *Verts;
  v3 *Colors;
  v3 *Normals;

  v2 *TransEmiss;

  // TODO(Jesse): The fuck are these doing as 32bit?!
  u32 End;
  u32 At;

  u64 Timestamp;

  // NOTE(Jesse): This keeps track of what buffer the current reservation buffer came from.
  untextured_3d_geometry_buffer *Parent;
  umm BufferNeedsToGrow;
};


untextured_3d_geometry_buffer
ReserveBufferSpace(untextured_3d_geometry_buffer *Src, u32 ElementsToReserve)
{
  /* TIMED_FUNCTION(); */
  Assert(ElementsToReserve);

  // NOTE(Jesse): During testing I'm going to limit the chain to 1 link.
  // @single_parent_chain_link_untextured_3d
  Assert(Src->Parent == False);

  /* auto Parent = Src; */
  /* while (Src->Parent) Parent = Src->Parent; */

  untextured_3d_geometry_buffer Result = {};

  for (;;)
  {
    umm ReservationAt = Src->At;
    umm ReservationRequest = ReservationAt + ElementsToReserve;
    if (ReservationRequest < Src->End)
    {
      if ( AtomicCompareExchange(&Src->At, (u32)ReservationRequest, (u32)ReservationAt) )
      {
        Result.Verts      = Src->Verts + ReservationAt;
        Result.Colors     = Src->Colors + ReservationAt;
        Result.Normals    = Src->Normals + ReservationAt;
        Result.TransEmiss = Src->TransEmiss + ReservationAt;

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

  return Result;
}

link_internal void
DeepCopy(untextured_3d_geometry_buffer *Src, untextured_3d_geometry_buffer *Dest)
{
  umm Count = Src->At;
  Assert(Dest->End >= Count);

  CopyMemory((u8*)Src->Verts,   (u8*)Dest->Verts,   Count*sizeof(v3));
  CopyMemory((u8*)Src->Colors,  (u8*)Dest->Colors,  Count*sizeof(v3));
  CopyMemory((u8*)Src->Normals, (u8*)Dest->Normals, Count*sizeof(v3));
  CopyMemory((u8*)Src->TransEmiss, (u8*)Dest->TransEmiss, Count*sizeof(v2));

  Dest->At = u32(Count);
  Dest->Timestamp = Src->Timestamp;
}
