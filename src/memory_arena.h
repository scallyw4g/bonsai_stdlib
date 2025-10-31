#define MEMPROTECT (MEMPROTECT_OVERFLOW || MEMPROTECT_UNDERFLOW)
/* #define MEMPROTECT_UNDERFLOW (0) */
/* #define MEMPROTECT_OVERFLOW (0) */

#if MEMPROTECT_UNDERFLOW && MEMPROTECT_OVERFLOW
#error "Underflow and Overflow protection are mutually exclusive"
#endif

enum memory_protection_type
{
  MemoryProtection_Protected,
  MemoryProtection_RW,
};


inline umm
SafeTruncateToUMM(s64 Size)
{
  Assert(Size >= 0);
  Assert(umm(Size) <= umm_MAX);
  umm Result = (umm)Size;
  return Result;
}

inline umm
SafeTruncateToUMM(u64 Size)
{
  Assert(Size <= umm_MAX);
  umm Result = (umm)Size;
  return Result;
}

inline f32
SafeTruncateTo_f32(f64 Size)
{
  Assert(Size >= r64(f32_MIN));
  Assert(Size <= r64(f32_MAX));
  return f32(Size);
}

inline s32
SafeTruncateToS32(u64 Size)
{
  Assert(Size <= s32_MAX);
  s32 Result = (s32)Size;
  return Result;
}

#if BONSAI_EMCC
inline s32
SafeTruncateToS32(umm Size)
{
  Assert(Size <= s32_MAX);
  s32 Result = (s32)Size;
  return Result;
}
#endif

inline u32
SafeTruncateToU32(u64 Size)
{
  Assert(Size <= u32_MAX);
  u32 Result = (u32)Size;
  return Result;
}

inline u16
SafeTruncateToU16(umm Size)
{
  Assert(Size <= u16_MAX);
  u16 Result = (u16)Size;
  return Result;
}

link_internal u8
SafeTruncateU8(u32 Size)
{
  Assert(Size <= u8_MAX);
  u8 Result = (u8)Size;
  return Result;
}

link_internal u8
SafeTruncateU8(s32 Size)
{
  Assert(Size >= 0);
  Assert(Size <= s32(u8_MAX));
  u8 Result = (u8)Size;
  return Result;
}

link_internal s8
SafeTruncateS8(u32 Size)
{
  Assert(s32(Size) <= s32(s8_MAX));
  s8 Result = (s8)Size;
  return Result;
}

link_internal s8
SafeTruncateS8(s32 Size)
{
  Assert(Size >= s32(s8_MIN));
  Assert(Size <= s32(s8_MAX));
  s8 Result = (s8)Size;
  return Result;
}


inline umm
Kilobytes(umm Bytes)
{
  umm Result = SafeTruncateToUMM(Bytes*1024);
  return Result;
}

inline umm
Megabytes(umm Number)
{
  umm Result = SafeTruncateToUMM(Number*Kilobytes(1024));
  return Result;
}

inline umm
Gigabytes(umm Number)
{
  umm Result = SafeTruncateToUMM(Number*Megabytes(1024));
  return Result;
}

inline umm
Terabytes(umm Number)
{
  umm Result = SafeTruncateToUMM(Number*Gigabytes(1024));
  return Result;
}

#define INVALID_THREAD_HANDLE (u64_MAX)

struct memory_arena
{
  u8* Start;
  u8* At;
  u8* End;

  umm NextBlockSize;
  memory_arena *Prev;

#if BONSAI_INTERNAL
  umm Pushes;
#endif

  bonsai_futex DebugFutex;
};

// TODO(Jesse, globals_cleanup): Put this on stdlib ..?
global_variable memory_arena Global_PermMemory = {};

#if BONSAI_DEBUG_SYSTEM_API

