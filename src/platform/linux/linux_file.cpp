
link_internal b32
PlatformCloseFile(native_file* File)
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
PlatformRenameFile(cs CurrentFilePath, cs NewFilePath)
{
  cs TmpFilename = {};
  if (FileExists(NewFilePath))
  {
    TmpFilename = GetTmpFilename(&TempFileEntropy, GetTranArena());
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
PlatformRemoveFile(cs Filepath)
{
  const char* NullTerminated = GetNullTerminated(Filepath);
  b32 Result = (remove(NullTerminated) == 0) ? True : False;
  return Result;
}

link_internal native_file
PlatformOpenFile(const char* FilePath, file_permission Permissions)
{
  native_file Result = {
    .Path = CS(FilePath)
  };

  const char *PermissionChars = 0;
  switch (s32(Permissions))
  {
    case FilePermission_Read:
    {
      PermissionChars = "r";
    } break;

    case FilePermission_Write:
    {
      PermissionChars = "w";
    } break;

    case FilePermission_Read|FilePermission_Write:
    {
      PermissionChars = "w+";
    } break;

    InvalidDefaultCase();
  }

  /* cs PermStr = CS(Permissions); */
  /* if (!Contains(PermStr, CSz("b"))) */
  /* { */
  /*   Error("Files must be opened in binary mode."); */
  /* } */

  if (PermissionChars == 0)
  {
    Warn("Invalid Permissions value (null) passed to OpenFile");
  }

  if (FilePath == 0)
  {
    Warn("Invalid FilePath value (null) passed to OpenFile");
  }

  errno = 0;
  s32 Code = fopen(&Result.Handle, FilePath, PermissionChars);
  Assert(Code == 0);
  Assert(errno == 0);

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

link_internal inline b32
PlatformWriteToFile(native_file* File, u8 *Buf, umm Count)
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

link_internal b32
ReadBytesIntoBuffer(FILE *Src, umm BytesToRead, u8* Dest)
{
  Assert(BytesToRead);
  u64 BytesRead = fread(Dest, 1, BytesToRead, Src);
  b32 Result = BytesRead == BytesToRead;
  Assert(Result);
  return Result;
}

link_internal void
Rewind(native_file *File)
{
  rewind(File->Handle);
}

