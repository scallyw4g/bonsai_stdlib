
poof(block_array_c(shader_ptr, {64}))
#include <generated/block_array_c_shader_ptr_688853971.h>

#define INVALID_SHADER_UNIFORM (-1)
#define INVALID_SHADER (u32)(-1)

link_internal u32
CompileShader(ansi_stream Header, ansi_stream Code, u32 Type)
{
  const int InfoLogLength = 0;

  u32 ShaderID = GL.CreateShader(Type);

  const char *Sources[2] = {Header.Start, Code.Start};
  const s32 Lengths[2] = {(s32)TotalSize(&Header), (s32)TotalSize(&Code)};


  // Compile
  GL.ShaderSource(ShaderID, 2, Sources, Lengths);
  GL.CompileShader(ShaderID);

  // Check Status
  s32 CompileSuccess = GL_FALSE;
  GL.GetShaderiv(ShaderID, GL_COMPILE_STATUS, &CompileSuccess);
  GL.GetShaderiv(ShaderID, GL_INFO_LOG_LENGTH, (s32*)&InfoLogLength);
  AssertNoGlErrors;

  // TODO(Jesse): We should probably return a flag from this function that indicates
  // the compilation failed..?  Or reinstate CheckShaderCompilationStatus?
  if (CompileSuccess == GL_FALSE)
  {
    char *ProgramErrorMessage = Allocate(char, GetTranArena(), InfoLogLength);
    s32 ActualLength = 0;
    GL.GetShaderInfoLog(ShaderID, InfoLogLength, &ActualLength, ProgramErrorMessage);
    if (ActualLength == 0)
    {
      SoftError("Compiling Shader : Unfortunately, the driver did not provide an error message.  Sorry, friend.");
    }
    else
    {
      SoftError("Compiling Shader : (%d)(%S)", InfoLogLength, CS(ProgramErrorMessage, umm(ActualLength)));
    }

    // NOTE(Jesse): Need this to get the actual error message later ... sigh ...
    /* GL.DeleteShader(ShaderID); */
    /* ShaderID = INVALID_SHADER; */
  }

  u32 Result = ShaderID;
  return Result;
}

#if 1
link_internal void
CheckShaderCompilationStatus(cs ShaderPath, u32 ShaderId)
{
  s32 Success = GL_FALSE;
  GL.GetShaderiv(ShaderId, GL_COMPILE_STATUS, &Success);

  if (Success == GL_FALSE)
  {
    s32 InfoLogLength;
    GL.GetProgramiv(ShaderId, GL_INFO_LOG_LENGTH, &InfoLogLength);
    // Apparently the log length includes the null terminator
    char *ProgramErrorMessage = Allocate(char, GetTranArena(), InfoLogLength);
    GL.GetShaderInfoLog(ShaderId, InfoLogLength, NULL, ProgramErrorMessage);

    SoftError("Error Compiling shader (%S)", ShaderPath);
    Error("%d (%s)", InfoLogLength, ProgramErrorMessage);
  }
  else
  {
    Assert(Success == GL_TRUE);
  }

}
#endif

global_variable ansi_stream Global_ShaderHeaderCode;

link_internal cs
ValueFromSetting(shader_language_setting ShaderLanguage)
{
  cs Result = CSz("## ERROR: Invalid ShaderLanguage setting ##");

  switch (ShaderLanguage)
  {
    case ShaderLanguageSetting_310es:
    {
      Result = CSz("#version 310 es\n\n");
    } break;

    case ShaderLanguageSetting_330core:
    {
      Result = CSz("#version 330 core\n\n");
    } break;
  }

  return Result;
}

link_internal void
LoadGlobalShaderHeaderCode(shader_language_setting ShaderLanguage)
{
  cs ShaderVersion = ValueFromSetting(ShaderLanguage);
  cs HeaderCode    = ReadEntireFileIntoString(CSz(STDLIB_SHADER_PATH "/header.glsl"), GetTranArena());
  Global_ShaderHeaderCode = AnsiStream(Concat(ShaderVersion, HeaderCode, GetThreadLocalState(ThreadLocal_ThreadIndex)->PermMemory));
}

