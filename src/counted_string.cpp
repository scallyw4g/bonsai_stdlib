
// This is to silence the warnings when passing counted_strings
#define FormatCountedString(Memory, Fmt, ...)             \
  _Pragma("clang diagnostic push")                        \
  _Pragma("clang diagnostic ignored \"-Wclass-varargs\"") \
  FormatCountedString_(Memory, Fmt, __VA_ARGS__)          \
  _Pragma("clang diagnostic pop")

#define FCS(Fmt, ...) FormatCountedString(GetTranArena(), Fmt, __VA_ARGS__)
#define FSz(Fmt, ...) FormatCountedString(GetTranArena(), CSz(Fmt), __VA_ARGS__)

link_internal u8*
HeapAllocate(heap_allocator *Allocator, umm RequestedSize);

link_internal char *
GetTempFmtBuffer()
{
  if (ThreadLocal_ThreadIndex == -1)
  {
    return TempStdoutFormatStringBuffer;
  }
  else
  {
    thread_local_state *Thread = GetThreadLocalState(ThreadLocal_ThreadIndex);
    return Thread->TempStdoutFormatStringBuffer;
  }
}

poof(buffer(counted_string))
#include <generated/buffer_counted_string.h>

typedef counted_string_buffer cs_buffer;

poof(index_of(counted_string))
#include <generated/index_of_counted_string.h>

poof(hashtable(counted_string))
#include <generated/hashtable_counted_string.h>

poof(tuple({cs cs}))
#include <generated/tuple_822959015.h>

poof(generate_stream(tuple_cs_cs))
#include <generated/generate_stream_tuple_cs_cs.h>

link_internal void
ConcatStreams(counted_string_stream* S1, counted_string_stream* S2, memory_arena* Memory)
{
  ITERATE_OVER(S2)
  {
    counted_string* Element = GET_ELEMENT(Iter);
    Push(S1, *Element);
  }
  return;
}

poof(buffer_builder(tuple_cs_cs))
#include <generated/buffer_builder_tuple_cs_cs.h>



counted_string
CountedString(umm Count, memory_arena* Memory)
{
  TIMED_FUNCTION();
  counted_string Result = {
    .Count = Count,
    .Start = AllocateProtection(const char, Memory, Count, False),
  };
  return Result;
}

counted_string
CountedString(const char* Start, memory_arena* Memory)
{
  TIMED_FUNCTION();
  umm StringLength = Length(Start);

  counted_string Result = CountedString(StringLength, Memory);
  MemCopy((u8*)Start, (u8*)Result.Start, StringLength);

  return Result;
}

const char *
CopyZString(const char* Start, memory_arena* Memory)
{
  TIMED_FUNCTION();

  umm Count = Length(Start);

  char *Result =  Allocate(char, Memory, Count+1);
  MemCopy((u8*)Start, (u8*)Result, Count);

  return Cast(const char*, Result);
}

counted_string
CopyString(const char* Start, memory_arena* Memory)
{
  TIMED_FUNCTION();

  umm Count = Length(Start);

  counted_string Result = {
    .Count = Count,
    .Start = AllocateProtection(const char, Memory, Count, False),
  };

  MemCopy((u8*)Start, (u8*)Result.Start, Count);

  return Result;
}

counted_string
CopyString(const char* Start, umm Count, memory_arena* Memory)
{
  TIMED_FUNCTION();
  counted_string Result = {
    .Count = Count,
    .Start = AllocateProtection(const char, Memory, Count, False),
  };

  MemCopy((u8*)Start, (u8*)Result.Start, Count);

  return Result;
}

counted_string
CopyString(counted_string S, memory_arena* Memory)
{
  TIMED_FUNCTION();
  counted_string Result = {
    .Count = S.Count,
    .Start = AllocateProtection(const char, Memory, S.Count, False),
  };

  MemCopy((u8*)S.Start, (u8*)Result.Start, S.Count);

  return Result;
}

counted_string
CopyString(counted_string S, heap_allocator* Memory)
{
  TIMED_FUNCTION();
  counted_string Result = {
    .Count = S.Count,
    .Start = (const char*)HeapAllocate(Memory, S.Count),
  };

  MemCopy((u8*)S.Start, (u8*)Result.Start, S.Count);

  return Result;
}

