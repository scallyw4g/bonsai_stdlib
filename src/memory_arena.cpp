
inline b32
VaporizeArena(memory_arena *Arena)
{
  TIMED_FUNCTION();
  DEBUG_UNREGISTER_ARENA(Arena);

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

