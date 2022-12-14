#define TMP_DIR_ROOT "tmp/"



// TODO(Jesse): The Create and Delete functions here are identical mirrors of
// each other.. metaprogram them.
link_internal b32
CreateDirectory(const char *zPath)
{
  b32 Result = False;
  if (PlatformCreateDir(zPath))
  {
    Result = True;
  }
  return Result;
}

link_internal b32
CreateDirectory(counted_string Filepath)
{
  const char* zPath = GetNullTerminated(Filepath);
  b32 Result = CreateDirectory(zPath);
  return Result;
}

link_internal b32
TryCreateDirectory(const char* zPath)
{
  b32 Result = True;
  if (FileExists(zPath) == False)
  {
    if (CreateDirectory(zPath) == False)
    {
      Result = False;
    }
  }
  return Result;
}

link_internal b32
TryCreateDirectory(counted_string Filepath)
{
  const char* zPath = GetNullTerminated(Filepath);
  b32 Result = TryCreateDirectory(zPath);
  return Result;
}







link_internal b32
DeleteDirectory(const char *zPath)
{
  b32 Result = False;
  if (PlatformDeleteDir(zPath))
  {
    Result = True;
  }
  return Result;
}

link_internal b32
DeleteDirectory(counted_string Filepath)
{
  const char* zPath = GetNullTerminated(Filepath);
  b32 Result = DeleteDirectory(zPath);
  return Result;
}
link_internal b32
TryDeleteDirectory(const char* zPath)
{
  b32 Result = True;
  if (FileExists(zPath))
  {
    if (DeleteDirectory(zPath) == False)
    {
      Result = False;
    }
  }
  return Result;
}

link_internal b32
TryDeleteDirectory(counted_string Filepath)
{
  const char* zPath = GetNullTerminated(Filepath);
  b32 Result = TryDeleteDirectory(zPath);
  return Result;
}







link_internal b32
CloseFile(native_file* File)
{
  b32 Result = False;
  if (File->Handle)
  {
    Result = fclose(File->Handle) == 0 ? True : False;
    File->Handle = 0;
  }
  else
  {
    Error("Attempted to close %S, which was not open.", File->Path);
  }

  if (!Result)
  {
    Error("Closing file %S", File->Path);
  }

  Assert(File->Handle == 0);
  return Result;
}

global_variable random_series TempFileEntropy = {3215432};

link_internal b32
Rename(counted_string CurrentFilePath, counted_string NewFilePath)
{
  counted_string TmpFilename = {};
  if (FileExists(NewFilePath))
  {
    TmpFilename = GetTmpFilename(&TempFileEntropy, TranArena);
    Rename(NewFilePath, TmpFilename);
  }


  const char* Old = GetNullTerminated(CurrentFilePath);
  const char* New = GetNullTerminated(NewFilePath);
  b32 Result = (rename(Old, New) == 0);

  if (Result)
  {
    if (TmpFilename.Count)
    {
      Remove(TmpFilename);
    }
  }
  else
  {
    /* Info("Error renaming %S -> %S , errno(%d)", CurrentFilePath, NewFilePath, errno); */
    if (TmpFilename.Count)
    {
      Rename(TmpFilename, NewFilePath);
    }
  }

  return Result;
}

link_internal b32
Remove(counted_string Filepath)
{
  const char* NullTerminated = GetNullTerminated(Filepath);
  b32 Result = (remove(NullTerminated) == 0) ? True : False;
  return Result;
}

link_internal native_file
OpenFile(const char* FilePath, const char* Permissions)
{
  native_file Result = {
    .Path = CS(FilePath)
  };

  if (Permissions == 0)
  {
    Warn("Invalid Permissions value (null) passed to OpenFile");
  }

  if (FilePath == 0)
  {
    Warn("Invalid FilePath value (null) passed to OpenFile");
  }

  errno = 0;
  fopen_s(&Result.Handle, FilePath, Permissions);

  if (!Result.Handle)
  {
    switch(errno)
    {
      case 0: { } break;

      case EINVAL:
      {
        Warn("fopen_s failed with EINVAL on Filepath (%s) with Permissions (%s)", FilePath, Permissions);
      } break;

      default:
      {
        /* Warn("Error opening file %s. Errno==(%d) Message=(%s)", FilePath, errno, ErrnoToString(errno)); */
      } break;
    }

    errno = 0;
  }

  Assert(errno == 0);

  return Result;
}

