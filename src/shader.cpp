poof(buffer_c(shader_uniform, u32))
#include <generated/buffer_c_struct_u32.h>

poof(block_array_c(shader_ptr, {64}))
#include <generated/block_array_c_shader_ptr_688853971.h>

struct shader_compile_result
{
  b32 Success;
  u32 ShaderId;
};

link_internal shader_compile_result
CompileShader(ansi_stream Header, ansi_stream Code, u32 Type, b32 DumpErrors)
{
  const int InfoLogLength = 0;

  u32 ShaderID = GetGL()->CreateShader(Type);

  const char *Sources[2] = {Header.Start, Code.Start};
  const s32 Lengths[2] = {(s32)TotalSize(&Header), (s32)TotalSize(&Code)};


  // Compile
  GetGL()->ShaderSource(ShaderID, 2, Sources, Lengths);
  GetGL()->CompileShader(ShaderID);

  // Check Status
  s32 CompileSuccess = GL_FALSE;
  GetGL()->GetShaderiv(ShaderID, GL_COMPILE_STATUS, &CompileSuccess);
  GetGL()->GetShaderiv(ShaderID, GL_INFO_LOG_LENGTH, (s32*)&InfoLogLength);
  AssertNoGlErrors;

  // TODO(Jesse): We should probably return a flag from this function that indicates
  // the compilation failed..?  Or reinstate CheckShaderCompilationStatus?
  if (DumpErrors && CompileSuccess == GL_FALSE)
  {
    char *ProgramErrorMessage = Allocate(char, GetTranArena(), InfoLogLength);
    s32 ActualLength = 0;
    GetGL()->GetShaderInfoLog(ShaderID, InfoLogLength, &ActualLength, ProgramErrorMessage);
    SoftError("Compiling Shader : ApparentLogLength(%d) ActualLogLength(%d) DriverInfoMessage (%S)", InfoLogLength, ActualLength, CS(ProgramErrorMessage, umm(ActualLength)));

    // NOTE(Jesse): Need this to get the actual error message later ... sigh ...
    /* GetGL()->DeleteShader(ShaderID); */
    /* ShaderID = INVALID_SHADER; */
  }

  shader_compile_result Result = {CompileSuccess == GL_TRUE, ShaderID};
  return Result;
}

#if 1
link_internal void
CheckShaderCompilationStatus(cs ShaderPath, u32 ShaderId)
{
  s32 Success = GL_FALSE;
  GetGL()->GetShaderiv(ShaderId, GL_COMPILE_STATUS, &Success);

  if (Success == GL_FALSE)
  {
    s32 InfoLogLength;
    GetGL()->GetProgramiv(ShaderId, GL_INFO_LOG_LENGTH, &InfoLogLength);
    // Apparently the log length includes the null terminator
    char *ProgramErrorMessage = Allocate(char, GetTranArena(), InfoLogLength);
    GetGL()->GetShaderInfoLog(ShaderId, InfoLogLength, NULL, ProgramErrorMessage);

    SoftError("Error Compiling shader (%S)", ShaderPath);
    Error("%d (%s)", InfoLogLength, ProgramErrorMessage);
  }
  else
  {
    Assert(Success == GL_TRUE);
  }

}
#endif

// TODO(Jesse, globals_cleanup)
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

link_internal void
DumpShaderSource(u32 ShaderId)
{
  s32 BufferSize = s32(Megabytes(4));
  s32 BytesWritten = 0;
  char *Buffer = Allocate(char, GetTranArena(), BufferSize);
  GetGL()->GetShaderSource(ShaderId, BufferSize, &BytesWritten, Buffer);
  AssertNoGlErrors;

  u32 LineNumber = 1;
  DebugChars("%4d | ", LineNumber++);
  RangeIterator(CharIndex, BytesWritten)
  {
    char C = Buffer[CharIndex];
    DebugChars("%c", C);
    if (C == '\n') { DebugChars("%4d | ", LineNumber++); }
  }
}

