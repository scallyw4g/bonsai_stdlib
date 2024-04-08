#define INITGUID // Force SystemTraceControlGuid to be defined
#include <Windows.h>
#include <windowsx.h> // Macros to retrieve mouse coordinates
#include <WinBase.h>
#include <wingdi.h>
/* #include <Winuser.h> */

#include <bonsai_stdlib/src/platform/wgl.h>

#include <chrono> // Timer

#define PLATFORM_RUNTIME_BREAK() __debugbreak()

#define runtime_lib_export __declspec(dllexport)

#define Newline "\n"

#define PLATFORM_OFFSET (sizeof(void*))

#define PLATFORM_RUNTIME_LIB_EXTENSION ".dll"

#define THREAD_MAIN_RETURN DWORD WINAPI
#define GAME_MAIN_PROC FARPROC GameMain

#define SWAP_BUFFERS SwapBuffers(hDC)

#define bonsaiGlGetProcAddress(procName) wglGetProcAddress(procName)
typedef PFNWGLSWAPINTERVALEXTPROC PFNSWAPINTERVALPROC;

typedef HANDLE thread_handle;
typedef HANDLE semaphore;
typedef HANDLE native_mutex;


// ???
typedef HMODULE shared_lib;
typedef HWND window;
typedef HGLRC gl_context;
typedef HDC display;

struct native_file
{
  HANDLE Handle;
  counted_string Path;
};

struct os
{
  HWND Window;
  HDC Display;
  HGLRC GlContext;

  b32 ContinueRunning = True;
};

link_internal void
Win32PrintLastError();

link_internal void
PlatformInitializeStdout(native_file *Stdout)
{
  Stdout->Handle = CreateFileA("CONOUT$", GENERIC_WRITE, FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
  Stdout->Path = CSz("stdout");

  Assert(Stdout->Handle != INVALID_HANDLE_VALUE);
}


link_internal u64
GetCycleCount()
{
  u64 Result = __rdtsc();
  return Result;
}

inline b32
AtomicCompareExchange( volatile u32 *Source, u32 Exchange, u32 Comparator )
{
  u32 Val = (u32)InterlockedCompareExchange( (LONG volatile *)Source, Exchange, Comparator);
  b32 Result = (Val == Comparator);
  return Result;
}

inline b32
AtomicCompareExchange( volatile void **Source, void *Exchange, void *Comparator )
{
  u64 Val = InterlockedCompareExchange( (u64 volatile *)Source, (u64)Exchange, (u64)Comparator);
  b32 Result = (Val == (u64)Comparator);
  return Result;
}

inline u32
AtomicIncrement( u32 volatile *Dest)
{
  u32 Result = InterlockedIncrement(Dest);
  return Result;
}

inline u64
AtomicIncrement( u64 volatile *Dest)
{
  u64 Result = InterlockedIncrement(Dest);
  return Result;
}

inline u32
AtomicWrite( volatile u32 *Source, u32 Value)
{
  u32 Result = InterlockedExchange( Source, Value );
  return Result;
}

inline u64
AtomicWrite( volatile u64 *Source, u64 Value)
{
  u64 Result = InterlockedExchange( Source, Value );
  return Result;
}

inline void*
AtomicWrite( volatile void **Source, void *Value)
{
  void *Result = (void*)InterlockedExchange( (u64*)Source, (u64)Value );
  return Result;
}


inline u64
AtomicExchange( volatile u64 *Source, const u64 NewValue )
{
  u64 Result = InterlockedExchange( Source, NewValue );
  return Result;
}

inline void*
AtomicExchange( volatile void **Source, const void *NewValue )
{
  void *Result = (void*)InterlockedExchange( (u64*)Source, (u64)NewValue );
  return Result;
}

inline u32
AtomicExchange( volatile u32 *Source, const u32 NewValue )
{
  u32 Result = InterlockedExchange( Source, NewValue );
  return Result;
}

link_internal b32
PlatformStdoutIsRedirected()
{
  b32 Result = False; //True;
  return Result;
}

b32 PlatformCreateDir(const char* Path)
{
  b32 Result = CreateDirectoryA(Path, 0) != 0;
  return Result;
}

b32 PlatformDeleteDir(const char* Path)
{
  b32 Result = RemoveDirectoryA(Path) != 0;
  return 0;
}

inline u32
AtomicDecrement( u32 volatile *Dest)
{
  u32 Result = InterlockedDecrement(Dest);
  return Result;
}

inline u64
AtomicDecrement( u64 volatile *Dest)
{
  u64 Result = InterlockedDecrement(Dest);
  return Result;
}

link_internal void*
PlatformGetGlFunction(const char* Name)
{
  void *Result = 0;

  void *WglProc = (void*)wglGetProcAddress(Name);
  if ((s64)WglProc == -1 ||
      (s64)WglProc ==  0 ||
      (s64)WglProc ==  1 ||
      (s64)WglProc ==  2 ||
      (s64)WglProc ==  3 )
  {
    HMODULE OpenglDllHandle = LoadLibraryA("opengl32.dll");
    if (OpenglDllHandle)
    {
      Result = (void*)GetProcAddress(OpenglDllHandle, Name);
    }
    else
    {
      Error("Could not load opengl32.dll");
    }
  }
  else
  {
    Result = WglProc;
  }

  if (!Result)
  {
    Error("Couldn't load Opengl fucntion (%s)", Name);
  }

  return Result;
}

// TODO(Jesse)(omfg gross): OMFG GROSS
inline r64
GetHighPrecisionClock()
{
  global_variable auto FirstTime = std::chrono::high_resolution_clock::now();
  // cout << "FirstTime Time : " << chrono::time_point_cast<chrono::nanoseconds>(FirstTime).time_since_epoch().count() << " ns \n";

  r64 ResultMs = (r64)(std::chrono::high_resolution_clock::now() - FirstTime).count()/1000000.0;;
  // cout << "Time/iter, clock: " << chrono::duration_cast<chrono::nanoseconds>(Result).count() << " ns \n";

  return ResultMs;
}

inline void*
GetProcFromLib(shared_lib Lib, const char *Name)
{
  void* Result = (void*)GetProcAddress(Lib, Name);
  return Result;
}

inline shared_lib
OpenLibrary(const char *LibPath)
{
  shared_lib Result = LoadLibrary(LibPath);

  if (!Result)
  {
    Warn("Could not open library (%s)", LibPath);
  }
  else
  {
    Info("Library (%s) Loaded Successfully.", LibPath);
  }

  return Result;
}

inline b32
CloseLibrary(shared_lib Lib)
{
  b32 Result = (b32)FreeLibrary(Lib);
  return Result;
}

link_internal b32
PlatformChangeDirectory(const char *Dir)
{
  b32 Result = (SetCurrentDirectory(Dir) != 0);
  return Result;
}
