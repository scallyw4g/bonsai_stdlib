#include <dlfcn.h>
#include <execinfo.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define GL_SILENCE_DEPRECATION
// MacTypes.h's Fract typedef conflicts with the math function.
#define Fract BonsaiSdkFract
#import <Cocoa/Cocoa.h>
#include <Carbon/Carbon.h>
#undef Fract

#import <OpenGL/OpenGL.h>

#define PLATFORM_RUNTIME_LIB_EXTENSION ".dylib"

typedef NSWindow        *window;
typedef NSView          *display;
typedef NSOpenGLContext *gl_context;

inline u64
GetCycleCount()
{
  u64 Result = __rdtsc();
  return Result;
}

link_internal b32
PlatformStdoutIsRedirected()
{
  b32 Result = isatty(fileno(stdout)) == 0;
  return Result;
}

void PlatformDebugStacktrace();

struct os
{
  window Window;
  display Display;
  gl_context GlContext;
  id<NSWindowDelegate> WindowDelegate;
  r32 ScrollRemainder;

  b32 ContinueRunning = True;
};

// @compat_with_windows_barf
inline s32
_chdir(const char* DirName)
{
  s32 Result = chdir(DirName);
  return Result;
}

link_internal void*
PlatformGetGlFunction(const char* Name)
{
  void *Result = dlsym(RTLD_DEFAULT, Name);

  if (!Result) { Warn("Couldn't load Opengl function (%s)", Name); }

  return Result;
}

link_internal b32
PlatformCreateDir(const char* Path, mode_t Mode = 0774)
{
  b32 Result = True;
  if (mkdir(Path, Mode) == -1)
  {
    Result = False;
  }
  return Result;
}

link_internal b32
PlatformDeleteDir(const char* Path, mode_t Mode = 0774)
{
  b32 Result = True;
  if (rmdir(Path) == -1)
  {
    Result = False;
  }
  return Result;
}

void *
OpenLibrary(const char *filename)
{
  void* Result = dlopen(filename, RTLD_NOW);

  if (!Result)
  {
    char *error = dlerror();
    Warn("OpenLibrary Failed (%s)", error);
  }
  else
  {
    Info("Library (%s) loaded!", filename);
  }

  return Result;
}

void
CloseLibrary(shared_lib Lib)
{
  s32 E = dlclose(Lib);
  if (E != 0)
  {
    Error("Closing Shared Library");
  }

  return;
}

inline void*
GetProcFromLib(shared_lib Lib, const char *Name)
{
  void* Result = dlsym(Lib, Name);
  if (Result == 0) { Warn("Unable to retrieve (%s) from shared library", Name); }
  return Result;
}

inline u32
GetCurrentThreadId()
{
  u32 Result = (u32)pthread_mach_thread_np(pthread_self());
  return Result;
}

link_internal b32
PlatformChangeDirectory(const char *Dir)
{
  b32 Result = (_chdir(Dir) == 0);
  return Result;
}

link_internal void
PlatformInitializeStdout(native_file *Stdout, native_file *Log);