link_internal shader
CompileShaderPair(cs VertShaderPath, cs FragShaderPath)
{
  Info("Creating shader : %S | %S", VertShaderPath, FragShaderPath);

  if (Global_ShaderHeaderCode.Start == 0) { LoadGlobalShaderHeaderCode(ShaderLanguageSetting_330core); } // Default to 330 core if nobody did this already

  ansi_stream HeaderCode       = Global_ShaderHeaderCode;
  ansi_stream VertexShaderCode = ReadEntireFileIntoAnsiStream(VertShaderPath, GetTranArena());
  ansi_stream FragShaderCode   = ReadEntireFileIntoAnsiStream(FragShaderPath, GetTranArena());

  int InfoLogLength;

  u32 VertexShaderID = CompileShader(HeaderCode, VertexShaderCode, GL_VERTEX_SHADER);
    AssertNoGlErrors;
  /* CheckShaderCompilationStatus(VertShaderPath, VertexShaderID); // NOTE(Jesse): This happens inline in CompileShader */

  u32 FragmentShaderID = CompileShader(HeaderCode, FragShaderCode, GL_FRAGMENT_SHADER);
    AssertNoGlErrors;
  /* CheckShaderCompilationStatus(FragShaderPath, FragmentShaderID); */

  memory_arena *PermMemory = GetThreadLocalState(ThreadLocal_ThreadIndex)->PermMemory;
  shader Shader = { INVALID_SHADER, 0, CopyString(VertShaderPath, PermMemory), CopyString(FragShaderPath, PermMemory), 0, 0, False};

  // NOTE(Jesse): Not doing this because the errors come through when you go to link the program.  Of course ..
  /* if (VertexShaderID != INVALID_SHADER && FragmentShaderID != INVALID_SHADER) */
  {
    // Link the program
    u32 ProgramID = GL.CreateProgram();
    Assert(ProgramID);
    GL.AttachShader(ProgramID, VertexShaderID);
    GL.AttachShader(ProgramID, FragmentShaderID);
    GL.LinkProgram(ProgramID);

    // Check the program linked
    s32 LinkResult = GL_FALSE;
    GL.GetProgramiv(ProgramID, GL_LINK_STATUS, &LinkResult);
    GL.GetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    GL.DetachShader(ProgramID, VertexShaderID);
    GL.DetachShader(ProgramID, FragmentShaderID);
    GL.DeleteShader(VertexShaderID);
    GL.DeleteShader(FragmentShaderID);

    if (LinkResult == GL_FALSE)
    {
      char *ProgramErrorMessage = Allocate(char, GetTranArena(), InfoLogLength+1);
      GL.GetProgramInfoLog(ProgramID, InfoLogLength, NULL, ProgramErrorMessage);
      SoftError("Linking shader pair %S | %S", VertShaderPath, FragShaderPath);
      SoftError("%s", ProgramErrorMessage);
      ProgramID = INVALID_SHADER;
    }

    Shader.ID = ProgramID;

    // NOTE(Jesse): Kind of a hack to set the last modified times on the shader.  Should
    // really be able to do this when reading the source code into buffers, but .. meh ..
    FileIsNew(GetNullTerminated(Shader.VertexSourceFilename), &Shader.VertexTimeModifiedWhenLoaded);
    FileIsNew(GetNullTerminated(Shader.FragSourceFilename), &Shader.FragmentTimeModifiedWhenLoaded);
  }

  AssertNoGlErrors;
  return Shader;
}

link_internal void
RegisterShaderForHotReload(bonsai_stdlib *Stdlib, shader *Shader)
{
  if (Stdlib->AllShaders.Memory == 0)
  {
    Stdlib->AllShaders.Memory = AllocateArena();
  }

  Push(&Stdlib->AllShaders, &Shader);
}

link_internal void
HotReloadShaders(bonsai_stdlib *Stdlib)
{
  IterateOver(&Stdlib->AllShaders, Shader, ShaderIndex)
  {
    Shader->HotReloaded = False;

    b32 VertIsNew = FileIsNew(GetNullTerminated(Shader->VertexSourceFilename), &Shader->VertexTimeModifiedWhenLoaded);
    b32 FragIsNew = FileIsNew(GetNullTerminated(Shader->FragSourceFilename),   &Shader->FragmentTimeModifiedWhenLoaded);
    if (VertIsNew || FragIsNew)
    {
      SleepMs(5);

      auto T1 = Shader->VertexTimeModifiedWhenLoaded;
      auto T2 = Shader->FragmentTimeModifiedWhenLoaded;
      shader LoadedShader = CompileShaderPair(Shader->VertexSourceFilename, Shader->FragSourceFilename);

      b32 RetryCount = 0;
      while (LoadedShader.ID == INVALID_SHADER)
      {
        SleepMs(5);
        LoadedShader = CompileShaderPair(Shader->VertexSourceFilename, Shader->FragSourceFilename);

        if (++RetryCount > 5) { break; } // If it doesn't work after 5 tries, it's probably a syntax error.
      }


      if (LoadedShader.ID != INVALID_SHADER)
      {
        GL.DeleteProgram(Shader->ID);
        *Shader = LoadedShader;
        Shader->HotReloaded = True;
      }
      AssertNoGlErrors;
    }
  }
}



