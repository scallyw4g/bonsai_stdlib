// TODO(Jesse): Pretty sure this and _chdir were for compatibility with windows
// .. which is pretty barf
// @compat_with_windows_barf
inline b32
fopen_s(FILE **HandleOut, const char *FilePath, const char *Permissions)
{
  *HandleOut = fopen(FilePath, Permissions);
  b32 Result = *HandleOut != 0;
  return Result;
}

inline void
PrintSemValue(semaphore *Semaphore)
{
  s32 SemValue;
  if (sem_getvalue(Semaphore, &SemValue) == 0)
  {
    NotImplemented;
    /* DebugPrint(SemValue); */
  }
  else
  {
    Warn("sem_getvalue() failed.");
  }
  return;
}

inline b32 
PlatformInitializeMutex(mutex *Mutex)
{
  b32 Result = (pthread_mutex_init(&Mutex->M, NULL) == 0);
  return Result;
}

void
PlatformUnlockMutex(mutex *Mutex)
{
  s32 Fail = pthread_mutex_unlock(&Mutex->M);
  TIMED_MUTEX_RELEASED(Mutex);

  if (Fail)
  {
    Error("Failed to un-lock mutex");
    Assert(False);
  }
  return;
}

void
PlatformLockMutex(mutex *Mutex)
{
  TIMED_MUTEX_WAITING(Mutex);

  s32 Fail = pthread_mutex_lock(&Mutex->M);

  TIMED_MUTEX_AQUIRED(Mutex);

  if (Fail)
  {
    Error("Failed to aquire lock");
    Assert(False);
  }


  return;
}

link_internal umm
PlatformGetPageSize()
{
  umm InvalidSysconfReturn = ((umm)-1);
  local_persist umm PageSize = (umm)sysconf(_SC_PAGESIZE);
  Assert(PageSize != InvalidSysconfReturn);
#if BONSAI_EMCC
  Assert(PageSize == 65536);
#else
  Assert(PageSize == 4096);
#endif

  return PageSize;
}

u32
PlatformGetLogicalCoreCount()
{
  local_persist u32 CoreCount = (u32)sysconf(_SC_NPROCESSORS_ONLN);

  return CoreCount;
}

link_internal b32
PlatformSetProtection(u8 *Base, umm Size, memory_protection_type Protection)
{
  b32 Result = False;

  u64 PageSize = PlatformGetPageSize();
  if ( (umm)Base % PageSize == 0 &&
            Size % PageSize == 0 )
  {
    s32 NativeProt = 0;
    switch (Protection)
    {
      case MemoryProtection_RW:
      {
        NativeProt = PROT_READ | PROT_WRITE;
      } break;

      case MemoryProtection_Protected:
      {
        NativeProt = PROT_NONE;
      } break;
    }

    errno = 0;
    if (mprotect(Base, Size, NativeProt) == 0)
    {
      Result = True;
    }
    else
    {
      Error("mprotect failed with code : (%d) ", errno);
      switch (errno)
      {
        case EACCES:
        {
          Error("EACCES");
        } break;
        case EINVAL:
        {
          Error("EINVAL");
        } break;

        case ENOMEM:
        {
          Error("ENOMEM");
        } break;

        default:
        {
          Error("Unknown error code");
        } break;
      }

      PlatformDebugStacktrace();
      InvalidCodePath();
    }
  }
  else
  {
    InvalidCodePath();
  }


  Assert(errno == 0);
  return Result;
}

link_internal b32
PlatformDeallocate(u8 *Base, umm Size)
{
  Assert( (umm)Base % PlatformGetPageSize() == 0);
  Assert( Size % PlatformGetPageSize() == 0);

  b32 Deallocated = (munmap(Base, Size) == 0);

  return Deallocated;
}

link_internal u8*
PlatformAllocateSize(umm AllocationSize)
{
  Assert(AllocationSize % PlatformGetPageSize() == 0);

  s32 Protection = PROT_READ|PROT_WRITE;
  s32 Flags = MAP_SHARED|MAP_ANONYMOUS|MAP_NORESERVE;

  errno = 0;
  u8 *Bytes = (u8*)mmap(0, AllocationSize, Protection, Flags,  -1, 0);
  if (Bytes == MAP_FAILED)
  {
    Bytes = 0;
    s32 E = errno;
    if (E == ENOMEM)
    {
      Error("Out of memory, or exhausted virtual page table for allocation sized (%lu)", AllocationSize);
      Assert(False);
    }
    else
    {
      Error("Unknown error allocating pages: %d", E);
      Assert(False);
    }
  }


  return Bytes;
}

