// TODO(Jesse): Seems like this and ansi_stream.h have a lot of overlap..

poof(buffer(u32))
#include <generated/buffer_u32.h>

/* poof(deep_copy(u32_buffer)) */
/* #include <generated/deep_copy_u32_buffer.h> */

poof(stream_and_cursor(u32))
#include <generated/stream_and_cursor_u32.h>

/* poof(generate_cursor(u8)) */
/* #include <generated/generate_cursor_u8.h> */

poof(deep_copy(u32_cursor))
#include <generated/deep_copy_u32_stream.h>

poof(generate_stream_compact(u32))
#include <generated/generate_stream_compact_u32.h>

typedef u32_cursor* u32_cursor_ptr;

poof(buffer(u64))
#include <generated/buffer_u64.h>

poof(generate_cursor_functions(u64))
#include <generated/generate_cursor_functions_u64.h>

poof(generate_stream(u64))
#include <generated/generate_stream_u64.h>

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

  u8_stream Result = {};

  native_file File = PlatformOpenFile(SourceFile, FilePermission_Read);
  if (File.Handle)
  {
    umm FileSize = PlatformGetFileSize(&File);
    if (FileSize)
    {
      if (FileSize != INVALID_FILE_SIZE)
      {
        u8 *FileContents = (u8*)AllocateProtection(u8, Memory, FileSize, False);
        ReadBytesIntoBuffer(&File, FileContents, FileSize);

        Result = {
          FileContents,
          FileContents,
          FileContents + FileSize
        };
      }
      else
      {
        SoftError("PlatformGetFileSize failed on file (%s).", SourceFile);
      }
    }
    else
    {
      Warn("File %s is empty!", SourceFile);
    }

    CloseFile(&File);
  }
  else
  {
    SoftError("Opening %s", SourceFile);
  }

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

poof(
  func gen_read_primitive_from_native_file(type_poof_symbol PrimitiveTypes)
  {
    PrimitiveTypes.map(prim)
    {
      link_internal prim.name
      Read_(prim.name)(native_file *File)
      {
        prim.name Result;
        Ensure( ReadBytesIntoBuffer(File, (u8*)&Result, sizeof((prim.name))) );
        return Result;
      }
    }
  }
)

poof(gen_read_primitive_from_native_file({s8 u8 s16 u16 s32 u32 s64 u64}))
#include <generated/gen_read_primitive_from_native_file_803324607.h>

//
// Little endian
//

poof(
  func gen_read_primitive_from_u8_cursor_little_endian(type_poof_symbol PrimitiveTypes)
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

poof(gen_read_primitive_from_u8_cursor_little_endian({s8 s16 u16 s32 u32 s64 u64}))
#include <generated/gen_read_primitive_from_u8_cursor_little_endian_851742148.h>



// TODO(Jesse): Make the rest of the reading routines match this prototype (return b32, read into pointer)
// TODO(Jesse): This is more like SnapPointerTo_bytes ..
link_internal b32
Read_bytes(u8_cursor *Cursor, u8 **Dest, umm Count)
{
  b32 Result = False;

  if (Cursor->At + Count < Cursor->End)
  {
    Result = True;
    *Dest = Cursor->At;
    Cursor->At += Count;
  }

  return Result;
}

link_internal b32
Copy_bytes(u8_cursor *Cursor, u8 *Dest, umm Count)
{
  b32 Result = False;

  if (Cursor->At + Count < Cursor->End)
  {
    CopyMemory(Cursor->At, Dest, Count);
    Result = True;
    Cursor->At += Count;
  }

  return Result;
}


#define Read_struct(Buf, StructPtr) \
    Copy_bytes(Buf, Cast(u8*, (StructPtr)), sizeof((StructPtr)[0]))

poof(
  func gen_read_primitive(type)
  {
    link_internal bool
    Read_(type.name)(u8_cursor *Cursor, (type.name) *Dest)
    {
      b32 Result = False;
      u8 *Bytes = {};
      if (Read_bytes(Cursor, &Bytes, sizeof((type.name))))
      {
        *Dest = *((type.name)*)Bytes;
        Result = True;
      }
      return Result;
    }
  }
)

poof(gen_read_primitive(u32))
#include <generated/gen_read_primitive_u32.h>

poof(gen_read_primitive(u64))
#include <generated/gen_read_primitive_u64.h>



link_internal b32
Write(u8_stream *Dest, u8 *Src, umm Count)
{
  b32 Result = Dest->At+Count <= Dest->End;
  if (Result)
  {
    MemCopy(Src, Dest->At, Count);
    Dest->At += Count;
  }
  return Result;
}

link_internal b32
Write(u8_cursor_block_array *Dest, u8 *Src, umm Count)
{
  if (Dest->Memory == 0)
  {
    Dest->BlockSize = Megabytes(32);
    Dest->Memory = AllocateArena(Dest->BlockSize);
    u8_cursor First = U8Cursor(Dest->BlockSize, Dest->Memory);
    Ensure( Push(Dest, &First) );
  }

  u8_cursor *Last = GetPtr(Dest, LastIndex(Dest));

  b32 Result = False;
  if (Last)
  {
    Result = Write(Last, Src, Count);
    if (Result == False)
    {
      Assert(Dest->BlockSize);
      Assert(Dest->BlockSize >= Count);
      u8_cursor Next = U8Cursor(Dest->BlockSize, Dest->Memory);
      Result = Write(&Next, Src, Count);
      Ensure( Push(Dest, &Next) );
    }
  }
  else
  {
    Assert(False);
  }

  return Result;
}



poof(
  func gen_write_primitive_from_u8_stream_little_endian(type_poof_symbol PrimitiveTypes)
  {
    PrimitiveTypes.map(prim)
    {
      link_internal b32
      Write(u8_stream *Dest, prim.name *Src)
      {
        b32 Result = Write(Dest, (u8*)Src, sizeof((prim.name)));
        return Result;
      }

      link_internal b32
      Write(u8_stream *Dest, prim.name Src)
      {
        b32 Result = Write(Dest, (u8*)&Src, sizeof((prim.name)));
        return Result;
      }

      link_internal b32
      Write(u8_cursor_block_array *Dest, prim.name *Src)
      {
        b32 Result = Write(Dest, (u8*)Src, sizeof((prim.name)));
        return Result;
      }

      link_internal b32
      Write(u8_cursor_block_array *Dest, prim.name Src)
      {
        b32 Result = Write(Dest, (u8*)&Src, sizeof((prim.name)));
        return Result;
      }
    }
  }
)

poof(gen_write_primitive_from_u8_stream_little_endian({s8 u8 s16 u16 s32 u32 s64 u64}))
#include <generated/gen_write_primitive_from_u8_stream_803324607.h>


link_internal b32
Read_cs(u8_cursor *Cursor, cs *Dest)
{
  // NOTE(Jesse): This routine is kinda tortured because the string type in
  // bonsai switches between 32 and 64 bit, whereas strings from a binary should
  // always have a 64-bit length
  u32 Len = {};

  b32 Result  = Read_u32(Cursor, &Len);
      Result &= Read_bytes(Cursor, (u8**)&Dest->Start, Len);

  if (Result)
  {
    Dest->Count = Len;
  }

  return Result;
}


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




