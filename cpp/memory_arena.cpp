global_variable memory_arena gTranArena;
global_variable memory_arena* TranArena = &gTranArena;

inline b32
VaporizeArena(memory_arena *Arena)
{
  TIMED_FUNCTION();

  b32 Result = True;
  if(Arena->Prev)
  {
    Result = VaporizeArena(Arena->Prev);
    Arena->Prev = 0;
  }

  if (Arena->Start)
  {
    Result &= DeallocateArena(Arena);
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

inline b32
RewindArena(memory_arena *Arena, umm RestartBlockSize = Megabytes(1) )
{
  TIMED_FUNCTION();

  b32 Result = True;

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
    Result &= UnprotectArena(Arena);
  }

  TIMED_BLOCK("ArenaClear");
  u8* ClearByte = Arena->Start;
  while( ClearByte < Arena->At )
  {
    *ClearByte++ = 0;
  }
  END_BLOCK("ArenaClear");

  Arena->At = Arena->Start;
  Arena->NextBlockSize = RestartBlockSize;

#if BONSAI_INTERNAL
  Arena->Pushes = 0;
#endif
#if BONSAI_DEBUG_SYSTEM_API
  DEBUG_CLEAR_META_RECORDS_FOR(Arena);
#endif

  return Result;
}

