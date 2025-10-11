
struct ui_geometry_buffer
{
  v3 *Verts;
  v3 *Colors; // TODO(Jesse): Remove this in favor of a 16bit HSV color
  v3 *UVs;

  u32 End;
  u32 At;
};