poof(
  func string_concat(type_poof_index arg_count, type_poof_symbol allocator_type, type_poof_symbol allocator_func)
  {
    link_internal counted_string
    Concat( (arg_count.map(N) {cs S(N), }) (allocator_type)* Memory, umm ExtraChars = 0)
    {
      umm TotalLength =  arg_count.map(n) { S(n).Count + } ExtraChars ;
      counted_string Result = {
        .Count = TotalLength,
        .Start = allocator_func(char, Memory, TotalLength, False),
      };

      u64 At = 0;
      arg_count.map(n)
      {
        MemCopy((u8*)S(n).Start, (u8*)Result.Start+At, S(n).Count);
        At += S(n).Count;
      }

      return Result;
    }
  }
)

poof(string_concat(2, {memory_arena}, {AllocateProtection}))
#include <generated/string_concat_2.h>
poof(string_concat(3, {memory_arena}, {AllocateProtection}))
#include <generated/string_concat_3.h>
poof(string_concat(4, {memory_arena}, {AllocateProtection}))
#include <generated/string_concat_4.h>
poof(string_concat(5, {memory_arena}, {AllocateProtection}))
#include <generated/string_concat_5.h>
poof(string_concat(6, {memory_arena}, {AllocateProtection}))
#include <generated/string_concat_6.h>
poof(string_concat(7, {memory_arena}, {AllocateProtection}))
#include <generated/string_concat_7.h>
poof(string_concat(8, {memory_arena}, {AllocateProtection}))
#include <generated/string_concat_8.h>
poof(string_concat(9, {memory_arena}, {AllocateProtection}))
#include <generated/string_concat_9.h>
poof(string_concat(10, {memory_arena}, {AllocateProtection}))
#include <generated/string_concat_10.h>


poof(string_concat(2, {heap_allocator} {HeapAlloc}))
#include <generated/string_concat_e9LuOotf.h>
poof(string_concat(3, {heap_allocator} {HeapAlloc}))
#include <generated/string_concat_VITTv1fC.h>
poof(string_concat(4, {heap_allocator} {HeapAlloc}))
#include <generated/string_concat_BtCc0m88.h>
poof(string_concat(5, {heap_allocator} {HeapAlloc}))
#include <generated/string_concat_JS19evGm.h>
poof(string_concat(6, {heap_allocator} {HeapAlloc}))
#include <generated/string_concat_plWVg6MZ.h>
poof(string_concat(7, {heap_allocator} {HeapAlloc}))
#include <generated/string_concat_6fJo9rZ9.h>
poof(string_concat(8, {heap_allocator} {HeapAlloc}))
#include <generated/string_concat_vmDblFNI.h>
poof(string_concat(9, {heap_allocator} {HeapAlloc}))
#include <generated/string_concat_j2qXwRyR.h>
poof(string_concat(10, {heap_allocator} {HeapAlloc}))
#include <generated/string_concat_oql3ieBj.h>


link_internal const char*
ConcatZ(cs S1, cs S2, memory_arena* Memory)
{
  cs S = Concat(S1, S2, Memory, 1);
  Assert(LastChar(S) == 0);
  return S.Start;
}

link_internal const char*
ConcatZ(cs S1, cs S2, cs S3, memory_arena* Memory)
{
  cs S = Concat(S1, S2, S3, Memory, 1);
  Assert(LastChar(S) == 0);
  return S.Start;
}

link_internal const char*
GetNullTerminated(counted_string Str, memory_arena* Memory = 0)
{
  if (Memory == 0) { Memory = GetTranArena(); }
  /* if (Memory == 0) { Memory = GetTranArena(); } */

  const char* Result = AllocateProtection(const char, Memory, Str.Count+1, False);
  MemCopy((u8*)Str.Start, (u8*)Result, Str.Count);
  return Result;
}

link_internal void
ToCapitalCaseInplace(counted_string* Source)
{
  b32 NextCharToUpper = True;
  for (u32 CharIndex = 0;
      CharIndex < Source->Count;
      )
  {
    char* At = (char*)Source->Start + CharIndex;

    if (*At == '_')
    {
      NextCharToUpper = True;
      ++CharIndex;
      continue;
    }

    if (NextCharToUpper)
    {
      *At = ToUpper(*At);
      NextCharToUpper = False;
    }

    ++CharIndex;
  }
}

