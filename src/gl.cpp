
#define Draw(VertexCount) Draw_(VertexCount, __FUNCTION__)



link_internal debug_state *GetDebugState();

link_internal void
Draw_(u32 VertexCount, const char *Caller)
{
  TIMED_FUNCTION();
  Assert(VertexCount);

  // NOTE(Jesse): This is hella slow, so I'm just going to do a quick-n-dirty
  // version for the moment
  /* DEBUG_TRACK_DRAW_CALL(Caller, VertexCount); */

#if BONSAI_DEBUG_SYSTEM_API
  GetDebugState()->DrawCallCountLastFrame ++;
  GetDebugState()->VertexCountLastFrame += VertexCount;;
#endif

  AssertNoGlErrors;
  GetGL()->DrawArrays(GL_TRIANGLES, 0, s32(VertexCount) );
  AssertNoGlErrors;
}

inline void
SetDefaultFramebufferClearColors()
{
  GetGL()->ClearDepth(1.0); // NOTE(Jesse): Depth 1.0 is the furthest from the camera
  GetGL()->ClearColor(0.f, 0.f, 0.f, 1.f);
}

link_internal void
SetVSync(os *Os, s32 VSyncFrames)
{
  AssertNoGlErrors;

#if BONSAI_LINUX
  // TODO(Jesse, id: 151, tags: open_question, platform_linux): Not getting vsync on my arch laptop.
  PFNSWAPINTERVALPROC glSwapInterval = (PFNSWAPINTERVALPROC)PlatformGetGlFunction("glXSwapIntervalEXT");
  if ( glSwapInterval )
  { glSwapInterval(Os->Display, Os->Window, VSyncFrames); }
  else
  { Info("No Vsync"); }
#elif BONSAI_WIN32
  PFNSWAPINTERVALPROC glSwapInterval = (PFNSWAPINTERVALPROC)PlatformGetGlFunction("wglSwapIntervalEXT");
  if ( glSwapInterval )
  { glSwapInterval(VSyncFrames); }
  else
  { Info("No Vsync"); }
#elif EMCC
  // TODO(Jesse id: 368): How do we get vsync here?
  // @emcc_vsync
#endif

}

inline void
SetViewport(v2i Dim)
{
  GetGL()->Viewport(0, 0, Dim.x, Dim.y);
}

inline void
SetViewport(v2 Dim)
{
  GetGL()->Viewport(0, 0, (s32)Dim.x, (s32)Dim.y);
}

struct texture_buffer_binding
{
  u32 Texture;
  u32 Buffer;
};

link_internal void
BindTextureBuffer(texture_buffer_binding *Binding, s32 SamplerUniform, u32 TextureUnit, void *Data, umm SizeBytes)
{
  auto GL = GetGL();

  Assert(SamplerUniform >= 0);
  Assert(SizeBytes && SizeBytes % 16 == 0);
  Assert(GL->MaxTextureBufferTexels > 0 &&
         SizeBytes / 16 <= Cast(umm, GL->MaxTextureBufferTexels));

  if (Binding->Buffer == 0)
  {
    GL->GenBuffers(1, &Binding->Buffer);
    GL->GenTextures(1, &Binding->Texture);
    Assert(Binding->Buffer);
    Assert(Binding->Texture);
  }

  GL->BindBuffer(GL_TEXTURE_BUFFER, Binding->Buffer);
  GL->BufferData(GL_TEXTURE_BUFFER, Cast(GLsizeiptr, SizeBytes), Data, GL_DYNAMIC_DRAW);

  // Integer texels preserve float bit patterns and integer fields without conversion.
  GL->ActiveTexture(GL_TEXTURE0 + TextureUnit);
  GL->BindTexture(GL_TEXTURE_BUFFER, Binding->Texture);
  GL->TexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32UI, Binding->Buffer);
  GL->Uniform1i(SamplerUniform, Cast(s32, TextureUnit));
  GL->ActiveTexture(GL_TEXTURE0);

  AssertNoGlErrors;
}

