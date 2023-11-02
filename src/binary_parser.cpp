
poof(buffer(u32))
#include <generated/buffer_u32.h>

/* poof(deep_copy(u32_buffer)) */
/* #include <generated/deep_copy_u32_buffer.h> */

poof(stream_and_cursor(u32))
#include <generated/stream_and_cursor_u32.h>

poof(generate_cursor(u8))
#include <generated/generate_cursor_u8.h>

poof(deep_copy(u32_cursor))
#include <generated/deep_copy_u32_stream.h>

poof(generate_stream_compact(u32))
#include <generated/generate_stream_compact_u32.h>

typedef u32_cursor* u32_cursor_ptr;

poof(buffer(u64))
#include <generated/buffer_u64.h>

poof(stream_and_cursor(u64))
#include <generated/stream_and_cursor_u64.h>

poof(generate_stream_compact(u64))
#include <generated/generate_stream_compact_u64.h>


poof(stream_and_cursor(s32))
#include <generated/stream_and_cursor_s32.h>


poof(generate_cursor(v3))
#include <generated/generate_cursor_v3.h>

u32_cursor
U32Cursor(u32* Start, u32* End)
{
  u32_cursor Result = {
    Start,
    Start,
    End,
    /* OWNED_BY_THREAD_MEMBER_INIT() */
  };
  return Result;
}

template <typename stream_t, typename element_t>inline stream_t
AllocateBuffer(u32 Count, memory_arena* Memory)
{
  element_t* Start = Allocate(element_t, Memory, Count);
  stream_t Result = {
    .Start = Start,
    .At = Start,
    .End = Start + Count,
  };

  return Result;
}

r32_stream
R32_Stream(u32 Count, memory_arena *Memory)
{
  r32 *Elements = Allocate(r32, Memory, Count);
  r32_stream Result = {};

  Result.Start = Elements;
  Result.At = Elements;
  Result.End = Elements + Count;

  return Result;
}

#if 0
u32_cursor
U32Cursor(u32 Count, memory_arena *Memory)
{
  u32 *Elements = Allocate(u32, Memory, Count);
  u32_cursor Result = {};

  Result.Start = Elements;
  Result.At = Elements;
  Result.End = Elements + Count;

  return Result;
}
#endif

#if 0
v3_cursor
V3Cursor(u32 Count, memory_arena *Memory)
{
  v3 *Elements = Allocate(v3, Memory, Count);
  v3_cursor Result = {};

  Result.Start = Elements;
  Result.At = Elements;
  Result.End = Elements + Count;

  return Result;
}
#endif

u8_stream
U8_StreamFromFile(const char* SourceFile, memory_arena *Memory)
{
  TIMED_FUNCTION();

  u8* FileContents = 0;
  umm FileSize = 0;

  native_file File = OpenFile(SourceFile, "r+b");
  if (File.Handle)
  {
    fseek(File.Handle, 0L, SEEK_END);
    FileSize = (umm)ftell(File.Handle);
    if (FileSize)
    {
      rewind(File.Handle);
      FileContents = (u8*)AllocateProtection(u8, Memory, FileSize, False);
      ReadBytesIntoBuffer(File.Handle, FileSize, FileContents);
    }
    else
    {
      Warn("File %s is empty!", SourceFile);
    }

    CloseFile(&File);
  }
  else
  {
    Error("Opening %s", SourceFile);
  }

  u8_stream Result = {
    FileContents,
    FileContents,
    FileContents + FileSize
  };

  return Result;
}

u8_stream
U8_StreamFromFile(counted_string SourceFile, memory_arena *Memory)
{
  u8_stream Result = U8_StreamFromFile( GetNullTerminated(SourceFile), Memory );
  return Result;
}

ansi_stream
AnsiStreamFromFile(const char* SourceFile, memory_arena *Memory)
{
  TIMED_FUNCTION();

  u8_stream Binary = U8_StreamFromFile(SourceFile, Memory);
  ansi_stream Result = AnsiStream(&Binary);
  Result.Filename = CS(SourceFile);

  return Result;
}

ansi_stream
AnsiStreamFromFile(counted_string SourceFile, memory_arena *Memory)
{
  const char* NullTerminated = GetNullTerminated(SourceFile, Memory);
  ansi_stream Result = AnsiStreamFromFile(NullTerminated, Memory);
  return Result;
}

ansi_stream
ReadEntireFileIntoAnsiStream(counted_string SourceFile, memory_arena *Memory)
{
  ansi_stream Stream = AnsiStreamFromFile(SourceFile, Memory);
  return Stream;
}

counted_string
CS(u8_stream Stream)
{
  counted_string Result = {
    .Count = TotalSize(&Stream),
    .Start = (const char*)Stream.Start,
  };
  return Result;
}

counted_string
ReadEntireFileIntoString(counted_string SourceFile, memory_arena *Memory)
{
  counted_string Stream = CS(U8_StreamFromFile(SourceFile, Memory));
  return Stream;
}