s32
GetShaderUniform(shader *Shader, const char *Name)
{
  s32 Result = GL.GetUniformLocation(Shader->ID, Name);
  if (Result == INVALID_SHADER_UNIFORM)
  {
    Warn("Couldn't retreive %s shader uniform - was it optimized out?", Name);
  }

  return Result;
}

shader_uniform *
PushShaderUniform( memory_arena *Mem, const char *Name)
{
  shader_uniform *Uniform = Allocate(shader_uniform, Mem, 1);
  Uniform->Name = Name;
  return Uniform;
}


poof(gen_shader_uniform_push(texture));
#include <generated/gen_shader_uniform_push_texture.h>

poof(gen_shader_uniform_push(m4));
#include <generated/gen_shader_uniform_push_m4.h>

poof(gen_shader_uniform_push(v2));
#include <generated/gen_shader_uniform_push_v2.h>

poof(gen_shader_uniform_push(v3));
#include <generated/gen_shader_uniform_push_v3.h>

poof(gen_shader_uniform_push(u32));
#include <generated/gen_shader_uniform_push_u32.h>

poof(gen_shader_uniform_push(s32));
#include <generated/gen_shader_uniform_push_s32.h>

poof(gen_shader_uniform_push(r32));
#include <generated/gen_shader_uniform_push_r32.h>



shader
MakeSimpleTextureShader(texture *Texture, memory_arena *GraphicsMemory)
{
  shader Shader = CompileShaderPair( CSz(STDLIB_SHADER_PATH "Passthrough.vertexshader"), CSz(STDLIB_SHADER_PATH "SimpleTexture.fragmentshader") );

  shader_uniform **Current = &Shader.FirstUniform;

  *Current = GetUniform(GraphicsMemory, &Shader, Texture, "Texture");
  Current = &(*Current)->Next;

  AssertNoGlErrors;

  return Shader;
}

shader
MakeFullTextureShader(texture *Texture, memory_arena *GraphicsMemory)
{
  shader Shader = CompileShaderPair( CSz(STDLIB_SHADER_PATH "FullPassthrough.vertexshader"), CSz(STDLIB_SHADER_PATH "SimpleTexture.fragmentshader") );

  shader_uniform **Current = &Shader.FirstUniform;

  *Current = GetUniform(GraphicsMemory, &Shader, Texture, "Texture");
  Current = &(*Current)->Next;

  *Current = GetUniform(GraphicsMemory, &Shader, Texture, "TextureArray");
  Current = &(*Current)->Next;

  *Current = GetUniform(GraphicsMemory, &Shader, (b32*)0, "IsDepthTexture");
  Current = &(*Current)->Next;

  *Current = GetUniform(GraphicsMemory, &Shader, (b32*)0, "HasAlphaChannel");
  Current = &(*Current)->Next;

  AssertNoGlErrors;

  return Shader;
}

b32
CheckAndClearFramebuffer()
{
  u32 FramebufferStatus = GL.CheckFramebufferStatus(GL_FRAMEBUFFER);

  switch (FramebufferStatus)
  {
    case GL_FRAMEBUFFER_UNDEFINED:
    {
      SoftError("GL_FRAMEBUFFER_UNDEFINED");
    } break;

    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
    {
      SoftError("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT");
    } break;

    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
    {
      SoftError("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER");
    } break;

    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
    {
      SoftError("GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER");
    } break;

    case GL_FRAMEBUFFER_UNSUPPORTED:
    {
      SoftError("GL_FRAMEBUFFER_UNSUPPORTED");
    } break;

    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
    {
      SoftError("GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE");
    } break;

    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
    {
      SoftError("GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS");
    } break;

  }

  SetDefaultFramebufferClearColors();
  GL.Clear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  GL.BindFramebuffer(GL_FRAMEBUFFER, 0);

  b32 Result = (FramebufferStatus == GL_FRAMEBUFFER_COMPLETE);
  return Result;
}

