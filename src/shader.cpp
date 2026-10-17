poof(buffer_c(shader_uniform, u32))
#include <generated/buffer_c$shader_uniform.u32$ER5w4Qva.h>

poof(block_array_c(shader_ptr, {64}))
#include <generated/block_array_c$shader_ptr.688853971$rd53rW0F.h>

struct shader_compile_result
{
  b32 Success;
  u32 ShaderId;
};


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

    case ShaderLanguageSetting_460core:
    {
      Result = CSz("#version 460 core\n\n");
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

  auto Index = Find(&Stdlib->AllShaders, Shader).Index;
  if (Index == INVALID_BLOCK_ARRAY_INDEX)
  {
    Shader("Registered (%S) successfully for hot-reload at Index (%d).", Shader->DebugName, AtElements(&Stdlib->AllShaders));
    Push(&Stdlib->AllShaders, Shader);
  }
  else
  {
    Warn("Shader (%S) already registered for hot-reload, not registering duplicate.", Shader->DebugName);
  }
}

link_internal void
UnregisterShaderForHotReload(bonsai_stdlib *Stdlib, shader *Shader)
{
  Assert(Shader->ID != INVALID_SHADER);

  auto I = Find(&Stdlib->AllShaders, Shader);
  if (IsValid(&I))
  {
    Shader("Unregistered (%S) successfully for hot-reload at Index (%d).", Shader->DebugName);
    RemoveUnordered(&Stdlib->AllShaders, I);
  }
  else
  {
    Warn("Shader pair (%S) not registered for hot-reload.", Shader->DebugName);
  }
}

