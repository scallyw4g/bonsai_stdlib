link_internal void
CopyString(cs *S1, cs *S2)
{
  Assert(S1->Count <= S2->Count);
  MemCopy((u8*)S1->Start, (u8*)S2->Start, S1->Count);
  S2->Count = S1->Count;
}

link_internal void
DeepCopy(cs *S1, cs *S2)
{
  CopyString(S1, S2);
}

inline b32
StringsMatch(const char *S1, const char *S2)
{
  b32 Result = S1 && S2;
  while (Result && (*S1 || *S2))
  {
    Result &= (*S1++ == *S2++);
  }

  return Result;
}

inline s32
CompareStrings(cs* S1, cs* S2)
{
  s32 Result = 0;

  umm End = Min(S1->Count, S2->Count);
  for (u32 CharIndex = 0;
           CharIndex < End;
         ++CharIndex)
  {
    s32 Diff = s32(S2->Start[CharIndex]) - s32(S1->Start[CharIndex]);
    if (Diff) { Result = Diff; break; }
  }

  if (Result == 0 && (S1->Count != S2->Count))
  {
    Result = s32(S2->Count) - s32(S1->Count);
  }

  return Result;
}

inline b32
StringsMatch(cs* S1, cs* S2)
{
  b32 Result = True;

  if (S1 && S2 && S1->Count == S2->Count)
  {
    for (u32 CharIndex = 0;
        CharIndex < S1->Count;
        ++CharIndex)
    {
      Result = (Result && (S1->Start[CharIndex] == S2->Start[CharIndex]));
    }
  }
  else
  {
    Result = False;
  }

  return Result;
}

inline b32
StringsMatch(cs S1, cs* S2)
{
  b32 Result = StringsMatch(&S1, S2);
  return Result;
}

inline b32
StringsMatch(cs* S1, cs S2)
{
  b32 Result = StringsMatch(S1, &S2);
  return Result;
}

inline b32
StringsMatch(cs S1, cs S2)
{
  b32 Result = StringsMatch(&S1, &S2);
  return Result;
}

inline b32
AreEqual(cs *S1, cs *S2)
{
  b32 Result = StringsMatch(S1, S2);
  return Result;
}

inline b32
AreEqual(cs S1, cs S2)
{
  b32 Result = StringsMatch(&S1, &S2);
  return Result;
}

poof(stream_and_cursor(counted_string))
#include <generated/stream_and_cursor_counted_string.h>

struct tagged_counted_string_stream
{
  counted_string Tag;
  counted_string_stream Stream;
};
poof(generate_stream(tagged_counted_string_stream))
#include <generated/generate_stream_tagged_counted_string_stream.h>

// TODO(Jesse, id: 99, tags: compiler_feature, metaprogramming): Generate this?  Need a compiler feature to generate stuff from primitive types.
struct char_cursor
{
  char* Start;
  char* At;
  char* End;
  memory_arena* Memory; // TODO(Jesse, id: 100, tags: open_question, metaprogramming): Do we actually want this in here?
};

// Note(Jesse): Shamelessly copied from the Handmade Hero codebase
global_variable char DecChars[] = "0123456789";
global_variable char LowerHexChars[] = "0123456789abcdef";
global_variable char UpperHexChars[] = "0123456789ABCDEF";

umm
Length(const char *Str)
{
  const char *Start = Str;
  const char *End = Str;

  while (*End++);

  umm Result = (umm)(End - Start) - 1;
  return Result;
}

counted_string
CountedString(const char *S)
{
  counted_string Result = {};
  Result.Start = S;
  Result.Count = Length(S);
  return Result;
}

counted_string
CS(const u8 *S)
{
  counted_string Result = CountedString((char*)S);
  return Result;
}

counted_string
CS(const char *S)
{
  counted_string Result = CountedString(S);
  return Result;
}

counted_string
CS(char_cursor* Cursor)
{
  counted_string Result = {
    .Count = (umm)(Cursor->At - Cursor->Start),
    .Start = Cursor->Start,
  };

  return Result;
}