// NOTE(Jesse): The engine defines this function such that we can bind
// engine-specific structs as uniforms, instead of using a pile of values.
// We're using `weak` magic here such that we can have this live in stdlib,
// but also support thunking through to engine code, if it's present.
link_weak void BindEngineUniform(shader_uniform*);


// TODO(Jesse): We should generate the set of these?
link_internal void
BindUniformByName(shader *Shader, const char *Name, s32 Value)
{
  s32 Uniform = GL.GetUniformLocation(Shader->ID, Name); 
  if (Uniform != INVALID_SHADER_UNIFORM)
  {
    GL.Uniform1i(Uniform, Value);
  }
  else
  {
    Warn("Couldn't retieve uniform %s", Name);
  }
}

link_internal void
BindUniformByName(shader *Shader, const char *Name, b32 Value)
{
  s32 Uniform = GL.GetUniformLocation(Shader->ID, Name); 
  if (Uniform != INVALID_SHADER_UNIFORM)
  {
    GL.Uniform1i(Uniform, s32(Value));
  }
  else
  {
    Warn("Couldn't retieve uniform %s", Name);
  }
}


link_internal void
BindUniformByName(shader *Shader, const char *Name, texture *Texture, u32 TextureUnit)
{
  GL.ActiveTexture(GL_TEXTURE0 + TextureUnit);
  s32 Uniform = GL.GetUniformLocation(Shader->ID, Name);
  if (Uniform != INVALID_SHADER_UNIFORM)
  {
    GL.Uniform1i(Uniform, s32(TextureUnit));
  }
  else
  {
    Warn("Couldn't retieve uniform %s", Name);
  }

  if (Texture->Slices > 1)
  {
    GL.BindTexture(GL_TEXTURE_2D_ARRAY, Texture->ID);
  }
  else
  {
    GL.BindTexture(GL_TEXTURE_2D, Texture->ID);
  }

  AssertNoGlErrors;
}

link_internal void
BindUniformByName(shader *Shader, const char *Name, r32 Value)
{
  s32 Uniform = GL.GetUniformLocation(Shader->ID, Name);
  if (Uniform != INVALID_SHADER_UNIFORM)
  {
    GL.Uniform1f(Uniform, Value);
  }
  else
  {
    Warn("Couldn't retieve uniform %s", Name);
  }
}

link_internal b32
TryBindUniform(shader *Shader, const char *Name, v2 *V)
{
  b32 Result = False;
  s32 Uniform = GL.GetUniformLocation(Shader->ID, Name);
  if (Uniform != INVALID_SHADER_UNIFORM)
  {
    GL.Uniform2fv(Uniform, 1, (r32*)V);
    Result = True;
  }
  return Result;
}

link_internal b32
TryBindUniform(shader *Shader, const char *Name, v3 *V)
{
  b32 Result = False;
  s32 Uniform = GL.GetUniformLocation(Shader->ID, Name);
  if (Uniform != INVALID_SHADER_UNIFORM)
  {
    GL.Uniform3fv(Uniform, 1, (r32*)V);
    Result = True;
  }
  return Result;
}

link_internal void
BindUniformByName(shader *Shader, const char *Name, v2 *V)
{
  if (TryBindUniform(Shader, Name, V) == False)
  {
    Warn("Couldn't retieve uniform %s", Name);
  }
}

link_internal void
BindUniformByName(shader *Shader, const char *Name, v3 *V)
{
  if (TryBindUniform(Shader, Name, V) == False)
  {
    Warn("Couldn't retieve uniform %s", Name);
  }
}

link_internal b32
TryBindUniform(shader *Shader, const char *Name, m4 *Matrix)
{
  b32 Result = False;
  s32 Uniform = GL.GetUniformLocation(Shader->ID, Name);
  if (Uniform != INVALID_SHADER_UNIFORM)
  {
    GL.UniformMatrix4fv(Uniform, 1, GL_FALSE, (r32*)Matrix);
    Result = True;
  }
  return Result;
}


link_internal void
BindUniformByName(shader *Shader, const char *Name, m4 *Matrix)
{
  if (TryBindUniform(Shader, Name, Matrix) == False)
  {
    Warn("Couldn't retieve uniform %s", Name);
  }
}

