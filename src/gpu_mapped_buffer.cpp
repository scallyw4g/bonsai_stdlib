link_internal void
AllocateGpuBuffer( triple_buffered_gpu_mapped_ui_buffer *Buf, data_type Type, u32 ElementCount, memory_arena *Memory)
{
  AllocateGpuBuffer_gpu_mapped_ui_buffer(Buf->Handles+0, Type, ElementCount);
  AllocateGpuBuffer_gpu_mapped_ui_buffer(Buf->Handles+1, Type, ElementCount);
  AllocateGpuBuffer_gpu_mapped_ui_buffer(Buf->Handles+2, Type, ElementCount);

  Buf->Buffer.Verts  = Allocate(v3, Memory, ElementCount);
  Buf->Buffer.UVs    = Allocate(v3, Memory, ElementCount);
  Buf->Buffer.Colors = Allocate(v3, Memory, ElementCount);
  Buf->Buffer.End = ElementCount;
}


// NOTE(Jesse): This hooks up the vertex attribs because in some cases
// (immediate geo buffer) we flush and draw immediately afterwards.
// 
// Should probably move to using VAOs so we don't have to do this.
//
link_inline b32
UnmapGpuBuffer(gpu_mapped_element_buffer *Buf)
{
  return UnmapGpuBuffer(&Buf->Handles);
}

link_inline b32
UnmapGpuBuffer(gpu_mapped_ui_buffer *Buf)
{
#if 0
  return UnmapGpuBuffer(&Buf->Handles);
#else

  Buf->Handles.Mapped = False;
  u32 v3Size   = sizeof(v3)*Buf->Handles.ElementCount;

  AssertNoGlErrors;
  auto GL = GetGL();

  GL->BindBuffer(GL_ARRAY_BUFFER, Buf->Handles.Handles[0]);
  GL->BufferData(GL_ARRAY_BUFFER, v3Size, Buf->Buffer.Verts, GL_STREAM_DRAW);
  AssertNoGlErrors;

  GL->BindBuffer(GL_ARRAY_BUFFER, Buf->Handles.Handles[1]);
  GL->BufferData(GL_ARRAY_BUFFER, v3Size, Buf->Buffer.UVs, GL_STREAM_DRAW);
  AssertNoGlErrors;

  GL->BindBuffer(GL_ARRAY_BUFFER, Buf->Handles.Handles[2]);
  GL->BufferData(GL_ARRAY_BUFFER, v3Size, Buf->Buffer.Colors, GL_STREAM_DRAW);
  AssertNoGlErrors;

  return 1;

#endif
}

link_inline b32
FlushBuffersToCard(gpu_mapped_element_buffer *Buf)
{
  return UnmapGpuBuffer(Buf);
}

link_inline b32
FlushBuffersToCard(gpu_mapped_ui_buffer *Buf)
{
  return UnmapGpuBuffer(Buf);
}


link_internal b32
UnmapGpuBuffer(gpu_element_buffer_handles *Handles)
{
  Assert(Handles->ElementCount);
  Assert(Handles->VAO);
  Assert(Handles->Handles[0]);
  Assert(Handles->Handles[1]);
  Assert(Handles->Handles[2]);

  b32 Result = True;

  auto GL = GetGL();
  GL->BindBuffer(GL_ARRAY_BUFFER, Handles->Handles[0]);
      AssertNoGlErrors;
  Result &= GL->UnmapBuffer(GL_ARRAY_BUFFER);
      AssertNoGlErrors;
  GL->BindBuffer(GL_ARRAY_BUFFER, Handles->Handles[1]);
      AssertNoGlErrors;
  Result &= GL->UnmapBuffer(GL_ARRAY_BUFFER);
      AssertNoGlErrors;
  GL->BindBuffer(GL_ARRAY_BUFFER, Handles->Handles[2]);
      AssertNoGlErrors;
  Result &= GL->UnmapBuffer(GL_ARRAY_BUFFER);
      AssertNoGlErrors;

  Handles->Mapped = False;

  return Result;
}

