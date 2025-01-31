
inline b32
VaporizeArena(memory_arena *Arena)
{
  TIMED_FUNCTION();
  DEBUG_UNREGISTER_ARENA(Arena);

  Info("Vaporizing Arena (0x%x)", Arena);
  b32 Result = True;

  if (Arena)
  {
    if(Arena->Prev)
    {
      Result = VaporizeArena(Arena->Prev);
      Arena->Prev = 0;
    }

    if (Arena->Start)
    {
      Result &= DeallocateArena(Arena);
    }
  }

  return Result;
}

link_internal b32
UnprotectArena(memory_arena *Arena)
{
  TIMED_FUNCTION();

  umm Size = (umm)Arena->End - (umm)Arena->Start;
  b32 Result = PlatformSetProtection(Arena->Start, Size, MemoryProtection_RW);
  if (Result == False)
  {
    Error("Unprotecting arena failed");
  }

  return Result;
}

#if 1
//  TODO(Jesse): The zeroing this function does is pretty slow.  I decided to
//  not use it as much as possible in favor of vaporizing/reallocating arenas,
//  however that amortizes the cost of zeroing when fresh pages are faulted in.
//
//  I don't love that the cost becomes invisible, and would actually probably
//  rather it showed up explicitly on the profile.  Unfortunately at the moment
//  the byte-wise zeroing is extremely slow and should be vectorized before it
//  goes back in the worker-thread.
//
//  @turn_rewind_arena_back_on
//
inline b32
RewindArena(memory_arena *Arena, umm RestartBlockSize = Megabytes(1) )
{
  TIMED_FUNCTION();

  b32 Result = True;

#if BONSAI_INTERNAL
  AcquireFutex(&Arena->DebugFutex);
#endif

  // Check for start because when we allocate an arena on the stack it's
  // cleared to zero and treated as a sentinal.
  if (Arena->Prev && Arena->Prev->Start)
  {
    Result &= UnprotectArena(Arena->Prev);
    Result &= VaporizeArena(Arena->Prev);
    Arena->Prev = 0;
  }

  if (Arena->Start)
  {
    if ( Arena->At > Arena->Start )
    {
      Result &= UnprotectArena(Arena);
      TIMED_BLOCK("ArenaClear");
      memset((void*)Arena->Start, 0, (size_t)(Arena->At-Arena->Start) );
#if 0
      u8* ClearByte = Arena->Start;
      while( ClearByte < Arena->At )
      {
        *ClearByte++ = 0;
      }
#endif
      END_BLOCK("ArenaClear");

      Arena->At = Arena->Start;
      Arena->NextBlockSize = RestartBlockSize;

#if BONSAI_INTERNAL
      Arena->Pushes = 0;
#endif

#if BONSAI_DEBUG_SYSTEM_API
      DEBUG_CLEAR_MEMORY_RECORDS_FOR(Arena);
#endif

    }
  }

#if BONSAI_INTERNAL
  ReleaseFutex(&Arena->DebugFutex);
#endif

  return Result;
}
#endif

// @temp-string-builder-memory
// TODO(Jesse, id: 98, tags: robustness, api_improvement): Make allocating these on the stack work!
link_internal memory_arena*
AllocateArena_(const char *Id, umm RequestedBytes /* = Megabytes(1) */, b32 MemProtect /* = True */, b32 DebugRegister)
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

#if BONSAI_INTERNAL
  InitializeFutex(&Result->DebugFutex);
#endif

  if (DebugRegister)
  {
    DEBUG_REGISTER_NAMED_ARENA(Result, ThreadLocal_ThreadIndex, Id );
  }

  return Result;
}