link_internal shader
CompileShaderPair(cs VertShaderPath, cs FragShaderPath, b32 DumpErrors = True)
{
  Info("Creating shader : %S | %S", VertShaderPath, FragShaderPath);

  if (Global_ShaderHeaderCode.Start == 0) { LoadGlobalShaderHeaderCode(ShaderLanguageSetting_330core); } // Default to 330 core if nobody did this already

  ansi_stream HeaderCode       = Global_ShaderHeaderCode;
  ansi_stream VertexShaderCode = ReadEntireFileIntoAnsiStream(VertShaderPath, GetTranArena());
  ansi_stream FragShaderCode   = ReadEntireFileIntoAnsiStream(FragShaderPath, GetTranArena());

  int InfoLogLength;

  shader_compile_result VertexResult = CompileShader(HeaderCode, VertexShaderCode, GL_VERTEX_SHADER, DumpErrors);
  AssertNoGlErrors;

  shader_compile_result FragResult = CompileShader(HeaderCode, FragShaderCode, GL_FRAGMENT_SHADER, DumpErrors);
  AssertNoGlErrors;

  memory_arena *PermMemory = GetThreadLocalState(ThreadLocal_ThreadIndex)->PermMemory;
  shader Shader = { INVALID_SHADER, {}, CopyString(VertShaderPath, PermMemory), CopyString(FragShaderPath, PermMemory), 0, 0, False};

  // NOTE(Jesse): Not doing this because the errors come through when you go to link the program.  Of course ..
  /* if (VertexShaderID != INVALID_SHADER && FragmentShaderID != INVALID_SHADER) */
  {
    // Link the program
    u32 ProgramID = GetGL()->CreateProgram();
    Assert(ProgramID);
    GetGL()->AttachShader(ProgramID, VertexResult.ShaderId);
    GetGL()->AttachShader(ProgramID, FragResult.ShaderId);
    GetGL()->LinkProgram(ProgramID);
    AssertNoGlErrors;

    // Check the program linked
    s32 LinkResult = GL_FALSE;
    GetGL()->GetProgramiv(ProgramID, GL_LINK_STATUS, &LinkResult);
    GetGL()->GetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    AssertNoGlErrors;


    s32 ActiveUniformSlots;
    GetGL()->GetProgramiv(ProgramID, GL_ACTIVE_UNIFORMS, &ActiveUniformSlots);
    AssertNoGlErrors;

    if (DumpErrors && VertexResult.Success == False)
    {
      DumpShaderSource(VertexResult.ShaderId);
    }

    if (DumpErrors && FragResult.Success == False)
    {
      DumpShaderSource(FragResult.ShaderId);
    }

    Info("Shader program (%d) reported (%d) active uniform slots.", ProgramID, ActiveUniformSlots);

    if (DumpErrors && LinkResult == GL_FALSE)
    {
      char *ProgramErrorMessage = Allocate(char, GetTranArena(), InfoLogLength+1);
      GetGL()->GetProgramInfoLog(ProgramID, InfoLogLength, NULL, ProgramErrorMessage);
      SoftError("Linking shader pair %S | %S", VertShaderPath, FragShaderPath);
      SoftError("\n%s", ProgramErrorMessage);
      AssertNoGlErrors;
    }

    GetGL()->DetachShader(ProgramID, VertexResult.ShaderId);
    GetGL()->DetachShader(ProgramID, FragResult.ShaderId);
    GetGL()->DeleteShader(VertexResult.ShaderId);
    GetGL()->DeleteShader(FragResult.ShaderId);
    AssertNoGlErrors;

    if (LinkResult == GL_FALSE)
    {
      GetGL()->DeleteProgram(ProgramID);
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

  Push(&Stdlib->AllShaders, Shader);
}

link_internal s32
GetShaderUniform(shader *Shader, const char *Name)
{
  s32 Result = GetGL()->GetUniformLocation(Shader->ID, Name);
  if (Result == INVALID_SHADER_UNIFORM)
  {
    Warn("Couldn't retreive %s shader uniform - was it optimized out?", Name);
  }

  return Result;
}

link_internal b32
ReloadShaderUniform(shader *Shader, shader_uniform *Uniform)
{
  Assert(Uniform->Name);
  auto PrevLoc = Uniform->ID;
  Uniform->ID = GetShaderUniform(Shader, Uniform->Name);

  b32 Result = Uniform->ID != INVALID_SHADER_UNIFORM;
  if (Result) { Info("Reloaded Shader Uniform (%s) at location (%d), previously (%d)", Uniform->Name, Uniform->ID, PrevLoc); }
  return Result;
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

      s32 MaxRetryCount = 5;
      shader LoadedShader = {};
      RangeIterator(RetryCount, MaxRetryCount)
      {
        b32 DumpErrors = (RetryCount == MaxRetryCount-1);
        LoadedShader = CompileShaderPair(Shader->VertexSourceFilename, Shader->FragSourceFilename, DumpErrors);
        if (LoadedShader.ID != INVALID_SHADER) { break; }

        SleepMs(5);
      }

      if (LoadedShader.ID == INVALID_SHADER)
      {
        // Hit an error loading the shader .. leave the old stuff intact.
      }
      else
      {
        // Delete old shader, and replace with new
        //
        GetGL()->DeleteProgram(Shader->ID);

        auto Uniforms = Shader->Uniforms;
        *Shader = LoadedShader;
        Shader->Uniforms = Uniforms;

        Shader->HotReloaded = True;

        IterateOver(&Shader->Uniforms, Uniform, UniformIndex)
        {
          ReloadShaderUniform(Shader, Uniform);
        }

      }

      AssertNoGlErrors;
    }
  }
}




