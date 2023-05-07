

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
    GetNthSetBit((T.name) Target, (T.name) NBit)
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
#include <generated/gen_bit_operators_unsigned int .h>

poof(gen_bit_operators(u64))
#include <generated/gen_bit_operators_unsigned long long int .h>
