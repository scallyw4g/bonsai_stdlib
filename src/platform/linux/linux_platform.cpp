#if BONSAI_NETWORK_IMPLEMENTATION
#include <bonsai_net/network.h>
#endif

#include "linux_file.cpp"

/* #if PLATFORM_WINDOW_IMPLEMENTATIONS */

// TODO(Jesse id: 267): Unnecessary .. I just added these as a hack get parsing to work
typedef Colormap x_colormap;
typedef XSetWindowAttributes x_set_window_attribs;

link_internal b32
OpenAndInitializeWindow(os *Os, platform *Plat, s32 VSyncFrames)
{
  // @duplicate_screen_dim_init_code
  v2i StartingWindowDim = V2i(1920, 1080);
  if (Plat->ScreenDim.x > 0.f && Plat->ScreenDim.y > 0.f) { StartingWindowDim = V2i(Plat->ScreenDim); }
  else                                                    { Plat->ScreenDim = V2(StartingWindowDim);  }

  GLint GlAttribs[] = {
    GLX_RGBA,
    GLX_DEPTH_SIZE, 24,
    GLX_DOUBLEBUFFER,
    None };

  Os->Display = XOpenDisplay(0);
  if (!Os->Display) { Error("Cannot connect to X Server"); return False; }

  window RootWindow = DefaultRootWindow(Os->Display);
  if (!RootWindow) { Error("Unable to get RootWindow"); return False; }

  s32 Screen = DefaultScreen(Os->Display);

  XVisualInfo *VisualInfo = glXChooseVisual(Os->Display, Screen, GlAttribs);
  if (!VisualInfo) { Error("Unable to get Visual Info"); return False; }

  Assert(VisualInfo->screen == Screen);

  x_colormap ColorInfo = XCreateColormap(Os->Display, RootWindow, VisualInfo->visual, AllocNone);

  x_set_window_attribs WindowAttribs;
  WindowAttribs.colormap = ColorInfo;
  WindowAttribs.event_mask = WindowEventMasks;

  window xWindow = XCreateWindow( Os->Display, RootWindow,
                                  0, 0,
                                  u32(StartingWindowDim.x), u32(StartingWindowDim.y),
                                  0, VisualInfo->depth, InputOutput, VisualInfo->visual,
                                  CWColormap | CWEventMask, &WindowAttribs);

  if (!xWindow) { Error("Unable to Create Window"); return False; }

  XMapWindow(Os->Display, xWindow);
  XStoreName(Os->Display, xWindow, "Bonsai");

  const s32 FramebufferAttributes[] = {None};

  s32 ValidConfigCount = 0;
  GLXFBConfig* ValidConfigs =
    glXChooseFBConfig( Os->Display,  VisualInfo->screen, FramebufferAttributes, &ValidConfigCount);

  Assert(ValidConfigCount);
  GLXFBConfig FramebufferConfig = ValidConfigs[0];

  const s32 OpenGlContextAttribs[] = {
    GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
    GLX_CONTEXT_MINOR_VERSION_ARB, 0,
    GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_DEBUG_BIT_ARB,
    GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
    None
  };

  GLXContext ShareContext = 0;
  PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB =
    (PFNGLXCREATECONTEXTATTRIBSARBPROC)PlatformGetGlFunction("glXCreateContextAttribsARB");
  Assert(glXCreateContextAttribsARB);

  Os->GlContext = glXCreateContextAttribsARB(Os->Display, FramebufferConfig, ShareContext, GL_TRUE, OpenGlContextAttribs);
  if (!Os->GlContext) { Error("Unable to Create GLXContext"); return False; }

  Assert(Os->Display != None && Os->GlContext);

  glXMakeCurrent(Os->Display, xWindow, Os->GlContext);

  Os->Window = xWindow;

  /* InitializeOpenglFunctions(); */
  /* SetVSync(Os, VSyncFrames); */

  return True;
}

inline void
Terminate(os *Os, platform *Plat)
{
  if (Os->Display && Os->Window)
  {
    XDestroyWindow(Os->Display, Os->Window);
    XCloseDisplay(Os->Display);
  }
}

// TODO(Jesse id: 268): Unnecessary .. I just added these as a hack get parsing to work
typedef XEvent x_event;

