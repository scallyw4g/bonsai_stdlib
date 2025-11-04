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
  auto GL = GetGL();

  const int InfoLogLength = 0;

  u32 ShaderID = GL->CreateShader(Type);

  const char *Sources[2] = {Header.Start, Code.Start};
  const s32 Lengths[2] = {(s32)TotalSize(&Header), (s32)TotalSize(&Code)};


  // Compile
  GL->ShaderSource(ShaderID, 2, Sources, Lengths);
  GL->CompileShader(ShaderID);

  // Check Status
  s32 CompileSuccess = GL_FALSE;
  GL->GetShaderiv(ShaderID, GL_COMPILE_STATUS, &CompileSuccess);
  GL->GetShaderiv(ShaderID, GL_INFO_LOG_LENGTH, (s32*)&InfoLogLength);
  AssertNoGlErrors;

  // TODO(Jesse): We should probably return a flag from this function that indicates
  // the compilation failed..?  Or reinstate CheckShaderCompilationStatus?
  if (DumpErrors && CompileSuccess == GL_FALSE)
  {
    char *ProgramErrorMessage = Allocate(char, GetTranArena(), InfoLogLength);
    s32 ActualLength = 0;
    GL->GetShaderInfoLog(ShaderID, InfoLogLength, &ActualLength, ProgramErrorMessage);
    SoftError("Compiling Shader : ApparentLogLength(%d) ActualLogLength(%d) DriverInfoMessage (%S)", InfoLogLength, ActualLength, CS(ProgramErrorMessage, umm(ActualLength)));

    // NOTE(Jesse): Need this to get the actual error message later ... sigh ...
    /* GL->DeleteShader(ShaderID); */
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

  auto GL = GetGL();

  GL->GetShaderiv(ShaderId, GL_COMPILE_STATUS, &Success);

  if (Success == GL_FALSE)
  {
    s32 InfoLogLength;
    GL->GetProgramiv(ShaderId, GL_INFO_LOG_LENGTH, &InfoLogLength);
    // Apparently the log length includes the null terminator
    char *ProgramErrorMessage = Allocate(char, GetTranArena(), InfoLogLength);
    GL->GetShaderInfoLog(ShaderId, InfoLogLength, NULL, ProgramErrorMessage);

    SoftError("Error Compiling shader (%S)", ShaderPath);
    Error("%d (%s)", InfoLogLength, ProgramErrorMessage);
  }
  else
  {
    Assert(Success == GL_TRUE);
  }

}
#endif


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
RegisterShaderForHotReload(bonsai_stdlib *Stdlib, shader *Shader)
{
  Assert(Shader->ID != INVALID_SHADER);

  if (Stdlib->AllShaders.Memory == 0)
  {
    Stdlib->AllShaders.Memory = AllocateArena();
  }

  if (Find(&Stdlib->AllShaders, Shader).Index == INVALID_BLOCK_ARRAY_INDEX)
  {
    Shader("Registered (%S|%S) successfully for hot-reload at Index (%d).", Shader->VertexSourceFilename, Shader->FragSourceFilename, AtElements(&Stdlib->AllShaders));
    Push(&Stdlib->AllShaders, Shader);
  }
  else
  {
    Warn("Shader pair (%S|%S) already registered for hot-reload, not registering duplicate.", Shader->VertexSourceFilename, Shader->FragSourceFilename);
  }
}

