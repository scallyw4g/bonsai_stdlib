debug_global float
g_quad_vertex_buffer_data[] =
{
  -1.0f, -1.0f, 1.0f,
   1.0f, -1.0f, 1.0f,
  -1.0f,  1.0f, 1.0f,
  -1.0f,  1.0f, 1.0f,
   1.0f, -1.0f, 1.0f,
   1.0f,  1.0f, 1.0f,
};

global_variable u32 Global_QuadVertexBuffer;
global_variable u32 Global_QuadVAO;
global_variable v3 GlobalLightPosition = {{0.20f, 1.0f, 1.0f}};

global_variable m4 NdcToScreenSpace =
{
  V4(0.5, 0.0, 0.0, 0.0),
  V4(0.0, 0.5, 0.0, 0.0),
  V4(0.0, 0.0, 0.5, 0.0),
  V4(0.5, 0.5, 0.5, 1.0)
};

void
Init_Global_QuadVertexBuffer()
{
  GetStdlib()->GL.GenVertexArrays(1, &Global_QuadVAO);
  GetStdlib()->GL.BindVertexArray(Global_QuadVAO);

  GetStdlib()->GL.GenBuffers(1, &Global_QuadVertexBuffer);
  Assert(Global_QuadVertexBuffer);

  GetStdlib()->GL.BindBuffer(GL_ARRAY_BUFFER, Global_QuadVertexBuffer);
  GetStdlib()->GL.BufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);
  GetStdlib()->GL.BindBuffer(GL_ARRAY_BUFFER, 0);

  return;
}

void
RenderQuad()
{
  if (!Global_QuadVertexBuffer) { Init_Global_QuadVertexBuffer(); }

  GetStdlib()->GL.BindBuffer(GL_ARRAY_BUFFER, Global_QuadVertexBuffer);
  AssertNoGlErrors;
  GetStdlib()->GL.EnableVertexAttribArray(0);
  AssertNoGlErrors;
  GetStdlib()->GL.VertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
  AssertNoGlErrors;

  Draw(6);

  GetStdlib()->GL.BindBuffer(GL_ARRAY_BUFFER, 0);
  GetStdlib()->GL.DisableVertexAttribArray(0);
}

#if 0
void
SetupToDrawTexturedQuad(shader *SimpleTextureShader, texture *Texture)
{
  /* GetStdlib()->GL.BindFramebuffer(GL_FRAMEBUFFER, 0); */

  // TODO(Jesse): ?
  GetStdlib()->GL.Disable(GL_DEPTH_TEST);
  /* GetStdlib()->GL.DepthFunc(GL_LEQUAL); */

  /* r32 Scale = 0.5f; */
  /* SetViewport( V2(Texture->Dim.x, Texture->Dim.y)*Scale ); */

  GetStdlib()->GL.UseProgram(SimpleTextureShader->ID);

  SimpleTextureShader->FirstUniform->Texture = Texture;
  Assert(SimpleTextureShader->FirstUniform->Next == 0);
  BindShaderUniforms(SimpleTextureShader);

  GetStdlib()->GL.Enable(GL_DEPTH_TEST);
  /* RenderQuad(); */
  /* AssertNoGlErrors; */
}
#endif