char *
ReadUntilTerminatorList(ansi_stream *Cursor, const char *TerminatorList, memory_arena *Arena)
{
  /* TODO(Jesse, id: 146, tags: robustness, cleanup): Make this return a
   * counted string and everything that depends on it _NOT_ rely on the fact
   * it's currently null terminated
   */

  counted_string String = ReadUntilTerminatorList(Cursor, TerminatorList);

  char *Result = AllocateProtection(char, Arena, String.Count + 1, False);
  MemCopy((u8*)String.Start, (u8*)Result, String.Count);

  return Result;
}

char *
PopWord(ansi_stream *Cursor, memory_arena *Arena, const char *Delimeters = 0)
{
  if (!Delimeters)
    Delimeters = " \n\r";

  EatWhitespace(Cursor);
  char *Result = ReadUntilTerminatorList(Cursor, Delimeters, Arena);
  EatAllCharacters(Cursor, Delimeters);
  return Result;
}

r32
PopFloat(ansi_stream *Cursor, memory_arena *Arena)
{
  char *Float = PopWord(Cursor, Arena);
  r32 Result = (r32)atof(Float);
  return Result;
}

u32
PopU32(ansi_stream *Cursor, memory_arena *Arena, const char* Delim = 0)
{
  char *Str = PopWord(Cursor, Arena, Delim);
  u32 Result = (u32)atoi(Str);
  return Result;
}

counted_string
PopQuotedCharLiteral(ansi_stream* Cursor, b32 IncludeQuotes = False)
{
  if (*Cursor->At == '\'')
  {
    Advance(Cursor);
  }
  else
  {
    if (Cursor->At-1 >= Cursor->Start)
    {
      Assert(*(Cursor->At-1) == '\'' );
    }
    else
    {
      Warn("Tried to pop a singularly quoted string off a stream that doesn't start with one!");
    }
  }

  char Terminator[2] = {};
  Terminator[0] = '\'';

  counted_string Result = ReadUntilTerminatorList(Cursor, Terminator, True);
  if (IncludeQuotes)
  {
    --Result.Start;
    Result.Count += 2;
    Assert(Cursor->Start <= Result.Start);
    Assert(Result.Start+Result.Count <= Cursor->End);
  }

  return Result;
}

counted_string
PopQuotedString(ansi_stream* Cursor, b32 IncludeQuotes = False)
{
  if (*Cursor->At == '"' || *Cursor->At == '\'' )
  {
    Advance(Cursor);
  }
  else
  {
    if (Cursor->At-1 >= Cursor->Start)
    {
      Assert( *(Cursor->At-1) == '"' || *(Cursor->At-1) == '\'' );
    }
    else
    {
      Warn("Tried to pop a quoted string off a stream that doesn't start with one!");
    }
  }

  char Terminator[2] = {};
  Terminator[0] = *(Cursor->At-1);

  counted_string Result = ReadUntilTerminatorList(Cursor, Terminator, true);
  if (IncludeQuotes)
  {
    --Result.Start;
    Result.Count += 2;
    Assert(Cursor->Start <= Result.Start);
    Assert(Result.Start+Result.Count <= Cursor->End);
  }
  return Result;
}

char *
PopLine(ansi_stream *Cursor, memory_arena *Arena)
{
  char *Result = ReadUntilTerminatorList(Cursor, "\n", Arena);
  EatWhitespace(Cursor);
  return Result;
}

v3_cursor
ParseV3Array(u32 ElementCount, ansi_stream FloatStream, memory_arena* Memory)
{
  v3_cursor Result = V3Cursor(ElementCount, Memory);

  for (umm DestIndex = 0;
      DestIndex < ElementCount;
      ++DestIndex)
  {
    for (u32 Inner = 0;
         Inner < 3;
        ++Inner)
    {
      counted_string Float = PopWordCounted(&FloatStream);
      Result.Start[DestIndex].E[Inner] = ToF64(&Float);
    }
    ++Result.At;
  }

  return Result;
}

r32_stream
ParseFloatArray(u32 TotalFloatCount, ansi_stream FloatStream, memory_arena* Memory)
{
  r32_stream Result = R32_Stream(TotalFloatCount, Memory);

  for (umm DestIndex = 0;
      DestIndex < TotalFloatCount;
      ++DestIndex)
  {
    counted_string Float = PopWordCounted(&FloatStream);
    Result.Start[DestIndex] = ToF64(&Float);
  }

  return Result;
}

#if 0
void
Dump(v3_cursor* Array)
{
  umm ElementCount = AtElements(Array);

  for (umm ElementIndex = 0;
      ElementIndex < ElementCount;
      ++ElementIndex)
  {
    Print(Array->Start[ElementIndex]);
  }

  return;
}

void
Dump(xml_tag_hashtable *Table)
{
  for (u32 ElementIndex = 0;
      ElementIndex < Table->Size;
      ++ElementIndex)
  {
    xml_tag* Element = GetByHash(ElementIndex, Table->Elements);
    if (Element)
    {
      Print(ElementIndex);
      Print(Element);
      while ( (Element = Element->NextInHash) )
      {
        Print(Element);
      }
    }
  }

  return;
}

