
#if BONSAI_LINUX || BONSAI_EMCC
#include <bonsai_stdlib/src/platform/posix_platform.cpp>
#endif

#if BONSAI_WIN32
#include <bonsai_stdlib/src/platform/win32/win32_platform.cpp>
#elif BONSAI_LINUX
#include <bonsai_stdlib/src/platform/linux/linux_platform.cpp>
#elif BONSAI_EMCC
#include <bonsai_stdlib/src/platform/wasm_platform.cpp>
#include <bonsai_stdlib/src/platform/linux/linux_file.cpp>
#else
#error "Unsupported Platform"
#endif

link_internal void
MakeRenderContextCurrent(os *Os, gl_context Context)
{
  AcquireFutex(&Os->MakeRenderContextCurrentFutex);
  if (PlatformMakeRenderContextCurrent(Os, Context) && Context == Os->GlContext)
  {
    Assert(Os->RenderThread == INVALID_THREAD_LOCAL_THREAD_INDEX ||
           Os->RenderThread == ThreadLocal_ThreadIndex);
    Os->RenderThread = ThreadLocal_ThreadIndex;
  }
  ReleaseFutex(&Os->MakeRenderContextCurrentFutex);
}

link_internal void
ReleaseRenderContext(os *Os)
{
  AcquireFutex(&Os->MakeRenderContextCurrentFutex);
  if (PlatformReleaseRenderContext(Os) && Os->RenderThread == ThreadLocal_ThreadIndex)
  {
    Os->RenderThread = INVALID_THREAD_LOCAL_THREAD_INDEX;
  }
  ReleaseFutex(&Os->MakeRenderContextCurrentFutex);
}
