
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
  GetStdlib()->GL.DrawArrays(GL_TRIANGLES, 0, s32(VertexCount) );
  AssertNoGlErrors;
}

inline void
SetDefaultFramebufferClearColors()
{
  GetStdlib()->GL.ClearDepth(1.0); // NOTE(Jesse): Depth 1.0 is the furthest from the camera
  GetStdlib()->GL.ClearColor(0.f, 0.f, 0.f, 1.f);
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
  GetStdlib()->GL.Viewport(0, 0, Dim.x, Dim.y);
}

inline void
SetViewport(v2 Dim)
{
  GetStdlib()->GL.Viewport(0, 0, (s32)Dim.x, (s32)Dim.y);
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

  Assert(GetStdlib()->GL.Initialized == False);
  GetStdlib()->GL.Initialized = True;

  GetStdlib()->GL.GetString                = (OpenglGetString)PlatformGetGlFunction("glGetString");
  GetStdlib()->GL.Initialized              &= GetStdlib()->GL.GetString != 0;

  s32 GLMajor = -1;
  s32 GLMinor = -1;

  if (GetStdlib()->GL.GetString)
  {
    char* Vendor   = (char*)GetStdlib()->GL.GetString(GL_VENDOR);
    char* Renderer = (char*)GetStdlib()->GL.GetString(GL_RENDERER);
    char* Version  = (char*)GetStdlib()->GL.GetString(GL_VERSION);
    if (Vendor && Renderer && Version)
    {
      Info("Opengl Driver Info : (%s) (%s) (%s)", Vendor, Renderer, Version);

      GetStdlib()->GL.GetError                  = (OpenglGetError)PlatformGetGlFunction("glGetError");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.GetError != 0;

      GetStdlib()->GL.Enable                    = (OpenglEnable)PlatformGetGlFunction("glEnable");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.Enable != 0;

      GetStdlib()->GL.Disable                   = (OpenglDisable)PlatformGetGlFunction("glDisable");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.Disable != 0;

      GetStdlib()->GL.CullFace                  = (OpenglCullFace)PlatformGetGlFunction("glCullFace");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.CullFace != 0;

      GetStdlib()->GL.Viewport                  = (OpenglViewport)PlatformGetGlFunction("glViewport");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.Viewport != 0;

      GetStdlib()->GL.DepthFunc                 = (OpenglDepthFunc)PlatformGetGlFunction("glDepthFunc");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.DepthFunc != 0;

      GetStdlib()->GL.BlendFunc                 = (OpenglBlendFunc)PlatformGetGlFunction("glBlendFunc");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.BlendFunc != 0;

      GetStdlib()->GL.DrawArrays                = (OpenglDrawArrays)PlatformGetGlFunction("glDrawArrays");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.DrawArrays != 0;

      GetStdlib()->GL.Clear                     = (OpenglClear)PlatformGetGlFunction("glClear");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.Clear != 0;

      GetStdlib()->GL.ClearColor                = (OpenglClearColor)PlatformGetGlFunction("glClearColor");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.ClearColor != 0;

      GetStdlib()->GL.ClearDepth                = (OpenglClearDepth)PlatformGetGlFunction("glClearDepth");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.ClearDepth != 0;

      GetStdlib()->GL.GenTextures               = (OpenglGenTextures)PlatformGetGlFunction("glGenTextures");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.GenTextures != 0;

      GetStdlib()->GL.BindTextures              = (OpenglBindTextures)PlatformGetGlFunction("glBindTextures");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.BindTextures != 0;

      GetStdlib()->GL.BindTexture               = (OpenglBindTexture)PlatformGetGlFunction("glBindTexture");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.BindTexture != 0;

      GetStdlib()->GL.DeleteTextures            = (OpenglDeleteTextures)PlatformGetGlFunction("glDeleteTextures");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.DeleteTextures != 0;

      GetStdlib()->GL.ActiveTexture             = (OpenglActiveTexture)PlatformGetGlFunction("glActiveTexture");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.ActiveTexture != 0;

      GetStdlib()->GL.GetTexImage               = (OpenglGetTexImage)PlatformGetGlFunction("glGetTexImage");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.GetTexImage != 0;

      GetStdlib()->GL.ReadPixels                = (OpenglReadPixels)PlatformGetGlFunction("glReadPixels");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.ReadPixels != 0;

      GetStdlib()->GL.TexStorage1D              = (OpenglTexStorage1D)PlatformGetGlFunction("glTexStorage1D");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.TexStorage1D != 0;

      GetStdlib()->GL.TexStorage2D              = (OpenglTexStorage2D)PlatformGetGlFunction("glTexStorage2D");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.TexStorage2D != 0;

      GetStdlib()->GL.TexStorage3D              = (OpenglTexStorage3D)PlatformGetGlFunction("glTexStorage3D");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.TexStorage3D != 0;

      GetStdlib()->GL.TexImage1D                = (OpenglTexImage1D)PlatformGetGlFunction("glTexImage1D");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.TexImage1D != 0;

      GetStdlib()->GL.TexImage2D                = (OpenglTexImage2D)PlatformGetGlFunction("glTexImage2D");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.TexImage2D != 0;

      GetStdlib()->GL.TexImage3D                = (OpenglTexImage3D)PlatformGetGlFunction("glTexImage3D");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.TexImage3D != 0;

      GetStdlib()->GL.TexSubImage1D             = (OpenglTexSubImage1D)PlatformGetGlFunction("glTexSubImage1D");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.TexSubImage1D != 0;

      GetStdlib()->GL.TexSubImage2D             = (OpenglTexSubImage2D)PlatformGetGlFunction("glTexSubImage2D");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.TexSubImage2D != 0;

      GetStdlib()->GL.TexSubImage3D             = (OpenglTexSubImage3D)PlatformGetGlFunction("glTexSubImage3D");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.TexSubImage3D != 0;

      GetStdlib()->GL.TexParameterf             = (OpenglTexParameterf)PlatformGetGlFunction("glTexParameterf");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.TexParameterf != 0;

      GetStdlib()->GL.TexParameterfv            = (OpenglTexParameterfv)PlatformGetGlFunction("glTexParameterfv");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.TexParameterfv != 0;

      GetStdlib()->GL.TexParameteri             = (OpenglTexParameteri)PlatformGetGlFunction("glTexParameteri");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.TexParameteri != 0;

      GetStdlib()->GL.TexParameteriv            = (OpenglTexParameteriv)PlatformGetGlFunction("glTexParameteriv");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.TexParameteriv != 0;

      GetStdlib()->GL.CompressedTexImage3D      = (OpenglCompressedTexImage3D)PlatformGetGlFunction("glCompressedTexImage3D");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.CompressedTexImage3D != 0;

      GetStdlib()->GL.CompressedTexImage2D      = (OpenglCompressedTexImage2D)PlatformGetGlFunction("glCompressedTexImage2D");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.CompressedTexImage2D != 0;

      GetStdlib()->GL.CompressedTexImage1D      = (OpenglCompressedTexImage1D)PlatformGetGlFunction("glCompressedTexImage1D");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.CompressedTexImage1D != 0;

      GetStdlib()->GL.PixelStoref               = (OpenglPixelStoref)PlatformGetGlFunction("glPixelStoref");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.PixelStoref != 0;

      GetStdlib()->GL.PixelStorei               = (OpenglPixelStorei)PlatformGetGlFunction("glPixelStorei");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.PixelStorei != 0;

      GetStdlib()->GL.EnableVertexAttribArray   = (OpenglEnableVertexAttribArray)PlatformGetGlFunction("glEnableVertexAttribArray");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.EnableVertexAttribArray != 0;

      GetStdlib()->GL.DisableVertexAttribArray  = (OpenglDisableVertexAttribArray)PlatformGetGlFunction("glDisableVertexAttribArray");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.DisableVertexAttribArray != 0;

      GetStdlib()->GL.VertexAttribPointer       = (OpenglVertexAttribPointer)PlatformGetGlFunction("glVertexAttribPointer");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.VertexAttribPointer != 0;

      GetStdlib()->GL.VertexAttribIPointer       = (OpenglVertexAttribIPointer)PlatformGetGlFunction("glVertexAttribIPointer");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.VertexAttribPointer != 0;

      GetStdlib()->GL.BindFramebuffer           = (OpenglBindFramebuffer)PlatformGetGlFunction("glBindFramebuffer");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.BindFramebuffer != 0;

      GetStdlib()->GL.DeleteFramebuffers        = (OpenglDeleteFramebuffers)PlatformGetGlFunction("glDeleteFramebuffers");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.DeleteFramebuffers != 0;

      GetStdlib()->GL.GenFramebuffers           = (OpenglGenFramebuffers)PlatformGetGlFunction("glGenFramebuffers");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.GenFramebuffers != 0;

      GetStdlib()->GL.CheckFramebufferStatus    = (OpenglCheckFramebufferStatus)PlatformGetGlFunction("glCheckFramebufferStatus");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.CheckFramebufferStatus != 0;

      GetStdlib()->GL.FramebufferTexture1D      = (OpenglFramebufferTexture1D)PlatformGetGlFunction("glFramebufferTexture1D");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.FramebufferTexture1D != 0;

      GetStdlib()->GL.FramebufferTexture2D      = (OpenglFramebufferTexture2D)PlatformGetGlFunction("glFramebufferTexture2D");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.FramebufferTexture2D != 0;

      GetStdlib()->GL.FramebufferTexture3D      = (OpenglFramebufferTexture3D)PlatformGetGlFunction("glFramebufferTexture3D");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.FramebufferTexture3D != 0;

      GetStdlib()->GL.FramebufferTextureLayer   = (OpenglFramebufferTextureLayer)PlatformGetGlFunction("glFramebufferTextureLayer");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.FramebufferTextureLayer != 0;

      GetStdlib()->GL.FramebufferRenderbuffer   = (OpenglFramebufferRenderbuffer)PlatformGetGlFunction("glFramebufferRenderbuffer");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.FramebufferRenderbuffer != 0;

      GetStdlib()->GL.CompileShader             = (OpenglCompileShader)PlatformGetGlFunction("glCompileShader");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.CompileShader != 0;

      GetStdlib()->GL.CreateProgram             = (OpenglCreateProgram)PlatformGetGlFunction("glCreateProgram");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.CreateProgram != 0;

      GetStdlib()->GL.CreateShader              = (OpenglCreateShader)PlatformGetGlFunction("glCreateShader");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.CreateShader != 0;

      GetStdlib()->GL.LinkProgram               = (OpenglLinkProgram)PlatformGetGlFunction("glLinkProgram");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.LinkProgram != 0;

      GetStdlib()->GL.ShaderSource              = (OpenglShaderSource)PlatformGetGlFunction("glShaderSource");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.ShaderSource != 0;

      GetStdlib()->GL.GetShaderSource           = (OpenglGetShaderSource)PlatformGetGlFunction("glGetShaderSource");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.GetShaderSource != 0;

      GetStdlib()->GL.UseProgram                = (OpenglUseProgram)PlatformGetGlFunction("glUseProgram");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.UseProgram != 0;

      GetStdlib()->GL.GetProgramInfoLog         = (OpenglGetProgramInfoLog)PlatformGetGlFunction("glGetProgramInfoLog");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.GetProgramInfoLog != 0;

      GetStdlib()->GL.GetShaderInfoLog          = (OpenglGetShaderInfoLog)PlatformGetGlFunction("glGetShaderInfoLog");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.GetShaderInfoLog != 0;

      GetStdlib()->GL.DeleteProgram             = (OpenglDeleteProgram)PlatformGetGlFunction("glDeleteProgram");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.DeleteProgram != 0;

      GetStdlib()->GL.DeleteShader              = (OpenglDeleteShader)PlatformGetGlFunction("glDeleteShader");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.DeleteShader != 0;

      GetStdlib()->GL.DetachShader              = (OpenglDetachShader)PlatformGetGlFunction("glDetachShader");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.DetachShader != 0;

      GetStdlib()->GL.Uniform1f                 = (OpenglUniform1f)PlatformGetGlFunction("glUniform1f");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.Uniform1f != 0;

      GetStdlib()->GL.Uniform2f                 = (OpenglUniform2f)PlatformGetGlFunction("glUniform2f");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.Uniform2f != 0;

      GetStdlib()->GL.Uniform3f                 = (OpenglUniform3f)PlatformGetGlFunction("glUniform3f");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.Uniform3f != 0;

      GetStdlib()->GL.Uniform4f                 = (OpenglUniform4f)PlatformGetGlFunction("glUniform4f");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.Uniform4f != 0;

      GetStdlib()->GL.Uniform1i                 = (OpenglUniform1i)PlatformGetGlFunction("glUniform1i");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.Uniform1i != 0;

      GetStdlib()->GL.Uniform2i                 = (OpenglUniform2i)PlatformGetGlFunction("glUniform2i");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.Uniform2i != 0;

      GetStdlib()->GL.Uniform3i                 = (OpenglUniform3i)PlatformGetGlFunction("glUniform3i");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.Uniform3i != 0;

      GetStdlib()->GL.Uniform4i                 = (OpenglUniform4i)PlatformGetGlFunction("glUniform4i");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.Uniform4i != 0;

      GetStdlib()->GL.Uniform1fv                = (OpenglUniform1fv)PlatformGetGlFunction("glUniform1fv");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.Uniform1fv != 0;

      GetStdlib()->GL.Uniform2fv                = (OpenglUniform2fv)PlatformGetGlFunction("glUniform2fv");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.Uniform2fv != 0;

      GetStdlib()->GL.Uniform3fv                = (OpenglUniform3fv)PlatformGetGlFunction("glUniform3fv");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.Uniform3fv != 0;

      GetStdlib()->GL.Uniform4fv                = (OpenglUniform4fv)PlatformGetGlFunction("glUniform4fv");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.Uniform4fv != 0;

      GetStdlib()->GL.Uniform1iv                = (OpenglUniform1iv)PlatformGetGlFunction("glUniform1iv");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.Uniform1iv != 0;

      GetStdlib()->GL.Uniform2iv                = (OpenglUniform2iv)PlatformGetGlFunction("glUniform2iv");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.Uniform2iv != 0;

      GetStdlib()->GL.Uniform3iv                = (OpenglUniform3iv)PlatformGetGlFunction("glUniform3iv");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.Uniform3iv != 0;

      GetStdlib()->GL.Uniform4iv                = (OpenglUniform4iv)PlatformGetGlFunction("glUniform4iv");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.Uniform4iv != 0;

      GetStdlib()->GL.UniformMatrix2fv          = (OpenglUniformMatrix2fv)PlatformGetGlFunction("glUniformMatrix2fv");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.UniformMatrix2fv != 0;

      GetStdlib()->GL.UniformMatrix3fv          = (OpenglUniformMatrix3fv)PlatformGetGlFunction("glUniformMatrix3fv");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.UniformMatrix3fv != 0;

      GetStdlib()->GL.UniformMatrix4fv          = (OpenglUniformMatrix4fv)PlatformGetGlFunction("glUniformMatrix4fv");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.UniformMatrix4fv != 0;

      GetStdlib()->GL.Uniform1ui                = (OpenglUniform1ui)PlatformGetGlFunction("glUniform1ui");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.Uniform1ui != 0;

      GetStdlib()->GL.Uniform2ui                = (OpenglUniform2ui)PlatformGetGlFunction("glUniform2ui");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.Uniform2ui != 0;

      GetStdlib()->GL.Uniform3ui                = (OpenglUniform3ui)PlatformGetGlFunction("glUniform3ui");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.Uniform3ui != 0;

      GetStdlib()->GL.Uniform4ui                = (OpenglUniform4ui)PlatformGetGlFunction("glUniform4ui");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.Uniform4ui != 0;

      GetStdlib()->GL.Uniform1uiv               = (OpenglUniform1uiv)PlatformGetGlFunction("glUniform1uiv");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.Uniform1uiv != 0;

      GetStdlib()->GL.Uniform2uiv               = (OpenglUniform2uiv)PlatformGetGlFunction("glUniform2uiv");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.Uniform2uiv != 0;

      GetStdlib()->GL.Uniform3uiv               = (OpenglUniform3uiv)PlatformGetGlFunction("glUniform3uiv");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.Uniform3uiv != 0;

      GetStdlib()->GL.Uniform4uiv               = (OpenglUniform4uiv)PlatformGetGlFunction("glUniform4uiv");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.Uniform4uiv != 0;

      GetStdlib()->GL.GetUniformLocation        = (OpenglGetUniformLocation)PlatformGetGlFunction("glGetUniformLocation");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.GetUniformLocation != 0;

      GetStdlib()->GL.GetShaderiv               = (OpenglGetShaderiv)PlatformGetGlFunction("glGetShaderiv");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.GetShaderiv != 0;

      GetStdlib()->GL.GetProgramiv              = (OpenglGetProgramiv)PlatformGetGlFunction("glGetProgramiv");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.GetProgramiv != 0;

      GetStdlib()->GL.AttachShader              = (OpenglAttachShader)PlatformGetGlFunction("glAttachShader");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.AttachShader != 0;

      GetStdlib()->GL.BindBuffer                = (OpenglBindBuffer)PlatformGetGlFunction("glBindBuffer");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.BindBuffer != 0;

      GetStdlib()->GL.BindVertexArray           = (OpenglBindVertexArray)PlatformGetGlFunction("glBindVertexArray");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.BindVertexArray != 0;

      GetStdlib()->GL.DeleteBuffers             = (OpenglDeleteBuffers)PlatformGetGlFunction("glDeleteBuffers");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.DeleteBuffers != 0;

      GetStdlib()->GL.GenBuffers                = (OpenglGenBuffers)PlatformGetGlFunction("glGenBuffers");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.GenBuffers != 0;

      GetStdlib()->GL.GenVertexArrays           = (OpenglGenVertexArrays)PlatformGetGlFunction("glGenVertexArrays");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.GenVertexArrays != 0;

      GetStdlib()->GL.BufferData                = (OpenglBufferData)PlatformGetGlFunction("glBufferData");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.BufferData != 0;

      GetStdlib()->GL.MapBuffer                 = (OpenglMapBuffer)PlatformGetGlFunction("glMapBuffer");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.MapBuffer != 0;

      GetStdlib()->GL.MapBufferRange            = (OpenglMapBufferRange)PlatformGetGlFunction("glMapBufferRange");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.MapBufferRange != 0;

      GetStdlib()->GL.UnmapBuffer               = (OpenglUnmapBuffer)PlatformGetGlFunction("glUnmapBuffer");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.UnmapBuffer != 0;

      GetStdlib()->GL.DrawBuffers               = (OpenglDrawBuffers)PlatformGetGlFunction("glDrawBuffers");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.DrawBuffers != 0;

      GetStdlib()->GL.GetIntegerv               = (OpenglGetIntegerv)PlatformGetGlFunction("glGetIntegerv");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.GetIntegerv != 0;

      GetStdlib()->GL.DebugMessageCallback      = (OpenglDebugMessageCallback)PlatformGetGlFunction("glDebugMessageCallback");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.DebugMessageCallback != 0;

      GetStdlib()->GL.Finish                    = (OpenglFinish)PlatformGetGlFunction("glFinish");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.Finish != 0;

      GetStdlib()->GL.GenQueries                = (OpenglGenQueries)PlatformGetGlFunction("glGenQueries");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.GenQueries != 0;

      GetStdlib()->GL.DeleteQueries                = (OpenglDeleteQueries)PlatformGetGlFunction("glDeleteQueries");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.DeleteQueries != 0;

      GetStdlib()->GL.BeginQuery                = (OpenglBeginQuery)PlatformGetGlFunction("glBeginQuery");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.BeginQuery != 0;

      GetStdlib()->GL.EndQuery                  = (OpenglEndQuery)PlatformGetGlFunction("glEndQuery");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.EndQuery != 0;


      GetStdlib()->GL.FenceSync                 = (OpenglFenceSync)PlatformGetGlFunction("glFenceSync");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.FenceSync != 0;

      GetStdlib()->GL.DeleteSync                = (OpenglDeleteSync)PlatformGetGlFunction("glDeleteSync");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.DeleteSync != 0;

      /* GetStdlib()->GL.WaitSync                  = (OpenglWaitSync)PlatformGetGlFunction("glWaitSync"); */
      /* GetStdlib()->GL.Initialized               &= GetStdlib()->GL.WaitSync != 0; */

      GetStdlib()->GL.ClientWaitSync            = (OpenglClientWaitSync)PlatformGetGlFunction("glClientWaitSync");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.ClientWaitSync != 0;




      GetStdlib()->GL.GetQueryObjectiv          = (OpenglGetQueryObjectiv)PlatformGetGlFunction("glGetQueryObjectiv");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.GetQueryObjectiv != 0;

      GetStdlib()->GL.GetQueryObjectuiv         = (OpenglGetQueryObjectuiv)PlatformGetGlFunction("glGetQueryObjectuiv");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.GetQueryObjectuiv != 0;

      GetStdlib()->GL.GetQueryObjecti64v        = (OpenglGetQueryObjecti64v)PlatformGetGlFunction("glGetQueryObjecti64v");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.GetQueryObjecti64v != 0;

      GetStdlib()->GL.GetQueryObjectui64v       = (OpenglGetQueryObjectui64v)PlatformGetGlFunction("glGetQueryObjectui64v");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.GetQueryObjectui64v != 0;

      GetStdlib()->GL.GetQueryBufferObjectiv    = (OpenglGetQueryBufferObjectiv)PlatformGetGlFunction("glGetQueryBufferObjectiv");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.GetQueryBufferObjectiv != 0;

      GetStdlib()->GL.GetQueryBufferObjectuiv   = (OpenglGetQueryBufferObjectuiv)PlatformGetGlFunction("glGetQueryBufferObjectuiv");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.GetQueryBufferObjectuiv != 0;

      GetStdlib()->GL.GetQueryBufferObjecti64v  = (OpenglGetQueryBufferObjecti64v)PlatformGetGlFunction("glGetQueryBufferObjecti64v");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.GetQueryBufferObjecti64v != 0;

      GetStdlib()->GL.GetQueryBufferObjectui64v = (OpenglGetQueryBufferObjectui64v)PlatformGetGlFunction("glGetQueryBufferObjectui64v");
      GetStdlib()->GL.Initialized               &= GetStdlib()->GL.GetQueryBufferObjectui64v != 0;



      GetStdlib()->GL.GetIntegerv(GL_MAJOR_VERSION, &GLMajor);
      GetStdlib()->GL.GetIntegerv(GL_MINOR_VERSION, &GLMinor);

      if (GLMajor >= 4)
      {
        GetStdlib()->GL.BlendFunci                = (OpenglBlendFunci)PlatformGetGlFunction("glBlendFunci");
        GetStdlib()->GL.Initialized              &= GetStdlib()->GL.BlendFunci != 0;
      }

    }
    else
    {
      Warn("Invalid Opengl Driver Info : (%s) (%s) (%s)", Vendor, Renderer, Version);
      GetStdlib()->GL.Initialized = False;
    }
  }


  if (GetStdlib()->GL.Initialized)
  {
    /* GetStdlib()->GL.DebugMessageCallback(HandleGlDebugMessage, 0); */

    GetStdlib()->GL.Enable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    GetStdlib()->GL.Enable(GL_DEPTH_TEST);

    GetStdlib()->GL.DepthFunc(GL_LEQUAL);

    AssertNoGlErrors;
  }

  if (GetStdlib()->GL.Initialized && CheckOpenglVersion(GLMajor, GLMinor) == False)
  {
    Warn("Unsupported Version of Opengl (%d.%d) ::  Minimum 3.3 required.", GLMajor, GLMinor);
    Warn("The driver successfully supplied all required function pointers, however your program may not run correctly.");
    Warn("If you experience issues, please upgrade to an OpenGL 3.3 compliant driver.");
  }

  b32 Result = GetStdlib()->GL.Initialized;
  return Result;
}