link_internal b32
InitializeOpenglFunctions()
{
  Info("Initializing OpenGL Extensions");

#if 0
  const char* glxExtensionString = glXQueryExtensionsString(Os->Display, DefaultScreen(Os->Display));
  const char* glExtensionString = (const char*)glGetString(GL_EXTENSIONS);
  Debug(glExtensionString);
  Debug(glxExtensionString);
#endif

  auto GL = GetGL();

  Assert(GL->Initialized == False);
  GL->Initialized = True;

  GL->GetString                = (OpenglGetString)PlatformGetGlFunction("glGetString");
  GL->Initialized              &= GL->GetString != 0;

  s32 GLMajor = -1;
  s32 GLMinor = -1;

  if (GL->GetString)
  {
    char* Vendor   = (char*)GL->GetString(GL_VENDOR);
    char* Renderer = (char*)GL->GetString(GL_RENDERER);
    char* Version  = (char*)GL->GetString(GL_VERSION);
    if (Vendor && Renderer && Version)
    {
      Info("Opengl Driver Info : (%s) (%s) (%s)", Vendor, Renderer, Version);

      GL->GetError                  = (OpenglGetError)PlatformGetGlFunction("glGetError");
      GL->Initialized               &= GL->GetError != 0;

      GL->Enable                    = (OpenglEnable)PlatformGetGlFunction("glEnable");
      GL->Initialized               &= GL->Enable != 0;

      GL->Disable                   = (OpenglDisable)PlatformGetGlFunction("glDisable");
      GL->Initialized               &= GL->Disable != 0;

      GL->CullFace                  = (OpenglCullFace)PlatformGetGlFunction("glCullFace");
      GL->Initialized               &= GL->CullFace != 0;

      GL->Viewport                  = (OpenglViewport)PlatformGetGlFunction("glViewport");
      GL->Initialized               &= GL->Viewport != 0;

      GL->DepthFunc                 = (OpenglDepthFunc)PlatformGetGlFunction("glDepthFunc");
      GL->Initialized               &= GL->DepthFunc != 0;

      GL->BlendFunc                 = (OpenglBlendFunc)PlatformGetGlFunction("glBlendFunc");
      GL->Initialized               &= GL->BlendFunc != 0;

      GL->DrawArrays                = (OpenglDrawArrays)PlatformGetGlFunction("glDrawArrays");
      GL->Initialized               &= GL->DrawArrays != 0;

      GL->DrawArraysInstanced       = (OpenglDrawArraysInstanced)PlatformGetGlFunction("glDrawArraysInstanced");
      GL->Initialized               &= GL->DrawArraysInstanced != 0;

#if !BONSAI_MACOS
      GL->MultiDrawArraysIndirect   = (OpenglMultiDrawArraysIndirect)PlatformGetGlFunction("glMultiDrawArraysIndirect");
      GL->Initialized               &= GL->MultiDrawArraysIndirect != 0;
#endif

      GL->DrawArraysIndirect        = (OpenglDrawArraysIndirect)PlatformGetGlFunction("glDrawArraysIndirect");
      GL->Initialized               &= GL->DrawArraysIndirect != 0;

      GL->Clear                     = (OpenglClear)PlatformGetGlFunction("glClear");
      GL->Initialized               &= GL->Clear != 0;

      GL->ClearColor                = (OpenglClearColor)PlatformGetGlFunction("glClearColor");
      GL->Initialized               &= GL->ClearColor != 0;

      GL->ClearDepth                = (OpenglClearDepth)PlatformGetGlFunction("glClearDepth");
      GL->Initialized               &= GL->ClearDepth != 0;

      GL->GenTextures               = (OpenglGenTextures)PlatformGetGlFunction("glGenTextures");
      GL->Initialized               &= GL->GenTextures != 0;

      GL->BindTexture               = (OpenglBindTexture)PlatformGetGlFunction("glBindTexture");
      GL->Initialized               &= GL->BindTexture != 0;

      GL->DeleteTextures            = (OpenglDeleteTextures)PlatformGetGlFunction("glDeleteTextures");
      GL->Initialized               &= GL->DeleteTextures != 0;

      GL->ActiveTexture             = (OpenglActiveTexture)PlatformGetGlFunction("glActiveTexture");
      GL->Initialized               &= GL->ActiveTexture != 0;

      GL->GetTexImage               = (OpenglGetTexImage)PlatformGetGlFunction("glGetTexImage");
      GL->Initialized               &= GL->GetTexImage != 0;

      GL->ReadPixels                = (OpenglReadPixels)PlatformGetGlFunction("glReadPixels");
      GL->Initialized               &= GL->ReadPixels != 0;

#if !BONSAI_MACOS
      GL->TexStorage1D              = (OpenglTexStorage1D)PlatformGetGlFunction("glTexStorage1D");
      GL->Initialized               &= GL->TexStorage1D != 0;

      GL->TexStorage2D              = (OpenglTexStorage2D)PlatformGetGlFunction("glTexStorage2D");
      GL->Initialized               &= GL->TexStorage2D != 0;

      GL->TexStorage3D              = (OpenglTexStorage3D)PlatformGetGlFunction("glTexStorage3D");
      GL->Initialized               &= GL->TexStorage3D != 0;
#endif

      GL->TexImage1D                = (OpenglTexImage1D)PlatformGetGlFunction("glTexImage1D");
      GL->Initialized               &= GL->TexImage1D != 0;

      GL->TexImage2D                = (OpenglTexImage2D)PlatformGetGlFunction("glTexImage2D");
      GL->Initialized               &= GL->TexImage2D != 0;

      GL->TexImage3D                = (OpenglTexImage3D)PlatformGetGlFunction("glTexImage3D");
      GL->Initialized               &= GL->TexImage3D != 0;

      GL->TexSubImage1D             = (OpenglTexSubImage1D)PlatformGetGlFunction("glTexSubImage1D");
      GL->Initialized               &= GL->TexSubImage1D != 0;

      GL->TexSubImage2D             = (OpenglTexSubImage2D)PlatformGetGlFunction("glTexSubImage2D");
      GL->Initialized               &= GL->TexSubImage2D != 0;

      GL->TexSubImage3D             = (OpenglTexSubImage3D)PlatformGetGlFunction("glTexSubImage3D");
      GL->Initialized               &= GL->TexSubImage3D != 0;

      GL->TexParameterf             = (OpenglTexParameterf)PlatformGetGlFunction("glTexParameterf");
      GL->Initialized               &= GL->TexParameterf != 0;

      GL->TexParameterfv            = (OpenglTexParameterfv)PlatformGetGlFunction("glTexParameterfv");
      GL->Initialized               &= GL->TexParameterfv != 0;

      GL->TexParameteri             = (OpenglTexParameteri)PlatformGetGlFunction("glTexParameteri");
      GL->Initialized               &= GL->TexParameteri != 0;

      GL->TexParameteriv            = (OpenglTexParameteriv)PlatformGetGlFunction("glTexParameteriv");
      GL->Initialized               &= GL->TexParameteriv != 0;

      GL->TexBuffer                 = (OpenglTexBuffer)PlatformGetGlFunction("glTexBuffer");
      GL->Initialized               &= GL->TexBuffer != 0;

      GL->CompressedTexImage3D      = (OpenglCompressedTexImage3D)PlatformGetGlFunction("glCompressedTexImage3D");
      GL->Initialized               &= GL->CompressedTexImage3D != 0;

      GL->CompressedTexImage2D      = (OpenglCompressedTexImage2D)PlatformGetGlFunction("glCompressedTexImage2D");
      GL->Initialized               &= GL->CompressedTexImage2D != 0;

      GL->CompressedTexImage1D      = (OpenglCompressedTexImage1D)PlatformGetGlFunction("glCompressedTexImage1D");
      GL->Initialized               &= GL->CompressedTexImage1D != 0;

      GL->PixelStoref               = (OpenglPixelStoref)PlatformGetGlFunction("glPixelStoref");
      GL->Initialized               &= GL->PixelStoref != 0;

      GL->PixelStorei               = (OpenglPixelStorei)PlatformGetGlFunction("glPixelStorei");
      GL->Initialized               &= GL->PixelStorei != 0;

      GL->EnableVertexAttribArray   = (OpenglEnableVertexAttribArray)PlatformGetGlFunction("glEnableVertexAttribArray");
      GL->Initialized               &= GL->EnableVertexAttribArray != 0;

      GL->DisableVertexAttribArray  = (OpenglDisableVertexAttribArray)PlatformGetGlFunction("glDisableVertexAttribArray");
      GL->Initialized               &= GL->DisableVertexAttribArray != 0;

      GL->VertexAttribPointer       = (OpenglVertexAttribPointer)PlatformGetGlFunction("glVertexAttribPointer");
      GL->Initialized               &= GL->VertexAttribPointer != 0;

      GL->VertexAttribIPointer      = (OpenglVertexAttribIPointer)PlatformGetGlFunction("glVertexAttribIPointer");
      GL->Initialized               &= GL->VertexAttribIPointer != 0;

      GL->BindFramebuffer           = (OpenglBindFramebuffer)PlatformGetGlFunction("glBindFramebuffer");
      GL->Initialized               &= GL->BindFramebuffer != 0;

      GL->DeleteFramebuffers        = (OpenglDeleteFramebuffers)PlatformGetGlFunction("glDeleteFramebuffers");
      GL->Initialized               &= GL->DeleteFramebuffers != 0;

      GL->GenFramebuffers           = (OpenglGenFramebuffers)PlatformGetGlFunction("glGenFramebuffers");
      GL->Initialized               &= GL->GenFramebuffers != 0;

      GL->CheckFramebufferStatus    = (OpenglCheckFramebufferStatus)PlatformGetGlFunction("glCheckFramebufferStatus");
      GL->Initialized               &= GL->CheckFramebufferStatus != 0;

      GL->FramebufferTexture1D      = (OpenglFramebufferTexture1D)PlatformGetGlFunction("glFramebufferTexture1D");
      GL->Initialized               &= GL->FramebufferTexture1D != 0;

      GL->FramebufferTexture2D      = (OpenglFramebufferTexture2D)PlatformGetGlFunction("glFramebufferTexture2D");
      GL->Initialized               &= GL->FramebufferTexture2D != 0;

      GL->FramebufferTexture3D      = (OpenglFramebufferTexture3D)PlatformGetGlFunction("glFramebufferTexture3D");
      GL->Initialized               &= GL->FramebufferTexture3D != 0;

      GL->FramebufferTextureLayer   = (OpenglFramebufferTextureLayer)PlatformGetGlFunction("glFramebufferTextureLayer");
      GL->Initialized               &= GL->FramebufferTextureLayer != 0;

      GL->FramebufferRenderbuffer   = (OpenglFramebufferRenderbuffer)PlatformGetGlFunction("glFramebufferRenderbuffer");
      GL->Initialized               &= GL->FramebufferRenderbuffer != 0;

      GL->CompileShader             = (OpenglCompileShader)PlatformGetGlFunction("glCompileShader");
      GL->Initialized               &= GL->CompileShader != 0;

      GL->CreateProgram             = (OpenglCreateProgram)PlatformGetGlFunction("glCreateProgram");
      GL->Initialized               &= GL->CreateProgram != 0;

      GL->CreateShader              = (OpenglCreateShader)PlatformGetGlFunction("glCreateShader");
      GL->Initialized               &= GL->CreateShader != 0;

      GL->LinkProgram               = (OpenglLinkProgram)PlatformGetGlFunction("glLinkProgram");
      GL->Initialized               &= GL->LinkProgram != 0;

      GL->ShaderSource              = (OpenglShaderSource)PlatformGetGlFunction("glShaderSource");
      GL->Initialized               &= GL->ShaderSource != 0;

      GL->GetShaderSource           = (OpenglGetShaderSource)PlatformGetGlFunction("glGetShaderSource");
      GL->Initialized               &= GL->GetShaderSource != 0;

      GL->UseProgram                = (OpenglUseProgram)PlatformGetGlFunction("glUseProgram");
      GL->Initialized               &= GL->UseProgram != 0;

      GL->GetProgramInfoLog         = (OpenglGetProgramInfoLog)PlatformGetGlFunction("glGetProgramInfoLog");
      GL->Initialized               &= GL->GetProgramInfoLog != 0;

      GL->GetShaderInfoLog          = (OpenglGetShaderInfoLog)PlatformGetGlFunction("glGetShaderInfoLog");
      GL->Initialized               &= GL->GetShaderInfoLog != 0;

      GL->DeleteProgram             = (OpenglDeleteProgram)PlatformGetGlFunction("glDeleteProgram");
      GL->Initialized               &= GL->DeleteProgram != 0;

      GL->DeleteShader              = (OpenglDeleteShader)PlatformGetGlFunction("glDeleteShader");
      GL->Initialized               &= GL->DeleteShader != 0;

      GL->DetachShader              = (OpenglDetachShader)PlatformGetGlFunction("glDetachShader");
      GL->Initialized               &= GL->DetachShader != 0;

      GL->Uniform1f                 = (OpenglUniform1f)PlatformGetGlFunction("glUniform1f");
      GL->Initialized               &= GL->Uniform1f != 0;

      GL->Uniform2f                 = (OpenglUniform2f)PlatformGetGlFunction("glUniform2f");
      GL->Initialized               &= GL->Uniform2f != 0;

      GL->Uniform3f                 = (OpenglUniform3f)PlatformGetGlFunction("glUniform3f");
      GL->Initialized               &= GL->Uniform3f != 0;

      GL->Uniform4f                 = (OpenglUniform4f)PlatformGetGlFunction("glUniform4f");
      GL->Initialized               &= GL->Uniform4f != 0;

      GL->Uniform1i                 = (OpenglUniform1i)PlatformGetGlFunction("glUniform1i");
      GL->Initialized               &= GL->Uniform1i != 0;

      GL->Uniform2i                 = (OpenglUniform2i)PlatformGetGlFunction("glUniform2i");
      GL->Initialized               &= GL->Uniform2i != 0;

      GL->Uniform3i                 = (OpenglUniform3i)PlatformGetGlFunction("glUniform3i");
      GL->Initialized               &= GL->Uniform3i != 0;

      GL->Uniform4i                 = (OpenglUniform4i)PlatformGetGlFunction("glUniform4i");
      GL->Initialized               &= GL->Uniform4i != 0;

      GL->Uniform1fv                = (OpenglUniform1fv)PlatformGetGlFunction("glUniform1fv");
      GL->Initialized               &= GL->Uniform1fv != 0;

      GL->Uniform2fv                = (OpenglUniform2fv)PlatformGetGlFunction("glUniform2fv");
      GL->Initialized               &= GL->Uniform2fv != 0;

      GL->Uniform3fv                = (OpenglUniform3fv)PlatformGetGlFunction("glUniform3fv");
      GL->Initialized               &= GL->Uniform3fv != 0;

      GL->Uniform4fv                = (OpenglUniform4fv)PlatformGetGlFunction("glUniform4fv");
      GL->Initialized               &= GL->Uniform4fv != 0;

      GL->Uniform1iv                = (OpenglUniform1iv)PlatformGetGlFunction("glUniform1iv");
      GL->Initialized               &= GL->Uniform1iv != 0;

      GL->Uniform2iv                = (OpenglUniform2iv)PlatformGetGlFunction("glUniform2iv");
      GL->Initialized               &= GL->Uniform2iv != 0;

      GL->Uniform3iv                = (OpenglUniform3iv)PlatformGetGlFunction("glUniform3iv");
      GL->Initialized               &= GL->Uniform3iv != 0;

      GL->Uniform4iv                = (OpenglUniform4iv)PlatformGetGlFunction("glUniform4iv");
      GL->Initialized               &= GL->Uniform4iv != 0;

      GL->UniformMatrix2fv          = (OpenglUniformMatrix2fv)PlatformGetGlFunction("glUniformMatrix2fv");
      GL->Initialized               &= GL->UniformMatrix2fv != 0;

      GL->UniformMatrix3fv          = (OpenglUniformMatrix3fv)PlatformGetGlFunction("glUniformMatrix3fv");
      GL->Initialized               &= GL->UniformMatrix3fv != 0;

      GL->UniformMatrix4fv          = (OpenglUniformMatrix4fv)PlatformGetGlFunction("glUniformMatrix4fv");
      GL->Initialized               &= GL->UniformMatrix4fv != 0;

      GL->Uniform1ui                = (OpenglUniform1ui)PlatformGetGlFunction("glUniform1ui");
      GL->Initialized               &= GL->Uniform1ui != 0;

      GL->Uniform2ui                = (OpenglUniform2ui)PlatformGetGlFunction("glUniform2ui");
      GL->Initialized               &= GL->Uniform2ui != 0;

      GL->Uniform3ui                = (OpenglUniform3ui)PlatformGetGlFunction("glUniform3ui");
      GL->Initialized               &= GL->Uniform3ui != 0;

      GL->Uniform4ui                = (OpenglUniform4ui)PlatformGetGlFunction("glUniform4ui");
      GL->Initialized               &= GL->Uniform4ui != 0;

      GL->Uniform1uiv               = (OpenglUniform1uiv)PlatformGetGlFunction("glUniform1uiv");
      GL->Initialized               &= GL->Uniform1uiv != 0;

      GL->Uniform2uiv               = (OpenglUniform2uiv)PlatformGetGlFunction("glUniform2uiv");
      GL->Initialized               &= GL->Uniform2uiv != 0;

      GL->Uniform3uiv               = (OpenglUniform3uiv)PlatformGetGlFunction("glUniform3uiv");
      GL->Initialized               &= GL->Uniform3uiv != 0;

      GL->Uniform4uiv               = (OpenglUniform4uiv)PlatformGetGlFunction("glUniform4uiv");
      GL->Initialized               &= GL->Uniform4uiv != 0;

      GL->GetUniformLocation        = (OpenglGetUniformLocation)PlatformGetGlFunction("glGetUniformLocation");
      GL->Initialized               &= GL->GetUniformLocation != 0;

      GL->GetShaderiv               = (OpenglGetShaderiv)PlatformGetGlFunction("glGetShaderiv");
      GL->Initialized               &= GL->GetShaderiv != 0;

      GL->GetProgramiv              = (OpenglGetProgramiv)PlatformGetGlFunction("glGetProgramiv");
      GL->Initialized               &= GL->GetProgramiv != 0;

      GL->AttachShader              = (OpenglAttachShader)PlatformGetGlFunction("glAttachShader");
      GL->Initialized               &= GL->AttachShader != 0;

      GL->BindBuffer                = (OpenglBindBuffer)PlatformGetGlFunction("glBindBuffer");
      GL->Initialized               &= GL->BindBuffer != 0;

      GL->BindBufferBase            = (OpenglBindBufferBase)PlatformGetGlFunction("glBindBufferBase");
      GL->Initialized               &= GL->BindBufferBase != 0;

      GL->BindVertexArray           = (OpenglBindVertexArray)PlatformGetGlFunction("glBindVertexArray");
      GL->Initialized               &= GL->BindVertexArray != 0;

      GL->DeleteVertexArrays             = (OpenglDeleteBuffers)PlatformGetGlFunction("glDeleteVertexArrays");
      GL->Initialized               &= GL->DeleteVertexArrays != 0;

      GL->DeleteBuffers             = (OpenglDeleteBuffers)PlatformGetGlFunction("glDeleteBuffers");
      GL->Initialized               &= GL->DeleteBuffers != 0;

      GL->GenBuffers                = (OpenglGenBuffers)PlatformGetGlFunction("glGenBuffers");
      GL->Initialized               &= GL->GenBuffers != 0;

      GL->GenVertexArrays           = (OpenglGenVertexArrays)PlatformGetGlFunction("glGenVertexArrays");
      GL->Initialized               &= GL->GenVertexArrays != 0;

      GL->BufferData                = (OpenglBufferData)PlatformGetGlFunction("glBufferData");
      GL->Initialized               &= GL->BufferData != 0;

      GL->BufferSubData             = (OpenglBufferSubData)PlatformGetGlFunction("glBufferSubData");
      GL->Initialized               &= GL->BufferSubData != 0;

      GL->MapBuffer                 = (OpenglMapBuffer)PlatformGetGlFunction("glMapBuffer");
      GL->Initialized               &= GL->MapBuffer != 0;

      GL->MapBufferRange            = (OpenglMapBufferRange)PlatformGetGlFunction("glMapBufferRange");
      GL->Initialized               &= GL->MapBufferRange != 0;

      GL->UnmapBuffer               = (OpenglUnmapBuffer)PlatformGetGlFunction("glUnmapBuffer");
      GL->Initialized               &= GL->UnmapBuffer != 0;

      GL->DrawBuffers               = (OpenglDrawBuffers)PlatformGetGlFunction("glDrawBuffers");
      GL->Initialized               &= GL->DrawBuffers != 0;

      GL->GetIntegerv               = (OpenglGetIntegerv)PlatformGetGlFunction("glGetIntegerv");
      GL->Initialized               &= GL->GetIntegerv != 0;

      GL->Finish                    = (OpenglFinish)PlatformGetGlFunction("glFinish");
      GL->Initialized               &= GL->Finish != 0;

      GL->GenQueries                = (OpenglGenQueries)PlatformGetGlFunction("glGenQueries");
      GL->Initialized               &= GL->GenQueries != 0;

      GL->DeleteQueries                = (OpenglDeleteQueries)PlatformGetGlFunction("glDeleteQueries");
      GL->Initialized               &= GL->DeleteQueries != 0;

      GL->BeginQuery                = (OpenglBeginQuery)PlatformGetGlFunction("glBeginQuery");
      GL->Initialized               &= GL->BeginQuery != 0;

      GL->EndQuery                  = (OpenglEndQuery)PlatformGetGlFunction("glEndQuery");
      GL->Initialized               &= GL->EndQuery != 0;


      GL->FenceSync                 = (OpenglFenceSync)PlatformGetGlFunction("glFenceSync");
      GL->Initialized               &= GL->FenceSync != 0;

      GL->DeleteSync                = (OpenglDeleteSync)PlatformGetGlFunction("glDeleteSync");
      GL->Initialized               &= GL->DeleteSync != 0;

      /* GL->WaitSync                  = (OpenglWaitSync)PlatformGetGlFunction("glWaitSync"); */
      /* GL->Initialized               &= GL->WaitSync != 0; */

      GL->ClientWaitSync            = (OpenglClientWaitSync)PlatformGetGlFunction("glClientWaitSync");
      GL->Initialized               &= GL->ClientWaitSync != 0;




      GL->GetQueryObjectiv          = (OpenglGetQueryObjectiv)PlatformGetGlFunction("glGetQueryObjectiv");
      GL->Initialized               &= GL->GetQueryObjectiv != 0;

      GL->GetQueryObjectuiv         = (OpenglGetQueryObjectuiv)PlatformGetGlFunction("glGetQueryObjectuiv");
      GL->Initialized               &= GL->GetQueryObjectuiv != 0;

      GL->GetQueryObjecti64v        = (OpenglGetQueryObjecti64v)PlatformGetGlFunction("glGetQueryObjecti64v");
      GL->Initialized               &= GL->GetQueryObjecti64v != 0;

      GL->GetQueryObjectui64v       = (OpenglGetQueryObjectui64v)PlatformGetGlFunction("glGetQueryObjectui64v");
      GL->Initialized               &= GL->GetQueryObjectui64v != 0;

      GL->GenerateMipmap            = (OpenglGenerateMipmap)PlatformGetGlFunction("glGenerateMipmap");
      GL->Initialized               &= GL->GenerateMipmap != 0;

      GL->GetIntegerv(GL_MAJOR_VERSION, &GLMajor);
      GL->GetIntegerv(GL_MINOR_VERSION, &GLMinor);
      GL->GetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &GL->MaxTextureBufferTexels);
      GL->GetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS_ARB, &GL->MaxFragmentTextureUnits);

      if (GLMajor >= 4)
      {
        GL->BlendFunci                = (OpenglBlendFunci)PlatformGetGlFunction("glBlendFunci");
        GL->Initialized              &= GL->BlendFunci != 0;
      }

    }
    else
    {
      Warn("Invalid Opengl Driver Info : (%s) (%s) (%s)", Vendor, Renderer, Version);
      GL->Initialized = False;
    }
  }


  if (GL->Initialized)
  {
    GL->Enable(GL_DEPTH_TEST);

    GL->DepthFunc(GL_LEQUAL);

    AssertNoGlErrors;
  }

  if (GL->Initialized && CheckOpenglVersion(GLMajor, GLMinor) == False)
  {
    Warn("Unsupported Version of Opengl (%d.%d) ::  Minimum 3.3 required.", GLMajor, GLMinor);
    Warn("The driver successfully supplied all required function pointers, however your program may not run correctly.");
    Warn("If you experience issues, please upgrade to an OpenGL 3.3 compliant driver.");
  }

  AssertNoGlErrors;

  // TODO(Jesse): Should we actually not just use a local for this .. it seems
  // weird to toggle it to true then false afterwards ..
  b32 Result = GL->Initialized;
  return Result;
}