link_internal counted_string
ToUpperCase(counted_string Source, memory_arena* Memory)
{
  counted_string Result = CountedString(Source.Count, Memory);

  for (u32 CharIndex = 0;
           CharIndex < Result.Count;
         ++CharIndex)
  {
    char At = ToUpper(Source.Start[CharIndex]);
    ((char*)Result.Start)[CharIndex] = At;
  }

  return Result;
}

link_internal counted_string
ToLowerCase(counted_string Source, memory_arena* Memory)
{
  counted_string Result = CountedString(Source.Count, Memory);

  for (u32 CharIndex = 0;
      CharIndex < Result.Count;
      ++CharIndex)
  {
    char At = ToLower(Source.Start[CharIndex]);
    ((char*)Result.Start)[CharIndex] = At;
  }

  return Result;
}

link_internal counted_string
StripPrefix(counted_string Source, u32 Count, cs *Prefix = 0)
{
  u32 Hits = 0;
  u32 CharAfterUnderscore = 0;
  for (u32 CharIndex = 0;
           CharIndex < Source.Count;
         ++CharIndex)
  {
    if (Source.Start[CharIndex] == '_')
    {
      CharAfterUnderscore = CharIndex+1;
      if (Hits++ == Count)
      {
        break;
      }
    }
  }

  cs Result = Source;
  if (Source.Count > CharAfterUnderscore)
  {
    umm ResultLength = Source.Count - CharAfterUnderscore;
    Result = CS(Source.Start+CharAfterUnderscore, ResultLength);

    if (Prefix)
    {
      Assert(CharAfterUnderscore <= Source.Count);
      umm PrefixLength = CharAfterUnderscore;
      *Prefix = CS(Source.Start, CharAfterUnderscore);
    }
  }

  return Result;
}

link_internal counted_string
StripPrefixesUntilDoubleUnderscore(counted_string Source)
{
  u32 Hits = 0;
  cs Current = Source;

  cs Prefix = {};
  do
  {
    Prefix = {};
    Current = StripPrefix(Current, 0, &Prefix);
  } while (Prefix.Count && !StringsMatch(Prefix, CSz("_")));

  cs Result = Current;
  return Result;
}

link_internal counted_string
ToSnakeCase(counted_string Source, memory_arena* Memory)
{
  u32 ResultLength = Cast(u32, Source.Count);
  for (u32 CharIndex = 0;
      CharIndex < Source.Count;
      ++CharIndex)
  {
    if (IsUpper(Source.Start[CharIndex]))
    {
      if (CharIndex > 0)
      {
        ++ResultLength;
      }
    }
  }

  counted_string Result = CountedString(ResultLength, Memory);

  b32 NextCharToUpper = True;
  u32 SourceIndex = 0;
  u32 ResultIndex = 0;
  for (;
      ResultIndex < Result.Count;
      ++ResultIndex)
  {
    char AtChar = Source.Start[SourceIndex];
    if (IsUpper(AtChar))
    {
      if (ResultIndex > 0)
      {
        Cast(char*, &Result.Start[ResultIndex])[0] = '_';
        ++ResultIndex;
      }
      AtChar = ToLower(AtChar);
    }

    Cast(char*, &Result.Start[ResultIndex])[0] = AtChar;
    ++SourceIndex;
  }

  Assert(ResultIndex == ResultLength);

  return Result;
}

link_internal counted_string
ToCapitalCase(counted_string Source, memory_arena* Memory)
{
  u32 ResultLength = 0;
  for (u32 CharIndex = 0;
      CharIndex < Source.Count;
      ++CharIndex)
  {
    if (Source.Start[CharIndex] != '_')
    {
      ++ResultLength;
    }
  }

  counted_string Result = CountedString(ResultLength, Memory);

  b32 NextCharToUpper = True;
  u32 SourceIndex = 0;
  for (u32 ResultIndex = 0;
      ResultIndex < Result.Count;
      )
  {
    char At = Source.Start[SourceIndex];

    if (At == '_')
    {
      NextCharToUpper = True;
      ++SourceIndex;
      continue;
    }

    if (NextCharToUpper)
    {
      At = ToUpper(At);
      NextCharToUpper = False;
    }

    // TODO(Jesse, id: 52, tags: gross): Gross..
    *(char*)(Result.Start + ResultIndex) = At;
    ++SourceIndex;
    ++ResultIndex;
  }

  return Result;
}

