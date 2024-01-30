

poof(
  func gen_bit_operators(T)
  {
    link_internal (T.name)
    UnsetLeastSignificantSetBit((T.name) *Input)
    {
      (T.name) StartValue = *Input;
      (T.name) Cleared = StartValue & (StartValue - 1);
      *Input = Cleared;

      (T.name) Result = (StartValue & ~(Cleared));
      return Result;
    }

    link_internal u32
    CountBitsSet_Kernighan((T.name) Input)
    {
      u32 Result = 0;
      while (Input != 0)
      {
          Input = Input & (Input - 1);
          Result++;
      }
      return Result;
    }

    link_internal (T.name)
    GetNthSetBit((T.name) Target, u32 NBit)
    {
      (T.name) Result = u32_MAX;
      for ((T.name) BitIndex = 0; BitIndex < NBit; ++BitIndex)
      {
        Result = UnsetLeastSignificantSetBit(&Target);
      }
      return Result;
    }

    link_internal u32
    GetIndexOfNthSetBit((T.name) Target, (T.name) NBit)
    {
      Assert(NBit > 0);

      u32 Result = u32_MAX;
      u64 TypeWidth = sizeof((T.name))*8;
      u32 Hits = 0;
      for (u32 BitIndex = 0; BitIndex < TypeWidth; ++BitIndex)
      {
        if (Target & (1<<BitIndex))
        {
          ++Hits;
        }
        if (Hits == NBit)
        {
          Result = BitIndex;
          break;
        }
      }
      return Result;
    }


  }
)

poof(gen_bit_operators(u32))
#include <generated/gen_bit_operators_u32.h>

poof(gen_bit_operators(u64))
#include <generated/gen_bit_operators_u64.h>

// TODO(Jesse): This is probably _hella_ braindead, but I couldn't think of a
// clever way of doing this, so I used a vim macro ;)
link_inline u32
GetIndexOfSingleSetBit( u64 O )
{
  u32 Result = {};

  switch (O)
  {
    case (1llu <<  0): { Result =  0; break; }
    case (1llu <<  1): { Result =  1; break; }
    case (1llu <<  2): { Result =  2; break; }
    case (1llu <<  3): { Result =  3; break; }
    case (1llu <<  4): { Result =  4; break; }
    case (1llu <<  5): { Result =  5; break; }
    case (1llu <<  6): { Result =  6; break; }
    case (1llu <<  7): { Result =  7; break; }
    case (1llu <<  8): { Result =  8; break; }
    case (1llu <<  9): { Result =  9; break; }
    case (1llu << 10): { Result = 10; break; }
    case (1llu << 11): { Result = 11; break; }
    case (1llu << 12): { Result = 12; break; }
    case (1llu << 13): { Result = 13; break; }
    case (1llu << 14): { Result = 14; break; }
    case (1llu << 15): { Result = 15; break; }
    case (1llu << 16): { Result = 16; break; }
    case (1llu << 17): { Result = 17; break; }
    case (1llu << 18): { Result = 18; break; }
    case (1llu << 19): { Result = 19; break; }
    case (1llu << 20): { Result = 20; break; }
    case (1llu << 21): { Result = 21; break; }
    case (1llu << 22): { Result = 22; break; }
    case (1llu << 23): { Result = 23; break; }
    case (1llu << 24): { Result = 24; break; }
    case (1llu << 25): { Result = 25; break; }
    case (1llu << 26): { Result = 26; break; }
    case (1llu << 27): { Result = 27; break; }
    case (1llu << 28): { Result = 28; break; }
    case (1llu << 29): { Result = 29; break; }
    case (1llu << 30): { Result = 30; break; }
    case (1llu << 31): { Result = 31; break; }
    case (1llu << 32): { Result = 32; break; }
    case (1llu << 33): { Result = 33; break; }
    case (1llu << 34): { Result = 34; break; }
    case (1llu << 35): { Result = 35; break; }
    case (1llu << 36): { Result = 36; break; }
    case (1llu << 37): { Result = 37; break; }
    case (1llu << 38): { Result = 38; break; }
    case (1llu << 39): { Result = 39; break; }
    case (1llu << 40): { Result = 40; break; }
    case (1llu << 41): { Result = 41; break; }
    case (1llu << 42): { Result = 42; break; }
    case (1llu << 43): { Result = 43; break; }
    case (1llu << 44): { Result = 44; break; }
    case (1llu << 45): { Result = 45; break; }
    case (1llu << 46): { Result = 46; break; }
    case (1llu << 47): { Result = 47; break; }
    case (1llu << 48): { Result = 48; break; }
    case (1llu << 49): { Result = 49; break; }
    case (1llu << 50): { Result = 50; break; }
    case (1llu << 51): { Result = 51; break; }
    case (1llu << 52): { Result = 52; break; }
    case (1llu << 53): { Result = 53; break; }
    case (1llu << 54): { Result = 54; break; }
    case (1llu << 55): { Result = 55; break; }
    case (1llu << 56): { Result = 56; break; }
    case (1llu << 57): { Result = 57; break; }
    case (1llu << 58): { Result = 58; break; }
    case (1llu << 59): { Result = 59; break; }
    case (1llu << 60): { Result = 60; break; }
    case (1llu << 61): { Result = 61; break; }
    case (1llu << 62): { Result = 62; break; }
    case (1llu << 63): { Result = 63; break; }
    default: { Error("GetIndexOfSetBit was passed an option value with more than one bit set! (%lu)", O); }
  }

  return Result;
}

// https://graphics.stanford.edu/%7Eseander/bithacks.html#RoundUpPowerOf2
//
link_inline u64
NextPowerOfTwo(u64 Input)
{
  u64 Result = Input;

  Result--;
  Result |= Result >> 1;
  Result |= Result >> 2;
  Result |= Result >> 4;
  Result |= Result >> 8;
  Result |= Result >> 16;
  Result |= Result >> 32;
  Result++;

  Assert(CountBitsSet_Kernighan(Result) == 1);
  return Result;
}

link_inline u32
NextPowerOfTwo(u32 Input)
{
  u32 Result = Input;

  Result--;
  Result |= Result >> 1;
  Result |= Result >> 2;
  Result |= Result >> 4;
  Result |= Result >> 8;
  Result |= Result >> 16;
  Result++;

  Assert(CountBitsSet_Kernighan(Result) == 1);
  return Result;
}
