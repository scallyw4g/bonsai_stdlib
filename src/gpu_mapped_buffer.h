poof(
  func triple_buffered(type, buffer_t)
  {
    @var new_t triple_buffered_(type.name)
    @var buf_count 3

    struct new_t
    {
      u32 CurrentIndex;
      gpu_element_buffer_handles  Handles[buf_count];
                 (buffer_t.name)  Buffer;
    };

    link_internal gpu_element_buffer_handles *
    CurrentHandles( new_t *Buf )
    {
      Assert(Buf->CurrentIndex < buf_count);
      gpu_element_buffer_handles *Result = Buf->Handles + Buf->CurrentIndex;
      return Result;
    }

    link_internal void
    MapGpuBuffer( new_t *Buf )
    {
      Buf->CurrentIndex = (Buf->CurrentIndex + 1) % buf_count;
      Buf->Buffer = MapGpuBuffer_(type.name)(CurrentHandles(Buf)).Buffer;
    }

    link_internal void
    DrawBuffer( new_t *Buf, v2 *ScreenDim )
    {
      auto Handles = CurrentHandles(Buf);
      // TODO(Jesse): Probably smooth this out ..
      type.name SyntheticBuffer = {
        *Handles,
        Buf->Buffer,
      };

      DrawBuffer(&SyntheticBuffer, ScreenDim);

      *Handles  = SyntheticBuffer.Handles;
    }

    link_internal void
    AllocateGpuBuffer( new_t *Buf, data_type Type, u32 ElementCount )
    {
      AllocateGpuBuffer_(type.name)(Buf->Handles+0, Type, ElementCount);
      AllocateGpuBuffer_(type.name)(Buf->Handles+1, Type, ElementCount);
      AllocateGpuBuffer_(type.name)(Buf->Handles+2, Type, ElementCount);
      Buf->Buffer.End = ElementCount;
    }
  }
)



enum gpu_element_buffer_flags
{
};


enum mesh_element_buffer_handles
{
  mesh_VertexHandle,
  mesh_NormalHandle,
  mesh_MatHandle,
};

enum ui_element_buffer_handles
{
  ui_VertexHandle,
  ui_UVHandle,
  ui_ColorHandle,
};

struct gpu_element_buffer_handles
{
  u32 VAO;

  u32 Handles[3];

  u32 ElementCount;
  data_type ElementType;

  b8  Mapped;
  b8  Pad;
  u16 Flags; // (gpu_element_buffer_flags)
};

//{ This is a buffer we ask for and directly copy into
  struct gpu_mapped_element_buffer
  {
    gpu_element_buffer_handles    Handles;
    untextured_3d_geometry_buffer Buffer;
  };
  // TODO(Jesse): Remove this typedef by changing the name everywhere.
  typedef gpu_mapped_element_buffer gpu_mapped_untextured_3d_geometry_buffer;
// }

struct gpu_mapped_ui_buffer
{
  gpu_element_buffer_handles Handles;
  ui_geometry_buffer         Buffer;
};



link_internal void AllocateGpuBuffer_gpu_mapped_ui_buffer(gpu_element_buffer_handles *Handles, data_type Type, u32 ElementCount);
link_internal void AllocateGpuBuffer_gpu_mapped_element_buffer(gpu_element_buffer_handles *Handles, data_type Type, u32 ElementCount);

/* link_internal gpu_mapped_ui_buffer MapGpuBuffer_gpu_mapped_ui_buffer(gpu_element_buffer_handles *Handles); */
link_internal gpu_mapped_untextured_3d_geometry_buffer MapGpuBuffer_gpu_mapped_element_buffer(gpu_element_buffer_handles *Handles);

link_internal gpu_mapped_ui_buffer UnmapGpuBuffer_gpu_mapped_ui_buffer(gpu_element_buffer_handles *Handles);
link_internal gpu_mapped_untextured_3d_geometry_buffer UnmapGpuBuffer_gpu_mapped_element_buffer(gpu_element_buffer_handles *Handles);

link_internal void DrawBuffer(      gpu_mapped_ui_buffer *, v2 *);
link_internal void DrawBuffer( gpu_mapped_element_buffer *, v2 *);

link_internal b32 UnmapGpuBuffer(gpu_element_buffer_handles *Handles);


/* poof(triple_buffered(gpu_mapped_ui_buffer, ui_geometry_buffer)) */
#include <generated/triple_buffered_iGeuhioE.h>

poof(triple_buffered(gpu_mapped_element_buffer, untextured_3d_geometry_buffer))
#include <generated/triple_buffered_HQIb5E82.h>

link_internal void
SetupVertexAttribsFor_u3d_geo_element_buffer(gpu_element_buffer_handles *Handles);
