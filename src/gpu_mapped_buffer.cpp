
// NOTE(Jesse): This hooks up the vertex attribs because in some cases
// (immediate geo buffer) we flush and draw immediately afterwards.
// 
// Should probably move to using VAOs so we don't have to do this.
//
link_inline b32
FlushBuffersToCard_gpu_mapped_element_buffer(gpu_element_buffer_handles *Handles)
{
#if 1
  return UnmapGpuBuffer(Handles);
#else
  TIMED_FUNCTION();

  auto GL = GetGL();
  GL->BindVertexArray(Handles->VAO);

  Assert(Handles->VAO);
  Assert(Handles->Mapped == True);
  Handles->Mapped = False;

  AssertNoGlErrors;

  AssertNoGlErrors;

  GL->EnableVertexAttribArray(0);
  GL->EnableVertexAttribArray(1);
  GL->EnableVertexAttribArray(2);
  GL->EnableVertexAttribArray(3);

  u32 BufferUnmapped = 0;
  switch (Handles->ElementType)
  {
    InvalidCase(DataType_Undefinded);

    case DataType_v3:
    {
      GetGL()->BindBuffer(GL_ARRAY_BUFFER, Handles->Handles[mesh_VertexHandle]);
      GetGL()->VertexAttribPointer(VERTEX_POSITION_LAYOUT_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
      BufferUnmapped = GetGL()->UnmapBuffer(GL_ARRAY_BUFFER);
      AssertNoGlErrors;

      GetGL()->BindBuffer(GL_ARRAY_BUFFER, Handles->Handles[mesh_NormalHandle]);
      GetGL()->VertexAttribPointer(VERTEX_NORMAL_LAYOUT_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
      BufferUnmapped &= GetGL()->UnmapBuffer(GL_ARRAY_BUFFER);
      AssertNoGlErrors;
    } break;

    case DataType_v3_u8:
    {
      GetGL()->BindBuffer(GL_ARRAY_BUFFER, Handles->Handles[mesh_VertexHandle]);
      GetGL()->VertexAttribIPointer(VERTEX_POSITION_LAYOUT_LOCATION, 3, GL_BYTE, 0, (void*)0);
      BufferUnmapped = GetGL()->UnmapBuffer(GL_ARRAY_BUFFER);
      AssertNoGlErrors;

      GetGL()->BindBuffer(GL_ARRAY_BUFFER, Handles->Handles[mesh_NormalHandle]);
      GetGL()->VertexAttribPointer(VERTEX_NORMAL_LAYOUT_LOCATION, 3, GL_BYTE, GL_TRUE, 0, (void*)0);
      BufferUnmapped &= GetGL()->UnmapBuffer(GL_ARRAY_BUFFER);
      AssertNoGlErrors;

    } break;
  }

  // NOTE(Jesse): This is just here to break when the size of these changes,
  // serving as a reminder to update this code.
  const u32 MtlFloatElements = sizeof(matl)/sizeof(u8);
  CAssert(MtlFloatElements == 4);

  GetGL()->BindBuffer(GL_ARRAY_BUFFER, Handles->Handles[mesh_MatHandle]);
  GetGL()->VertexAttribIPointer(VERTEX_COLOR_LAYOUT_LOCATION,       1, GL_SHORT, sizeof(matl), Cast(void*, OffsetOf(  ColorIndex, matl)) );
  GetGL()->VertexAttribIPointer(VERTEX_TRANS_EMISS_LAYOUT_LOCATION, 2, GL_BYTE,  sizeof(matl), Cast(void*, OffsetOf(Transparency, matl)) ); // @vertex_attrib_I_pointer_transparency_offsetof
  BufferUnmapped &= GetGL()->UnmapBuffer(GL_ARRAY_BUFFER);
  AssertNoGlErrors;

  if (BufferUnmapped == False) { Error("glUnmapBuffer Failed"); }
  return BufferUnmapped;
#endif
}

link_inline b32
FlushBuffersToCard_gpu_mapped_ui_buffer(gpu_element_buffer_handles *Handles)
{
  return UnmapGpuBuffer(Handles);
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

  /* Handles->ElementType = Type; */
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
  WARN_TIMED_FUNCTION(100000);
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
      Buffer.Verts = (v3*) GetGL()->MapBufferRange(GL_ARRAY_BUFFER, 0, BufferSize, GL_MAP_WRITE_BIT);
      AssertNoGlErrors;

      GetGL()->BindBuffer(GL_ARRAY_BUFFER, Handles->Handles[mesh_NormalHandle]);
      AssertNoGlErrors;
      Buffer.Normals = (v3*) GetGL()->MapBufferRange(GL_ARRAY_BUFFER, 0, BufferSize, GL_MAP_WRITE_BIT);
      AssertNoGlErrors;

      // Color data
      GetGL()->BindBuffer(GL_ARRAY_BUFFER, Handles->Handles[mesh_MatHandle]);
      AssertNoGlErrors;
      Buffer.Mat = (matl*) GetGL()->MapBufferRange(GL_ARRAY_BUFFER, 0, MaterialBufferSize, GL_MAP_WRITE_BIT);
      AssertNoGlErrors;

    } break;

    case DataType_v3_u8:
    {
      u32 BufferSize = sizeof(v3_u8)*Handles->ElementCount;

      GetGL()->BindBuffer(GL_ARRAY_BUFFER, Handles->Handles[mesh_VertexHandle]);
      AssertNoGlErrors;
      Buffer.Verts = (v3_u8*) GetGL()->MapBufferRange(GL_ARRAY_BUFFER, 0, BufferSize, GL_MAP_WRITE_BIT);
      AssertNoGlErrors;

      GetGL()->BindBuffer(GL_ARRAY_BUFFER, Handles->Handles[mesh_NormalHandle]);
      AssertNoGlErrors;
      Buffer.Normals = (v3_u8*) GetGL()->MapBufferRange(GL_ARRAY_BUFFER, 0, BufferSize, GL_MAP_WRITE_BIT);
      AssertNoGlErrors;

      // Color data
      GetGL()->BindBuffer(GL_ARRAY_BUFFER, Handles->Handles[mesh_MatHandle]);
      AssertNoGlErrors;
      Buffer.Mat = (matl*) GetGL()->MapBufferRange(GL_ARRAY_BUFFER, 0, MaterialBufferSize, GL_MAP_WRITE_BIT);
      AssertNoGlErrors;

    } break;

  }

  if (!Buffer.Verts)   { Error("Allocating gpu_mapped_element_buffer::Verts");   }
  if (!Buffer.Normals) { Error("Allocating gpu_mapped_element_buffer::Normals"); }
  if (!Buffer.Mat)     { Error("Allocating gpu_mapped_element_buffer::Mat");     }

  GetGL()->BindBuffer(GL_ARRAY_BUFFER, 0);

  return {*Handles, Buffer};
}

