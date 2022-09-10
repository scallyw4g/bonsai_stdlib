#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

#define RED_TERMINAL ""
#define BLUE_TERMINAL ""
#define GREEN_TERMINAL ""
#define YELLOW_TERMINAL ""
#define WHITE_TERMINAL ""

#define PLATFORM_RUNTIME_LIB_EXTENSION ".so"

inline u64
GetCycleCount()
{
  u64 Result = 0;
  return Result;
}

void PlatformDebugStacktrace();

b32 PlatformCreateDir(const char* Path, mode_t Mode = 0774);
b32 PlatformDeleteDir(const char* Path, mode_t Mode = 0774);

void* PlatformGetGlFunction(const char* Name);
const char * PlatformGetEnvironmentVar(const char *VarName);

// @compat_with_windows_barf
s32 _chdir(const char* DirName);

link_internal b32
PlatformStdoutIsRedirected()
{
  b32 Result = True;
  return Result;
}

typedef EMSCRIPTEN_WEBGL_CONTEXT_HANDLE gl_context;
struct os
{
  gl_context GlContext;
  b32 ContinueRunning = True;

  b32 Window = true; // NOTE(Jesse): Hack to play nice with other platforms
};