char_cursor
CharCursor(counted_string S)
{
  char_cursor Result = {
    .Start = (char*)S.Start,
    .At = (char*)S.Start,
    .End = (char*)(S.Start + S.Count),
  };
  return Result;
}

link_internal char
LastChar(counted_string *Str)
{
  char Result = Str->Count ? Str->Start[Str->Count-1] : 0;
  return Result;
}

link_internal char
LastChar(counted_string Str)
{
  char Result = Str.Count ? Str.Start[Str.Count-1] : 0;
  return Result;
}

link_internal counted_string
StripBookends(counted_string S, char First, char Last)
{
  Assert(S.Count >= 2);
  Assert(S.Start[0] == First);
  Assert(LastChar(S) == Last);

  counted_string Result = {
    .Count = S.Count - 2,
    .Start = S.Start + 1,
  };
  return Result;
}

link_internal counted_string
StripQuotes(counted_string S)
{
  counted_string Result = StripBookends(S, '"', '"');
  return Result;
}

b32
Contains(counted_string Haystack, char Needle)
{
  b32 Result = False;
  for (u32 Index = 0;
      Index < Haystack.Count;
      ++Index)
  {
    Result |= (Needle == Haystack.Start[Index]);
  }
  return Result;
}

b32
Contains(const char* Haystack, char Needle)
{
  b32 Result = Contains(CS(Haystack), Needle);
  return Result;
}

// TODO(Jesse, id: 101, tags: string_hash, profile, speed, hash, already_done_elsewhere, duplicate, high_priority):
// Profile and check collision rate of this!
inline u64
StringHash(const char* S1)
{
  u64 Result = 0x4fa691fd6a2c2f69;
  while (*S1++)
  {
    u64 Char = (u64)*S1;
    u64 Mix = Char | (Char<<8) | (Char<<16) | (Char<<24) | (Char<<32) | (Char<<40) | (Char<<48) | (Char<<56);
    Result ^= ((Result<<32) | (Result>>32)) & Mix;
  }

  return Result;
}

link_internal b32
IsPathSeparator(char C)
{
  b32 Result = C == '/';
#if BONSAI_WIN32
  Result |= C == '\\';
#endif
  return Result;
}

link_internal counted_string
Extension(counted_string FilePath)
{
  counted_string Result = FilePath;

  for (umm CharIndex = 0;
      CharIndex < Result.Count;
      ++CharIndex)
  {
    if (Result.Start[CharIndex] == '.')
    {
      Result.Start = FilePath.Start + CharIndex + 1;
      Result.Count = FilePath.Count - CharIndex - 1;
      break;
    }
  }

  return Result;
}

link_internal counted_string
StripExtension(counted_string FilePath)
{
  counted_string Result = FilePath;

  for (umm CharIndex = 0;
      CharIndex < Result.Count;
      ++CharIndex)
  {
    if (Result.Start[CharIndex] == '.')
    {
      Result.Count = CharIndex;
      break;
    }
  }

  return Result;
}

// Given "path/to/file.whatever", returns "path/to"
link_internal counted_string
Dirname(counted_string FilePath)
{
  umm OneAfterLastPathSep = 0;
  for (umm CharIndex = 0;
      CharIndex < FilePath.Count;
      ++CharIndex)
  {
    if (IsPathSeparator(FilePath.Start[CharIndex]))
    {
      OneAfterLastPathSep = CharIndex+1;
    }
  }

  Assert(OneAfterLastPathSep <= FilePath.Count);

  counted_string Result = {
    .Count = FilePath.Count - (FilePath.Count-OneAfterLastPathSep),
    .Start = FilePath.Start,
  };

  if (Result.Count)
  {
    Assert(IsPathSeparator(LastChar(Result)));
  }

  return Result;
}