link_internal void
PadForFormatWidth(char_cursor *Dest, u32 PadCount)
{
  {
    while (PadCount) { CopyToDest(Dest, ' '); --PadCount; }
  }
}

link_internal counted_string
FormatCountedString_(char_cursor* DestCursor, counted_string FS, va_list Args)
{
  TIMED_FUNCTION();

  char_cursor FormatCursor_ = {
    .Start = (char*)FS.Start,
    .At    = (char*)FS.Start,
    .End   = (char*)FS.Start + FS.Count,
    {},
  };

  char_cursor* FormatCursor = &FormatCursor_;

  while (Remaining(FormatCursor))
  {
    char CursorAt = Advance(FormatCursor);

    if (CursorAt == '%')
    {
      u32 FormatWidth = 0;
      char FormatWidthPeek = Peek(FormatCursor);
      if (FormatWidthPeek == '*')
      {
        CursorAt = Advance(FormatCursor); // *
        CursorAt = Advance(FormatCursor); // Next
        FormatWidth = va_arg(Args, u32);
      }
      else if (IsNumeric(FormatWidthPeek))
      {
        FormatWidth = EmbeddedU32(FormatCursor);
        CursorAt = Advance(FormatCursor);
      }
      else
      {
        CursorAt = Advance(FormatCursor);
      }


      u32 FormatPrecision = 0;
      if (CursorAt == '.')
      {
        char FormatPrecisionPeek = Peek(FormatCursor);

        if (FormatPrecisionPeek == '*')
        {
          CursorAt = Advance(FormatCursor); // *
          CursorAt = Advance(FormatCursor); // Next
          FormatPrecision = va_arg(Args, u32);
        }
        else if (IsNumeric(FormatPrecisionPeek))
        {
          FormatPrecision = EmbeddedU32(FormatCursor);
          CursorAt = Advance(FormatCursor);
        }
        else
        {
          Error("Invalid dot specifier in format string");
        }
      }

      switch (CursorAt)
      {
        case 'd':
        {
          s32 Value = va_arg(Args, s32);
          s64ToChar(DestCursor, (s64)Value, FormatWidth);
        } break;

        case 'l':
        {
          CursorAt = Advance(FormatCursor);
          if (CursorAt == 'l') { CursorAt = Advance(FormatCursor); }

          if (CursorAt == 'u')
          {
            u64 Value = va_arg(Args, u64);
            u64ToChar(DestCursor, Value, FormatWidth);
          }
          else if (CursorAt == 'd')
          {
            s64 Value = va_arg(Args, s64);
            s64ToChar(DestCursor, Value, FormatWidth);
          }
          else if (CursorAt == 'x')
          {
            u64 Value = va_arg(Args, u64);
            u64ToChar(DestCursor, Value, FormatWidth, 16, UpperHexChars);
          }
        } break;

        case 'p':
        {
          CopyToDest(DestCursor, '0');
          CopyToDest(DestCursor, 'x');
        } [[fallthrough]];

        case 'x':
        {
          u32 Value = va_arg(Args, u32);
          u64ToChar(DestCursor, Cast(u64, Value), FormatWidth, 16, UpperHexChars);
        } break;

        case 'u':
        {
          u32 Value = va_arg(Args, u32);
          u64ToChar(DestCursor, (u64)Value, FormatWidth);
        } break;

        case 'c':
        {
          char Value = (char)va_arg(Args, s32);

          if (FormatWidth)
          {
            u32 PadCount = FormatWidth - 1;
            PadForFormatWidth(DestCursor, PadCount);
          }

          CopyToDest(DestCursor, Value);
        } break;

        case 's':
        {
          char* Value = va_arg(Args, char*);
          if (Value)
          {
            u32 ValueLen = FormatPrecision ? FormatPrecision : (u32)Length(Value);

            if (FormatWidth)
            {
              u32 PadCount = FormatWidth - ValueLen;
              PadForFormatWidth(DestCursor, PadCount);
            }

            u32 Count = 0;
            while (*Value)
            {
              CopyToDest(DestCursor, *Value++);
              if (FormatPrecision && ++Count == FormatPrecision)
              {
                break;
              }
            }
          }
          else
          {
            CopyToDest(DestCursor, 'n');
            CopyToDest(DestCursor, 'u');
            CopyToDest(DestCursor, 'l');
            CopyToDest(DestCursor, 'l');
          }
        } break;

        case 'f':
        {
          r64 Value = va_arg(Args, r64);
          f64ToChar(DestCursor, Value, FormatPrecision ? FormatPrecision : DEFAULT_FORMAT_PRECISION, FormatWidth );
        } break;

        case 'b':
        {
          b32 BoolVal = (b32)va_arg(Args, u32);
          BoolVal ?
            CopyToDest(DestCursor, 'T') :
            CopyToDest(DestCursor, 'F');
        } break;

        case 'V':
        {
          char VectorWidth = Advance(FormatCursor);
          s32 Width = s32(ToU32(VectorWidth));

          // NOTE(Jesse): Just cast to the maximally wide vector so we can iterate over any size
          v4 *Vector = Cast(v4*, va_arg(Args, void*));
          RangeIterator(Index, Width)
          {
            r32 Value = Vector->E[Index];
            f64ToChar(DestCursor, r64(Value), FormatPrecision ? FormatPrecision : DEFAULT_FORMAT_PRECISION, FormatWidth );
            if (Index < Width-1)
            {
              CopyToDest(DestCursor, CSz(", "));
            }
          }

        } break;

        case 'S':
        {
          counted_string String = va_arg(Args, counted_string);

          if (FormatWidth > String.Count)
          {
            PadForFormatWidth(DestCursor, FormatWidth - (u32)String.Count);
          }

          CopyToDest(DestCursor, String);

        } break;

        case '%':
        {
          CopyToDest(DestCursor, '%');
        } break;

        default:
        {
          va_arg(Args, void*);
          Error("Invalid Format character (%c) in format string (%S)", CursorAt, FS);
        } break;

      }
    }
    else
    {
      CopyToDest(DestCursor, CursorAt);
    }
  }

  umm DestElementsWritten = AtElements(DestCursor);
  counted_string Result = CS((const char*)DestCursor->Start, DestElementsWritten);
  return Result;
}

