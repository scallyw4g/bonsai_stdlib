struct string_builder
{
  memory_arena* Memory = AllocateArena();
  counted_string_stream Chunks;
};

link_internal string_builder
StringBuilder(memory_arena *Memory)
{
  string_builder Result = {Memory, {}};
  return Result;
}

link_internal void
Append(string_builder* Builder, counted_string String)
{
  Push(&Builder->Chunks, String);
}