/* shader_uniform * */
/* PushShaderUniform( memory_arena *Mem, const char *Name) */
/* { */
/*   shader_uniform *Uniform = Allocate(shader_uniform, Mem, 1); */
/*   Uniform->Name = Name; */
/*   return Uniform; */
/* } */


poof(set_shader_uniform(texture));
#include <generated/gen_shader_uniform_push_texture.h>

poof(set_shader_uniform(m4));
#include <generated/gen_shader_uniform_push_m4.h>

poof(set_shader_uniform(v2));
#include <generated/gen_shader_uniform_push_v2.h>

poof(set_shader_uniform(v3));
#include <generated/gen_shader_uniform_push_v3.h>

poof(set_shader_uniform(u32));
#include <generated/gen_shader_uniform_push_u32.h>

poof(set_shader_uniform(s32));
#include <generated/gen_shader_uniform_push_s32.h>

poof(set_shader_uniform(r32));
#include <generated/gen_shader_uniform_push_r32.h>


#if 0
shader
MakeSimpleTextureShader(texture *Texture, memory_arena *GraphicsMemory)
{
  shader Shader = CompileShaderPair( CSz(STDLIB_SHADER_PATH "Passthrough.vertexshader"), CSz(STDLIB_SHADER_PATH "SimpleTexture.fragmentshader") );

  Shader.Uniforms = ShaderUniformBuffer(1, GraphicsMemory);

  SetShaderUniform(&Shader, 0, Texture, "Texture");
  /* *Current = GetUniform(GraphicsMemory, &Shader, Texture, "Texture"); */
  /* Current = &(*Current)->Next; */

  AssertNoGlErrors;

  return Shader;
}
#endif

shader
MakeFullTextureShader(texture *Texture, memory_arena *GraphicsMemory)
{
  shader Shader = CompileShaderPair( CSz(STDLIB_SHADER_PATH "FullPassthrough.vertexshader"), CSz(STDLIB_SHADER_PATH "SimpleTexture.fragmentshader") );
  Shader.Uniforms = ShaderUniformBuffer(4, GraphicsMemory);

  SetShaderUniform(&Shader, 0, Texture, "Texture");
  SetShaderUniform(&Shader, 1, Texture, "TextureArray");
  SetShaderUniform(&Shader, 2, (b32*)0, "IsDepthTexture");
  SetShaderUniform(&Shader, 3, (b32*)0, "HasAlphaChannel");
  AssertNoGlErrors;

  return Shader;
}

b32
CheckAndClearFramebuffer()
{
  u32 FramebufferStatus = GetGL()->CheckFramebufferStatus(GL_FRAMEBUFFER);

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
  GetGL()->Clear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  GetGL()->BindFramebuffer(GL_FRAMEBUFFER, 0);

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
  s32 Uniform = GetGL()->GetUniformLocation(Shader->ID, Name); 
  if (Uniform != INVALID_SHADER_UNIFORM)
  {
    GetGL()->Uniform1i(Uniform, Value);
  }
  else
  {
    Warn("Couldn't retieve uniform %s", Name);
  }
}

link_internal void
BindUniformByName(shader *Shader, const char *Name, b32 Value)
{
  s32 Uniform = GetGL()->GetUniformLocation(Shader->ID, Name); 
  if (Uniform != INVALID_SHADER_UNIFORM)
  {
    GetGL()->Uniform1i(Uniform, s32(Value));
  }
  else
  {
    Warn("Couldn't retieve uniform %s", Name);
  }
}


link_internal void
BindUniformByName(shader *Shader, const char *Name, texture *Texture, u32 TextureUnit)
{
  GetGL()->ActiveTexture(GL_TEXTURE0 + TextureUnit);
  s32 Uniform = GetGL()->GetUniformLocation(Shader->ID, Name);
  if (Uniform != INVALID_SHADER_UNIFORM)
  {
    GetGL()->Uniform1i(Uniform, s32(TextureUnit));
  }
  else
  {
    Warn("Couldn't retieve uniform %s", Name);
  }

  if (Texture->Slices > 1)
  {
    GetGL()->BindTexture(GL_TEXTURE_2D_ARRAY, Texture->ID);
  }
  else
  {
    GetGL()->BindTexture(GL_TEXTURE_2D, Texture->ID);
  }

  AssertNoGlErrors;
}