void
HandleGlDebugMessage(GLenum Source,
                     GLenum Type,
                     GLuint Id,
                     GLenum Severity,
                     GLsizei MessageLength,
                     const GLchar* Message,
                     const void* UserData)
{
  /* if (Severity != GL_DEBUG_SEVERITY_NOTIFICATION) */
  {

    /* GLInfo("%s", Message); */

    const char* MessageTypeName = 0;
    switch(Type) {
      case(GL_DEBUG_TYPE_ERROR):
      {
        MessageTypeName = "ERROR";
      } break;
      case(GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR):
      {
        MessageTypeName = "DEPRECATED_BEHAVIOR";
      } break;
      case(GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR):
      {
        MessageTypeName = "UNDEFINED_BEHAVIOR";
      } break;
      case(GL_DEBUG_TYPE_PORTABILITY):
      {
        MessageTypeName = "PORTABILITY";
      } break;
      case(GL_DEBUG_TYPE_PERFORMANCE):
      {
        MessageTypeName = "PERFORMANCE";
      } break;
      case(GL_DEBUG_TYPE_OTHER):
      {
        MessageTypeName = "OTHER";
      } break;
      InvalidDefaultCase;
    }

    OpenGlDebugMessage("Source %u, Type: %s, Id %u - %.*s", Source, MessageTypeName, Id, MessageLength, Message);
    if (UserData)
    {
      OpenGlDebugMessage("User Data At %p", UserData);
    }
  }

  return;
}

