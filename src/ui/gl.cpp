
link_internal void
FramebufferTextureLayer(framebuffer *FBO, texture *Tex, debug_texture_array_slice Layer)
{
  u32 Attachment = FBO->Attachments++;
  GetStdlib()->GL.FramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + Attachment, Tex->ID, 0, Layer);
  return;
}

link_internal void
DrawUiBuffer(render_buffers_2d *TextGroup, textured_2d_geometry_buffer *Geo, v2 *ScreenDim)
{
  GetStdlib()->GL.BindFramebuffer(GL_FRAMEBUFFER, 0);

  u32 AttributeIndex = 0;
  BufferVertsToCard( TextGroup->SolidUIVertexBuffer, Geo, &AttributeIndex);
  BufferUVsToCard(   TextGroup->SolidUIUVBuffer,     Geo, &AttributeIndex);
  BufferColorsToCard(TextGroup->SolidUIColorBuffer,  Geo, &AttributeIndex);

  if (Geo->At) { Draw(Geo->At); }
  Geo->At = 0;

  GetStdlib()->GL.DisableVertexAttribArray(0);
  GetStdlib()->GL.DisableVertexAttribArray(1);
  GetStdlib()->GL.DisableVertexAttribArray(2);

  AssertNoGlErrors;
}

link_internal void
DrawUiBuffer(render_buffers_2d *TextGroup, untextured_2d_geometry_buffer *Buffer, v2 *ScreenDim)
{
  TIMED_FUNCTION();

  if (TextGroup)
  {
    GetStdlib()->GL.BindFramebuffer(GL_FRAMEBUFFER, 0);
    AssertNoGlErrors;
    /* UseShader(&TextGroup->SolidUIShader); */

    u32 AttributeIndex = 0;
    BufferVertsToCard(TextGroup->SolidUIVertexBuffer, Buffer, &AttributeIndex);
    BufferColorsToCard(TextGroup->SolidUIColorBuffer, Buffer, &AttributeIndex);

    // TODO(Jesse): Hoist this if out of here so we can elide all of the drawing
    // stuff.  Have to call the functions that unmap the buffers, which is why
    // I didn't do it in the first place.
    if (Buffer->At) { Draw(Buffer->At); }
    Buffer->At = 0;

    GetStdlib()->GL.DisableVertexAttribArray(0);
    GetStdlib()->GL.DisableVertexAttribArray(1);

    AssertNoGlErrors;
  }
  else
  {
    Warn("DrawUiBuffer call issued without a RenderGroup!");
  }

  return;
}

link_internal void
DrawUiBuffers(renderer_2d *UiGroup, v2 *ScreenDim)
{
  Assert(UiGroup->TextGroup);
  auto TextGroup = UiGroup->TextGroup;

  GetStdlib()->GL.Disable(GL_CULL_FACE);

  SetViewport(*ScreenDim);

  GetStdlib()->GL.UseProgram(TextGroup->SolidUIShader.ID);

    DrawUiBuffer(TextGroup, &UiGroup->Geo, ScreenDim);
  /* UiGroup->Geo.At = 0; */



  GetStdlib()->GL.UseProgram(TextGroup->Text2DShader.ID);
  GetStdlib()->GL.ActiveTexture(GL_TEXTURE0);
  GetStdlib()->GL.BindTexture(GL_TEXTURE_2D_ARRAY, TextGroup->DebugTextureArray.ID);
  GetStdlib()->GL.Uniform1i(TextGroup->TextTextureUniform, 0); // Assign texture unit 0 to the TextTexureUniform

  GetStdlib()->GL.Enable(GL_BLEND);
  GetStdlib()->GL.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    DrawUiBuffer(UiGroup->TextGroup, &UiGroup->TextGroup->Geo, ScreenDim);

  GetStdlib()->GL.BindTexture(GL_TEXTURE_2D_ARRAY, 0);

  GetStdlib()->GL.Disable(GL_BLEND);
  GetStdlib()->GL.Enable(GL_CULL_FACE);
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
