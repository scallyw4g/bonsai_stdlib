#define CAssert(condition) static_assert((condition), #condition )
#define OffsetOf(member_name, type_name) offsetof(type_name, member_name)

#define PI64 (3.1415926535897932384626433832795028841971693993)
#define PI32 ((float)PI64)

#define local_persist     static
#define global_variable   static
#define debug_global      static

// Internal linkage
#define link_internal     static
#define link_inline       inline
#define link_weak         link_external __attribute__((weak))

// External linkage (pointer is from another lib)
#define link_external     extern "C"

// Export linkage (expose function pointer)
#if BONSAI_WIN32
#define link_export __declspec(dllexport) link_external

#else
#define link_export link_external

#endif

#define ArrayCount(a) (sizeof(a)/sizeof(a[0]))

// 8 Bit types
typedef char           s8;
typedef unsigned char  u8;
typedef bool           b8;
CAssert(sizeof(b8) == 1);

// 16 Bit types
typedef short          s16;
typedef unsigned short u16;

// 32 Bit types
typedef int          s32;
typedef unsigned int u32;
typedef float        r32;
typedef float        f32;
typedef u32          b32;

typedef r32      radians;
typedef r32      degrees;

// 64 Bit types
typedef long long int           s64;
typedef unsigned long long int  u64;
typedef double                  r64;
typedef double                  f64;
#if BONSAI_EMCC
typedef u32                     umm;
#else
typedef u64                     umm;
#endif
typedef u64                     b64;

#if POOF_PREPROCESSOR

// Unfortunately Microsoft has these types and we have to support them.  This
// is the best way I could figure out at the time .. though once the
// typechecker is better we might actually be able to typedef these
#define __int8 char
#define __int16 short
#define __int32 int
#define __int64 long long

CAssert(sizeof(__int64) == 8);
CAssert(sizeof(__int32) == 4);
CAssert(sizeof(__int16) == 2);
CAssert(sizeof(__int8) == 1);
#endif

CAssert(sizeof(s64) == 8);
CAssert(sizeof(u64) == 8);
CAssert(sizeof(r64) == 8);
CAssert(sizeof(umm) == sizeof(void*));

CAssert(sizeof(s32) == 4);
CAssert(sizeof(u32) == 4);
CAssert(sizeof(r32) == 4);

CAssert(sizeof(s16) == 2);
CAssert(sizeof(u16) == 2);

CAssert(sizeof(s8) == 1);
CAssert(sizeof(u8) == 1);



#define bitsof(thing) (sizeof(thing)*8)

CAssert(bitsof(s64) == 64);
CAssert(bitsof(u64) == 64);
CAssert(bitsof(r64) == 64);
CAssert(bitsof(umm) == bitsof(void*));

CAssert(bitsof(s32) == 32);
CAssert(bitsof(u32) == 32);
CAssert(bitsof(r32) == 32);

CAssert(bitsof(s16) == 16);
CAssert(bitsof(u16) == 16);

CAssert(bitsof(s8) == 8);
CAssert(bitsof(u8) == 8);



#define s8_MAX  ((s8) 0x7f)
#define s16_MAX ((s16)0x7fff)
#define s32_MAX ((s32)0x7fffffff)
#define s64_MAX ((s64)0x7fffffffffffffff)

#define s8_MIN  ((s8) 0x80)
#define s16_MIN ((s16)0x8000)
#define s32_MIN ((s32)0x80000000)
#define s64_MIN ((s64)0x8000000000000000)

#define u8_MAX  (0xffu)
#define u16_MAX (0xffffu)
#define u32_MAX (0xffffffffu)
#define u64_MAX (0xffffffffffffffffllu)

#define u8_MIN  (0x0)
#define u16_MIN (0x0)
#define u32_MIN (0x0)
#define u64_MIN (0x0)


#define f32_MAX (1E+37f)
#define f32_MIN (1E-37f)
#define r32_MAX (f32_MAX)
#define r32_MIN (f32_MIN)


// https://learn.microsoft.com/en-us/dotnet/visual-basic/language-reference/data-types/double-data-type
#define f64_MAX ( 1.79769313486231570E+308)
#define f64_MIN (-1.79769313486231570E+308)
#define r64_MAX (f64_MAX)
#define r64_MIN (f64_MIN)

#if BONSAI_EMCC
#define umm_MAX u32_MAX
#define umm_MIN u32_MIN
#else
#define umm_MAX u64_MAX
#define umm_MIN u64_MIN
#endif

#if 0
// #include <stdint.h>
CAssert(s8_MAX  == INT8_MAX);
CAssert(s16_MAX == INT16_MAX);
CAssert(s32_MAX == INT32_MAX);
CAssert(s64_MAX == INT64_MAX);

CAssert(s8_MIN  == INT8_MIN);
CAssert(s16_MIN == INT16_MIN);
CAssert(s32_MIN == INT32_MIN);
CAssert(s64_MIN == INT64_MIN);

CAssert(u8_MAX  == UINT8_MAX);
CAssert(u16_MAX == UINT16_MAX);
CAssert(u32_MAX == UINT32_MAX);
CAssert(u64_MAX == UINT64_MAX);
#endif

#define True  (1)
#define False (0)

global_variable u32 Global_False = 0;
global_variable u32 Global_True = 1;

enum maybe_tag
{
  Maybe_No,
  Maybe_Yes,
};

struct counted_string
{
  umm Count;
  const char* Start; // TODO(Jesse, id: 94, tags: cleanup, open_question): Make this non-const?
                     //
#if 0
  // NOTE(Jesse): This implicit conversion fucks me over often enough that I'm taking it out
  operator bool()
  {
    bool Result = (this->Start && this->Count);
    return Result;
  }
#endif
};

typedef counted_string cs;

link_internal b32 AreEqual(cs *S1, cs *S2);
link_internal b32 AreEqual(cs S1, cs S2);
link_internal b32 AreEqual(const char *S1, const char *S2);

template <typename T> inline void
Clear(T *Struct)
{
  Fill(Struct, 0);
}

poof(
  func gen_are_equal( type_poof_symbol Types )
  {
    Types.map (Type)
    {
      link_internal b32
      AreEqual( (Type.name) E1, (Type.name) E2 )
      {
        b32 Result = E1 == E2;
        return Result;
      }

      link_internal b32
      AreEqual( (Type.name) *E1, (Type.name) *E2 )
      {
        b32 Result = *E1 == *E2;
        return Result;
      }
    }
  }
)

poof( gen_are_equal({s64 u64 r64 s32 u32 r32 s16 u16 s8 u8 }) )
#include <generated/gen_are_equal_665365505.h>

poof(
  func gen_primitive_deep_copy( type_poof_symbol Types )
  {
    Types.map (Type)
    {
      link_internal void
      DeepCopy( (Type.name) *Src, (Type.name) *Dest )
      {
        *Dest = *Src;
      }
    }
  }
)

poof( gen_primitive_deep_copy({s64 u64 r64 s32 u32 r32 s16 u16 s8 u8 }) )
#include <generated/gen_primitive_deep_copy_715421923.h>

link_internal void PlatformDebugStacktrace();

#define DETECT_CHANGES_ON(Value, PrevValueName, Block) \
  do {                                                 \
    static auto PrevValueName = Value;                 \
    if (AreEqual(Value, PrevValueName) == False) Block \
    PrevValueName = (Value);                           \
  } while (false)