// Given "path/to/file.whatever", returns "file.whatever"
link_internal counted_string
Basename(counted_string FilePath)
{
  umm LastPathSeparator = 0;
  for (umm CharIndex = 0;
      CharIndex < FilePath.Count;
      ++CharIndex)
  {
    if (IsPathSeparator(FilePath.Start[CharIndex]))
    {
      LastPathSeparator = CharIndex+1;
    }
  }

  Assert(LastPathSeparator <= FilePath.Count);

  counted_string Result = {
    .Count = FilePath.Count - LastPathSeparator,
    .Start = FilePath.Start + LastPathSeparator,
  };

  if (Result.Count)
  {
    Assert(IsPathSeparator(Result.Start[0]) == False);
  }

  return Result;
}

link_internal b32
StartsWith(counted_string S1, counted_string S2)
{
  umm TruncLength = Min(S1.Count, S2.Count);
  S1.Count = TruncLength;

  b32 Result = StringsMatch(S1, S2);
  return Result;
}

link_internal b32
EndsWith(cs S1, cs S2)
{
  b32 Result = S2.Count <= S1.Count;

  if (Result)
  {
    umm Offset = S1.Count - S2.Count;
    Assert(Offset+S2.Count == S1.Count);
    Result = StringsMatch(CS(S1.Start+Offset, S2.Count), S2);
  }

  return Result;
}

inline b32
Contains(counted_string S1, counted_string S2)
{
  b32 Result = False;

  if (S1.Count >= S2.Count)
  {
    u32 Diff = (u32)(S1.Count - S2.Count);
    for (u32 S1Index = 0;
        S1Index <= Diff;
        ++S1Index)
    {
      counted_string Temp1 = CS(S1.Start+S1Index, S2.Count);
      Assert(Temp1.Start+Temp1.Count <= S1.Start+S1.Count);
      Result = StringsMatch(&Temp1, &S2);
      if (Result) { return Result; }
    }
  }

  return Result;
}

inline b32
Contains(const char *S1, const char *S2)
{
  const char *S1At = S1;
  while (*S1At)
  {
    const char *S2At = S2;

    b32 Result = (*S1At == *S2At);
    while (Result && *S1At && *S2At)
    {
      Result &= (*S1At++ == *S2At++);
    }

    if (Result && *S2At == 0)
    {
      return True;
    }
    else
    {
      ++S1At;
    }
  }

  return False;
}

link_internal b32
IsWhitespace(char Type)
{
  b32 Result = Type == '\n' || Type == '\r' || Type == ' ' || Type == '\t';
  return Result;
}

link_internal b32
IsNBSP(char Type)
{
  b32 Result = Type == ' ' || Type == '\t';
  return Result;
}

link_internal b32
IsNBSP(counted_string *S)
{
  b32 Result = True;
  for (u32 CharIndex = 0;
      Result && CharIndex < S->Count;
      ++CharIndex)
  {
    Result &= IsNBSP(S->Start[CharIndex]);
  }
  return Result;
}

link_internal b32
TrimTrailingNewline(counted_string *S)
{
  b32 Result = False;
  if (LastChar(S) == '\n')
  {
    Result = True;
    Assert(S->Count);
    S->Count -= 1;
  }
  return Result;
}

// TODO(Jesse): Not sure if returning a 0 here is appropriate, but it'll do for now
link_internal char
LastNBSPChar(counted_string *S)
{
  s32 Index = (s32)Desaturate(S->Count, 1);
  if (S)
  {
    while (Index >= 0)
    {
      if ( IsNBSP(S->Start[Index]) )
      {
        --Index;
      }
      else
      {
        break;
      }
    }
  }

  char Result = 0;
  if (Index >= 0)
    Result = S->Start[Index];

  return Result;
}

link_internal void
TrimTrailingNBSP(counted_string *S)
{
  if (S)
  {
    while (S->Count)
    {
      if ( IsNBSP(S->Start[S->Count-1]) )
      {
        --(S->Count);
      }
      else
      {
        break;
      }
    }
  }
}

link_internal counted_string
TrimTrailingNBSP(counted_string String)
{
  counted_string Result = String;
  TrimTrailingNBSP(&Result);
  return Result;
}