link_internal b32
ReloadShaderHeaderCode(bonsai_stdlib *Stdlib, hot_reloadable_file *ShaderHeaderFile, shader_language_setting ShaderLanguage)
{
  TIMED_FUNCTION();
  b32 Result = TryOpenHotReloadableFile(ShaderHeaderFile, FilePermission_Read);
  if ( Result )
  {
    Shader("Reloading (%S)", ShaderHeaderFile->File.Path);

    // ReadEntireFileIntoString closes the file
    cs HeaderCode =  ReadEntireFileIntoString(&Stdlib->ShaderHeaderFile.File, GetThreadLocalState(ThreadLocal_ThreadIndex)->PermMemory);

    cs ShaderVersion = ValueFromSetting(ShaderLanguage);
    Stdlib->ShaderHeaderCode = AnsiStream(Concat(ShaderVersion, HeaderCode, GetThreadLocalState(ThreadLocal_ThreadIndex)->PermMemory));
  }
  return Result;
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

link_internal void
AssertAllFilenameStringsHeapAllocated(hot_reloadable_file_block_array *Files)
{
  auto Stdlib = GetStdlib();
  IterateOver(Files, File, FileIndex)
  {
    Assert(IsHeapAllocated(&Stdlib->Ui.Strings, Cast(void*, File->File.Path.Start)));
  }
}

link_internal shader_compile_result
CompileShader(const char **Sources, const s32 *Lengths, u32 Count, u32 Type, b32 DumpErrors = False)
{
  auto GL = GetGL();

  const int InfoLogLength = 0;

  u32 ShaderID = GL->CreateShader(Type);

  // Compile
  GL->ShaderSource(ShaderID, s32(Count), Sources, Lengths);
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

link_internal shader_compile_result
CompileShader(ansi_stream *HeaderCode, hot_reloadable_file_block_array *Files, u32 Type, b32 DumpErrors)
{
  auto Tran = GetTranArena();

  // +1 for header code
  u32 FileCount = u32(AtElements(Files).Index);
  u32 TotalSources = FileCount+1;
  const char **Sources = Allocate(const char *, Tran, TotalSources);
         s32  *Lengths = Allocate(s32, Tran, TotalSources);

  Sources[0] = HeaderCode->Start;
  Lengths[0] = Cast(s32, TotalSize(HeaderCode));


  for (u32 Index = 0; Index < FileCount; ++Index)
  {
    cs Path = GetPtr(Files, Index)->File.Path;
    ansi_stream Code = ReadEntireFileIntoAnsiStream(Path, Tran);
    Sources[Index+1] = Code.Start;
    Lengths[Index+1] = Cast(s32, TotalSize(&Code));
  }

  shader_compile_result Result = CompileShader(Sources, Lengths, TotalSources, Type, DumpErrors);
  AssertNoGlErrors;

  return Result;
}

link_internal b32
poof(@async @render)
CompileShaderPair(shader *Shader, b32 DumpErrors = True, b32 RegisterForHotReload = True)
{
  Shader("Creating (%S)", Shader->DebugName);

  auto Stdlib = GetStdlib();
  auto GL = GetGL();

  Assert(TotalElements(&Shader->VertFiles));
  Assert(TotalElements(&Shader->FragFiles));

  AssertAllFilenameStringsHeapAllocated(&Shader->VertFiles);
  AssertAllFilenameStringsHeapAllocated(&Shader->FragFiles);

  Assert(Stdlib->ShaderHeaderCode.Start);

  shader_compile_result VertexResult = CompileShader(&Stdlib->ShaderHeaderCode, &Shader->VertFiles, GL_VERTEX_SHADER,   DumpErrors);
  shader_compile_result   FragResult = CompileShader(&Stdlib->ShaderHeaderCode, &Shader->FragFiles, GL_FRAGMENT_SHADER, DumpErrors);

  Shader->ID = INVALID_SHADER;

  // Link the program
  u32 ProgramID = GL->CreateProgram();
  Assert(ProgramID);
  GL->AttachShader(ProgramID, VertexResult.ShaderId);
  GL->AttachShader(ProgramID, FragResult.ShaderId);
  GL->LinkProgram(ProgramID);
  AssertNoGlErrors;

  // Check the program linked
  int InfoLogLength;
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

  Shader("(%S) reported (%d) active uniform slots.", Shader->DebugName, ActiveUniformSlots);

  if (CompileSucceeded == False && DumpErrors)
  {
    char *ProgramErrorMessage = Allocate(char, GetTranArena(), InfoLogLength+1);
    GL->GetProgramInfoLog(ProgramID, InfoLogLength, NULL, ProgramErrorMessage);
    SoftError("Linking shader (%S)", Shader->DebugName);
    SoftError("\n%s\n", ProgramErrorMessage);
    AssertNoGlErrors;
  }

  GL->DetachShader(ProgramID, VertexResult.ShaderId);
  GL->DetachShader(ProgramID, FragResult.ShaderId);
  GL->DeleteShader(VertexResult.ShaderId);
  GL->DeleteShader(FragResult.ShaderId);
  AssertNoGlErrors;

  if (CompileSucceeded == False)
  {
    GL->DeleteProgram(ProgramID);
    ProgramID = INVALID_SHADER;
  }

  Shader->ID = ProgramID;

  if (CompileSucceeded && RegisterForHotReload)
  {
    RegisterShaderForHotReload(GetStdlib(), Shader);
  }

  AssertNoGlErrors;
  return CompileSucceeded;
}

link_internal b32
poof(@async @render)
CompileNewShader(
    shader *Shader,
    cs ShaderName,
    cs *VertSourcePaths, s32 VertSourcePathsCount,
    cs *FragSourcePaths, s32 FragSourcePathsCount,
    memory_arena *PermMem,
    b32 DumpErrors = True, b32 RegisterForHotReload = True)
{
  Assert(Shader->ID == INVALID_SHADER);

  Shader->DebugName = ShaderName;

  Shader->VertFiles.Memory = PermMem;
  Shader->FragFiles.Memory = PermMem;

  RangeIterator(Index, VertSourcePathsCount)
  {
    hot_reloadable_file File = HotReloadableFile(VertSourcePaths[Index]);
    Push(&Shader->VertFiles, &File);
  }

  RangeIterator(Index, FragSourcePathsCount)
  {
    hot_reloadable_file File = HotReloadableFile(FragSourcePaths[Index]);
    Push(&Shader->FragFiles, &File);
  }

  b32 Result = CompileShaderPair(Shader, DumpErrors, RegisterForHotReload);
  return Result;
}

// Convenience path for compiling a new shader.  Just here for compat with an old API ..should be removed?
//
link_internal b32
poof(@async @render)
CompileNewShader( shader *Shader,
                      cs  ShaderName,
                      cs  VertPath,
                      cs  FragPath,
            memory_arena *PermMem,
                     b32  DumpErrors = True,
                     b32  RegisterForHotReload = True )
{
  Assert(Shader->ID == INVALID_SHADER);

  Shader->DebugName = ShaderName;

  Shader->VertFiles.Memory = PermMem;
  Shader->FragFiles.Memory = PermMem;

  hot_reloadable_file VertFile = HotReloadableFile(VertPath);
  hot_reloadable_file FragFile = HotReloadableFile(FragPath);

  Push(&Shader->VertFiles, &VertFile);
  Push(&Shader->FragFiles, &FragFile);

  b32 Result = CompileShaderPair(Shader, DumpErrors, RegisterForHotReload);
  return Result;
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


link_internal b32
AnyNew(hot_reloadable_file_block_array *Files)
{
  b32 Result = False;

  IterateOver(Files, File, Index)
  {
    Result |= FileIsNew(File);
  }

  return Result;
}

link_internal void
HotReloadShaders(bonsai_stdlib *Stdlib)
{
  TIMED_FUNCTION();

  auto GL = GetGL();

  b32 HeaderIsNew = ReloadShaderHeaderCode(Stdlib, &Stdlib->ShaderHeaderFile, ShaderLanguageSetting_330core);

  {
    TIMED_NAMED_BLOCK(ReloadLoop);
    IterateOver(&Stdlib->AllShaders, Shader, ShaderIndex)
    {
      // TODO(Jesse): This seems like it should be an assert instead .. shouldn't this just be False already?
      Shader->HotReloaded = False;

      u64 NewVertTime = AnyNew(&Shader->VertFiles);
      u64 NewFragTime = AnyNew(&Shader->FragFiles);
      if (HeaderIsNew || NewVertTime || NewFragTime)
      {
        s32 MaxRetryCount = 5;
        shader LoadedShader = {};
        NotImplemented;
        /* LoadedShader.Vert = Shader->Vert; */
        /* LoadedShader.Frag = Shader->Frag; */

        b32 RegisterForHotReload = False; // We're hot reloading now ..
        RangeIterator(RetryCount, MaxRetryCount)
        {
          b32 DumpErrors = (RetryCount == MaxRetryCount-1);
          /* if (CompileShaderPair(&LoadedShader, DumpErrors, RegisterForHotReload)) */
          {
            break;
          }

          // TODO(Jesse): Is this actually necessary anymore?
          /* SpinlockNs(100); */
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
}




/* shader_uniform * */
/* PushShaderUniform( memory_arena *Mem, const char *Name) */
/* { */
/*   shader_uniform *Uniform = Allocate(shader_uniform, Mem, 1); */
/*   Uniform->Name = Name; */
/*   return Uniform; */
/* } */


poof(set_shader_uniform(texture));
#include <generated/set_shader_uniform$texture$eruQ9d3W.h>

poof(set_shader_uniform(m4));
#include <generated/set_shader_uniform$m4$r4tRKWYG.h>

poof(set_shader_uniform(v2));
#include <generated/set_shader_uniform$v2$hRTrq6Ll.h>

poof(set_shader_uniform(v3));
#include <generated/set_shader_uniform$v3$PlhUY9mt.h>

poof(set_shader_uniform(u32));
#include <generated/set_shader_uniform$u32$CQ86uERe.h>

poof(set_shader_uniform(s32));
#include <generated/set_shader_uniform$s32$k7y71ARU.h>

poof(set_shader_uniform(r32));
#include <generated/set_shader_uniform$r32$qG1h5ut2.h>


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
  Assert(Name);
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
  Assert(Name);
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
  Assert(Name);
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
  Assert(Name);
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
  Assert(Name);
  if (TryBindUniform(Shader, Name, V) == False)
  {
    Warn("Couldn't retieve uniform %s", Name);
  }
}

link_internal void
BindUniformByName(shader *Shader, const char *Name, v3 *V)
{
  Assert(Name);
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
  Assert(Name);
  if (TryBindUniform(Shader, Name, Matrix) == False)
  {
    Warn("Couldn't retieve uniform %s", Name);
  }
}

link_internal void
BindUniformById(shader_uniform *Uniform, s32 *TextureUnit)
{
  auto GL = GetGL();

#if BONSAI_INTERNAL
  s32 CurrentProgram = 0;
  GL->GetIntegerv(GL_CURRENT_PROGRAM, &CurrentProgram);
  s32 Location = GL->GetUniformLocation(u32(CurrentProgram), Uniform->Name);
  Assert(Location == Uniform->ID);
#endif

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
          AssertNoGlErrors;
          GL->Uniform1i(Uniform->ID, *TextureUnit);
          AssertNoGlErrors;

          auto Texture = Uniform->Texture;
          if (Texture->Slices > 1)
          {
            GL->BindTexture(GL_TEXTURE_2D_ARRAY, Texture->ID);
          }
          else
          {
            GL->BindTexture(GL_TEXTURE_2D, Texture->ID);
          }

          *TextureUnit = *TextureUnit + 1;
          END_BLOCK();
        } break;

        case ShaderUniform_U32:
        {
          TIMED_BLOCK("ShaderUniform_U32");
          Assert(Count == 1);
          GL->Uniform1ui(Uniform->ID, *Uniform->U32);
          END_BLOCK();
          AssertNoGlErrors;
        } break;

        case ShaderUniform_R32:
        {
          TIMED_BLOCK("ShaderUniform_R32");
          Assert(Count == 1);
          GL->Uniform1f(Uniform->ID, *Uniform->R32);
          END_BLOCK();
          AssertNoGlErrors;
        } break;

        case ShaderUniform_S32:
        {
          TIMED_BLOCK("ShaderUniform_S32");
          Assert(Count == 1);
          GL->Uniform1i(Uniform->ID, *Uniform->S32);
          END_BLOCK();
          AssertNoGlErrors;
        } break;

        case ShaderUniform_M4:
        {
          TIMED_BLOCK("ShaderUniform_M4");
          Assert(Count);
          GL->UniformMatrix4fv(Uniform->ID, Count, GL_FALSE, (r32*)Uniform->M4);
          END_BLOCK();
          AssertNoGlErrors;
        } break;

        case ShaderUniform_V2:
        {
          TIMED_BLOCK("ShaderUniform_V2");

          Assert(Count);
          GL->Uniform2fv(Uniform->ID, Count, (r32*)Uniform->V2);
          END_BLOCK();
          AssertNoGlErrors;
        } break;

        case ShaderUniform_V3:
        {
          TIMED_BLOCK("ShaderUniform_V3");

          Assert(Count);
          GL->Uniform3fv(Uniform->ID, Count, (r32*)Uniform->V3);
          END_BLOCK();
          AssertNoGlErrors;
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
    Assert(Uniform->Name);
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

        auto Texture = Uniform->Texture;
        if (Texture->Slices > 1)
        {
          GL->BindTexture(GL_TEXTURE_2D_ARRAY, 0);
        }
        else
        {
          GL->BindTexture(GL_TEXTURE_2D, 0);
        }

        TextureUnit++;
        AssertNoGlErrors;
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
    SoftError("Attempted to bind uncompiled Shader (%S)", Shader->DebugName);
  }
}

void
UseShader(void *Shader)
{
  UseShader(Cast(shader *, Shader));
}

link_internal cs
StdlibShaderPath(cs ShaderFile)
{
  return ConcatZs(GetAbsoluteStdlibShaderDir(), ShaderFile, GetTranArena());
}

