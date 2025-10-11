
link_internal void
FramebufferTextureLayer(framebuffer *FBO, texture *Tex, ui_texture_slice Layer)
{
  u32 Attachment = FBO->Attachments++;
  GetGL()->FramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + Attachment, Tex->ID, 0, Layer);
  return;
}

link_internal shader
MakeRenderToTextureShader(memory_arena *Memory, m4 *ViewProjection, texture *ColorPalette)
{
  shader Shader = CompileShaderPair( CSz(BONSAI_SHADER_PATH "RenderToTexture.vertexshader"), CSz(BONSAI_SHADER_PATH "RenderToTexture.fragmentshader") );
  Shader.Uniforms = ShaderUniformBuffer(2, Memory);

  SetShaderUniform(&Shader, 0, ViewProjection, "ViewProjection");
  SetShaderUniform(&Shader, 1, ColorPalette,  "ColorPalette");

  return Shader;
}
