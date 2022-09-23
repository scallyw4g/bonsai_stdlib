global_variable memory_arena gTranArena;
global_variable memory_arena* TranArena = &gTranArena;

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

  u8* ClearByte = Arena->Start;
  while( ClearByte < Arena->At )
  {
    *ClearByte++ = 0;
  }

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