inline b32
BufferFloatDataToCard(u32 BufferId, u32 Stride, u32 ByteCount, void *Data, u32 *AttributeIndex)
{
#if BONSAI_DEBUG_SYSTEM_API
  debug_state *DebugState = GetDebugState();
  DebugState->BytesBufferedToCard += ByteCount;
#endif

  GetGL()->BindBuffer(GL_ARRAY_BUFFER, BufferId);
  AssertNoGlErrors;
  b32 BufferUnmapped = GetGL()->UnmapBuffer(GL_ARRAY_BUFFER);
  /* GetGL()->BufferData(GL_ARRAY_BUFFER, ByteCount, Data, GL_STATIC_DRAW); */
  AssertNoGlErrors;

  GetGL()->EnableVertexAttribArray(*AttributeIndex);
  AssertNoGlErrors;
  GetGL()->VertexAttribPointer(*AttributeIndex, (s32)Stride, GL_FLOAT, GL_FALSE, 0, (void*)0);
  *AttributeIndex += 1;
  AssertNoGlErrors;

  return BufferUnmapped;
}

template <typename T> inline b32
BufferVertsToCard(u32 BufferId, T *Mesh, u32 *AttributeIndex)
{
  TIMED_FUNCTION();
  u32 ByteCount = Mesh->At*sizeof(*Mesh->Verts);
  u32 Stride = sizeof(*Mesh->Verts)/sizeof(Mesh->Verts[0].E[0]);

  b32 Result = BufferFloatDataToCard(BufferId, Stride, ByteCount, (void*)Mesh->Verts, AttributeIndex);
  return Result;
}

