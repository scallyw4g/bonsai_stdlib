struct random_series;
struct memory_arena;
struct ansi_stream;


enum file_permission
{
  FilePermission_None  = 0,
  FilePermission_Read  = (1 << 0),
  FilePermission_Write = (1 << 1),
};

link_internal b32 CloseFile(native_file* File);

link_internal b32 Rename(counted_string CurrentFilePath, counted_string NewFilePath);

link_internal b32 Remove(counted_string Filepath);

link_internal native_file OpenFile(const char* FilePath, file_permission Permissions);

link_internal native_file OpenFile(counted_string FilePath, file_permission Permissions);

link_internal counted_string GetRandomString(u32 Length, random_series* Entropy, memory_arena* Memory);

link_internal counted_string GetRandomString(u32 Length, umm EntropySeed, memory_arena* Memory);

link_internal counted_string GetTmpFilename(random_series* Entropy, memory_arena* Memory);

link_internal native_file GetTempFile(random_series* Entropy, memory_arena* Memory);

link_internal inline b32 WriteToFile(native_file* File, counted_string Str);

link_internal inline b32 WriteToFile(native_file* File, ansi_stream* Str);

link_internal b32 FileExists(const char* Path);

link_internal b32 FileExists(counted_string Path);

link_internal b32 ReadBytesIntoBuffer(native_file *Src, u8* Dest, umm BytesToRead);


enum file_traversal_type
{
  FileTraversalType_None,

  FileTraversalType_Dir,
  FileTraversalType_File,
};

struct file_traversal_node
{
  file_traversal_type Type;
  cs Dir;
  cs Name;
};

poof(are_equal(file_traversal_node))
#include <generated/are_equal_file_traversal_node.h>

poof(maybe(file_traversal_node))
#include <generated/maybe_file_traversal_node.h>

typedef maybe_file_traversal_node (*directory_traversal_callback)(file_traversal_node, u64 UserData);