link_internal void
BindUniformByName(shader *Shader, const char *Name, r32 Value)
{
  s32 Uniform = GetGL()->GetUniformLocation(Shader->ID, Name);
  if (Uniform != INVALID_SHADER_UNIFORM)
  {
    GetGL()->Uniform1f(Uniform, Value);
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
  s32 Uniform = GetGL()->GetUniformLocation(Shader->ID, Name);
  if (Uniform != INVALID_SHADER_UNIFORM)
  {
    GetGL()->Uniform2fv(Uniform, 1, (r32*)V);
    Result = True;
  }
  return Result;
}

link_internal b32
TryBindUniform(shader *Shader, const char *Name, v3 *V)
{
  b32 Result = False;
  s32 Uniform = GetGL()->GetUniformLocation(Shader->ID, Name);
  if (Uniform != INVALID_SHADER_UNIFORM)
  {
    GetGL()->Uniform3fv(Uniform, 1, (r32*)V);
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
  s32 Uniform = GetGL()->GetUniformLocation(Shader->ID, Name);
  if (Uniform != INVALID_SHADER_UNIFORM)
  {
    GetGL()->UniformMatrix4fv(Uniform, 1, GL_FALSE, (r32*)Matrix);
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
BindUniformById(shader_uniform *Uniform, s32 *TextureUnit)
{
  if (Uniform->ID >= 0)
  {
    switch(Uniform->Type)
    {
      InvalidCase(ShaderUniform_Undefined);

      case ShaderUniform_Texture:
      {
        TIMED_BLOCK("ShaderUniform_Texture");
        Assert(*TextureUnit > -1);

        if (*TextureUnit > 8)
        {
          Warn("TODO(Jesse): TextureUnit > 8, query available texture units"); // TODO(Jesse, id: 135, tags: robustness, opengl, texture): Query max gpu textures?
        }
        /* Assert(TextureUnit < 8); // TODO(Jesse, id: 135, tags: robustness, opengl, texture): Query max gpu textures? */

        GetGL()->ActiveTexture(GL_TEXTURE0 + Cast(u32, *TextureUnit));
        GetGL()->Uniform1i(Uniform->ID, *TextureUnit);
        GetGL()->BindTexture(GL_TEXTURE_2D, Uniform->Texture->ID);

        *TextureUnit = *TextureUnit + 1;
        END_BLOCK();
      } break;

      case ShaderUniform_U32:
      {
        TIMED_BLOCK("ShaderUniform_U32");
        GetGL()->Uniform1ui(Uniform->ID, *Uniform->U32);
        END_BLOCK();
      } break;

      case ShaderUniform_R32:
      {
        TIMED_BLOCK("ShaderUniform_R32");
        GetGL()->Uniform1f(Uniform->ID, *Uniform->R32);
        END_BLOCK();
      } break;

      case ShaderUniform_S32:
      {
        TIMED_BLOCK("ShaderUniform_S32");
        GetGL()->Uniform1i(Uniform->ID, *Uniform->S32);
        END_BLOCK();
      } break;

      case ShaderUniform_M4:
      {
        TIMED_BLOCK("ShaderUniform_M4");
        GetGL()->UniformMatrix4fv(Uniform->ID, 1, GL_FALSE, (r32*)Uniform->M4);
        END_BLOCK();
      } break;

      case ShaderUniform_V2:
      {
        TIMED_BLOCK("ShaderUniform_V2");
        GetGL()->Uniform2fv(Uniform->ID, 1, (r32*)Uniform->V2);
        END_BLOCK();
      } break;

      case ShaderUniform_V3:
      {
        TIMED_BLOCK("ShaderUniform_V3");
        GetGL()->Uniform3fv(Uniform->ID, 1, (r32*)Uniform->V3);
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
    Uniform->ID = GetGL()->GetUniformLocation(Shader->ID, Uniform->Name);
    if (Uniform->ID != INVALID_SHADER_UNIFORM)
    {
      BindUniformById(Uniform, TextureUnit);
    }
  }
}

link_internal void
BindShaderUniforms(shader *Shader)
{
  TIMED_FUNCTION();

  s32 TextureUnit = 0;
  IterateOver(&Shader->Uniforms, Uniform, UniformIndex)
  {
    BindUniformById(Uniform, &TextureUnit);
    AssertNoGlErrors;
  }

  return;
}

link_internal void
CleanupTextureBindings(shader *Shader)
{
  TIMED_FUNCTION();

  u32 TextureUnit = 0;
  IterateOver(&Shader->Uniforms, Uniform, UniformIndex)
  {
    switch(Uniform->Type)
    {
      case ShaderUniform_Texture:
      {
        GetGL()->ActiveTexture(GL_TEXTURE0 + TextureUnit);
        GetGL()->BindTexture(GL_TEXTURE_2D, 0);
        TextureUnit++;
      } break;

      default: { } break;
    }
    AssertNoGlErrors;
  }
}

void
UseShader(shader *Shader)
{
  if (Shader->ID != INVALID_SHADER)
  {
    GetGL()->UseProgram(Shader->ID);
    BindShaderUniforms(Shader);
  }
  else
  {
    SoftError("Attempted to bind uncompiled Shader (%S) | (%S)", Shader->VertexSourceFilename, Shader->FragSourceFilename);
  }
}
