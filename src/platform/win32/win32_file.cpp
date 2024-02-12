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
#if 1
  s64 Result= _ftelli64(File->Handle);
#else
  umm Result = 0;
  LARGE_INTEGER Size;
  if (GetFileSizeEx(File->Handle, &Size))
  {
    Result = umm(Size.QuadPart);
  }
  else
  {
    Win32PrintLastError();
    SoftError("GetFileSizeEx failed on (%S). Handle(%p)", File->Path, File->Handle);
  }

#endif
  return SafeTruncateToUMM(Result);
}

link_internal native_file
PlatformOpenFile(const char *Filepath)
{
  native_file Result = {};
  NotImplemented;
  return Result;
}