#define AllocateProtection(Type, Arena, Number, Protection)                                                                                            \
( GetDebugState() ?                                                                                                                                    \
    (Type*)GetDebugState()->Debug_Allocate(Arena, sizeof(Type), umm(Number), #Type " " __FILE__ ":" LINE_STRING, __LINE__, __FILE__, 1, Protection ) : \
    (Type*)PushSize( Arena, sizeof(Type)*umm(Number), 1, Protection, __FILE__ ":" LINE_STRING)                                                         \
)

#define AllocateAlignedProtection(Type, Arena, Number, Alignment, Protection)                                                                                 \
( GetDebugState() ?                                                                                                                                           \
  (Type*)GetDebugState()->Debug_Allocate( Arena, sizeof(Type), umm(Number), #Type ":" __FILE__ ":" LINE_STRING, __LINE__, __FILE__, Alignment, Protection ) : \
  (Type*)PushSize( Arena, sizeof(Type)*umm(Number), Alignment, Protection, __FILE__ ":" LINE_STRING)                                                          \
)

#define AllocateAligned(Type, Arena, Number, Alignment)                                                                                                \
( GetDebugState() ?                                                                                                                                    \
  (Type*)GetDebugState()->Debug_Allocate( Arena, sizeof(Type), umm(Number), #Type ":" __FILE__ ":" LINE_STRING, __LINE__, __FILE__, Alignment, True) : \
  (Type*)PushSize( Arena, sizeof(Type)*umm(Number), Alignment, True, __FILE__ ":" LINE_STRING)                                                         \
)

#define Allocate(Type, Arena, Number)                                                                                                           \
( GetDebugState() ?                                                                                                                             \
  (Type*)GetDebugState()->Debug_Allocate( Arena, sizeof(Type), umm(Number), #Type ":" __FILE__ ":" LINE_STRING , __LINE__, __FILE__, 1, True) : \
  (Type*)PushSize( Arena, sizeof(Type)*umm(Number), 1, True, __FILE__ ":" LINE_STRING)                                                          \
)

void noop() {}

#define DEBUG_REGISTER_ARENA(Arena, ThreadId)             do { DebugRegisterArenaName(#Arena, Arena); } while (false)
#define DEBUG_REGISTER_NAMED_ARENA(Arena, ThreadId, Name) do { DebugRegisterArenaName(Name, Arena); } while (false)
#define DEBUG_UNREGISTER_ARENA(Arena)                     do { UnregisterArena(Arena); } while (false)
#define DEBUG_REGISTER_THREAD(TParams)                    do { RegisterThread(TParams) ; } while (false)


      //
#else // BONSAI_DEBUG_SYSTEM_API
      //

#define AllocateProtection(Type, Arena, Number, Protection) \
      (Type*)PushSize( Arena, sizeof(Type)*umm(Number), 1, Protection, __FILE__ ":" LINE_STRING)

#define AllocateAlignedProtection(Type, Arena, Number, Alignment, Protection) \
    (Type*)PushSize( Arena, sizeof(Type)*umm(Number), Alignment, Protection, __FILE__ ":" LINE_STRING)

#define AllocateAligned(Type, Arena, Number, Alignment) \
    (Type*)PushSize( Arena, sizeof(Type)*umm(Number), Alignment, True, __FILE__ ":" LINE_STRING)

#define Allocate(Type, Arena, Number) \
    (Type*)PushSize( Arena, sizeof(Type)*umm(Number), 1, True, __FILE__ ":" LINE_STRING)


#define DEBUG_REGISTER_ARENA(...)
#define DEBUG_UNREGISTER_ARENA(...)
#define DEBUG_REGISTER_NAMED_ARENA(...)
#define DEBUG_REGISTER_THREAD(...)
#define DebugRegisterArena(...)

#endif



link_internal umm PlatformGetPageSize();
link_internal b32 PlatformSetProtection(u8 *Base, umm Size, memory_protection_type Protection);

link_internal u8 * PlatformAllocateSize(umm AllocationSize);
link_internal b32  PlatformDeallocate(u8 *Base, umm Size);

// TODO(Jesse, metaprogramming): Metaprogram this!
// TODO(Jesse): Where should this actually go?
template<typename T> link_internal b32
Contains(T *Start, T *At, T* End)
{
  b32 Result = (At >= Start && At < End);
  return Result;
}

template <typename T> inline void
Fill(T *Struct, u8 ByteValue)
{
#if 1
  memset((void*)Struct, 0, sizeof(T));
#else
  for ( umm Byte = 0; Byte < sizeof(T); ++Byte)
  {
    *(((u8*)Struct) + Byte) = ByteValue;
  }
#endif
}

b32
OnPageBoundary(memory_arena *Arena, umm PageSize)
{
  b32 Result = (umm)Arena->At % PageSize == 0;
  return Result;
}

inline u64
AlignTo(umm Mem, umm Alignment)
{
  Assert(Alignment);
  umm At = Mem;
  umm ToNextAlignment = Alignment - (At % Alignment);
  if (ToNextAlignment < Alignment) // We're on a page boundary
  {
    At += ToNextAlignment;
  }

  Assert(At % Alignment == 0);

  return At;
}

inline u8*
AlignTo(u8 *Ptr, umm Alignment)
{
  Assert(Alignment);
  umm At = (umm)Ptr;
  umm ToNextAlignment = Alignment - (At % Alignment);
  Assert( (At+ToNextAlignment) % Alignment == 0);

  if (ToNextAlignment != Alignment) // We're on a page boundary
  {
    At += ToNextAlignment;
    Assert(At % Alignment == 0);
  }

  return (u8*)At;
}

inline void
AlignTo(memory_arena *Arena, umm Alignment)
{
  Arena->At = AlignTo(Arena->At, Alignment);
  Assert(Arena->At <= Arena->End);
  return;
}

inline u8 *
SetToPageBoundary(memory_arena *Arena)
{
  umm Align = PlatformGetPageSize();
  AlignTo(Arena, Align);
  return Arena->At;
}

template <typename T> umm
TotalSize(T *Sizable)
{
  Assert(Sizable->At <= Sizable->End);
  umm Result = (umm)(Sizable->End - Sizable->Start) * sizeof(Sizable->Start[0]);
  return Result;
}

template <typename T> umm
Remaining(T *Sizable, umm Modifier = 0)
{
  umm Result = 0;
  if (Sizable->At+Modifier < Sizable->End)
  {
    Result = (umm)( (umm)(Sizable->End-Sizable->At) + Modifier);
  }
  return Result;
}

template <typename T> umm
TotalElements(T *Sizable)
{
  Assert(Sizable->At >= Sizable->Start);
  umm Result = (umm)(Sizable->End-Sizable->Start);
  return Result;
}

// NOTE(Jesse): Kind of a dirty hack to thunk through for types that don't have
// a special indexing struct
template <typename T> umm
ZerothIndex(T *Sizable)
{
  return 0;
}

template <typename T> umm
AtElements(T *Sizable)
{
  Assert(Sizable->At >= Sizable->Start);
  umm Result = (umm)(Sizable->At-Sizable->Start);
  return Result;
}

link_internal b32
MemoryIsEqual(u8 *First, u8 *Second, umm Size)
{
  b32 Result = True;
  for (umm Index = 0;
      Index < Size;
      ++Index)
  {
    Result = Result && ( First[Index] == Second[Index]);
  }

  return Result;
}

inline void
MemSet(u8 *Src, umm Size, u8 Value)
{
  // TODO(Jesse, id: 96, tags: speed): Vectorize for speed boost!
  for( umm ByteIndex = 0;
       ByteIndex < Size;
       ++ByteIndex )
  {
     Src[ByteIndex] = Value;
  }
}

inline void
MemCopy(u8 *Src, u8 *Dest, umm Size)
{
  memcpy((void*)Dest, (void*)Src, Size);
#if 0
  // TODO(Jesse, id: 97, tags: speed): Vectorize for speed boost!
  for( umm BytesCopied = 0;
       BytesCopied < Size;
       ++BytesCopied )
  {
     Dest[BytesCopied] = Src[BytesCopied];
  }
#endif
}

#ifdef CopyMemory
#undef CopyMemory
#endif

inline void
CopyMemory(u8 *Src, u8 *Dest, umm Size)
{
  MemCopy(Src, Dest, Size);
}


inline void
ClearBuffer(u8 *Src, umm Size)
{
  MemSet(Src, Size, 0);
}

link_internal b32
ProtectPage(u8* Mem)
{
  umm PageSize = PlatformGetPageSize();
  Assert((umm)Mem % PageSize == 0);

  b32 Result = PlatformSetProtection(Mem, PageSize, MemoryProtection_Protected);
  return Result;
}

#define S1(x) #x
#define S2(x) S1(x)
#define LOCATION ()

#define POOF_SOURCE_LOCATION (__FILE__ ":" S2(__LINE__))

#ifndef POOF_PREPROCESSOR
#define   AllocateArena( ... )   AllocateArena_( POOF_SOURCE_LOCATION, True, ##__VA_ARGS__ )
#else
#define   AllocateArena( ... )
#endif

link_internal memory_arena*
AllocateArena_(const char *, b32 DebugRegister, umm RequestedBytes = 1<<20, b32 MemProtect = True);

link_internal b32
DeallocateArena(memory_arena *Arena)
{
  b32 Result = False;
  if (Arena->Start)
  {
    Result = PlatformDeallocate(Arena->Start, TotalSize(Arena));

#if MEMPROTECT_OVERFLOW
    {
      umm PageSize = PlatformGetPageSize();
      u8 *ArenaBytes =  (u8*)Arena - ((umm)Arena % PageSize);
      Result &= PlatformDeallocate(ArenaBytes, PageSize*2);
    }
#elif MEMPROTECT_UNDERFLOW
    NotImplemented;
#else
      umm PageSize = PlatformGetPageSize();
      Result &= PlatformDeallocate((u8*)Arena, PageSize);
#endif

  }


  return Result;
}

link_internal void
ReallocateArena(const char *Id, memory_arena *Arena, umm MinSize, b32 MemProtect)
{
  umm AllocationSize = Arena->NextBlockSize;
  if (MinSize > AllocationSize)
    AllocationSize = MinSize;

  memory_arena *NewArena = AllocateArena_(Id, False, AllocationSize, MemProtect);

  memory_arena OldArena = *Arena;

  *Arena = *NewArena;

   Arena->DebugFutex = OldArena.DebugFutex;
   OldArena.DebugFutex = {};

  *NewArena = OldArena;

  Arena->Prev = NewArena;

  Assert( (umm)(Arena->End - Arena->At) >= MinSize);
  Assert(Arena->At <= Arena->End);

  return;
}

link_internal u8*
Reallocate(u8* Allocation, memory_arena* Arena, umm CurrentSize, umm RequestedSize)
{
  u8* Result = 0;

  if (Allocation + CurrentSize == Arena->At)
  {
    s64 Diff = (s64)RequestedSize - (s64)CurrentSize;
    if (Diff >= 0)
    {
      if ((s64)Remaining(Arena) >= Diff)
      {
        Arena->At += Diff;
        Result = Allocation;
      }
      else
      {
        // TODO(Jesse): Should this acutally just reallocate a buffer large
        // enough to accomodate the whole request?  Probably not because that
        // would leak memory, but maybe that's fine?
        //
        // Alternatively, is there some way we can analyze the calling code to
        // ensure this case never happens?
        SoftError("Unable to reallocate : Arena didn't have enough space left to accommodate %ld bytes.", Diff);
      }
    }
    else if (Diff < 0)
    {
      if (Abs(Diff) <= CurrentSize)
      {
        Arena->At += Diff;
        Result = Arena->At;
      }
      else
      {
        Error("Unable to reallocate : Abs(Diff) was greater than CurrentSize (%lld) > (%llu) bytes.", (u64)Abs(Diff), (u64)CurrentSize);
      }
    }
    else
    {
      InvalidCodePath();
    }
  }
  else
  {
    Error("Unable to reallocate : Allocation is not the final one on the arena, or the allocation was memprotected.");
  }

  return Result;
}

// NOTE(Jesse): This is some random-ass code I put in to benchmark this bump
// allocator against malloc.  Turns out we're about 13x faster.
#define ALLOCATOR_USE_MALLOC 0
#if ALLOCATOR_USE_MALLOC

link_internal u8*
PushSize(memory_arena *Arena, umm SizeIn, umm Alignment, b32 MemProtect)
{
  umm ToAlignment = Alignment - (SizeIn % Alignment);
  umm AlignCorrectedSizeIn = SizeIn;

  u8* Result = (u8*)calloc(AlignCorrectedSizeIn,1);

  umm AtToAlignment = Alignment - ((umm)Result % Alignment);
  if (AtToAlignment != Alignment)
  {
    Result += AtToAlignment;
  }

  Assert(((umm)Result % Alignment) == 0);

  return Result;
}

#else

link_internal u8*
PushSize(memory_arena *Arena, umm Size, umm Alignment, b32 MemProtect, const char *SourceLocation)
{
  Assert(Arena->At <= Arena->End);              // Sanity checks
  Assert(Remaining(Arena) <= TotalSize(Arena));

  if (ThreadLocal_ThreadIndex != INVALID_THREAD_LOCAL_THREAD_INDEX)
  {
    /* if (Arena == &Global_PermMemory) { PrintToStdout(CSz("Acquire\n")); } */
    AcquireFutex(&Arena->DebugFutex);
  }

  umm ExtraPageBytes = 0;
#if MEMPROTECT
  umm PageSize = PlatformGetPageSize();
  if (MemProtect)
  {
    /* umm Pages = (u32)((Size/PageSize) + 1); */
    /* umm PageBytes = (Pages*PageSize) + PageSize; */
    ExtraPageBytes = PageSize;
    SetToPageBoundary(Arena); // NOTE(Jesse): This should be safe because we reallocate multiples of pages
    Assert( (umm)Arena->At % PageSize == 0);
    Assert( (umm)Arena->At % (umm)Alignment == 0);
    Assert( Arena->At <= Arena->End );
  }
#endif


  // TODO(Jesse): What should the policy here actually be?
  if (Alignment < 8) Alignment = 8;
  umm ToAlignment = (umm)Alignment - ((umm)Arena->At % Alignment);


  umm RemainingInArena = Remaining(Arena);
  umm TotalAllocationSize = Size+ToAlignment+ExtraPageBytes;

  if ( TotalAllocationSize > RemainingInArena)
  {
    ReallocateArena(SourceLocation, Arena, TotalAllocationSize, MemProtect);
    Assert((umm)Arena->At % (umm)Alignment == 0);
  }
  else
  {
    if (ToAlignment < Alignment)
    {
      Arena->At += ToAlignment;
    }
    Assert((umm)Arena->At % (umm)Alignment == 0);
  }

#if MEMPROTECT_OVERFLOW
  if (MemProtect)
  {
    umm EndOfStruct = (umm)Arena->At + Size;
    umm EndToNextPage = PageSize - (EndOfStruct % PageSize);
    Assert( (EndOfStruct+EndToNextPage) % PageSize == 0);

    Arena->At += EndToNextPage;
  }
#elif MEMPROTECT_UNDERFLOW
  if (Arena->MemProtect)
  {
    umm At = (umm)Arena->At;
    umm NextPageOffset = PageSize - (At % PageSize);
    Assert( (At+NextPageOffset) % PageSize == 0);

    u8* NextPage = Arena->At + NextPageOffset;
    Assert( (umm)NextPage % PageSize == 0);
    ProtectPage(NextPage);

    Result = NextPage + PageSize;
  }
#else

#endif

  u8* Result = Arena->At;
  Arena->At += Size;

#if MEMPROTECT_OVERFLOW
  if (MemProtect)
  {
    u8* LastPage = SetToPageBoundary(Arena);
    Assert( (umm)LastPage % PageSize == 0);
    ProtectPage(LastPage);
    Arena->At += PageSize;
  }
#elif MEMPROTECT_UNDERFLOW
#else
#endif




#if MEMPROTECT
#if BONSAI_INTERNAL
  ++Arena->Pushes;
#endif
  if (MemProtect)
  {
    Assert( ((umm)Result+Size) % PageSize == 0);
    Assert( (umm)Arena->At % PageSize == 0);
  }
#endif


  Assert(Arena->At <= Arena->End);
  Assert((umm)Result % Alignment == 0);

  Assert(Remaining(Arena) <= TotalSize(Arena)); // Sanity Check

  if (ThreadLocal_ThreadIndex != INVALID_THREAD_LOCAL_THREAD_INDEX)
  {
    /* if (Arena == &Global_PermMemory) { PrintToStdout(CSz("Release\n")); } */
    ReleaseFutex(&Arena->DebugFutex);
  }

  return Result;
}

#endif

link_internal void*
PushStruct(memory_arena *Memory, umm sizeofStruct, umm Alignment = 1, b32 MemProtect = True)
{
  void* Result = PushSize(Memory, sizeofStruct, Alignment, MemProtect, "PushStruct");
  return Result;
}


#if 1
struct temp_memory_handle;
link_internal void EndTemporaryMemory(temp_memory_handle *Handle, b32 ReportLeaks = True);

struct temp_memory_handle
{
  memory_arena *Arena;
  u8* BeginMark;


  // NOTE(Jesse): This is some fucking nonsense to shut up a warning because
  // apparently we're not allowed to have a destructor with implicitly defined constructors
  temp_memory_handle() = default;
  temp_memory_handle(const temp_memory_handle &) = default;

/* #if !POOF_PREPROCESSOR */
  ~temp_memory_handle()
  {
    if (Arena || BeginMark)
    {
      Assert(Arena);
      Assert(BeginMark);

      EndTemporaryMemory(this);
    }
  }
/* #endif */

};



link_internal void
EndTemporaryMemory(temp_memory_handle *Handle, b32 ReportLeaks)
{

  memory_arena *Arena = Handle->Arena;
  u8 *BeginMark = Handle->BeginMark;
  if (Contains(Arena->Start, BeginMark, Arena->End))
  {
    Assert(BeginMark <= Arena->At);

    /* s64 Size = Arena->At-BeginMark; */
    /* Info("Clearing Size(%lu)", Size); */

#if 1
    // TODO(Jesse): Improve this from its current state of a thunk to memset
    ZeroMemory(BeginMark, (size_t)(Arena->At-BeginMark));
#else
    u8* TmpAt = Handle->BeginMark;
    while (TmpAt < Arena->At)
    {
      *TmpAt = 0x00;
      TmpAt++;
    }
#endif

    Arena->At = BeginMark;
  }
  else
  {
    if (ReportLeaks)
    {
      Leak("Leaking memory when doing EndTemporaryMemory.");
    }
  }

  Handle->Arena = 0;
  Handle->BeginMark = 0;

  return;
}

#define BeginTemporaryMemory(...) BeginTemporaryMemory_(POOF_SOURCE_LOCATION, __VA_ARGS__)

link_internal temp_memory_handle
BeginTemporaryMemory_(const char *SourceLocation, memory_arena *Arena, b32 ReportLeaks = True)
{
  if (Arena->Start == 0)
  {
    ReallocateArena(SourceLocation, Arena, Megabytes(1), True);
  }

  temp_memory_handle Result = {};

  Result.Arena = Arena;
  Result.BeginMark = Arena->At;

  return Result;
}
#endif



inline b32 VaporizeArena(memory_arena *Arena);
link_internal b32 UnprotectArena(memory_arena *Arena);