void
Dump(xml_token_stream *Stream, umm TokenCount)
{
  for (u32 TokenIndex = 0;
      TokenIndex < TokenCount;
      ++TokenIndex)
  {
    Print(Stream->Start + TokenIndex);
  }
}
#endif

//
// Little endian
//

poof(
  func gen_read_primitive_funcs_little_endian(type_poof_symbol PrimitiveTypes)
  {
    PrimitiveTypes.map(prim)
    {
      inline prim.name
      Read_(prim.name)(u8 *Source)
      {
        prim.name Result = *((prim.name)*)Source;
        return Result;
      }

      inline prim.name
      Read_(prim.name)((prim.name) *Source)
      {
        prim.name Result = *Source;
        return Result;
      }

      inline prim.name
      Read_(prim.name)(u8_stream *Source)
      {
        prim.name Result = Read_(prim.name)(Source->At);
        Source->At += sizeof((prim.name));
        Assert(Source->At <= Source->End);
        return Result;
      }

      inline prim.name*
      ReadArray_(prim.name)(u8_stream *Source, u32 Count)
      {
        prim.name *Result = ((prim.name)*)Source->At;
        Source->At += (Count * sizeof((prim.name)));
        Assert(Source->At <= Source->End);
        return Result;
      }


    }
  }
)

inline u8
Read_u8(u8 *Source)
{
  u8 Result = *Source;
  return Result;
}

inline u8
Read_u8(u8_stream *Source)
{
  u8 Result = Read_u8(Source->At);
  Source->At += sizeof(u8);
  Assert(Source->At <= Source->End);
  return Result;
}

inline u8*
ReadArray_u8(u8_stream *Source, u32 Count)
{
  u8 *Result = (u8*)Source->At;
  Source->At += (Count * sizeof(u8));
  Assert(Source->At <= Source->End);
  return Result;
}

poof(gen_read_primitive_funcs_little_endian({s8 s16 u16 s32 u32 s64 u64}))
#include <generated/gen_read_primitive_funcs_624166848.h>


//
// Big endian
//



inline u8
Read_u8_be(u8* Source)
{
  u8 Result = Read_u8(Source);
  return Result;
}

inline s16
Read_s16_be(s16* Source)
{
  s16 Result = (((u8*)Source)[0]*256) + ((u8*)Source)[1];
  return Result;
}

inline s16
Read_s16_be(u8* Source)
{
  s16 Result = (Source[0]*256) + Source[1];
  return Result;
}

inline u16
Read_u16_be(u16* Source)
{
  u16 Result = (((u8*)Source)[0]*256) + ((u8*)Source)[1];
  return Result;
}

inline u16
Read_u16_be(u8* Source)
{
  u16 Result = (Source[0]*256) + Source[1];
  return Result;
}

inline s64
Read_s64_be(u8* Source)
{
  s64 Result = (s64)( ((u64)Source[0]<<56) + ((u64)Source[1]<<48) + ((u64)Source[2]<<40) + ((u64)Source[3]<<32) + ((u64)Source[4]<<24) + ((u64)Source[5]<<16) + ((u64)Source[6]<<8) + ((u64)Source[7]) );
  return Result;
}

inline u32
Read_u32_be(u8* Source)
{
  u32 Result = (u32)( (Source[0]<<24) + (Source[1]<<16) + (Source[2]<<8) + Source[3] );
  return Result;
}

inline u8*
ReadArray_u8_be(u8_stream *Source, u32 Count)
{
  u8 *Result = Source->At;
  Source->At += Count;
  Assert(Source->At <= Source->End);
  return Result;
}

inline u16*
ReadArray_u16_be(u8_stream *Source, u32 Count)
{
  u16 *Result = (u16*)Source->At;
  Source->At += sizeof(u16)*Count;
  Assert(Source->At <= Source->End);
  return Result;
}

inline s16*
ReadArray_s16_be(u8_stream *Source, u32 Count)
{
  s16 *Result = (s16*)Source->At;
  Source->At += sizeof(s16)*Count;
  Assert(Source->At <= Source->End);
  return Result;
}

inline u8
Read_u8_be(u8_stream *Source)
{
  u8 Result = Read_u8_be(Source->At);
  Source->At += sizeof(u8);
  Assert(Source->At <= Source->End);
  return Result;
}

inline s16
Read_s16_be(u8_stream *Source)
{
  s16 Result = Read_s16_be(Source->At);
  Source->At += sizeof(s16);
  Assert(Source->At <= Source->End);
  return Result;
}

inline u16
Read_u16_be(u8_stream *Source)
{
  u16 Result = Read_u16_be(Source->At);
  Source->At += sizeof(u16);
  Assert(Source->At <= Source->End);
  return Result;
}

inline s64
Read_s64_be(u8_stream *Source)
{
  s64 Result = Read_s64_be(Source->At);
  Source->At += sizeof(s64);
  Assert(Source->At <= Source->End);
  return Result;
}

inline u32
Read_u32_be(u8_stream *Source)
{
  u32 Result = Read_u32_be(Source->At);
  Source->At += sizeof(u32);
  Assert(Source->At <= Source->End);
  return Result;
}