link_internal gpu_mapped_ui_buffer
MapGpuBuffer_gpu_mapped_ui_buffer(gpu_element_buffer_handles *Handles)
{
  TIMED_FUNCTION();
  AssertNoGlErrors;

  Assert(Handles->Mapped == False);
  Handles->Mapped = True;

  ui_geometry_buffer Buffer = {};
  Buffer.End = Handles->ElementCount;

  u32 v3Size = sizeof(v3)*Handles->ElementCount;
  u32 v2Size = sizeof(v2)*Handles->ElementCount;

  GetGL()->BindBuffer(GL_ARRAY_BUFFER, Handles->Handles[ui_VertexHandle]);
  AssertNoGlErrors;
  Buffer.Verts = (v3*) GetGL()->MapBufferRange(GL_ARRAY_BUFFER, 0, v3Size, GL_MAP_WRITE_BIT);
  AssertNoGlErrors;

  GetGL()->BindBuffer(GL_ARRAY_BUFFER, Handles->Handles[ui_UVHandle]);
  AssertNoGlErrors;
  Buffer.UVs = (v3*) GetGL()->MapBufferRange(GL_ARRAY_BUFFER, 0, v2Size, GL_MAP_WRITE_BIT);
  AssertNoGlErrors;

  GetGL()->BindBuffer(GL_ARRAY_BUFFER, Handles->Handles[ui_ColorHandle]);
  AssertNoGlErrors;
  Buffer.Colors = (v3*) GetGL()->MapBufferRange(GL_ARRAY_BUFFER, 0, v3Size, GL_MAP_WRITE_BIT);
  AssertNoGlErrors;

  if (!Buffer.Verts)   { Error("Allocating gpu_mapped_element_buffer::Verts");   }
  if (!Buffer.UVs) { Error("Allocating gpu_mapped_element_buffer::UVs"); }
  if (!Buffer.Colors)     { Error("Allocating gpu_mapped_element_buffer::Colors");     }

  GetGL()->BindBuffer(GL_ARRAY_BUFFER, 0);

  return {*Handles, Buffer};
}

link_internal void
MapGpuBuffer(gpu_mapped_element_buffer *GpuMap)
{
  WARN_TIMED_FUNCTION(100000);

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

link_internal void
MapGpuBuffer(gpu_mapped_ui_buffer *GpuMap)
{
  WARN_TIMED_FUNCTION(100000);
  GpuMap->Buffer = MapGpuBuffer_gpu_mapped_ui_buffer(&GpuMap->Handles).Buffer;
}

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

