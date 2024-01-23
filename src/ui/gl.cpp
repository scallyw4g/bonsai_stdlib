
link_internal void
FramebufferTextureLayer(framebuffer *FBO, texture *Tex, debug_texture_array_slice Layer)
{
  u32 Attachment = FBO->Attachments++;
  GL.FramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + Attachment, Tex->ID, 0, Layer);
  return;
}

link_internal void
DrawUiBuffer(render_buffers_2d *TextGroup, textured_2d_geometry_buffer *Geo, v2 *ScreenDim)
{
  GL.BindFramebuffer(GL_FRAMEBUFFER, 0);
  /* SetViewport(*ScreenDim); */
  /* GL.UseProgram(TextGroup->Text2DShader.ID); */
  /* GL.ActiveTexture(GL_TEXTURE0); */
  /* GL.BindTexture(GL_TEXTURE_2D_ARRAY, TextGroup->DebugTextureArray->ID); */
  /* GL.Uniform1i(TextGroup->TextTextureUniform, 0); // Assign texture unit 0 to the TextTexureUniform */

  u32 AttributeIndex = 0;
  BufferVertsToCard( TextGroup->SolidUIVertexBuffer, Geo, &AttributeIndex);
  BufferUVsToCard(   TextGroup->SolidUIUVBuffer,     Geo, &AttributeIndex);
  BufferColorsToCard(TextGroup->SolidUIColorBuffer,  Geo, &AttributeIndex);

  if (Geo->At) { Draw(Geo->At); }
  Geo->At = 0;

  GL.DisableVertexAttribArray(0);
  GL.DisableVertexAttribArray(1);
  GL.DisableVertexAttribArray(2);

  AssertNoGlErrors;
}

link_internal void
DrawUiBuffer(render_buffers_2d *TextGroup, untextured_2d_geometry_buffer *Buffer, v2 *ScreenDim)
{
  TIMED_FUNCTION();

  if (TextGroup)
  {
    GL.BindFramebuffer(GL_FRAMEBUFFER, 0);
    /* UseShader(&TextGroup->SolidUIShader); */

    u32 AttributeIndex = 0;
    BufferVertsToCard(TextGroup->SolidUIVertexBuffer, Buffer, &AttributeIndex);
    BufferColorsToCard(TextGroup->SolidUIColorBuffer, Buffer, &AttributeIndex);

    // TODO(Jesse): Hoist this if out of here so we can elide all of the drawing
    // stuff.  Have to call the functions that unmap the buffers, which is why
    // I didn't do it in the first place.
    if (Buffer->At) { Draw(Buffer->At); }
    Buffer->At = 0;

    GL.DisableVertexAttribArray(0);
    GL.DisableVertexAttribArray(1);

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

  GL.Disable(GL_CULL_FACE);

  SetViewport(*ScreenDim);

  GL.UseProgram(TextGroup->SolidUIShader.ID);

    DrawUiBuffer(TextGroup, &UiGroup->Geo, ScreenDim);
  /* UiGroup->Geo.At = 0; */



  GL.UseProgram(TextGroup->Text2DShader.ID);
  GL.ActiveTexture(GL_TEXTURE0);
  GL.BindTexture(GL_TEXTURE_2D_ARRAY, TextGroup->DebugTextureArray->ID);
  GL.Uniform1i(TextGroup->TextTextureUniform, 0); // Assign texture unit 0 to the TextTexureUniform
  GL.Enable(GL_BLEND);
  GL.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    DrawUiBuffer(UiGroup->TextGroup, &UiGroup->TextGroup->Geo, ScreenDim);
  /* UiGroup->TextGroup->Geo.At = 0; */

  GL.Disable(GL_BLEND);
  GL.Enable(GL_CULL_FACE);
}

link_internal shader
MakeRenderToTextureShader(memory_arena *Memory, m4 *ViewProjection)
{
  shader Shader = LoadShaders( CSz(BONSAI_SHADER_PATH "RenderToTexture.vertexshader"), CSz(BONSAI_SHADER_PATH "RenderToTexture.fragmentshader") );

  shader_uniform **Current = &Shader.FirstUniform;

  *Current = GetUniform(Memory, &Shader, ViewProjection, "ViewProjection");
  Current = &(*Current)->Next;

  return Shader;
}
