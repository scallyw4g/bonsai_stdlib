link_internal b32
SearchForProjectRoot(void)
{
#if EMCC
  b32 Result = True;
#else
  b32 Result = FileExists(".root_marker");

  b32 ChdirSuceeded = True;
  b32 NotAtFilesystemRoot = True;

  /* ChdirSuceeded = (_chdir("/home/scallywag/bonsai") == 0); */
  while (!Result && ChdirSuceeded && NotAtFilesystemRoot)
  {
    ChdirSuceeded = PlatformChangeDirectory("..");
    NotAtFilesystemRoot = (!IsFilesystemRoot(GetCwd()));
    Result = FileExists(".root_marker");
  }
#endif
  return Result;
}

link_internal maybe_file_traversal_node
FindFileHelper(file_traversal_node Node, u64 StringPtr)
{
  maybe_file_traversal_node Result = {};

  if ( StringsMatch(&Node.Name, Cast(cs *, StringPtr)) )
  {
    Result.Tag = Maybe_Yes;
    Result.Value = Node;
  }
  return Result;
}

link_internal maybe_file_traversal_node
FindStdlibAssetsMarker(void)
{
  cs RootMarkerName = CSz(".bonsai_stdlib_assets_marker");
  maybe_file_traversal_node Result = PlatformTraverseDirectoryTreeUnordered(CSz("."), FindFileHelper, Cast(u64, &RootMarkerName));
  return Result;
}
