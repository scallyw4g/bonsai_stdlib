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

#define PLATFORM_RUNTIME_BREAK() raise(SIGTRAP)

#define Newline "\n"

#define THREAD_MAIN_RETURN void*


typedef s32 thread_id;
typedef sem_t semaphore;
typedef pthread_mutex_t native_mutex;

typedef void* shared_lib;

inline void
WakeThread( semaphore *Semaphore )
{
  sem_post(Semaphore);
  return;
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
AtomicExchange( volatile u32 *Source, const u32 Exchange )
{
  u32 Result = __sync_lock_test_and_set( Source, Exchange );
  return Result;
}

inline b32
AtomicCompareExchange( volatile char **Source, volatile char *Exchange, volatile char *Comparator )
{
  bool Result = __sync_bool_compare_and_swap ( Source, Comparator, Exchange );
  return Result;
}

inline bool
AtomicCompareExchange( volatile void** Source, void* Exchange, void* Comparator )
{
  bool Result = __sync_bool_compare_and_swap ( Source, Comparator, Exchange );
  return Result;
}

inline bool
AtomicCompareExchange( volatile u64 *Source, u64 Exchange, u64 Comparator )
{
  bool Result = __sync_bool_compare_and_swap ( Source, Comparator, Exchange );
  return Result;
}

inline bool
AtomicCompareExchange( volatile u32 *Source, u32 Exchange, u32 Comparator )
{
  bool Result = __sync_bool_compare_and_swap ( Source, Comparator, Exchange );
  return Result;
}

struct native_file
{
  FILE* Handle;
  counted_string Path;

  native_file() = default;
  native_file(native_file &) = default;

  ~native_file()
  {
    if (Handle)
    {
      Leak("file handle %S(%p)", Path, Handle);
    }
  }
};
