link_internal b32
SearchForProjectRoot(void)
{
  InvalidCodePath();
  return 0;
#if 0
#if EMCC
  b32 Result = True;
#else
  b32 Result = FileExists(".root_marker");

  b32 ChdirSuceeded = True;
  b32 NotAtFilesystemRoot = True;

  /* ChdirSuceeded = (_chdir("/home/scallywag/bonsai") == 0); */
  while (!Result && ChdirSuceeded && NotAtFilesystemRoot)
  {
    ChdirSuceeded = (_chdir("..") == 0);
    NotAtFilesystemRoot = (!IsFilesystemRoot(GetCwd()));
    Result = FileExists(".root_marker");
  }
#endif
  return Result;
#endif
}