#if 1
template <typename T> inline b32
BufferColorsToCard(u32 BufferId, T *Mesh, u32* AttributeIndex)
{
  TIMED_FUNCTION();
  u32 Stride = sizeof(*Mesh->Colors)/sizeof(Mesh->Colors[0].E[0]);
  u32 ByteCount = Mesh->At*sizeof(*Mesh->Colors);

  b32 Result = BufferFloatDataToCard(BufferId, Stride, ByteCount, (void*)Mesh->Colors, AttributeIndex);
  return Result;
}
#endif

template <typename T> inline b32
BufferNormalsToCard(u32 BufferId, T *Mesh, u32 *AttributeIndex)
{
  TIMED_FUNCTION();
  u32 Stride = sizeof(*Mesh->Normals)/sizeof(Mesh->Normals[0].E[0]);
  u32 ByteCount = Mesh->At*sizeof(*Mesh->Normals);

  b32 Result = BufferFloatDataToCard(BufferId, Stride, ByteCount, (void*)Mesh->Normals, AttributeIndex);
  return Result;
}

template <typename T> inline b32
BufferUVsToCard(u32 BufferId, T *Mesh, u32 *AttributeIndex)
{
  TIMED_FUNCTION();
  u32 ByteCount = Mesh->At*sizeof(*Mesh->UVs);
  u32 Stride = sizeof(*Mesh->UVs)/sizeof(Mesh->UVs[0].x);

  b32 Result = BufferFloatDataToCard(BufferId, Stride, ByteCount, (void*)Mesh->UVs, AttributeIndex);
  return Result;
}