link_internal void
ReloadShaderHeaderCode(bonsai_stdlib *Stdlib, shader_language_setting ShaderLanguage)
{
  Shader("Reloading header.glsl");

  cs ShaderVersion = ValueFromSetting(ShaderLanguage);

  Stdlib->ShaderHeaderFile = OpenHotReloadableFile(CSz(STDLIB_SHADER_PATH "header.glsl"), FilePermission_Read); 

  cs HeaderCode =  ReadEntireFileIntoString(&Stdlib->ShaderHeaderFile.File, GetThreadLocalState(ThreadLocal_ThreadIndex)->PermMemory);

  // ReadEntireFileIntoString closes the file
  // CloseFile(&Stdlib->ShaderHeaderFile.File);

  Stdlib->ShaderHeaderCode = AnsiStream(Concat(ShaderVersion, HeaderCode, GetThreadLocalState(ThreadLocal_ThreadIndex)->PermMemory));
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

link_internal b32
poof(@async @render)
CompileShaderPair(shader *Shader, cs VertShaderPath, cs FragShaderPath, b32 DumpErrors = True, b32 RegisterForHotReload = True)
{
  Shader("Creating : (%S | %S)", VertShaderPath, FragShaderPath);

  auto Stdlib = GetStdlib();
  auto GL = GetGL();

  if (Stdlib->ShaderHeaderCode.Start == 0) { ReloadShaderHeaderCode(Stdlib, ShaderLanguageSetting_330core); } // Default to 330 core if nobody did this already

  ansi_stream VertexShaderCode = ReadEntireFileIntoAnsiStream(VertShaderPath, GetTranArena());
  ansi_stream FragShaderCode   = ReadEntireFileIntoAnsiStream(FragShaderPath, GetTranArena());

  int InfoLogLength;

  shader_compile_result VertexResult = CompileShader(Stdlib->ShaderHeaderCode, VertexShaderCode, GL_VERTEX_SHADER, DumpErrors);
  AssertNoGlErrors;

  shader_compile_result FragResult = CompileShader(Stdlib->ShaderHeaderCode, FragShaderCode, GL_FRAGMENT_SHADER, DumpErrors);
  AssertNoGlErrors;

  memory_arena *PermMemory = GetThreadLocalState(ThreadLocal_ThreadIndex)->PermMemory;
  *Shader = { INVALID_SHADER, {}, CopyString(VertShaderPath, PermMemory), CopyString(FragShaderPath, PermMemory), 0, 0, False};

  // NOTE(Jesse): Not doing this because the errors come through when you go to link the program.  Of course ..
  /* if (VertexShaderID != INVALID_SHADER && FragmentShaderID != INVALID_SHADER) */
  /* { */
    // Link the program
    u32 ProgramID = GL->CreateProgram();
    Assert(ProgramID);
    GL->AttachShader(ProgramID, VertexResult.ShaderId);
    GL->AttachShader(ProgramID, FragResult.ShaderId);
    GL->LinkProgram(ProgramID);
    AssertNoGlErrors;



    // Check the program linked
    s32 LinkResult = GL_FALSE;
    GL->GetProgramiv(ProgramID, GL_LINK_STATUS, &LinkResult);
    GL->GetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    AssertNoGlErrors;

    b32 CompileSucceeded = (LinkResult == GL_TRUE);

    s32 ActiveUniformSlots;
    GL->GetProgramiv(ProgramID, GL_ACTIVE_UNIFORMS, &ActiveUniformSlots);
    AssertNoGlErrors;

    if (VertexResult.Success == False && DumpErrors)
    {
      DumpShaderSource(VertexResult.ShaderId);
    }

    if (FragResult.Success == False && DumpErrors)
    {
      DumpShaderSource(FragResult.ShaderId);
    }

    Shader("Program (%d) reported (%d) active uniform slots.", ProgramID, ActiveUniformSlots);

    if (CompileSucceeded == False && DumpErrors)
    {
      char *ProgramErrorMessage = Allocate(char, GetTranArena(), InfoLogLength+1);
      GL->GetProgramInfoLog(ProgramID, InfoLogLength, NULL, ProgramErrorMessage);
      SoftError("Linking shader pair %S | %S", VertShaderPath, FragShaderPath);
      SoftError("\n%s\n", ProgramErrorMessage);
      AssertNoGlErrors;
    }

    GL->DetachShader(ProgramID, VertexResult.ShaderId);
    GL->DetachShader(ProgramID, FragResult.ShaderId);
    GL->DeleteShader(VertexResult.ShaderId);
    GL->DeleteShader(FragResult.ShaderId);
    AssertNoGlErrors;

    // NOTE(Jesse): Kind of a hack to set the last modified times on the shader.  Should
    // really be able to do this when reading the source code into buffers, but .. meh ..
    FileIsNew(GetNullTerminated(Shader->VertexSourceFilename), &Shader->VertexTimeModifiedWhenLoaded);
    FileIsNew(GetNullTerminated(Shader->FragSourceFilename), &Shader->FragmentTimeModifiedWhenLoaded);

    if (CompileSucceeded == False)
    {
      GL->DeleteProgram(ProgramID);
      ProgramID = INVALID_SHADER;
    }

    Shader->ID = ProgramID;
  /* } */

  if (CompileSucceeded && RegisterForHotReload)
  {
    RegisterShaderForHotReload(GetStdlib(), Shader);
  }

  AssertNoGlErrors;
  return CompileSucceeded;
}

link_internal s32
GetShaderUniform(shader *Shader, const char *Name)
{
  auto GL = GetGL();

  s32 Result = GL->GetUniformLocation(Shader->ID, Name);
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
  if (Result) { Shader("Reloaded Uniform (%s) at location (%d), previously (%d)", Uniform->Name, Uniform->ID, PrevLoc); }
  return Result;
}


link_internal void
HotReloadShaders(bonsai_stdlib *Stdlib)
{
  auto GL = GetGL();

  b32 HeaderIsNew = FileIsNew(STDLIB_SHADER_PATH "header.glsl", &Stdlib->ShaderHeaderFile.LastModified);

  if (HeaderIsNew)
  {
    ReloadShaderHeaderCode(Stdlib, ShaderLanguageSetting_330core);
  }

  IterateOver(&Stdlib->AllShaders, Shader, ShaderIndex)
  {
    Shader->HotReloaded = False;

    b32 VertIsNew = FileIsNew(GetNullTerminated(Shader->VertexSourceFilename), &Shader->VertexTimeModifiedWhenLoaded);
    b32 FragIsNew = FileIsNew(GetNullTerminated(Shader->FragSourceFilename),   &Shader->FragmentTimeModifiedWhenLoaded);
    if (HeaderIsNew || VertIsNew || FragIsNew)
    {
      SleepMs(5);

      auto T1 = Shader->VertexTimeModifiedWhenLoaded;
      auto T2 = Shader->FragmentTimeModifiedWhenLoaded;

      s32 MaxRetryCount = 5;
      shader LoadedShader = {};
      b32 RegisterForHotReload = False; // We're hot reloading now ..
      RangeIterator(RetryCount, MaxRetryCount)
      {
        b32 DumpErrors = (RetryCount == MaxRetryCount-1);
        if (CompileShaderPair(&LoadedShader, Shader->VertexSourceFilename, Shader->FragSourceFilename, DumpErrors, RegisterForHotReload))
        {
          break;
        }

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
        GL->DeleteProgram(Shader->ID);

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

#if 0
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
#endif

b32
CheckAndClearFramebuffer()
{
  auto GL = GetGL();

  u32 FramebufferStatus = GL->CheckFramebufferStatus(GL_FRAMEBUFFER);

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
  GL->Clear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  GL->BindFramebuffer(GL_FRAMEBUFFER, 0);

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
  auto GL = GetGL();

  s32 Uniform = GL->GetUniformLocation(Shader->ID, Name); 
  if (Uniform != INVALID_SHADER_UNIFORM)
  {
    GL->Uniform1i(Uniform, Value);
  }
  else
  {
    Warn("Couldn't retieve uniform %s", Name);
  }
}

link_internal void
BindUniformByName(shader *Shader, const char *Name, b32 Value)
{
  auto GL = GetGL();

  s32 Uniform = GL->GetUniformLocation(Shader->ID, Name); 
  if (Uniform != INVALID_SHADER_UNIFORM)
  {
    GL->Uniform1i(Uniform, s32(Value));
  }
  else
  {
    Warn("Couldn't retieve uniform %s", Name);
  }
}


link_internal void
BindUniformByName(shader *Shader, const char *Name, texture *Texture, u32 TextureUnit)
{
  auto GL = GetGL();

  GL->ActiveTexture(GL_TEXTURE0 + TextureUnit);
  s32 Uniform = GL->GetUniformLocation(Shader->ID, Name);
  if (Uniform != INVALID_SHADER_UNIFORM)
  {
    GL->Uniform1i(Uniform, s32(TextureUnit));
  }
  else
  {
    Warn("Couldn't retieve uniform %s", Name);
  }

  if (Texture->Slices > 1)
  {
    GL->BindTexture(GL_TEXTURE_2D_ARRAY, Texture->ID);
  }
  else
  {
    GL->BindTexture(GL_TEXTURE_2D, Texture->ID);
  }

  AssertNoGlErrors;
}

link_internal void
BindUniformByName(shader *Shader, const char *Name, r32 Value)
{
  auto GL = GetGL();

  s32 Uniform = GL->GetUniformLocation(Shader->ID, Name);
  if (Uniform != INVALID_SHADER_UNIFORM)
  {
    GL->Uniform1f(Uniform, Value);
  }
  else
  {
    Warn("Couldn't retieve uniform %s", Name);
  }
}

link_internal b32
TryBindUniform(shader *Shader, const char *Name, v2 *V)
{
  auto GL = GetGL();

  b32 Result = False;

  s32 Uniform = GL->GetUniformLocation(Shader->ID, Name);
  if (Uniform != INVALID_SHADER_UNIFORM)
  {
    GL->Uniform2fv(Uniform, 1, (r32*)V);
    Result = True;
  }
  return Result;
}

link_internal b32
TryBindUniform(shader *Shader, const char *Name, v3 *V)
{
  auto GL = GetGL();

  b32 Result = False;

  s32 Uniform = GL->GetUniformLocation(Shader->ID, Name);
  if (Uniform != INVALID_SHADER_UNIFORM)
  {
    GL->Uniform3fv(Uniform, 1, (r32*)V);
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
  auto GL = GetGL();

  b32 Result = False;

  s32 Uniform = GL->GetUniformLocation(Shader->ID, Name);
  if (Uniform != INVALID_SHADER_UNIFORM)
  {
    GL->UniformMatrix4fv(Uniform, 1, GL_FALSE, (r32*)Matrix);
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
  auto GL = GetGL();

  s32 Count = Uniform->Count ? s32(*Uniform->Count) : 1;

  // NOTE(Jesse): It is actually valid to have a count of zero, for example if
  // we're specifying an array with 0 elements.
  //
  if (Count)
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
          Assert(Count == 1);

          if (*TextureUnit > 8)
          {
            Warn("TODO(Jesse): TextureUnit > 8, query available texture units"); // TODO(Jesse, id: 135, tags: robustness, opengl, texture): Query max gpu textures?
          }
          /* Assert(TextureUnit < 8); // TODO(Jesse, id: 135, tags: robustness, opengl, texture): Query max gpu textures? */

          GL->ActiveTexture(GL_TEXTURE0 + Cast(u32, *TextureUnit));
          GL->Uniform1i(Uniform->ID, *TextureUnit);
          GL->BindTexture(GL_TEXTURE_2D, Uniform->Texture->ID);

          *TextureUnit = *TextureUnit + 1;
          END_BLOCK();
        } break;

        case ShaderUniform_U32:
        {
          TIMED_BLOCK("ShaderUniform_U32");
          Assert(Count == 1);
          GL->Uniform1ui(Uniform->ID, *Uniform->U32);
          END_BLOCK();
        } break;

        case ShaderUniform_R32:
        {
          TIMED_BLOCK("ShaderUniform_R32");
          Assert(Count == 1);
          GL->Uniform1f(Uniform->ID, *Uniform->R32);
          END_BLOCK();
        } break;

        case ShaderUniform_S32:
        {
          TIMED_BLOCK("ShaderUniform_S32");
          Assert(Count == 1);
          GL->Uniform1i(Uniform->ID, *Uniform->S32);
          END_BLOCK();
        } break;

        case ShaderUniform_M4:
        {
          TIMED_BLOCK("ShaderUniform_M4");
          Assert(Count);
          GL->UniformMatrix4fv(Uniform->ID, Count, GL_FALSE, (r32*)Uniform->M4);
          END_BLOCK();
        } break;

        case ShaderUniform_V2:
        {
          TIMED_BLOCK("ShaderUniform_V2");

          Assert(Count);
          GL->Uniform2fv(Uniform->ID, Count, (r32*)Uniform->V2);
          END_BLOCK();
        } break;

        case ShaderUniform_V3:
        {
          TIMED_BLOCK("ShaderUniform_V3");

          Assert(Count);
          GL->Uniform3fv(Uniform->ID, Count, (r32*)Uniform->V3);
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

  AssertNoGlErrors;
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
  auto GL = GetGL();

  u32 TextureUnit = 0;
  IterateOver(&Shader->Uniforms, Uniform, UniformIndex)
  {
    switch(Uniform->Type)
    {
      case ShaderUniform_Texture:
      {
        GL->ActiveTexture(GL_TEXTURE0 + TextureUnit);
        GL->BindTexture(GL_TEXTURE_2D, 0);
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
  TIMED_FUNCTION();
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

void
UseShader(void *Shader)
{
  UseShader(Cast(shader *, Shader));
}