link_internal void
SleepMs(u32 Ms)
{
  TIMED_FUNCTION();

  s32 Ns = s32(Ms)*100000;
  timespec TReq = { .tv_sec = 0, .tv_nsec = Ns };
  timespec TRem = {};

  nanosleep(&TReq, &TRem);
}

inline void
ThreadSleep( semaphore *Semaphore )
{
  TIMED_FUNCTION();
  sem_wait(Semaphore);
  return;
}

semaphore
CreateSemaphore(void)
{
  semaphore Result;
  sem_init(&Result, 0, 0);
  return Result;
}

/* typedef pthread_t thread_handle; // TODO(Jesse id: 265): Unnecessary .. I just added it as a hack get parsing to work */
typedef pthread_attr_t thread_attributes; // TODO(Jesse id: 266): Unnecessary .. I just added it as a hack get parsing to work

u32
PlatformCreateThread( thread_main_callback_type ThreadMain, thread_startup_params *Params, s32 ThreadId)
{
  thread_attributes Attribs;
  pthread_attr_init(&Attribs);

  pthread_t Thread;
  b32 Success = (pthread_create(&Thread, &Attribs, ThreadMain, Params) == 0);

  u32 Result = u32(INVALID_THREAD_HANDLE);
  if (Success)
  {
    Result = u32(Thread);
  }

  return Result;
}


global_variable const u32 Global_CwdBufferLength = 4096;
global_variable char Global_CwdBuffer[Global_CwdBufferLength];

char*
GetCwd()
{
  ClearBuffer((u8*)Global_CwdBuffer, Global_CwdBufferLength);
  char *Result = getcwd(Global_CwdBuffer, Global_CwdBufferLength);
  return (Result);
}

b32
IsFilesystemRoot(char *Filepath)
{
  b32 Result = ( Filepath[0] == '/' && Filepath[1] == 0 );
  return Result;
}

// TODO(Jesse id: 270): Unnecessary .. I just added these as a hack get parsing to work
typedef sched_param bonsai_sched_param;

inline void
PlatformSetThreadPriority(s32 Priority)
{
  bonsai_sched_param Param = {};
  Param.sched_priority = Priority;

  errno = 0;
  s32 E = sched_setscheduler(0, SCHED_FIFO, &Param);
  if (E)
  {
    Error("Setting Scheduler for main thread");

    switch (errno)
    {
      case EINVAL:
      {
        Error(" Invalid arguments: pid is negative or param is NULL.");
        Error(" (sched_setscheduler()) policy is not one of the recognized policies.");
        Error(" (sched_setscheduler()) param does not make sense for the specified policy.");
      }  break;

      case EPERM:  { Error("  The calling thread does not have appropriate privileges."); break; }
      case ESRCH:  { Error("  The thread whose ID is pid could not be found."); break; }
      InvalidDefaultCase;
    }
  }

  return;
}

// It seemed to me doing this actually made performance _worse_
#if 0

// TODO(Jesse id: 271): Unnecessary .. I just added these as a hack get parsing to work
typedef cpu_set_t bonsai_cpu_set_t;

inline void
PlatformSetMainThreadPriority()
{
  bonsai_cpu_set_t Cpu;
  CPU_ZERO(&Cpu);
  CPU_SET(0, &Cpu);

/*   int SetSuccessful = pthread_setaffinity_np(pthread_self(), sizeof(Cpu), &Cpu); */
/*   if (SetSuccessful == -1) */
/*   { */
/*     Error("Setting CPU affinity"); */
/*   } */

  /* PlatformSetThreadPriority(99); */
  return;
}

inline void
PlatformSetWorkerThreadPriority()
{
  bonsai_cpu_set_t Cpu;
  CPU_ZERO(&Cpu);
  CPU_SET(1, &Cpu);

  /* int SetSuccessful = pthread_setaffinity_np(pthread_self(), sizeof(Cpu), &Cpu); */
  /* if (SetSuccessful == -1) */
  /* { */
  /*   Error("Setting CPU affinity"); */
  /* } */

  /* PlatformSetThreadPriority(90); */
  return;
}
#endif

