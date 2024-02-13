inline b32
IsFilesystemRoot(const char *Filepath)
{
  b32 Result = ( strlen(Filepath) == 3 && Filepath[1] == ':' && Filepath[2] == '\\');
  return Result;
}

link_internal maybe_file_traversal_node
PlatformTraverseDirectoryTree(cs Dirname, directory_traversal_callback Callback, u64 UserData)
{
  TIMED_FUNCTION();

  maybe_file_traversal_node Result = {};

  WIN32_FIND_DATA FindFileDescriptor;
  HANDLE FindHandle = 0;

  cs SearchTokens = CSz("/*.*");

  cs FindFilePattern = Concat(Dirname, SearchTokens, GetTranArena());
  if( (FindHandle = FindFirstFile(FindFilePattern.Start, &FindFileDescriptor)) == INVALID_HANDLE_VALUE)
  {
    SoftError("Path not found (%S)", Dirname);
  }
  else
  {
    do
    {
      cs Filename = CopyString(FindFileDescriptor.cFileName, GetTranArena());
      if(StringsMatch(Filename, CSz(".")) || StringsMatch(Filename, CSz("..")))
      {
        // Skip . and ..
      }
      else
      {
        file_traversal_type Type = FileTraversalType_File;
        if(FindFileDescriptor.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
          Type = FileTraversalType_Dir;
          // TODO(Jesse): This should use a string builder
          cs SubDir = Concat(Dirname, CSz("/"), GetTranArena());
             SubDir = Concat( SubDir, Filename, GetTranArena());
          maybe_file_traversal_node MaybeNode = PlatformTraverseDirectoryTree(SubDir, Callback, UserData);
          if (MaybeNode.Tag)
          {
            Result = MaybeNode;
          }
        }

        file_traversal_node CBArg = {
          Type,
          Dirname,
          Filename,
        };

        // NOTE(Jesse): It might be better perf-wise to pass a pointer here, but
        // I'm scared of forgetting you're not allowed to save it and read some
        // random stack garbage afterwards
        maybe_file_traversal_node MaybeNode = Callback(CBArg, UserData);
        if (MaybeNode.Tag) { Result = MaybeNode; }
      }
    }
    while (FindNextFile(FindHandle, &FindFileDescriptor));

    FindClose(FindHandle);
  }

  return Result;
}

link_internal umm
PlatformGetFileSize(native_file *File)
{
  umm Result = 0;
  LARGE_INTEGER Size;
  if (GetFileSizeEx(File->Handle, &Size))
  {
    Result = umm(Size.QuadPart);
  }
  else
  {
    Win32PrintLastError();
    SoftError("GetFileSizeEx failed on file (%S). Handle(%p)", File->Path, File->Handle);
  }

  return SafeTruncateToUMM(Result);
}

// Functional behavior
// 1) FilePermission_Write
//    - succeeds if file exists
//    - fails    if file does not exist
//
// 2) FilePermission_Write
//    - succeeds and truncates if file exists
//    - succeeds and creates if file does not exist
//
// 3) TODO(Jesse): FilePermission_Append ?
//
link_internal native_file
PlatformOpenFile(const char *Filepath, file_permission Permissions)
{
  native_file Result = {};

  DWORD CreationBehavior = 0;
  if (Permissions & FilePermission_Read)
  {
    CreationBehavior = OPEN_EXISTING;
  }

  if (Permissions & FilePermission_Write)
  {
    CreationBehavior = OPEN_ALWAYS;
  }

  DWORD PlatPermissions = 0;
  file_permission CurrentPermissions = Permissions;
  while (CurrentPermissions)
  {
    u32 Bit = UnsetLeastSignificantSetBit(Cast(u32*, &CurrentPermissions));

    switch (file_permission(Bit))
    {
      InvalidCase(FilePermission_None);

      case FilePermission_Read:
      {
        PlatPermissions |= GENERIC_READ;
      } break;

      case FilePermission_Write:
      {
        PlatPermissions |= GENERIC_WRITE;
      } break;
    }
  }

  DWORD ShareMode = 0;
  HANDLE hFile = CreateFileA(Filepath, PlatPermissions, ShareMode, 0, CreationBehavior, FILE_ATTRIBUTE_NORMAL, 0);
  if (hFile == INVALID_HANDLE_VALUE)
  {
    SoftError("Opening File (%s)", Filepath);
    Win32PrintLastError();
  }
  else
  {
    Result.Path = CS(Filepath);
    Result.Handle = hFile;

    if (Permissions & FilePermission_Write)
    {
      if (SetEndOfFile(hFile) == 0)
      {
        SoftError("Unable to truncate file (%s)", Filepath);
      }
    }
  }



  return Result;
}

link_internal DWORD
Win32GetSectorSize()
{
  // NOTE(Jesse): use the current disk .. whatever that means.
  const char *DiskName = 0;

  DWORD Ignored[3];
  local_persist DWORD BytesPerSector = 0;

  if (BytesPerSector == 0)
  {
    Ensure(GetDiskFreeSpaceA(0,  &Ignored[0], &BytesPerSector, &Ignored[1], &Ignored[2]));
    Assert(BytesPerSector > 0);
  }

  return BytesPerSector;
}

link_internal b32
PlatformWriteToFile(native_file *File, u8* Bytes, umm Count)
{
  /* DWORD BytesPerSector = Win32GetSectorSize(); */

  umm TotalBytesWritten = 0;
  b32 Result = False;
  do
  {
    DWORD BytesWritten;
    DWORD BytesPerWrite = DWORD(Min(Gigabytes(2), Count-TotalBytesWritten));
    Result = (WriteFile(File->Handle, Bytes+TotalBytesWritten, BytesPerWrite, &BytesWritten, 0) != 0);
    TotalBytesWritten += u32(BytesWritten);

  } while (Result && TotalBytesWritten < Count);

  if (Result == False)
  {
    SoftError("Error Writing to file (%s)", File->Path.Start);
  }

  if (TotalBytesWritten != Count)
  {
    SoftError("Wrote (%llu)/(%llu) bytes to file (%s)", TotalBytesWritten, Count, File->Path.Start);
  }

  return Result;
}

link_internal b32
PlatformCloseFile(native_file *File)
{
  b32 Result = (CloseHandle(File->Handle) != 0);
  return Result;
}

link_internal b32 
PlatformReadFile(native_file *File, u8 *Dest, umm Count)
{
  /* DWORD BytesPerSector = Win32GetSectorSize(); */

  BOOL Result = False;

  umm TotalBytesRead = 0;
  do
  {
    DWORD BytesThisRead = 0;
    DWORD BytesPerRead = DWORD(Min(Gigabytes(2), Count-TotalBytesRead));
    Result = ReadFile(File->Handle, Dest+TotalBytesRead, BytesPerRead, &BytesThisRead, 0);
    TotalBytesRead += BytesThisRead;
  } while (Result && TotalBytesRead < Count);

  if (Result == False)
  {
    SoftError("Error reading file (%S)", File->Path);
  }

  if (TotalBytesRead != Count)
  {
    SoftError("Read (%llu)/(%llu) bytes from file (%s)", TotalBytesRead, Count, File->Path.Start);
  }

  return b32(Result);
}

link_internal b32
PlatformReadBytesIntoBuffer(native_file *Src, u8 *Dest, umm Count)
{
  b32 Result = PlatformReadFile(Src, Dest, Count);
  return Result;
}