link_internal void
AllocateGpuBuffer_untextured_3d_geometry_buffer(gpu_element_buffer_handles *Handles, data_type Type, u32 ElementCount)
{
  Assert(ElementCount);
  Assert(Handles->VAO == 0);
  Assert(Handles->Mapped == False);
  Assert(Handles->ElementType == DataType_Undefinded);

  Assert(Handles->Handles[mesh_VertexHandle] == 0);
  Assert(Handles->Handles[mesh_NormalHandle] == 0);
  Assert(Handles->Handles[mesh_MatHandle] == 0);

  Handles->ElementType = Type;
  Handles->ElementCount = ElementCount;


  auto GL = GetGL();

  GL->GenVertexArrays(1, &Handles->VAO);
  GL->BindVertexArray(Handles->VAO);


  GL->GenBuffers(3, &Handles->Handles[mesh_VertexHandle]);
  AssertNoGlErrors;

  u32 matlSize = sizeof(matl)*ElementCount;
  switch (Type)
  {
    InvalidCase(DataType_Undefinded);

    case DataType_v3:
    {
      u32 v3Size   = sizeof(v3)*ElementCount;

      GL->BindBuffer(GL_ARRAY_BUFFER, Handles->Handles[mesh_VertexHandle]);
      GL->BufferData(GL_ARRAY_BUFFER, v3Size, 0, GL_STREAM_DRAW);
      AssertNoGlErrors;
      GL->BindBuffer(GL_ARRAY_BUFFER, Handles->Handles[mesh_NormalHandle]);
      GL->BufferData(GL_ARRAY_BUFFER, v3Size, 0, GL_STREAM_DRAW);
      AssertNoGlErrors;

    } break;

    case DataType_v3_u8:
    {
      u32 v3u8Size   = sizeof(v3_u8)*ElementCount;

      GL->BindBuffer(GL_ARRAY_BUFFER, Handles->Handles[mesh_VertexHandle]);
      GL->BufferData(GL_ARRAY_BUFFER, v3u8Size, 0, GL_STREAM_DRAW);
      AssertNoGlErrors;

      GL->BindBuffer(GL_ARRAY_BUFFER, Handles->Handles[mesh_NormalHandle]);
      GL->BufferData(GL_ARRAY_BUFFER, v3u8Size, 0, GL_STREAM_DRAW);
      AssertNoGlErrors;

    } break;
  }

  GL->BindBuffer(GL_ARRAY_BUFFER, Handles->Handles[mesh_MatHandle]);
  GL->BufferData(GL_ARRAY_BUFFER, matlSize, 0, GL_STREAM_DRAW);
  AssertNoGlErrors;

  SetupVertexAttribsFor_u3d_geo_element_buffer(Handles);

  GL->BindVertexArray(0);
  GL->BindBuffer(GL_ARRAY_BUFFER, 0);

  AssertNoGlErrors;
}


link_internal void
AllocateGpuBuffer_gpu_mapped_element_buffer(gpu_element_buffer_handles *Handles, data_type Type, u32 ElementCount)
{
  AllocateGpuBuffer_untextured_3d_geometry_buffer(Handles, Type, ElementCount);
}