b32
ProcessOsMessages(os *Os, platform *Plat)
{
  TIMED_FUNCTION();

  x_event Event;
  b32 EventFound = (b32)XCheckWindowEvent( Os->Display, Os->Window, WindowEventMasks, &Event);

  if (EventFound)
  {
    switch (Event.type)
    {
      case DestroyNotify:
      {
        Os->ContinueRunning = False;
      } break;

      case ConfigureNotify:
      {
        Plat->ScreenDim.x = Event.xconfigure.width;
        Plat->ScreenDim.y = Event.xconfigure.height;
      } break;

      case MotionNotify:
      {
        Plat->MouseP.x = Event.xmotion.x;
        Plat->MouseP.y = Event.xmotion.y;
      } break;

      case ButtonPress:
      {
        if (Event.xbutton.button == Button1)
        {
          Plat->Input.LMB.Clicked = True;
          Plat->Input.LMB.Pressed = True;
        }
        if (Event.xbutton.button == Button2)
        {
          Plat->Input.MMB.Clicked = True;
          Plat->Input.MMB.Pressed = True;
        }
        if (Event.xbutton.button == Button3)
        {
          Plat->Input.RMB.Clicked = True;
          Plat->Input.RMB.Pressed = True;
        }
        if (Event.xbutton.button == Button4) // Mousewheel Up
        {
          Plat->Input.MouseWheelDelta = 120;
        }
        if (Event.xbutton.button == Button5) // Mousewheel Down
        {
          Plat->Input.MouseWheelDelta = -120;
        }
      } break;

      case ButtonRelease:
      {
        if (Event.xbutton.button == Button1)
        {
          Plat->Input.LMB.Pressed = False;
        }
        if (Event.xbutton.button == Button2)
        {
          Plat->Input.MMB.Pressed = False;
        }
        if (Event.xbutton.button == Button3)
        {
          Plat->Input.RMB.Pressed = False;
        }
      } break;

      case KeyRelease:
      {
        u64 KeySym = XLookupKeysym(&Event.xkey, 0);
        switch (KeySym)
        {
          BindKeyupToInput(XK_a, A);
          BindKeyupToInput(XK_b, B);
          BindKeyupToInput(XK_c, C);
          BindKeyupToInput(XK_d, D);
          BindKeyupToInput(XK_e, E);
          BindKeyupToInput(XK_f, F);
          BindKeyupToInput(XK_g, G);
          BindKeyupToInput(XK_h, H);
          BindKeyupToInput(XK_i, I);
          BindKeyupToInput(XK_j, J);
          BindKeyupToInput(XK_k, K);
          BindKeyupToInput(XK_l, L);
          BindKeyupToInput(XK_m, M);
          BindKeyupToInput(XK_n, N);
          BindKeyupToInput(XK_o, O);
          BindKeyupToInput(XK_p, P);
          BindKeyupToInput(XK_q, Q);
          BindKeyupToInput(XK_r, R);
          BindKeyupToInput(XK_s, S);
          BindKeyupToInput(XK_t, T);
          BindKeyupToInput(XK_u, U);
          BindKeyupToInput(XK_v, V);
          BindKeyupToInput(XK_w, W);
          BindKeyupToInput(XK_x, X);
          BindKeyupToInput(XK_y, Y);
          BindKeyupToInput(XK_z, Z);

          BindKeyupToInput(XK_F1, F1);
          BindKeyupToInput(XK_F2, F2);
          BindKeyupToInput(XK_F3, F3);
          BindKeyupToInput(XK_F4, F4);
          BindKeyupToInput(XK_F5, F5);
          BindKeyupToInput(XK_F6, F6);
          BindKeyupToInput(XK_F7, F7);
          BindKeyupToInput(XK_F8, F8);
          BindKeyupToInput(XK_F9, F9);
          BindKeyupToInput(XK_F10, F10);
          BindKeyupToInput(XK_F11, F11);
          BindKeyupToInput(XK_F12, F12);

          BindKeyupToInput(XK_space, Space);
          BindKeyupToInput(XK_Return, Enter);
          BindKeyupToInput(XK_Escape, Escape);

          default:
          {
          } break;
        }
      } break;

      case KeyPress:
      {
        u64 KeySym = XLookupKeysym(&Event.xkey, 0);
        switch (KeySym)
        {
          BindKeydownToInput(XK_a, A);
          BindKeydownToInput(XK_b, B);
          BindKeydownToInput(XK_c, C);
          BindKeydownToInput(XK_d, D);
          BindKeydownToInput(XK_e, E);
          BindKeydownToInput(XK_f, F);
          BindKeydownToInput(XK_g, G);
          BindKeydownToInput(XK_h, H);
          BindKeydownToInput(XK_i, I);
          BindKeydownToInput(XK_j, J);
          BindKeydownToInput(XK_k, K);
          BindKeydownToInput(XK_l, L);
          BindKeydownToInput(XK_m, M);
          BindKeydownToInput(XK_n, N);
          BindKeydownToInput(XK_o, O);
          BindKeydownToInput(XK_p, P);
          BindKeydownToInput(XK_q, Q);
          BindKeydownToInput(XK_r, R);
          BindKeydownToInput(XK_s, S);
          BindKeydownToInput(XK_t, T);
          BindKeydownToInput(XK_u, U);
          BindKeydownToInput(XK_v, V);
          BindKeydownToInput(XK_w, W);
          BindKeydownToInput(XK_x, X);
          BindKeydownToInput(XK_y, Y);
          BindKeydownToInput(XK_z, Z);

          BindKeydownToInput(XK_F1, F1);
          BindKeydownToInput(XK_F2, F2);
          BindKeydownToInput(XK_F3, F3);
          BindKeydownToInput(XK_F4, F4);
          BindKeydownToInput(XK_F5, F5);
          BindKeydownToInput(XK_F6, F6);
          BindKeydownToInput(XK_F7, F7);
          BindKeydownToInput(XK_F8, F8);
          BindKeydownToInput(XK_F9, F9);
          BindKeydownToInput(XK_F10, F10);
          BindKeydownToInput(XK_F11, F11);
          BindKeydownToInput(XK_F12, F12);

          BindKeydownToInput(XK_space, Space);
          BindKeydownToInput(XK_Return, Enter);
          BindKeydownToInput(XK_Escape, Escape);

          default:
          {
          } break;
        }

      } break;
    }

  }

  return EventFound;
}

