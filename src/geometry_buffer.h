
struct ui_geometry_buffer
{
  v3 *Verts;
  v3 *Colors; // TODO(Jesse): Remove this in favor of a u16 ColorIndex?
  v3 *UVs;

  u32 End;
  u32 At;
};