link_internal counted_string
FormatCountedString_(memory_arena* Memory, counted_string FS, ...)
{
  TIMED_FUNCTION();

  umm FinalBufferStartingSize = FS.Count + 256;
  char* FinalBuffer = AllocateProtection(char, Memory, FinalBufferStartingSize, False);
  Assert((u8*)(FinalBuffer+FinalBufferStartingSize) == Memory->At);

  char_cursor DestCursor = {
    .Start = FinalBuffer,
    .At    = FinalBuffer,
    .End   = FinalBuffer + FinalBufferStartingSize,
    .Memory = Memory
  };

  va_list Args;
  va_start(Args, FS);
  counted_string Result = FormatCountedString_(&DestCursor, FS, Args);
  va_end(Args);

  return Result;
}

// TODO(Jesse, id: 365, tags: speed): This should probably go away and make sure we always just use counted strings
link_internal counted_string
FormatCountedString_(char* Buffer, umm BufferSize, const char *FS, ...)
{
  TIMED_FUNCTION();

  char_cursor DestCursor = {
    .Start = Buffer,
    .At    = Buffer,
    .End   = Buffer + BufferSize,
    {},
  };

  va_list Args;
  va_start(Args, FS);
  counted_string Result = FormatCountedString_(&DestCursor, CS(FS), Args);
  va_end(Args);

  return Result;
}

link_internal counted_string
FormatCountedString_(char* Buffer, umm BufferSize, counted_string FS, ...)
{
  TIMED_FUNCTION();

  char_cursor DestCursor = {
    .Start = Buffer,
    .At    = Buffer,
    .End   = Buffer + BufferSize,
    {},
  };

  va_list Args;
  va_start(Args, FS);
  counted_string Result = FormatCountedString_(&DestCursor, FS, Args);
  va_end(Args);

  return Result;
}