inline void
BonsaiSwapBuffers(os *Os)
{
  TIMED_FUNCTION();
  glXSwapBuffers(Os->Display, Os->Window);
}

link_internal void
PlatformMakeRenderContextCurrent(os *Os)
{
  glXMakeCurrent(Os->Display, Os->Window, Os->GlContext);
}


link_internal void
PlatformReleaseRenderContext(os *Os)
{
  glXMakeCurrent(Os->Display, None, NULL);
}

/* #endif */

// TODO(Jesse): This has different semantics than the Windows one .. do we care?
// (This one you can change the values of the pointed-to memory, and the env var updates)
link_internal const char *
PlatformGetEnvironmentVar(const char *VarName, memory_arena *Memory)
{
  const char* Result = getenv(VarName);
  return Result;
}

#if BONSAI_NETWORK_IMPLEMENTATION
inline void
ConnectToServer(network_connection *Connection)
{
  if (!Connection->Socket.Id)
  {
    Connection->Socket = CreateSocket(Socket_NonBlocking);
  }

  errno = 0;
  s32 ConnectStatus = connect(Connection->Socket.Id,
                              (sockaddr *)&Connection->Address,
                              sizeof(sockaddr_in));

  if (ConnectStatus == 0)
  {
      DebugLine("Connected");
      Connection->State = ConnectionState_AwaitingHandshake;
  }
  else if (ConnectStatus == -1)
  {
    switch (errno)
    {
      case 0:
      {
      } break;

      case EINPROGRESS:
      case EALREADY:
      {
        // Connection in progress
      } break;

      case ECONNREFUSED:
      {
        // Host is down
      } break;

      case EISCONN:
      {
        // Not sure if we should ever call connect on an already-connected connection
        Assert(False);
      } break;

      default :
      {
        Error("Connecting to remote host encountered an unexpected error : %d", errno);
        Assert(False);
      } break;

    }
  }
  else
  {
    InvalidCodePath();
  }

  return;
}
#endif


link_internal void
PlatformInitializeStdout(native_file *StandardOutputFile, native_file *Log)
{
  StandardOutputFile->Handle = stdout;
  StandardOutputFile->Path = CSz("stdout");

  if (Log) { *Log = OpenFile("log.txt", FilePermission_Write); }
}


#if BONSAI_DEBUG_SYSTEM_API
link_internal void
Platform_EnableContextSwitchTracing()
{
  Warn("Context switch tracing not supported on Linux!");
}
#endif
