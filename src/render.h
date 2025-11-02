
struct render_to_texture_group
{
  framebuffer                FBO;
  shader                     Shader;
  gpu_mapped_element_buffer  GeoBuffer;
  m4 ViewProjection;
};

