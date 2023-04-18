#include <pthread.h>

// Assert
#include <signal.h>

// chdir
#include <unistd.h>

// dlopen
#include <dlfcn.h>

#include <semaphore.h>

#include <sys/mman.h>   // mmap

#include <arpa/inet.h>  // inet_addr
#include <sys/socket.h>
#include <cstdarg>      // va_arg

#include <time.h> // nanosleep

#define PLATFORM_RUNTIME_BREAK() raise(SIGTRAP)

#define Newline "\n"

#define THREAD_MAIN_RETURN void*

// Windows defines this for us
#define ZeroMemory(Start, Count) memset((Start), 0, (Count));

/* typedef s32 thread_handle; */
typedef sem_t semaphore;
typedef pthread_mutex_t native_mutex;

typedef void* shared_lib;

link_internal void SleepMs(u32 Ms);

inline void
WakeThread( semaphore *Semaphore )
{
  sem_post(Semaphore);
}

// @compat_with_windows_barf
inline b32 fopen_s(FILE **HandleOut, const char *FilePath, const char *Permissions);

#if 0
void
PlatformDebugStacktrace()
{
  Crash();
  /* void *StackSymbols[32]; */
  /* s32 SymbolCount = backtrace(StackSymbols, 32); */
  /* backtrace_symbols_fd(StackSymbols, SymbolCount, STDERR_FILENO); */
  return;
}
#endif

inline void
AtomicDecrement( volatile u32 *Source )
{
  __sync_sub_and_fetch( Source, 1 );
  return;
}

inline u64
AtomicIncrement( volatile u64 *Source )
{
  u64 Result = __sync_fetch_and_add( Source, 1 );
  return Result;
}

inline u32
AtomicIncrement( volatile u32 *Source )
{
  u32 Result = __sync_fetch_and_add( Source, 1 );
  return Result;
}

inline void
AtomicDecrement( volatile s32 *Source )
{
  __sync_sub_and_fetch( Source, 1 );
  return;
}

inline void
AtomicIncrement( volatile s32 *Source )
{
  __sync_add_and_fetch( Source, 1 );
  return;
}

inline u32
AtomicExchange( volatile u32 *Source, const u32 NewValue )
{
  u32 Result = __sync_lock_test_and_set( Source, NewValue );
  return Result;
}

inline b32
AtomicCompareExchange( volatile char **Source, volatile char *NewValue, volatile char *Comparator )
{
  bool Result = __sync_bool_compare_and_swap ( Source, Comparator, NewValue );
  return Result;
}

inline bool
AtomicCompareExchange( volatile void** Source, void* NewValue, void* Comparator )
{
  bool Result = __sync_bool_compare_and_swap ( Source, Comparator, NewValue );
  return Result;
}

inline bool
AtomicCompareExchange( volatile u64 *Source, u64 NewValue, u64 Comparator )
{
  bool Result = __sync_bool_compare_and_swap ( Source, Comparator, NewValue );
  return Result;
}

inline bool
AtomicCompareExchange( volatile u32 *Source, u32 NewValue, u32 Comparator )
{
  bool Result = __sync_bool_compare_and_swap ( Source, Comparator, NewValue );
  return Result;
}

struct native_file
{
  FILE* Handle;
  counted_string Path;

#if 0
  // NOTE(Jesse): Put this in to randomly check if we were leaking file handes.
  // Turns out we're not, but it would be nice to put back in some day.. maybe.
  // It started causing me headaches, so I 0'd it out
  //
  native_file() = default;
  native_file(native_file &) = default;

  ~native_file()
  {
    if (Handle)
    {
      Leak("file handle %S(%p)", Path, Handle);
    }
  }
#endif
};

inline r64
GetHighPrecisionClock()
{
  timespec Time;
  clock_gettime(CLOCK_MONOTONIC, &Time);

  r64 SecondsAsMs = (r64)Time.tv_sec*1000.0;
  r64 NsAsMs = Time.tv_nsec/1000000.0;

  r64 Ms = SecondsAsMs + NsAsMs;
  return Ms;
}
