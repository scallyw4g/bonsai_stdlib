#define MEMPROTECT (MEMPROTECT_OVERFLOW || MEMPROTECT_UNDERFLOW)
#define MEMPROTECT_UNDERFLOW (0)
#define MEMPROTECT_OVERFLOW (0)

#if MEMPROTECT_UNDERFLOW && MEMPROTECT_OVERFLOW
#error "Underflow and Overflow protection are mutually exclusive"
#endif

enum memory_protection_type
{
  MemoryProtection_Protected,
  MemoryProtection_RW,
};


inline umm
SafeTruncateToUMM(u64 Size)
{
  Assert(Size <= umm_MAX);
  umm Result = (umm)Size;
  return Result;
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
  Assert(Size < u8_MAX);
  u8 Result = (u8)Size;
  return Result;
}

link_internal u8
SafeTruncateU8(s32 Size)
{
  Assert(Size < u8_MAX);
  u8 Result = (u8)Size;
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
  bonsai_futex DebugFutex;
#endif
};

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define LINE_STRING STRINGIZE(__LINE__)

#if DEBUG_SYSTEM_API

#define AllocateProtection(Type, Arena, Number, Protection)                                                                                              \
  ( GetDebugState() ?                                                                                                                   \
      (Type*)GetDebugState()->Debug_Allocate(Arena, sizeof(Type), (umm)Number, #Type " " __FILE__ ":" LINE_STRING, __LINE__, __FILE__, 1, Protection ) : \
      (Type*)PushSize( Arena, sizeof(Type)*(umm)Number, 1, Protection)                                                                                   \
  )

#define AllocateAlignedProtection(Type, Arena, Number, Alignment, Protection)                                                                                   \
  ( GetDebugState() ?                                                                                                                          \
    (Type*)GetDebugState()->Debug_Allocate( Arena, sizeof(Type), (umm)Number, #Type ":" __FILE__ ":" LINE_STRING, __LINE__, __FILE__, Alignment, Protection ) : \
    (Type*)PushSize( Arena, sizeof(Type)*(umm)Number, Alignment, Protection)                                                                                    \
  )

#define AllocateAligned(Type, Arena, Number, Alignment)                                                                                                  \
  ( GetDebugState() ?                                                                                                                   \
    (Type*)GetDebugState()->Debug_Allocate( Arena, sizeof(Type), (umm)Number, #Type ":" __FILE__ ":" LINE_STRING, __LINE__, __FILE__, Alignment, True) : \
    (Type*)PushSize( Arena, sizeof(Type)*(umm)Number, Alignment, True)                                                                                   \
  )

#define Allocate(Type, Arena, Number)                                                                                                             \
  ( GetDebugState() ?                                                                                                            \
    (Type*)GetDebugState()->Debug_Allocate( Arena, sizeof(Type), (umm)Number, #Type ":" __FILE__ ":" LINE_STRING , __LINE__, __FILE__, 1, True) : \
    (Type*)PushSize( Arena, sizeof(Type)*(umm)Number, 1, True)                                                                                    \
  )

void noop() { }

#define DEBUG_REGISTER_ARENA(Arena, ThreadId)             do { GetDebugState() ? GetDebugState()->RegisterArena(#Arena, Arena, ThreadId) : noop(); } while (false)
#define DEBUG_REGISTER_NAMED_ARENA(Arena, ThreadId, Name) do { GetDebugState() ? GetDebugState()->RegisterArena(Name, Arena, ThreadId) : noop(); } while (false)
#define DEBUG_REGISTER_THREAD(TParams)                    do { GetDebugState() ? GetDebugState()->RegisterThread(TParams) : noop(); } while (false)

#else // BONSAI_INTERNAL

#define AllocateProtection(Type, Arena, Number, Protection) \
      (Type*)PushSize( Arena, sizeof(Type)*(umm)Number, 1, Protection)

#define AllocateAlignedProtection(Type, Arena, Number, Alignment, Protection) \
    (Type*)PushSize( Arena, sizeof(Type)*(umm)Number, Alignment, Protection)

#define AllocateAligned(Type, Arena, Number, Alignment) \
    (Type*)PushSize( Arena, sizeof(Type)*(umm)Number, Alignment, True)

#define Allocate(Type, Arena, Number) \
    (Type*)PushSize( Arena, sizeof(Type)*(umm)Number, 1, True)


#define DEBUG_REGISTER_ARENA(...)
#define DEBUG_REGISTER_NAMED_ARENA(...)
#define DEBUG_REGISTER_THREAD(...)

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

#if 1
struct temp_memory_handle
{
  memory_arena *Arena;
  u8* BeginMark;
};

link_internal void
EndTemporaryMemory(temp_memory_handle *Handle, b32 ReportLeaks = True)
{
  memory_arena *Arena = Handle->Arena;
  u8 *BeginMark = Handle->BeginMark;
  if (Contains(Arena->Start, BeginMark, Arena->End))
  {
    Assert(BeginMark <= Arena->At);

#if 1
    ZeroMemory(BeginMark, Arena->At-BeginMark);
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

  return;
}

link_internal temp_memory_handle
BeginTemporaryMemory(memory_arena *Arena, b32 ReportLeaks = True)
{
  temp_memory_handle Result = {
    .Arena = Arena,
    .BeginMark = Arena->At,
  };

  if (Arena->Start == 0)
  {
    Leak("Need to make sure the arena is initialized before starting temporary memory!");
  }

  return Result;
}
#endif


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
  // TODO(Jesse, id: 97, tags: speed): Vectorize for speed boost!
  for( umm BytesCopied = 0;
       BytesCopied < Size;
       ++BytesCopied )
  {
     Dest[BytesCopied] = Src[BytesCopied];
  }
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

// @temp-string-builder-memory
// TODO(Jesse, id: 98, tags: robustness, api_improvement): Make allocating these on the stack work!
link_internal memory_arena*
AllocateArena(umm RequestedBytes = Megabytes(1), b32 MemProtect = True)
{
  RequestedBytes = Max(RequestedBytes, Megabytes(1));

  umm PageSize = PlatformGetPageSize();
  umm ToNextPage = PageSize - (RequestedBytes % PageSize);
  umm AllocationSize = RequestedBytes + ToNextPage;

  Assert(AllocationSize % PageSize == 0);

#if MEMPROTECT_OVERFLOW
  Assert(sizeof(memory_arena) < PageSize);
  u8 *ArenaBytes = PlatformAllocateSize(PageSize*2);
  ArenaBytes += (PageSize - sizeof(memory_arena));

#elif MEMPROTECT_UNDERFLOW
  NotImplemented;
#else

  u8 *ArenaBytes = PlatformAllocateSize(PageSize);
#endif

  memory_arena *Result = (memory_arena*)ArenaBytes;

  u8 *Bytes = PlatformAllocateSize(AllocationSize);
  Result->Start = Bytes;
  Result->At = Bytes;

  Result->End = Bytes + AllocationSize;
  Result->NextBlockSize = Min(AllocationSize * 2, Gigabytes(1)); // Max out at 1gb blocks

#if MEMPROTECT_OVERFLOW
  if (MemProtect)
  {
    Assert(OnPageBoundary(Result, PageSize));
    ProtectPage(ArenaBytes + sizeof(memory_arena));
  }

  Assert((umm)Result->Start % PageSize == 0);
  Assert(Remaining(Result) >= RequestedBytes);
#elif MEMPROTECT_UNDERFLOW
  NotImplemented;
#else
  Assert(OnPageBoundary(Result, PageSize));
  Assert(Remaining(Result) >= RequestedBytes);
#endif

  return Result;
}

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
ReallocateArena(memory_arena *Arena, umm MinSize, b32 MemProtect)
{
  umm AllocationSize = Arena->NextBlockSize;
  if (MinSize > AllocationSize)
    AllocationSize = MinSize;

  memory_arena *NewArena = AllocateArena(AllocationSize, MemProtect);

  memory_arena OldArena = *Arena;
  *Arena = *NewArena;
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
        Error("Unable to reallocate : Arena didn't have enough space left to accommodate %ld bytes.", Diff);
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
PushSize(memory_arena *Arena, umm Size, umm Alignment, b32 MemProtect)
{
  Assert(Arena->At <= Arena->End);              // Sanity checks
  Assert(Remaining(Arena) <= TotalSize(Arena));

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
    ReallocateArena(Arena, TotalAllocationSize, MemProtect);
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
  return Result;
}

#endif

link_internal void*
PushStruct(memory_arena *Memory, umm sizeofStruct, umm Alignment = 1, b32 MemProtect = True)
{
  void* Result = PushSize(Memory, sizeofStruct, Alignment, MemProtect);
  return Result;
}


inline b32 VaporizeArena(memory_arena *Arena);
link_internal b32 UnprotectArena(memory_arena *Arena);
