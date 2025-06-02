
#include <ftw.h>

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

link_internal b32
PlatformRenameFile(cs CurrentFilePath, cs NewFilePath)
{
  cs TmpFilename = {};
  if (FileExists(NewFilePath))
  {
    local_persist random_series _TempFileEntropy = {3215432};

    TmpFilename = GetTmpFilename(&_TempFileEntropy, GetTranArena());
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
    .Path = CS(FilePath),
    .Handle = {},
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

    InvalidDefaultCase;
  }

  if (PermissionChars == 0)
  {
    Warn("Invalid Permissions value (null) passed to OpenFile");
  }

  if (FilePath == 0)
  {
    Warn("Invalid FilePath value (null) passed to OpenFile");
  }

  errno = 0;
  Result.Handle = fopen(FilePath, PermissionChars);

  if (Result.Handle == 0)
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
PlatformReadFile(native_file *Src, u8* Dest, umm BytesToRead)
{
  Assert(BytesToRead);
  u64 BytesRead = fread(Dest, 1, BytesToRead, Src->Handle);
  b32 Result = BytesRead == BytesToRead;
  Assert(Result);
  return Result;
}

link_internal b32
PlatformReadBytesIntoBuffer(native_file *Src, u8* Dest, umm BytesToRead)
{
  return PlatformReadFile(Src, Dest, BytesToRead);
}

link_internal void
Rewind(native_file *File)
{
  rewind(File->Handle);
}

global_variable directory_traversal_callback *Global_CurrentDirectoryTraversalCallback;
global_variable                          u64  Global_CurrentDirectoryTraversalUserData;
global_variable    maybe_file_traversal_node  Global_CurrentDirectoryTraversalResult;
global_variable                          b32  Global_CurrentDirectoryTraversalSkipFirstWin32Compat;

link_internal s32
LinuxDirectoryTraversalCallback(const char *FilePath, const struct stat *Stat, s32 TypeFlag, struct FTW *FTWBuf)
{
  /* Info("Visiting File (%s)", FilePath); */

  // NOTE(Jesse): The windows platform layer doesn't call us back for the root
  // directory.  The default linux behavior makes more sense (?), but it's more
  // annoying to put that behavior into the win32 layer, so I'm just doing this
  // until I have a concrete reason to prefer one over the other.
  if (Global_CurrentDirectoryTraversalSkipFirstWin32Compat) { Global_CurrentDirectoryTraversalSkipFirstWin32Compat = False; }
  else
  {
    cs Path = CopyString(FilePath, GetTranArena());
    file_traversal_type Type = FileTraversalType_None;
    switch (TypeFlag)
    {
      case FTW_F: { Type = FileTraversalType_File; } break;
      case FTW_D: { Type = FileTraversalType_Dir;  } break;
    }
    file_traversal_node Node = {Type, Dirname(Path), Basename(Path) };

    maybe_file_traversal_node MaybeNode = (*Global_CurrentDirectoryTraversalCallback)(Node, Global_CurrentDirectoryTraversalUserData);
    if (MaybeNode.Tag) { Global_CurrentDirectoryTraversalResult = MaybeNode; }
  }

  return 0; // Tell nftw to continue the traversal
}

link_internal maybe_file_traversal_node
PlatformTraverseDirectoryTreeUnordered(cs Dirname, directory_traversal_callback Callback, u64 UserData)
{
  // NOTE(Jesse): This function cannot run concurrently without some extra
  // leg-work because nftw doesn't give us a user-supplied parameter.
  Assert(Global_CurrentDirectoryTraversalCallback == 0);
  Assert(Global_CurrentDirectoryTraversalUserData == 0);
  Global_CurrentDirectoryTraversalCallback = &Callback;
  Global_CurrentDirectoryTraversalUserData = UserData;
  Global_CurrentDirectoryTraversalResult = {};
  Global_CurrentDirectoryTraversalSkipFirstWin32Compat = True;


  struct stat Stat = {};
  s32 TraversalResult = nftw(GetNullTerminated(Dirname), LinuxDirectoryTraversalCallback, 32, 0);

  Global_CurrentDirectoryTraversalCallback = 0;
  Global_CurrentDirectoryTraversalUserData = 0;

  maybe_file_traversal_node Result = Global_CurrentDirectoryTraversalResult;
  return Result;
}

#define INVALID_FILE_SIZE (umm(-1))
link_internal umm
PlatformGetFileSize(native_file *File)
{
  // NOTE(Jesse): I lifted this out of U8_StreamFromFile when I ported Windows
  // to use it's native file type.  I didn't actually test it on linux, so I'm
  // leaving this here until I get a chance to.
  /* NotImplemented; */

  errno = 0;
  fseek(File->Handle, 0L, SEEK_END);
  Assert(errno==0);

  errno = 0;
  umm Result = (umm)ftell(File->Handle);
  rewind(File->Handle);

  return Result;
}