link_internal void
BindUnifromById(shader_uniform *Uniform, s32 *TextureUnit)
{
  if (Uniform->ID >= 0)
  {
    switch(Uniform->Type)
    {
      case ShaderUniform_Texture:
      {
        TIMED_BLOCK("ShaderUniform_Texture");
        Assert(*TextureUnit > -1);

        if (*TextureUnit > 8)
        {
          Warn("TODO(Jesse): TextureUnit > 8, query available texture units"); // TODO(Jesse, id: 135, tags: robustness, opengl, texture): Query max gpu textures?
        }
        /* Assert(TextureUnit < 8); // TODO(Jesse, id: 135, tags: robustness, opengl, texture): Query max gpu textures? */

        GL.ActiveTexture(GL_TEXTURE0 + Cast(u32, *TextureUnit));
        GL.Uniform1i(Uniform->ID, *TextureUnit);
        GL.BindTexture(GL_TEXTURE_2D, Uniform->Texture->ID);

        *TextureUnit = *TextureUnit + 1;
        END_BLOCK();
      } break;

      case ShaderUniform_U32:
      {
        TIMED_BLOCK("ShaderUniform_U32");
        GL.Uniform1ui(Uniform->ID, *Uniform->U32);
        END_BLOCK();
      } break;

      case ShaderUniform_R32:
      {
        TIMED_BLOCK("ShaderUniform_R32");
        GL.Uniform1f(Uniform->ID, *Uniform->R32);
        END_BLOCK();
      } break;

      case ShaderUniform_S32:
      {
        TIMED_BLOCK("ShaderUniform_S32");
        GL.Uniform1i(Uniform->ID, *Uniform->S32);
        END_BLOCK();
      } break;

      case ShaderUniform_M4:
      {
        TIMED_BLOCK("ShaderUniform_M4");
        GL.UniformMatrix4fv(Uniform->ID, 1, GL_FALSE, (r32*)Uniform->M4);
        END_BLOCK();
      } break;

      case ShaderUniform_V2:
      {
        TIMED_BLOCK("ShaderUniform_V2");
        GL.Uniform2fv(Uniform->ID, 1, (r32*)Uniform->V2);
        END_BLOCK();
      } break;

      case ShaderUniform_V3:
      {
        TIMED_BLOCK("ShaderUniform_V3");
        GL.Uniform3fv(Uniform->ID, 1, (r32*)Uniform->V3);
        END_BLOCK();
      } break;

      default:
      {
#if BONSAI_ENGINE
        // NOTE(Jesse): If this fails, we changed the name of BindEngineUniform
        // without updating this callsite
        Assert(BindEngineUniform);
#endif

        // @use_shader_bind_engine_uniform_callsite
        if (BindEngineUniform)
        {
          BindEngineUniform(Uniform);
        }
        else
        {
          SoftError("Attempted to bind an engine uniform, but the engine bind function was not found!");
        }
      }
    }
  }
  else
  {
    // NOTE(Jesse): Uniforms that get optimized out hit this path and it spams the console like crazy
    /* SoftError("Attempted to bind a uniform (%s) with an invalid id (%d)", Uniform->Name, Uniform->ID); */
  }
}

link_internal void
BindUniformByName(shader *Shader, shader_uniform *Uniform, s32 *TextureUnit)
{
  if (Uniform->ID >= 0)
  {
    Perf("Bound shader uniform (%s) by name when it had a valid ID (%d)", Uniform->Name, Uniform->ID);
  }
  else
  {
    Uniform->ID = GL.GetUniformLocation(Shader->ID, Uniform->Name);
    if (Uniform->ID != INVALID_SHADER_UNIFORM)
    {
      BindUnifromById(Uniform, TextureUnit);
    }
  }
}

link_internal void
BindShaderUniforms(shader *Shader)
{
  TIMED_FUNCTION();

  shader_uniform *Uniform = Shader->FirstUniform;

  s32 TextureUnit = 0;

  while (Uniform)
  {
    BindUnifromById(Uniform, &TextureUnit);
    Uniform = Uniform->Next;
    AssertNoGlErrors;
  }

  return;
}

link_internal void
CleanupTextureBindings(shader *Shader)
{
  TIMED_FUNCTION();

  shader_uniform *Uniform = Shader->FirstUniform;

  u32 TextureUnit = 0;

  while (Uniform)
  {
    switch(Uniform->Type)
    {
      case ShaderUniform_Texture:
      {
        GL.ActiveTexture(GL_TEXTURE0 + TextureUnit);
        GL.BindTexture(GL_TEXTURE_2D, 0);
        TextureUnit++;
      } break;

      default: { } break;
    }

    Uniform = Uniform->Next;
  }
}

void
UseShader(shader *Shader)
{
  GL.UseProgram(Shader->ID);
  BindShaderUniforms(Shader);
}
