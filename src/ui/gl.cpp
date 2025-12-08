
link_internal void
FramebufferTextureLayer(framebuffer *FBO, texture *Tex, ui_texture_slice Layer)
{
  u32 Attachment = FBO->Attachments++;
  GetGL()->FramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + Attachment, Tex->ID, 0, Layer);
  return;
}

link_internal b32
MakeRenderToTextureShader(shader *Shader, memory_arena *Memory, m4 *ViewProjection, texture *ColorPalette)
{
  b32 Result = CompileShaderPair(Shader, CSz(BONSAI_SHADER_PATH "RenderToTexture.vertexshader"), CSz(BONSAI_SHADER_PATH "RenderToTexture.fragmentshader") );

  if (Result)
  {
    Shader->Uniforms = ShaderUniformBuffer(1, Memory);

    InitShaderUniform(Shader, 0, ViewProjection, "ViewProjection");
    /* InitShaderUniform(Shader, 1, ColorPalette,   "ColorPalette"); */
  }

  return Result;
}
