struct random_series;
struct memory_arena;
struct ansi_stream;


link_internal b32 CloseFile(native_file* File);

link_internal b32 Rename(counted_string CurrentFilePath, counted_string NewFilePath);

link_internal b32 Remove(counted_string Filepath);

link_internal native_file OpenFile(const char* FilePath, const char* Permissions);

link_internal native_file OpenFile(counted_string FilePath, const char* Permissions);

link_internal counted_string GetRandomString(u32 Length, random_series* Entropy, memory_arena* Memory);

link_internal counted_string GetRandomString(u32 Length, umm EntropySeed, memory_arena* Memory);

link_internal counted_string GetTmpFilename(random_series* Entropy, memory_arena* Memory);

link_internal native_file GetTempFile(random_series* Entropy, memory_arena* Memory);

link_internal inline b32 WriteToFile(native_file* File, counted_string Str);

link_internal inline b32 WriteToFile(native_file* File, ansi_stream* Str);

link_internal b32 FileExists(const char* Path);

link_internal b32 FileExists(counted_string Path);

link_internal b32 ReadBytesIntoBuffer(native_file *Src, umm BytesToRead, u8* Dest);
