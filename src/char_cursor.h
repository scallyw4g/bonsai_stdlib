

// TODO(Jesse, id: 99, tags: compiler_feature, metaprogramming): Generate this?  Need a compiler feature to generate stuff from primitive types.
struct char_cursor
{
  char* Start;
  char* At;
  char* End;
  memory_arena* Memory; // TODO(Jesse, id: 100, tags: open_question, metaprogramming): Do we actually want this in here?
};

cs
CS(char_cursor* Cursor)
{
  cs Result = {
    .Count = (umm)(Cursor->At - Cursor->Start),
    .Start = Cursor->Start,
  };

  return Result;
}

char_cursor
CharCursor(cs S)
{
  char_cursor Result = {
    .Start = (char*)S.Start,
    .At = (char*)S.Start,
    .End = (char*)(S.Start + S.Count),
    {},
  };
  return Result;
}

link_internal char
Peek(char_cursor* BufferCursor)
{
  char Result = *BufferCursor->At;
  return Result;
}

link_internal char
Advance(char_cursor* BufferCursor)
{
  char Result = 0;
  if (Remaining(BufferCursor))
  {
    Result = *BufferCursor->At++;
  }
  else
  {
    Error("Attempted to advance an empty char_cursor!");
  }
  return Result;
}

link_internal u32
EmbeddedU32(char_cursor* FormatCursor)
{
  char* NumStart = FormatCursor->At;
  Assert(IsNumeric(*NumStart));
  while(IsNumeric(Peek(FormatCursor))) { Advance(FormatCursor); }

  u32 CharCount = (u32)(FormatCursor->At - NumStart);
  counted_string NumberString = CS((const char*)NumStart, CharCount);
  u32 Result = ToU32(NumberString);
  return Result;
}

link_internal b32
Reallocate(char_cursor *Dest, umm Increment)
{
  b32 Result = False;
  umm CurrentSize = TotalSize(Dest);
  if (Reallocate((u8*)Dest->Start, Dest->Memory, CurrentSize, CurrentSize+Increment))
  {
    Dest->End += Increment;
    Result = True;
  }

  return Result;
}

link_internal void
CopyToDest(char_cursor *Dest, char C)
{
  b32 DoCopy = True;
  if (!Remaining(Dest))
  {
    if (Dest->Memory)
    {
      umm Increment = 32;
      if (Reallocate(Dest, Increment))
      {
        // Everyone's happy
      }
      else
      {
        // Reallocate already prints out an error for this case, no need to bother the user twice.
        DoCopy = False;
      }
    }
    else
    {
      Warn("Unable to reallocate char_cursor; no memory pointer!");
      DoCopy = False;
    }
  }

  if (DoCopy)
  {
    *Dest->At++ = C;
  }
}

link_internal void
CopyToDest(char_cursor *Dest, counted_string String)
{
  for (u32 CharIndex = 0;
      CharIndex < String.Count;
      ++CharIndex)
  {
    CopyToDest(Dest, String.Start[CharIndex]);
  }
}

link_internal void
u64ToChar(char_cursor* Dest, u64 Value, u32 Columns = 0, u32 Base = 10, char *Digits = DecChars)
{
  Assert(Base != 0);

  char *Start = Dest->At;
  do
  {
    u64 DigitIndex = (Value % Base);
    char Digit = Digits[DigitIndex];
    CopyToDest(Dest, Digit);
    Value /= Base;
  }
  while(Value != 0);

  char *End = Dest->At;

  s32 Length = (s32)(End - Start);
  s32 PadCount = (s32)Columns-(s32)Length;

  while (PadCount > 0)
  {
    --PadCount;
    CopyToDest(Dest, ' ');
  }

  End = Dest->At;

  while(Start < End)
  {
      --End;
      char Temp = *End;
      *End = *Start;
      *Start = Temp;
      ++Start;
  }

  return;
}

link_internal void
s64ToChar(char_cursor* Dest, s64 Value, u32 Columns, u32 Base = 10, char *Digits = DecChars)
{
  if (Value < 0)
  {
    CopyToDest(Dest, '-');
    Value = -Value;
  }

  u64ToChar(Dest, (u64)Value, Columns, Base, Digits);
  return;
}

// Note(Jesse): Shamelessly copied, then modified, from the Handmade Hero codebase
#define DEFAULT_FORMAT_PRECISION (16)
link_internal void
f64ToChar(char_cursor* Dest, r64 Value, u32 Precision = DEFAULT_FORMAT_PRECISION, u32 Columns = 0)
{
  if(Value < 0)
  {
    CopyToDest(Dest, '-');
    Value = -Value;
  }

  u64 IntegerPart = (u64)Value;
  Value -= (r64)IntegerPart;

  s32 ColumnPad = (s32)Columns - ((s32)Precision + 1); // NOTE(Jesse): +1 for the '.'

  ColumnPad = Max(ColumnPad, 0);
  u64ToChar(Dest, IntegerPart, (u32)ColumnPad);
  CopyToDest(Dest, '.');

  // Note(casey): Note that this is NOT an accurate way to do this!
  for(u32 PrecisionIndex = 0;
          PrecisionIndex < Precision;
        ++PrecisionIndex)
  {
      Value *= 10.;
      u32 Int = (u32)Value;
      Value -= Int;
      CopyToDest(Dest, DecChars[Int]);
  }

  return;
}