link_internal void
TrimLeadingWhitespace(cs *Str)
{
  while (Str->Count)
  {
    if ( IsWhitespace(Str->Start[0]) )
    {
      --Str->Count;
      ++Str->Start;
    }
    else
    {
      break;
    }
  }
}

link_internal void
TrimTrailingWhitespace(cs *Str)
{
  while (Str->Count)
  {
    if ( IsWhitespace(Str->Start[Str->Count-1]) )
    {
      --Str->Count;
    }
    else
    {
      break;
    }
  }
}

link_internal counted_string
Trim(counted_string String)
{
  counted_string Result = String;
  TrimLeadingWhitespace(&Result);
  TrimTrailingWhitespace(&Result);
  return Result;
}

counted_string
Split(counted_string* String, char SplitTarget)
{
  counted_string Result = {};

  for (u32 CharIndex = 0;
      CharIndex < String->Count;
      ++CharIndex)
  {
    if (String->Start[CharIndex] == SplitTarget)
    {
      Result.Start = String->Start + CharIndex + 1;
      Result.Count = String->Count - CharIndex - 1;

      String->Count = CharIndex;
    }
  }

  return Result;
}

link_internal b32
IsUpper(char C)
{
  b32 Result = (C >= 'A' && C <= 'Z');
  return Result;
}

link_internal b32
IsLower(char C)
{
  b32 Result = (C >= 'a' && C <= 'z');
  return Result;
}

link_internal char
ToLower(char C)
{
  if (IsUpper(C)) { C += 32; }
  return C;
}

link_internal char
ToUpper(char C)
{
  if (IsLower(C)) { C -= 32; }
  return C;
}

link_internal b32
IsAlpha(char C)
{
  b32 Result = IsUpper(C) || IsLower(C);
  return Result;
}

link_internal b32
IsHex(char C)
{
  b32 Result = ( (C >= '0' && C <= '9') ||
                 (C >= 'a' && C <= 'f') ||
                 (C >= 'A' && C <= 'F') );
  return Result;
}

link_internal b32
IsNumeric(char C)
{
  b32 Result = (C >= '0' && C <= '9');
  return Result;
}

link_internal b32
IsAlphaNumeric(char C)
{
  b32 Result = IsAlpha(C) || IsNumeric(C);
  return Result;
}

link_internal b32
IsAlphaNumeric(counted_string S)
{
  b32 Result = True;
  for (u32 CharIndex = 0;
      Result && CharIndex < S.Count;
      ++CharIndex)
  {
    Result &= IsAlphaNumeric(S.Start[CharIndex]);
  }
  return Result;
}

link_internal u64
ToU64(char C)
{
  Assert(IsNumeric(C));
  u64 Result = (u64)C - (u64)'0';
  return Result;
}

link_internal u32
ToU32(char C)
{
  u32 Result = SafeTruncateToU32(ToU64(C));
  return Result;
}

link_internal u32
GetColumnsFor(u32 N)
{
  u32 i = 0;
  while (N != 0) { N /= 10; ++i; }

  return i;
}

link_internal r64
Exp(r64 Base, s32 Exponent)
{
  s32 IterationDirection = Exponent > 0 ? 1 : -1;

  r64 Result = 1;

  for (s32 Iterator = 0;
      Iterator != Exponent;
      Iterator += IterationDirection)
  {
    IterationDirection == 1 ?
      Result *= Base :
      Result /= Base;
  }

  return Result;
}

link_internal u64
Exp(u64 Base, s32 Exponent)
{
  s32 IterationDirection = Exponent > 0 ? 1 : -1;

  u64 Result = 1;

  for (s32 Iterator = 0;
      Iterator != Exponent;
      Iterator += IterationDirection)
  {
    IterationDirection == 1 ?
      Result *= Base :
      Result /= Base;
  }

  return Result;
}

