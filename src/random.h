
v4 Floor(v4 v)
{
  v.x = Floorf(v.x);
  v.y = Floorf(v.y);
  v.z = Floorf(v.z);
  v.w = Floorf(v.w);
  return v;
}

v3 Floor(v3 v)
{
  v.x = Floorf(v.x);
  v.y = Floorf(v.y);
  v.z = Floorf(v.z);
  return v;
}

f32 Fract(f32 v)
{
  double intval;
  f32 Result = f32(modf((double)v, &intval));
  return Result;
}

v2 Fract(v2 v)
{
  double intval;
  v.x = f32(modf((double)v.x, &intval));
  v.y = f32(modf((double)v.y, &intval));
  return v;
}

v3 Fract(v3 v)
{
  double intval;
  v.x = f32(modf((double)v.x, &intval));
  v.y = f32(modf((double)v.y, &intval));
  v.z = f32(modf((double)v.z, &intval));
  return v;
}

v3 Sin(v3 v)
{
  v.x = f32(Sin(v.x));
  v.y = f32(Sin(v.y));
  v.z = f32(Sin(v.z));
  return v;
}

// NOTE(Jesse): These hash functions came from Inigo Quilez shadertoys.  When
// he says 'replace by something better' .. I don't know what would be better,
// so I'm just using them for now.  Maybe he means test that they produce a
// reasonable hash distribution ..?  I did this for the string hashing routines
// so maybe I should do something similar here.
//

// @iq_shadertoy_hash_function
link_inline v3
hash_v3( v3 p )   // this hash is not production ready, please
{                 // replace this by something better
  p = V3( Dot(p, V3(127.1f,311.7f, 74.7f)),
          Dot(p, V3(269.5f,183.3f,246.1f)),
          Dot(p, V3(113.5f,271.9f,124.6f)));
  return -1.0 + 2.0*Fract(Sin(p)*43758.5453123f);
}

// @iq_shadertoy_hash_function
link_inline f32
hash_f32( float n )
{
  return Fract(Sin(n)*753.5453123f);
}

// @iq_shadertoy_hash_function
link_inline f32
hashf(v3 p)       // replace this by something better
{
  p  = 50.0*Fract( p*0.3183099f + V3(0.71f,0.113f,0.419f));
  return Fract( p.x*p.y*p.z*(p.x+p.y+p.z) );
}









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
  func gen_map_value_funcs(t)
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

    link_internal r32
    MapValueToUnilateral( t.name LowestPossibleValue, r32 Value, t.name HighestPossibleValue)
    {
      Assert(Value >= LowestPossibleValue);
      Assert(Value <= HighestPossibleValue);
      r32 Range = r32(HighestPossibleValue - LowestPossibleValue);
      r32 Result = (Value-r32(LowestPossibleValue)) / Range;
      Assert(Result >= 0.f);
      Assert(Result <= 1.f);
      return Result;
    }
  }
)


poof(gen_map_value_funcs(r32))
#include <generated/gen_map_value_to_range_r32.h>

poof(gen_map_value_funcs(u32))
#include <generated/gen_map_value_to_range_u32.h>

poof(gen_map_value_funcs(s32))
#include <generated/gen_map_value_to_range_s32.h>


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
#include <generated/gen_random_between_r32.h>

poof(gen_random_between(u32))
#include <generated/gen_random_between_u32.h>

poof(gen_random_between(s32))
#include <generated/gen_random_between_s32.h>


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

inline v3i
RandomV3i(random_series *Entropy, v3i MaxValue)
{
  Assert(MaxValue.E[0] > 0);
  Assert(MaxValue.E[1] > 0);
  Assert(MaxValue.E[2] > 0);

  v3i Result =  {{ s32(RandomBetween(0u, Entropy, u32(MaxValue.x))),
                   s32(RandomBetween(0u, Entropy, u32(MaxValue.y))),
                   s32(RandomBetween(0u, Entropy, u32(MaxValue.z))) }};
  return Result;
}

inline random_series
RandomSeriesFromV3i(v3i Input)
{
  // TODO(Jesse): Better hash function .. ?
  s32 Bits0 = (Input.x) * 79;
  s32 Bits1 = (Input.y) * 3518;
  s32 Bits2 = (Input.z) * 984;
  s32 Bits3 = (Input.x) * 27823;

  random_series Result = {u64(Bits0) | u64(Bits1)<<15 | u64(Bits2)<<31 | u64(Bits3)<<47};
  return Result;
}

inline random_series
RandomSeriesFromV3(v3 Input)
{
  // TODO(Jesse): Better hash function .. ?
  u32 Bits0 = (ReinterpretCast(u32, Input.x)>>15) * 79;
  u32 Bits1 = (ReinterpretCast(u32, Input.y)>>15) * 3518;
  u32 Bits2 = (ReinterpretCast(u32, Input.z)>>15) * 984;
  /* u32 Bits3 = 1763547654; */
  u32 Bits3 = (ReinterpretCast(u32, Input.x)>>15) * 27823;

  random_series Basis = {u64(Bits0) | u64(Bits1)<<15 | u64(Bits2)<<31 | u64(Bits3)<<47};
  random_series Result = {RandomU32(&Basis) | (RandomU32(&Basis)<<31)};
  return Result;
}

inline f32
RandomUnilateralFromV3(v3 Input)
{
  random_series Entropy = RandomSeriesFromV3(Input);
  f32 Result =  RandomUnilateral(&Entropy);
  return Result;
}

inline v3
RandomV3FromV3(v3 Input)
{
  random_series Entropy = RandomSeriesFromV3(Input);
  v3 Result =  {{ RandomUnilateral(&Entropy),
                  RandomUnilateral(&Entropy),
                  RandomUnilateral(&Entropy) }};
  return Result;
}