link_internal counted_string
MemorySize(r64 Number)
{
  r64 KB = (r64)Kilobytes(1);
  r64 MB = (r64)Megabytes(1);
  r64 GB = (r64)Gigabytes(1);

  r64 Display = Number;
  char Units = ' ';

  if (Number >= KB && Number < MB)
  {
    Display = Number / KB;
    Units = 'K';
  }
  else if (Number >= MB && Number < GB)
  {
    Display = Number / MB;
    Units = 'M';
  }
  else if (Number >= GB)
  {
    Display = Number / GB;
    Units = 'G';
  }


  counted_string Result = FormatCountedString(GetTranArena(), CSz("%.1f%c"), Display, Units);
  return Result;
}

link_internal counted_string
MemorySize(umm Number)
{
  counted_string Result = MemorySize((r64)Number);
  return Result;
}

link_internal counted_string
CS(s64 Number)
{
  counted_string Result = FormatCountedString(GetTranArena(), CSz("%ld"), Number);
  return Result;
}

link_internal counted_string
CS(u64 Number)
{
  counted_string Result = FormatCountedString(GetTranArena(), CSz("%lu"), Number);
  return Result;
}

link_internal counted_string
CS(s32 Number)
{
  counted_string Result = FormatCountedString(GetTranArena(), CSz("%d"), Number);
  return Result;
}

link_internal counted_string
CS(u32 Number)
{
  counted_string Result = FormatCountedString(GetTranArena(), CSz("%u"), Number);
  return Result;
}

link_internal counted_string
CS(r64 Number)
{
  counted_string Result = FormatCountedString(GetTranArena(), CSz("%.2f"), Number);
  return Result;
}

link_internal counted_string
CS(r32 Number)
{
  counted_string Result = FormatCountedString(GetTranArena(), CSz("%.2f"), (r64)Number);
  return Result;
}

link_internal counted_string
CS(v2i V)
{
  counted_string Result = FormatCountedString(GetTranArena(), CSz("(%d,%d)"), V.x, V.y);
  return Result;
}

link_internal counted_string
CS(v2 V)
{
  counted_string Result = FormatCountedString(GetTranArena(), CSz("(%.2f,%.2f)"), (r64)V.x, (r64)V.y);
  return Result;
}

poof(
  func to_hex_64(type_poof_symbol type_list)
  {
    type_list.map(type)
    {
      link_internal cs
      ToHex( type.name Value )
      {
        cs Result = FSz("%lx", Value);
        return Result;
      }
    }
  }
)

poof(
  func to_hex_32(type_list)
  {
    type_list.map(type)
    {
      link_internal cs
      ToHex( type.name Value )
      {
        cs Result = FSz("%x", Value);
        return Result;
      }
    }
  }
)

poof(to_hex_64({s64 u64 r64}))
#include <generated/to_hex_64_272205387.h>

link_internal counted_string
FormatThousands(u64 Number)
{
  u64 OneThousand = 1000;
  r32 Display = (r32)Number;
  char Units = ' ';

  if (Number >= OneThousand)
  {
    Display = (r32)Number / (r32)OneThousand;
    Units = 'K';
  }

  counted_string Result = FormatCountedString(GetTranArena(), CSz("%.1f%c"), (r64)Display, Units);
  return Result;
}

link_internal r32
ToF64(counted_string* String)
{
  const char* Temp = GetNullTerminated(*String, GetTranArena());
  r32 Result = (r32)atof(Temp);
  return Result;
}

link_internal cs_buffer
Split(cs String, char SplitTarget, memory_arena *Memory)
{
  u32 Splits = 0;
  RangeIterator_t(umm, Index, String.Count)
  {
    if (String.Start[Index] == SplitTarget)
    {
      ++Splits;
    }
  }

  cs_buffer Result = CountedStringBuffer(1+Splits, Memory);

  cs Current = String;
  RangeIterator_t(umm, Index, Splits)
  {
    cs SecondHalf = Split(&Current, SplitTarget);

    Result.Start[Index] = Current;
    Current = SecondHalf;
  }

  Result.Start[LastIndex(&Result)] = Current;

  return Result;
}


poof(block_array_c(counted_string, {32}))
#include <generated/block_array_c_counted_string_688853862_counted_string_block_array_counted_string_block_counted_string_block_array_index.h>