link_internal u32
IsNumeric(counted_string S)
{
  u32 Result = S.Count > 0;

  for (u32 CharIndex = 0;
           CharIndex < S.Count;
         ++CharIndex)
  {
    Result &= IsNumeric(S.Start[CharIndex]);
  }

  return Result;
}

link_internal umm
ToUMM(u64 N)
{
  Assert(N < umm_MAX);
  umm Result = (umm)N;
  return Result;
}

link_internal u64
ToU64(counted_string S)
{
  u64 Result = 0;
  for (umm CharIndex = 0;
      CharIndex < S.Count;
      ++CharIndex)
  {
    u64 Digit = ToU64(S.Start[CharIndex]);
    Result += (Digit * Exp(10ULL, SafeTruncateToS32((umm)(S.Count - CharIndex - 1L)) ));
  }

  return Result;
}

link_internal s32
ToS32(counted_string S)
{
  s32 Negative = 1;
  if (S.Count && S.Start[0] == '-')
  {
    Negative = -1;
    S.Count--;
    S.Start++;
  }

  s32 Result = Negative*SafeTruncateToS32(ToUMM(ToU64(S)));
  return Result;
}

link_internal u32
ToU32(counted_string S)
{
  u32 Result = SafeTruncateToU32(ToU64(S));
  return Result;
}

link_internal u32
ToU32(counted_string *S)
{
  u32 Result = ToU32(*S);
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

// Shamelessly ripped from https://cp-algorithms.com/string/string-hashing.html

/* TODO(Jesse, id: 81, tags: string_hash): Theres' a null terminated
 * string-hash bonsai_function that should be replaced by this one most likely
 */
inline u64
Hash(counted_string* String)
{
  u64 Result = 0;

#if 1
  /* TODO(Jesse, id: 82, tags: robustness, rng, hashing): 257 seemed to produce
   * slightly worse results, but the source literature seemed to indicate using
   * a constant close to the total number of discrete characters you'll
   * encounter is advisable.  I believe this to be somewhat higher than 53, but
   * it would be worth testing on real-world data (source files).
   */
  u32 p = 53;
  u32 m = (u32)1e9 + 9;

  u64 p_pow = 1;

  for (umm CharIndex = 0;
      CharIndex < String->Count;
      ++CharIndex)
  {
    umm C = (umm)String->Start[CharIndex];
    Result = (Result + (C - (u64)('a' + 1)) * p_pow) % m;
    p_pow = (p_pow*p) % m;
  }
#else
  // Note(Jesse): This is just some random thing I came up with that I thought
  // sounded good ..it's worse than the implementation I got off the internet,
  // but I though I'd leave it here for posterity.

  u64 A = 6364136223846793005;
  u64 B = 1442695040888963407;
  u64 One = 1;
  u64 Mod = (One << 63);

  u32 Remainder = (u32)(String->Count % sizeof(u32));

  for (umm CharIndex = 0;
      CharIndex < String->Count;
      ++CharIndex)
  {
    Result = ((A * (umm)String->Start[CharIndex]) + B) % Mod;
  }

  u32 IterationCount = (u32)(String->Count / sizeof(u32));
  u32* Data = (u32*)String->Start+Remainder;
  for (umm IterIndex = 0;
      IterIndex < IterationCount;
      ++IterIndex)
  {
    Result = ((A * (umm)Data[IterIndex]) + B) % Mod;
  }

  Result = (u32)(Result >> 31);

#endif

  return Result;
}

inline u64
Hash(counted_string String)
{
  return Hash(&String);
}


link_internal void
TruncateAndNullTerminate(cs *S, umm Count)
{
  if (S->Count >= Count)
  {
    S->Count -= Count;
    Cast(char*, S->Start)[S->Count] = 0;
  }
}

link_internal void
Truncate(cs *S, umm Count)
{
  if (S->Count >= Count) { S->Count -= Count; }
}

link_internal void
Frontcate(cs *S, umm Count)
{
  if (S->Count >= Count) { S->Count -= Count; S->Start += Count; }
}