link_internal native_file
OpenFile(counted_string FilePath, const char* Permissions)
{
  const char* NullTerminatedFilePath = GetNullTerminated(FilePath);
  native_file Result = OpenFile(NullTerminatedFilePath, Permissions);
  return Result;
}

link_internal counted_string
GetRandomString(u32 Length, random_series* Entropy, memory_arena* Memory)
{
  counted_string Filename = {
    .Count = Length,
    .Start = Allocate(char, Memory, Length),
  };

  for (u32 CharIndex = 0;
      CharIndex < Length;
      ++CharIndex)
  {
    s8 Try = (s8)RandomBetween(48u, Entropy, 122u);
    while (!IsAlphaNumeric(Try))
    {
      Try = (s8)RandomBetween(48u, Entropy, 122u);
    }
    ((char*)Filename.Start)[CharIndex] = Try;
  }

  return Filename;
}

link_internal counted_string
GetRandomString(u32 Length, umm EntropySeed, memory_arena* Memory)
{
  random_series Entropy = { .Seed = EntropySeed };
  counted_string Result = GetRandomString(Length, &Entropy, Memory);
  return Result;
}

link_internal counted_string
GetTmpFilename(random_series* Entropy, memory_arena* Memory)
{
  counted_string Filename = GetRandomString(32, Entropy, Memory);
  Filename = Concat(CSz(TMP_DIR_ROOT), Filename, Memory);
  return Filename;
}

link_internal native_file
GetTempFile(random_series* Entropy, memory_arena* Memory)
{
  counted_string Filename = GetTmpFilename(Entropy, Memory);
  native_file Result = OpenFile(Filename, "w+b");
  if (!Result.Handle)
    { Warn("Error opening tmpfile %S, errno: %d", Filename, errno); }
  return Result;
}

link_internal inline b32
WriteToFile(native_file* File, u8 *Buf, umm Count)
{
  b32 Result = False;
  umm BytesWriten = fwrite(Buf, 1, Count, File->Handle);
  if (BytesWriten == Count)
  {
    Result = True;
  }
  else
  {
    Warn("Error Writing to file %.*s", (s32)File->Path.Count, File->Path.Start);
  }
  return Result;
}

link_internal inline b32
WriteToFile(native_file* File, u64 Value)
{
  b32 Result = WriteToFile(File, (u8*)&Value, sizeof(Value));
  return Result;
}

link_internal inline b32
WriteToFile(native_file* File, u32 Value)
{
  b32 Result = WriteToFile(File, (u8*)&Value, sizeof(Value));
  return Result;
}

link_internal inline b32
WriteToFile(native_file* File, counted_string Str)
{
  b32 Result = WriteToFile(File, (u8*)Str.Start, Str.Count);
  return Result;
}

link_internal inline b32
WriteToFile(native_file* File, ansi_stream *Str)
{
  b32 Result = WriteToFile(File, CountedString(Str));
  return Result;
}

link_internal b32
ReadBytesIntoBuffer(FILE *Src, umm BytesToRead, u8* Dest)
{
  Assert(BytesToRead);
  u64 BytesRead = fread(Dest, 1, BytesToRead, Src);
  b32 Result = BytesRead == BytesToRead;
  return Result;
}

link_internal b32
ReadBytesIntoBuffer(native_file *Src, umm BytesToRead, u8* Dest)
{
  b32 Result = ReadBytesIntoBuffer(Src->Handle, BytesToRead, Dest);
  return Result;
}

link_internal b32
FileExists(const char* Path)
{
  b32 Result = False;

  native_file File = OpenFile(Path, "r");
  if (File.Handle)
  {
    Result = True;
    if (!CloseFile(&File))
    {
      Error("Opened %s, but could not close it.", Path);
      Result = False;
    }
  }

  return Result;
}

link_internal b32
FileExists(counted_string Path)
{
  const char* NullTerminatedFilePath = GetNullTerminated(Path);
  b32 Result = FileExists(NullTerminatedFilePath);
  return Result;
}

link_internal void
PrintToStdout(counted_string Output)
{
  if (!WriteToFile(&Stdout, Output))
  {
    Error("Writing to Stdout");
  }

#if BONSAI_WIN32
  /* temp_memory_handle TMH = BeginTemporaryMemory(TranArena, False); */
  /* OutputDebugString(GetNullTerminated(Output, TranArena)); */
  /* EndTemporaryMemory(&TMH, False); */
#endif

}

