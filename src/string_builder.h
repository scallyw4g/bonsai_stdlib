struct string_builder
{
  counted_string_block_array Chunks;
};

link_internal string_builder StringBuilder();
link_internal void Append(string_builder* Builder, counted_string String);
link_internal void Prepend(string_builder* Builder, counted_string String);

