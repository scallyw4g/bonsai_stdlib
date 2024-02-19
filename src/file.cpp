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
CreateDirectory(cs Filepath)
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
TryCreateDirectory(cs Filepath)
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
DeleteDirectory(cs Filepath)
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
TryDeleteDirectory(cs Filepath)
{
  const char* zPath = GetNullTerminated(Filepath);
  b32 Result = TryDeleteDirectory(zPath);
  return Result;
}







link_internal native_file
OpenFile(const char *FilePath, file_permission Permissions)
{
  return PlatformOpenFile(FilePath, Permissions);
}

link_internal native_file
OpenFile(cs FilePath, file_permission Permissions)
{
  const char* NullTerminatedFilePath = GetNullTerminated(FilePath);
  native_file Result = PlatformOpenFile(NullTerminatedFilePath, Permissions);
  return Result;
}

link_internal b32 CloseFile(native_file* File)
{
  return PlatformCloseFile(File);
}

link_internal b32 Rename(cs CurrentFilePath, cs NewFilePath)
{
  return PlatformRenameFile(CurrentFilePath, NewFilePath);
}

link_internal cs
GetRandomString(u32 Length, random_series* Entropy, memory_arena* Memory)
{
  cs Filename = {
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

link_internal cs
GetRandomString(u32 Length, umm EntropySeed, memory_arena* Memory)
{
  random_series Entropy = { .Seed = EntropySeed };
  cs Result = GetRandomString(Length, &Entropy, Memory);
  return Result;
}

link_internal cs
GetTmpFilename(random_series* Entropy, memory_arena* Memory)
{
  cs Filename = GetRandomString(32, Entropy, Memory);
  Filename = Concat(CSz(TMP_DIR_ROOT), Filename, Memory);
  return Filename;
}

link_internal native_file
GetTempFile(random_series* Entropy, memory_arena* Memory)
{
  cs Filename = GetTmpFilename(Entropy, Memory);
  native_file Result = OpenFile(Filename, FilePermission_Write);
  if (!Result.Handle) { Warn("Error opening tmpfile %S", Filename); }
  return Result;
}

link_internal inline b32
WriteToFile(native_file* File, u64 Value)
{
  b32 Result = PlatformWriteToFile(File, (u8*)&Value, sizeof(Value));
  return Result;
}

link_internal inline b32
WriteToFile(native_file* File, u32 Value)
{
  b32 Result = PlatformWriteToFile(File, (u8*)&Value, sizeof(Value));
  return Result;
}

link_internal inline b32
WriteToFile(native_file* File, cs Str)
{
  b32 Result = PlatformWriteToFile(File, (u8*)Str.Start, Str.Count);
  return Result;
}

link_internal inline b32
WriteToFile(native_file* File, ansi_stream *Str)
{
  b32 Result =WriteToFile(File, CountedString(Str));
  return Result;
}

link_internal inline b32
WriteToFile(native_file* File, u8_cursor_block_array *Buf)
{
  b32 Result = True;

  IterateOver(Buf, Cursor, CursorIndex)
  {
    Result &= PlatformWriteToFile(File, Cursor->Start, AtElements(Cursor));
  }

  return Result;
}

link_internal b32
ReadBytesIntoBuffer(u8_cursor *Src, u8* Dest, umm BytesToRead)
{
  b32 Result = Src->At+BytesToRead <= Src->End;
  if (Result)
  {
    CopyMemory(Src->At, Dest, BytesToRead);
    Src->At += BytesToRead;
  }
  return Result;
}

link_internal b32
ReadBytesIntoBuffer(native_file *Src, u8* Dest, umm BytesToRead)
{
  b32 Result = PlatformReadBytesIntoBuffer(Src, Dest, BytesToRead);
  return Result;
}

// TODO(Jesse): There's sure to be a much more efficient way to do this.  I'm
// completely sure that's true on Windows, not sure about other platforms.
link_internal b32
FileExists(const char* Path)
{
  b32 Result = False;

  native_file File = PlatformOpenFile(Path, FilePermission_Read);
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
FileExists(cs Path)
{
  const char* NullTerminatedFilePath = GetNullTerminated(Path);
  b32 Result = FileExists(NullTerminatedFilePath);
  return Result;
}

global_variable bonsai_futex Lock = {};

link_internal void
PrintToStdout(cs Output)
{
  if (Stdout.Handle == 0) { PlatformInitializeStdout(&Stdout); }

  if (Lock.Initialized == False) { InitializeFutex(&Lock); }

  // TODO(Jesse): What's a better way than printf of notifying the user an error occurred if we can't write to stdout???
  if (ThreadLocal_ThreadIndex != INVALID_THREAD_LOCAL_THREAD_INDEX) { AcquireFutex(&Lock); }
  if (!WriteToFile(&Stdout, Output)) { printf("Error writing to stdout."); }
  if (ThreadLocal_ThreadIndex != INVALID_THREAD_LOCAL_THREAD_INDEX) { ReleaseFutex(&Lock); }

#if BONSAI_WIN32
  /* temp_memory_handle TMH = BeginTemporaryMemory(GetTranArena(), False); */
  /* OutputDebugString(GetNullTerminated(Output, GetTranArena())); */
  /* EndTemporaryMemory(&TMH, False); */
#endif
}


link_internal file_traversal_node
DeepCopy(heap_allocator *Memory, file_traversal_node *Node)
{
  file_traversal_node Result = {};

  Result.Type = Node->Type;
  Result.Dir = CopyString(Node->Dir, Memory);
  Result.Name = CopyString(Node->Name, Memory);

  return Result;
}