link_internal void
AllocateGpuBuffer_gpu_mapped_ui_buffer(gpu_element_buffer_handles *Handles, data_type Ignored, u32 ElementCount)
{
  Assert(Ignored == DataType_Undefinded);
  Assert(ElementCount);
  Assert(Handles->Mapped == False);
  Assert(Handles->ElementType == DataType_Undefinded);

  Assert(Handles->Handles[mesh_VertexHandle] == 0);
  Assert(Handles->Handles[mesh_NormalHandle] == 0);
  Assert(Handles->Handles[mesh_MatHandle] == 0);

  Handles->ElementCount = ElementCount;

  auto GL = GetGL();

  GL->GenVertexArrays(1, &Handles->VAO);
  GL->BindVertexArray(Handles->VAO);

  GL->GenBuffers(3, &Handles->Handles[mesh_VertexHandle]);
  AssertNoGlErrors;

  u32 v3Size   = sizeof(v3)*ElementCount;
  u32 matlSize = sizeof(matl)*ElementCount;
  {
    u32 AttributeIndex = 0;
    GL->BindBuffer(GL_ARRAY_BUFFER, Handles->Handles[ui_VertexHandle]);
    GL->BufferData(GL_ARRAY_BUFFER, v3Size, 0, GL_DYNAMIC_DRAW);
    GL->EnableVertexAttribArray(AttributeIndex);
    GL->VertexAttribPointer(AttributeIndex, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    AssertNoGlErrors;
  }

  {
    u32 AttributeIndex = 1;
    GL->BindBuffer(GL_ARRAY_BUFFER, Handles->Handles[ui_UVHandle]);
    GL->BufferData(GL_ARRAY_BUFFER, v3Size, 0, GL_DYNAMIC_DRAW);
    GL->EnableVertexAttribArray(AttributeIndex);
    GL->VertexAttribPointer(AttributeIndex, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    AssertNoGlErrors;
  }

  {
    u32 AttributeIndex = 2;
    GL->BindBuffer(GL_ARRAY_BUFFER, Handles->Handles[ui_ColorHandle]);
    GL->BufferData(GL_ARRAY_BUFFER, v3Size, 0, GL_DYNAMIC_DRAW);
    GL->EnableVertexAttribArray(AttributeIndex);
    GL->VertexAttribPointer(AttributeIndex, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    AssertNoGlErrors;
  }

  GL->BindBuffer(GL_ARRAY_BUFFER, 0);
  GL->BindVertexArray(0);
  AssertNoGlErrors;
}

link_internal void
AllocateGpuBuffer(gpu_mapped_element_buffer *GpuMap, data_type Type, u32 ElementCount)
{
  AllocateGpuBuffer_untextured_3d_geometry_buffer(&GpuMap->Handles, Type, ElementCount);
  GpuMap->Buffer.End = ElementCount;
}

link_internal void
AllocateGpuBuffer(gpu_mapped_ui_buffer *GpuMap, data_type Type, u32 ElementCount)
{
  AllocateGpuBuffer_gpu_mapped_ui_buffer(&GpuMap->Handles, Type, ElementCount);
  GpuMap->Buffer.End = ElementCount;
}


link_internal void 
DeallocateGpuBuffer(gpu_element_buffer_handles *Handles)
{
  GetGL()->DeleteBuffers(3, &Handles->Handles[mesh_VertexHandle]);
  Clear(Handles);
}

link_internal void 
DeallocateGpuBuffer(gpu_mapped_element_buffer *Buf)
{
  DeallocateGpuBuffer(&Buf->Handles);
  Clear(&Buf->Buffer);
}

link_internal void 
DeallocateGpuBuffer(gpu_mapped_ui_buffer *Buf)
{
  DeallocateGpuBuffer(&Buf->Handles);
  Clear(&Buf->Buffer);
}


link_internal gpu_mapped_untextured_3d_geometry_buffer
MapGpuBuffer_gpu_mapped_element_buffer(gpu_element_buffer_handles *Handles)
{
  /* WARN_TIMED_FUNCTION(100000); */
  TIMED_FUNCTION();
  AssertNoGlErrors;

  Assert(Handles->Mapped == False);
  Handles->Mapped = True;

  untextured_3d_geometry_buffer Buffer = {};
  Buffer.End = Handles->ElementCount;
  Buffer.Type = Handles->ElementType;

  u32 MaterialBufferSize = sizeof(matl)*Handles->ElementCount;

  switch (Handles->ElementType)
  {
    InvalidCase(DataType_Undefinded);

    case DataType_v3:
    {
      u32 BufferSize = sizeof(v3)*Handles->ElementCount;

      GetGL()->BindBuffer(GL_ARRAY_BUFFER, Handles->Handles[mesh_VertexHandle]);
      AssertNoGlErrors;
      Buffer.Verts = (v3*) GetGL()->MapBufferRange(GL_ARRAY_BUFFER, 0, BufferSize, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
      AssertNoGlErrors;

      GetGL()->BindBuffer(GL_ARRAY_BUFFER, Handles->Handles[mesh_NormalHandle]);
      AssertNoGlErrors;
      Buffer.Normals = (v3*) GetGL()->MapBufferRange(GL_ARRAY_BUFFER, 0, BufferSize, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
      AssertNoGlErrors;

      // Color data
      GetGL()->BindBuffer(GL_ARRAY_BUFFER, Handles->Handles[mesh_MatHandle]);
      AssertNoGlErrors;
      Buffer.Mat = (matl*) GetGL()->MapBufferRange(GL_ARRAY_BUFFER, 0, MaterialBufferSize, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
      AssertNoGlErrors;

    } break;

    case DataType_v3_u8:
    {
      u32 BufferSize = sizeof(v3_u8)*Handles->ElementCount;

      GetGL()->BindBuffer(GL_ARRAY_BUFFER, Handles->Handles[mesh_VertexHandle]);
      AssertNoGlErrors;
      Buffer.Verts = (v3_u8*) GetGL()->MapBufferRange(GL_ARRAY_BUFFER, 0, BufferSize, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
      AssertNoGlErrors;

      GetGL()->BindBuffer(GL_ARRAY_BUFFER, Handles->Handles[mesh_NormalHandle]);
      AssertNoGlErrors;
      Buffer.Normals = (v3_u8*) GetGL()->MapBufferRange(GL_ARRAY_BUFFER, 0, BufferSize, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
      AssertNoGlErrors;

      // Color data
      GetGL()->BindBuffer(GL_ARRAY_BUFFER, Handles->Handles[mesh_MatHandle]);
      AssertNoGlErrors;
      Buffer.Mat = (matl*) GetGL()->MapBufferRange(GL_ARRAY_BUFFER, 0, MaterialBufferSize, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
      AssertNoGlErrors;

    } break;

  }

  if (!Buffer.Verts)   { Error("Allocating gpu_mapped_element_buffer::Verts");   }
  if (!Buffer.Normals) { Error("Allocating gpu_mapped_element_buffer::Normals"); }
  if (!Buffer.Mat)     { Error("Allocating gpu_mapped_element_buffer::Mat");     }

  GetGL()->BindBuffer(GL_ARRAY_BUFFER, 0);

  return {*Handles, Buffer};
}

link_internal void
MapGpuBuffer(gpu_mapped_ui_buffer *Buf)
{
  TIMED_FUNCTION();
  AssertNoGlErrors;

  auto Buffer = &Buf->Buffer;
  gpu_element_buffer_handles *Handles = &Buf->Handles;

  Assert(Handles->Mapped == False);
  Handles->Mapped = True;

  Buffer->End = Handles->ElementCount;
  Buffer->At = 0;

  u32 v3Size = sizeof(v3)*Handles->ElementCount;
  /* u32 v2Size = sizeof(v2)*Handles->ElementCount; */

#if 1
#if 0
  if (Buffer->Verts == 0)
  {
    if (Handles->Arena == 0) { Handles->Arena = AllocateArena(); }
    Buffer->Verts  = Allocate(v3, Handles->Arena, Handles->ElementCount);
    Buffer->UVs    = Allocate(v3, Handles->Arena, Handles->ElementCount);
    Buffer->Colors = Allocate(v3, Handles->Arena, Handles->ElementCount);
  }
#endif
#else
  auto GL = GetGL();

  {
    TIMED_NAMED_BLOCK(BindBuffer);
    GetGL()->BindBuffer(GL_ARRAY_BUFFER, Handles->Handles[ui_VertexHandle]);
    AssertNoGlErrors;
    /* Info("(%d)(%d)(%d)", Handles->Handles[0], Handles->Handles[1], Handles->Handles[2] ); */
  }

  {
    TIMED_NAMED_BLOCK(BufferData);
    GL->BufferData(GL_ARRAY_BUFFER, v3Size, 0, GL_STREAM_DRAW);
    AssertNoGlErrors;
  }

  {
    TIMED_NAMED_BLOCK(MapBufferRange);
    Buffer.Verts = (v3*) GetGL()->MapBufferRange(GL_ARRAY_BUFFER, 0, v3Size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT  | GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_UNSYNCHRONIZED_BIT );
    AssertNoGlErrors;
  }


  GetGL()->BindBuffer(GL_ARRAY_BUFFER, Handles->Handles[ui_UVHandle]);
  AssertNoGlErrors;
  GL->BufferData(GL_ARRAY_BUFFER, v3Size, 0, GL_STREAM_DRAW);
  AssertNoGlErrors;
  Buffer.UVs = (v3*) GetGL()->MapBufferRange(GL_ARRAY_BUFFER, 0, v3Size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT );
  AssertNoGlErrors;

  GetGL()->BindBuffer(GL_ARRAY_BUFFER, Handles->Handles[ui_ColorHandle]);
  AssertNoGlErrors;
  GL->BufferData(GL_ARRAY_BUFFER, v3Size, 0, GL_STREAM_DRAW);
  AssertNoGlErrors;
  Buffer.Colors = (v3*) GetGL()->MapBufferRange(GL_ARRAY_BUFFER, 0, v3Size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
  AssertNoGlErrors;

  if (!Buffer.Verts)   { Error("Allocating gpu_mapped_element_buffer::Verts");   }
  if (!Buffer.UVs) { Error("Allocating gpu_mapped_element_buffer::UVs"); }
  if (!Buffer.Colors)     { Error("Allocating gpu_mapped_element_buffer::Colors");     }

  GetGL()->BindBuffer(GL_ARRAY_BUFFER, 0);
#endif
}

link_internal void
MapGpuBuffer(gpu_mapped_element_buffer *GpuMap)
{
  /* WARN_TIMED_FUNCTION(100000); */
  TIMED_FUNCTION();

  Assert(GpuMap->Buffer.Parent == False);
  if (GpuMap->Buffer.BufferNeedsToGrow)
  {
    GpuMap->Buffer.End += GpuMap->Buffer.BufferNeedsToGrow;
    DeallocateGpuBuffer(GpuMap);
    AllocateGpuBuffer(GpuMap, GpuMap->Handles.ElementType, GpuMap->Buffer.End);
    GpuMap->Buffer.BufferNeedsToGrow = 0;
  }

  GpuMap->Buffer = MapGpuBuffer_gpu_mapped_element_buffer(&GpuMap->Handles).Buffer;
}

/* link_internal void */
/* MapGpuBuffer(gpu_mapped_ui_buffer *GpuMap) */
/* { */
/*   /1* WARN_TIMED_FUNCTION(100000); *1/ */
/*   TIMED_FUNCTION(); */
/*   GpuMap->Buffer = MapGpuBuffer_gpu_mapped_ui_buffer(&GpuMap->Handles).Buffer; */
/* } */

link_internal void
SetupVertexAttribsFor_u3d_geo_element_buffer(gpu_element_buffer_handles *Handles)
{
  TIMED_FUNCTION();

  AssertNoGlErrors;
  GetGL()->EnableVertexAttribArray(VERTEX_POSITION_LAYOUT_LOCATION);
  GetGL()->EnableVertexAttribArray(VERTEX_NORMAL_LAYOUT_LOCATION);
  GetGL()->EnableVertexAttribArray(VERTEX_COLOR_LAYOUT_LOCATION);
  GetGL()->EnableVertexAttribArray(VERTEX_TRANS_EMISS_LAYOUT_LOCATION);
  AssertNoGlErrors;


  switch(Handles->ElementType)
  {
    InvalidCase(DataType_Undefinded);
    case DataType_v3:
    {
      GetGL()->BindBuffer(GL_ARRAY_BUFFER, Handles->Handles[mesh_VertexHandle]);
      GetGL()->VertexAttribPointer(VERTEX_POSITION_LAYOUT_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
      AssertNoGlErrors;

      GetGL()->BindBuffer(GL_ARRAY_BUFFER, Handles->Handles[mesh_NormalHandle]);
      GetGL()->VertexAttribPointer(VERTEX_NORMAL_LAYOUT_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
      AssertNoGlErrors;
    } break;

    case DataType_v3_u8:
    {
      GetGL()->BindBuffer(GL_ARRAY_BUFFER, Handles->Handles[mesh_VertexHandle]);
      GetGL()->VertexAttribPointer(VERTEX_POSITION_LAYOUT_LOCATION, 3, GL_BYTE, GL_FALSE, 0, (void*)0);
      AssertNoGlErrors;

      GetGL()->BindBuffer(GL_ARRAY_BUFFER, Handles->Handles[mesh_NormalHandle]);
      GetGL()->VertexAttribPointer(VERTEX_NORMAL_LAYOUT_LOCATION, 3, GL_BYTE, GL_TRUE, 0, (void*)0);
      AssertNoGlErrors;
    } break;
  }


  // NOTE(Jesse): This is just here to break when the size of these changes,
  // serving as a reminder to update this code.
  const u32 MtlFloatElements = sizeof(matl)/sizeof(u8);
  CAssert(MtlFloatElements == 4);

  GetGL()->BindBuffer(GL_ARRAY_BUFFER, Handles->Handles[mesh_MatHandle]);
  /* GetGL()->VertexAttribIPointer(VERTEX_COLOR_LAYOUT_LOCATION, 1, GL_UNSIGNED_INT, 0, 0); */
  GetGL()->VertexAttribIPointer(VERTEX_COLOR_LAYOUT_LOCATION, 1, GL_SHORT, sizeof(matl), Cast(void*, OffsetOf(ColorIndex, matl)));
  GetGL()->VertexAttribIPointer(VERTEX_TRANS_EMISS_LAYOUT_LOCATION, 2, GL_BYTE, sizeof(matl), Cast(void*, OffsetOf(Transparency, matl)) ); // @vertex_attrib_I_pointer_transparency_offsetof
  AssertNoGlErrors;
}

