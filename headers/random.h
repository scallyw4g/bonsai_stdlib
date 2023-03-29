
struct random_series
{
  u64 Seed;
};

#define DEFAULT_ENTROPY_SEED 3426544
link_internal random_series
RandomSeries(u64 Seed = DEFAULT_ENTROPY_SEED)
{
  random_series Result = {
    .Seed = Seed
  };
  return Result;
}

inline u32
RandomU32(random_series *Entropy)
{
  // TODO(Jesse, id: 95, tags: rng, improvement): This is LCG RNG - do we want a better one?

  // Values from Knuth
  u64 A = 6364136223846793005;
  u64 B = 1442695040888963407;
  u64 One = 1;
  u64 Mod = (One << 63);

  Entropy->Seed = ((A * Entropy->Seed) + B) % Mod;

  // The bottom-most bits do not contain a high amount of entropy, so shift the
  // most significant bits down and return them
  u32 Result = (u32)(Entropy->Seed >> 31);
  return Result;
}

inline u32
RandomPositiveS32(random_series *Entropy)
{
  u32 Result = Abs((s32)RandomU32(Entropy));
  Assert(Result >= 0);
  return Result;
}

inline s32
RandomS32(random_series *Entropy)
{
  s32 Result = (s32)RandomU32(Entropy);
  return Result;
}

inline r32
RandomUnilateral(random_series *Entropy)
{
  u32 Random = RandomU32(Entropy);
  r32 Result = (r32)Random/(r32)u32_MAX;
  return Result;
}

inline r32
RandomBilateral(random_series *Entropy)
{
  r32 Result = (RandomUnilateral(Entropy) * 2.0f) - 1.0f;
  return Result;
}

poof(
  func gen_map_value_to_range(t)
  {
    link_internal t.name
    MapValueToRange( t.name LowestPossibleValue, r32 Value, t.name HighestPossibleValue)
    {
      r32 Range = r32(HighestPossibleValue - LowestPossibleValue);
      t.name Result = t.name(Value*Range) + LowestPossibleValue;
      Assert(Result >= LowestPossibleValue);
      Assert(Result <= HighestPossibleValue);
      return Result;
    }
  }
)


// TODO(Jesse)(immediate, easy, bug, metaprogramming): Don't emit spaces in these filenames LOL
poof(gen_map_value_to_range(r32))
#include <generated/gen_map_value_to_range_float .h>

poof(gen_map_value_to_range(u32))
#include <generated/gen_map_value_to_range_unsigned int .h>

poof(
  func gen_random_between(t)
  {
    link_internal t.name
    RandomBetween( t.name LowestPossibleValue, random_series *Entropy, t.name HighestPossibleValue)
    {
      Assert(LowestPossibleValue <= HighestPossibleValue);
      r32 Value = RandomUnilateral(Entropy);
      t.name Result = MapValueToRange(LowestPossibleValue, Value, HighestPossibleValue);
      Assert(Result >= LowestPossibleValue);
      Assert(Result <= HighestPossibleValue);
      return Result;
    }
  }
)

poof(gen_random_between(r32))
#include <generated/gen_random_between_float .h>


poof(gen_random_between(u32))
#include <generated/gen_random_between_unsigned int .h>


inline b32
RandomChoice(random_series* Entropy)
{
  b32 Result = RandomBilateral(Entropy) > 0.0f;
  return Result;
}

inline v3
RandomV3Bilateral(random_series *Entropy)
{
  v3 Result =  {{ RandomBilateral(Entropy),
                  RandomBilateral(Entropy),
                  RandomBilateral(Entropy) }};
  return Result;
}

inline v3
RandomV3(random_series *Entropy)
{
  v3 Result =  {{ RandomUnilateral(Entropy),
                  RandomUnilateral(Entropy),
                  RandomUnilateral(Entropy) }};
  return Result;
}

