
link_internal void
FramebufferTextureLayer(framebuffer *FBO, texture *Tex, ui_texture_slice Layer)
{
  u32 Attachment = FBO->Attachments++;
  GetGL()->FramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + Attachment, Tex->ID, 0, Layer);
  return;
}

link_internal void
DrawUiBuffer(render_buffers_2d *TextGroup, ui_geometry_buffer *Geo, v2 *ScreenDim)
{
  GetGL()->BindFramebuffer(GL_FRAMEBUFFER, 0);

  u32 AttributeIndex = 0;
  BufferVertsToCard( TextGroup->Buf.Handles.Handles[ui_VertexHandle], Geo, &AttributeIndex);
  BufferUVsToCard(   TextGroup->Buf.Handles.Handles[ui_UVHandle],     Geo, &AttributeIndex);
  BufferColorsToCard(TextGroup->Buf.Handles.Handles[ui_ColorHandle],  Geo, &AttributeIndex);

  if (Geo->At) { Draw(Geo->At); }
  Geo->At = 0;

  GetGL()->DisableVertexAttribArray(0);
  GetGL()->DisableVertexAttribArray(1);
  GetGL()->DisableVertexAttribArray(2);

  AssertNoGlErrors;
}

#if 0
link_internal void
DrawUiBuffer(render_buffers_2d *TextGroup, ui_geometry_buffer *Buffer, v2 *ScreenDim)
{
  TIMED_FUNCTION();

  if (TextGroup)
  {
    GetGL()->BindFramebuffer(GL_FRAMEBUFFER, 0);
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

    GetGL()->DisableVertexAttribArray(0);
    GetGL()->DisableVertexAttribArray(1);

    AssertNoGlErrors;
  }
  else
  {
    Warn("DrawUiBuffer call issued without a RenderGroup!");
  }

  return;
}
#endif

link_internal void
DrawUiBuffers(renderer_2d *UiGroup, v2 *ScreenDim)
{
  Assert(UiGroup->TextGroup);
  auto TextGroup = UiGroup->TextGroup;

  GetGL()->Disable(GL_CULL_FACE);
  AssertNoGlErrors;

  SetViewport(*ScreenDim);
  AssertNoGlErrors;

  /* GetGL()->UseProgram(TextGroup->SolidUIShader.ID); */
  /* AssertNoGlErrors; */

  /* DrawUiBuffer(TextGroup, &UiGroup->Geo, ScreenDim); */
  /* AssertNoGlErrors; */
  /* UiGroup->Geo.At = 0; */



  GetGL()->UseProgram(TextGroup->Text2DShader.ID);

  GetGL()->ActiveTexture(GL_TEXTURE0);
  GetGL()->BindTexture(GL_TEXTURE_2D_ARRAY, TextGroup->DebugTextureArray.ID);
  GetGL()->Uniform1i(TextGroup->TextTextureUniform, 0); // Assign texture unit 0 to the TextTexureUniform

  GetGL()->Enable(GL_BLEND);
  GetGL()->BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  DrawUiBuffer(UiGroup->TextGroup, &UiGroup->TextGroup->Buf.Buffer, ScreenDim);

  GetGL()->BindTexture(GL_TEXTURE_2D_ARRAY, 0);

  GetGL()->Disable(GL_BLEND);
  GetGL()->Enable(GL_CULL_FACE);
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