void
HandleGlDebugMessage(GLenum Source, GLenum Type, GLuint Id, GLenum Severity,
                     GLsizei MessageLength, const GLchar* Message, const void* UserData)
{
  if (Severity != GL_DEBUG_SEVERITY_NOTIFICATION)
  {

    DebugLine("%s", Message);
    RuntimeBreak();
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

inline void
BufferFloatDataToCard(u32 BufferId, u32 Stride, u32 ByteCount, void *Data, u32 *AttributeIndex)
{
#if BONSAI_DEBUG_SYSTEM_API
  debug_state *DebugState = GetDebugState();
  DebugState->BytesBufferedToCard += ByteCount;
#endif

  GetStdlib()->GL.BindBuffer(GL_ARRAY_BUFFER, BufferId);
  GetStdlib()->GL.BufferData(GL_ARRAY_BUFFER, ByteCount, Data, GL_STATIC_DRAW);

  GetStdlib()->GL.EnableVertexAttribArray(*AttributeIndex);
  GetStdlib()->GL.VertexAttribPointer(*AttributeIndex, (s32)Stride, GL_FLOAT, GL_FALSE, 0, (void*)0);
  *AttributeIndex += 1;
  AssertNoGlErrors;

  return;
}

template <typename T> inline void
BufferVertsToCard(u32 BufferId, T *Mesh, u32 *AttributeIndex)
{
  TIMED_FUNCTION();
  u32 ByteCount = Mesh->At*sizeof(*Mesh->Verts);
  u32 Stride = sizeof(*Mesh->Verts)/sizeof(Mesh->Verts[0].E[0]);

  BufferFloatDataToCard(BufferId, Stride, ByteCount, (void*)Mesh->Verts, AttributeIndex);

  return;
}

#if 1
template <typename T> inline void
BufferColorsToCard(u32 BufferId, T *Mesh, u32* AttributeIndex)
{
  TIMED_FUNCTION();
  u32 Stride = sizeof(*Mesh->Colors)/sizeof(Mesh->Colors[0].E[0]);
  u32 ByteCount = Mesh->At*sizeof(*Mesh->Colors);

  BufferFloatDataToCard(BufferId, Stride, ByteCount, (void*)Mesh->Colors, AttributeIndex);

  return;
}
#endif

template <typename T> inline void
BufferNormalsToCard(u32 BufferId, T *Mesh, u32 *AttributeIndex)
{
  TIMED_FUNCTION();
  u32 Stride = sizeof(*Mesh->Normals)/sizeof(Mesh->Normals[0].E[0]);
  u32 ByteCount = Mesh->At*sizeof(*Mesh->Normals);

  BufferFloatDataToCard(BufferId, Stride, ByteCount, (void*)Mesh->Normals, AttributeIndex);

  return;
}

template <typename T> inline void
BufferUVsToCard(u32 BufferId, T *Mesh, u32 *AttributeIndex)
{
  TIMED_FUNCTION();
  u32 ByteCount = Mesh->At*sizeof(*Mesh->UVs);
  u32 Stride = sizeof(*Mesh->UVs)/sizeof(Mesh->UVs[0].x);

  BufferFloatDataToCard(BufferId, Stride, ByteCount, (void*)Mesh->UVs, AttributeIndex);

  return;
}
