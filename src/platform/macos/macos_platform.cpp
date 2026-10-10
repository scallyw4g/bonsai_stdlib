#include <bonsai_stdlib/src/platform/linux/linux_file.cpp>

#define BindMacKeyCode(KeyCode, InputField) case KeyCode: { Field = &Plat->Input.InputField; } break;

link_internal void
SetInputEvent(input_event *Event, b32 Down)
{
  Down = (Down != False);
  if (Down != Event->Pressed)
  {
    Event->Clicked |= Down;
    Event->Released |= !Down;
    Event->Pressed = Down;
  }

  return;
}

link_internal r32
BackingScaleFactor(os *Os)
{
  r32 Result = (r32)[Os->Window backingScaleFactor];
  return Result;
}

// ScreenDim and MouseP use backing pixels, including on Retina displays.
link_internal void
UpdateScreenDimFromBacking(os *Os, platform *Plat)
{
  NSRect Bounds  = [Os->Display bounds];
  NSRect Backing = [Os->Display convertRectToBacking:Bounds];

  if (Backing.size.width > 0 && Backing.size.height > 0)
  {
    Plat->ScreenDim = V2(r32(Backing.size.width), r32(Backing.size.height));
  }
  return;
}

link_internal void
UpdateMousePosition(os *Os, platform *Plat, NSEvent *Event)
{
  // Engine coordinates start at the top-left.
  NSPoint P     = [Os->Display convertPoint:[Event locationInWindow] fromView:nil];
  r32     Scale = BackingScaleFactor(Os);

  Plat->MouseP.x = (r32)P.x * Scale;
  Plat->MouseP.y = Plat->ScreenDim.y - (r32)P.y * Scale;
  return;
}

struct macos_gl_context_lock
{
  CGLContextObj Context;

  macos_gl_context_lock(os *Os) : Context([Os->GlContext CGLContextObj])
  {
    CGLLockContext(Context);
  }

  ~macos_gl_context_lock()
  {
    CGLUnlockContext(Context);
  }
};

#if PLATFORM_WINDOW_IMPLEMENTATIONS
@interface BonsaiWindowDelegate : NSObject <NSWindowDelegate>
@end

@implementation BonsaiWindowDelegate

- (void)windowWillClose:(NSNotification *)Notification
{
  GetStdlib()->Os.ContinueRunning = False;
}

- (void)windowDidResize:(NSNotification *)Notification
{
  bonsai_stdlib *Stdlib = GetStdlib();
  if (!Stdlib->Os.GlContext) { return; }

  macos_gl_context_lock Lock(&Stdlib->Os);
  UpdateScreenDimFromBacking(&Stdlib->Os, &Stdlib->Plat);
  [Stdlib->Os.GlContext update];
}

- (void)windowDidChangeBackingProperties:(NSNotification *)Notification
{
  bonsai_stdlib *Stdlib = GetStdlib();
  if (!Stdlib->Os.GlContext) { return; }

  macos_gl_context_lock Lock(&Stdlib->Os);
  UpdateScreenDimFromBacking(&Stdlib->Os, &Stdlib->Plat);
  [Stdlib->Os.GlContext update];
}

- (void)windowDidResignKey:(NSNotification *)Notification
{
  bonsai_stdlib *Stdlib = GetStdlib();
  input *Input = &Stdlib->Plat.Input;
  b32 LeftPressed = Input->LMB.Pressed;
  b32 RightPressed = Input->RMB.Pressed;
  b32 MiddlePressed = Input->MMB.Pressed;
  *Input = {};
  Input->LMB.Released = LeftPressed;
  Input->RMB.Released = RightPressed;
  Input->MMB.Released = MiddlePressed;
  Stdlib->Os.ScrollRemainder = 0;
}

- (void)windowDidBecomeKey:(NSNotification *)Notification
{
  input *Input = &GetStdlib()->Plat.Input;
  NSEventModifierFlags Flags = [NSEvent modifierFlags];
  SetInputEvent(&Input->Shift, Flags & NSEventModifierFlagShift);
  SetInputEvent(&Input->Ctrl, Flags & NSEventModifierFlagControl);
  SetInputEvent(&Input->Alt, Flags & NSEventModifierFlagOption);
}
@end
#endif

link_internal b32
OpenAndInitializeWindow(os *Os, platform *Plat, s32 VSyncFrames)
{
  Assert([NSThread isMainThread]);
  NSAutoreleasePool *Pool = [[NSAutoreleasePool alloc] init];
  // @duplicate_screen_dim_init_code
  v2i StartingWindowDim = V2i(1920, 1080);
  if (Plat->ScreenDim.x > 0.f && Plat->ScreenDim.y > 0.f) { StartingWindowDim = V2i(Plat->ScreenDim); }

  [NSApplication sharedApplication];
  [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
  [NSApp finishLaunching];

  NSOpenGLPixelFormatAttribute Attribs[] = {
    NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion4_1Core,
    NSOpenGLPFAColorSize,     24,
    NSOpenGLPFAAlphaSize,     8,
    NSOpenGLPFADepthSize,     24,
    NSOpenGLPFADoubleBuffer,
    NSOpenGLPFAAccelerated,
    0
  };

  NSOpenGLPixelFormat *PixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:Attribs];
  if (!PixelFormat)
  {
    [Pool drain];
    Error("Unable to create an NSOpenGLProfileVersion4_1Core pixel format");
    return False;
  }

  NSRect Frame = NSMakeRect(0, 0, StartingWindowDim.x, StartingWindowDim.y);
  NSWindowStyleMask Style = NSWindowStyleMaskTitled
                          | NSWindowStyleMaskClosable
                          | NSWindowStyleMaskMiniaturizable
                          | NSWindowStyleMaskResizable;

  NSWindow *Window = [[NSWindow alloc] initWithContentRect:Frame
                                                 styleMask:Style
                                                   backing:NSBackingStoreBuffered
                                                     defer:NO];
  if (!Window)
  {
    [PixelFormat release];
    [Pool drain];
    Error("Unable to create an NSWindow");
    return False;
  }

  // The delegate class belongs to the executable, not hot-reloaded game libraries.
  Class DelegateClass = NSClassFromString(@"BonsaiWindowDelegate");
  Assert(DelegateClass);
  id<NSWindowDelegate> Delegate = [[DelegateClass alloc] init];
  [Window setDelegate:Delegate];
  [Window setReleasedWhenClosed:NO];
  [Window setTitle:@"Bonsai"];
  [Window setAcceptsMouseMovedEvents:YES];

  NSView *View = [Window contentView];
  [View setWantsBestResolutionOpenGLSurface:YES];

  NSOpenGLContext *GlContext = [[NSOpenGLContext alloc] initWithFormat:PixelFormat shareContext:nil];
  [PixelFormat release];
  if (!GlContext)
  {
    [Window setDelegate:nil];
    [Delegate release];
    [Window release];
    [Pool drain];
    Error("Unable to create an NSOpenGLContext");
    return False;
  }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
  [GlContext setView:View];
#pragma clang diagnostic pop
  [GlContext makeCurrentContext];

  GLint SwapInterval = (VSyncFrames > 0) ? 1 : 0;
  [GlContext setValues:&SwapInterval forParameter:NSOpenGLContextParameterSwapInterval];

  Os->Window    = Window;
  Os->Display   = View;
  Os->GlContext = GlContext;
  Os->WindowDelegate = Delegate;
  Os->ContinueRunning = True;

  [Window makeKeyAndOrderFront:nil];
  if (@available(macOS 14.0, *))
  {
    [NSApp activate];
  }
  else
  {
    [NSApp activateIgnoringOtherApps:YES];
  }

  UpdateScreenDimFromBacking(Os, Plat);

  [Pool drain];
  return True;
}

inline void
Terminate(os *Os, platform *Plat)
{
  @autoreleasepool
  {
    [Os->Window setDelegate:nil];
    [NSOpenGLContext clearCurrentContext];
    [Os->GlContext clearDrawable];
    [Os->GlContext release];
    Os->GlContext = nil;
    [Os->Window close];
    [Os->Window release];
    Os->Window = nil;
    Os->Display = nil;
    [Os->WindowDelegate release];
    Os->WindowDelegate = nil;
  }
}

b32
ProcessOsMessages(os *Os, platform *Plat)
{
  TIMED_FUNCTION();
  NSAutoreleasePool *Pool = [[NSAutoreleasePool alloc] init];

  b32 EventFound = False;

  for (;;)
  {
    NSEvent *Event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                        untilDate:[NSDate distantPast]
                                           inMode:NSDefaultRunLoopMode
                                          dequeue:YES];
    if (!Event) { break; }

    EventFound = True;
    if ([Event window] != Os->Window)
    {
      [NSApp sendEvent:Event];
      continue;
    }
    NSEventMask MouseDownEvents = NSEventMaskLeftMouseDown | NSEventMaskRightMouseDown | NSEventMaskOtherMouseDown;
    if (NSEventMaskFromType([Event type]) & MouseDownEvents)
    {
      NSPoint P = [Os->Display convertPoint:[Event locationInWindow] fromView:nil];
      if (!NSPointInRect(P, [Os->Display bounds]))
      {
        [NSApp sendEvent:Event];
        continue;
      }
    }

    NSEventMask MouseEvents = NSEventMaskLeftMouseDown | NSEventMaskLeftMouseUp
                           | NSEventMaskRightMouseDown | NSEventMaskRightMouseUp
                           | NSEventMaskOtherMouseDown | NSEventMaskOtherMouseUp;
    if (NSEventMaskFromType([Event type]) & MouseEvents) { UpdateMousePosition(Os, Plat, Event); }

    switch ([Event type])
    {
      case NSEventTypeLeftMouseDown:  { SetInputEvent(&Plat->Input.LMB, True);  } break;
      case NSEventTypeLeftMouseUp:    { SetInputEvent(&Plat->Input.LMB, False); } break;
      case NSEventTypeRightMouseDown: { SetInputEvent(&Plat->Input.RMB, True);  } break;
      case NSEventTypeRightMouseUp:   { SetInputEvent(&Plat->Input.RMB, False); } break;

      case NSEventTypeOtherMouseDown:
      {
        if ([Event buttonNumber] == 2) { SetInputEvent(&Plat->Input.MMB, True); }
      } break;

      case NSEventTypeOtherMouseUp:
      {
        if ([Event buttonNumber] == 2) { SetInputEvent(&Plat->Input.MMB, False); }
      } break;

      case NSEventTypeMouseMoved:
      case NSEventTypeLeftMouseDragged:
      case NSEventTypeRightMouseDragged:
      case NSEventTypeOtherMouseDragged:
      {
        UpdateMousePosition(Os, Plat, Event);
      } break;

      case NSEventTypeScrollWheel:
      {
        r32 Delta = (r32)[Event scrollingDeltaY];
        if (![Event hasPreciseScrollingDeltas]) { Delta *= 120.f; }
        Os->ScrollRemainder += Delta;
        s32 WholeDelta = s32(Os->ScrollRemainder);
        Plat->Input.MouseWheelDelta += WholeDelta;
        Os->ScrollRemainder -= (r32)WholeDelta;
      } break;

      case NSEventTypeFlagsChanged:
      {
        NSEventModifierFlags Flags = [Event modifierFlags];

        SetInputEvent(&Plat->Input.Shift, Flags & NSEventModifierFlagShift);
        SetInputEvent(&Plat->Input.Ctrl,  Flags & NSEventModifierFlagControl);
        SetInputEvent(&Plat->Input.Alt,   Flags & NSEventModifierFlagOption);
      } break;

      case NSEventTypeKeyDown:
      case NSEventTypeKeyUp:
      {
        // Physical key codes are independent of the current keyboard layout.
        input_event *Field = 0;

        switch ([Event keyCode])
        {
          BindMacKeyCode(kVK_Return, Enter);
          BindMacKeyCode(kVK_Escape, Escape);

          BindMacKeyCode(kVK_Delete,        Backspace);
          BindMacKeyCode(kVK_ForwardDelete, Delete);

          BindMacKeyCode(kVK_F1,  F1);
          BindMacKeyCode(kVK_F2,  F2);
          BindMacKeyCode(kVK_F3,  F3);
          BindMacKeyCode(kVK_F4,  F4);
          BindMacKeyCode(kVK_F5,  F5);
          BindMacKeyCode(kVK_F6,  F6);
          BindMacKeyCode(kVK_F7,  F7);
          BindMacKeyCode(kVK_F8,  F8);
          BindMacKeyCode(kVK_F9,  F9);
          BindMacKeyCode(kVK_F10, F10);
          BindMacKeyCode(kVK_F11, F11);
          BindMacKeyCode(kVK_F12, F12);

          BindMacKeyCode(kVK_ANSI_Period, Dot);
          BindMacKeyCode(kVK_ANSI_Minus,  Minus);
          BindMacKeyCode(kVK_ANSI_Slash,  FSlash);
          BindMacKeyCode(kVK_Space,       Space);

          BindMacKeyCode(kVK_ANSI_0, N0);
          BindMacKeyCode(kVK_ANSI_1, N1);
          BindMacKeyCode(kVK_ANSI_2, N2);
          BindMacKeyCode(kVK_ANSI_3, N3);
          BindMacKeyCode(kVK_ANSI_4, N4);
          BindMacKeyCode(kVK_ANSI_5, N5);
          BindMacKeyCode(kVK_ANSI_6, N6);
          BindMacKeyCode(kVK_ANSI_7, N7);
          BindMacKeyCode(kVK_ANSI_8, N8);
          BindMacKeyCode(kVK_ANSI_9, N9);

          BindMacKeyCode(kVK_ANSI_A, A);
          BindMacKeyCode(kVK_ANSI_B, B);
          BindMacKeyCode(kVK_ANSI_C, C);
          BindMacKeyCode(kVK_ANSI_D, D);
          BindMacKeyCode(kVK_ANSI_E, E);
          BindMacKeyCode(kVK_ANSI_F, F);
          BindMacKeyCode(kVK_ANSI_G, G);
          BindMacKeyCode(kVK_ANSI_H, H);
          BindMacKeyCode(kVK_ANSI_I, I);
          BindMacKeyCode(kVK_ANSI_J, J);
          BindMacKeyCode(kVK_ANSI_K, K);
          BindMacKeyCode(kVK_ANSI_L, L);
          BindMacKeyCode(kVK_ANSI_M, M);
          BindMacKeyCode(kVK_ANSI_N, N);
          BindMacKeyCode(kVK_ANSI_O, O);
          BindMacKeyCode(kVK_ANSI_P, P);
          BindMacKeyCode(kVK_ANSI_Q, Q);
          BindMacKeyCode(kVK_ANSI_R, R);
          BindMacKeyCode(kVK_ANSI_S, S);
          BindMacKeyCode(kVK_ANSI_T, T);
          BindMacKeyCode(kVK_ANSI_U, U);
          BindMacKeyCode(kVK_ANSI_V, V);
          BindMacKeyCode(kVK_ANSI_W, W);
          BindMacKeyCode(kVK_ANSI_X, X);
          BindMacKeyCode(kVK_ANSI_Y, Y);
          BindMacKeyCode(kVK_ANSI_Z, Z);

          default:
          {
          } break;
        }

        if (Field)
        {
          SetInputEvent(Field, [Event type] == NSEventTypeKeyDown);
          if ([Event type] == NSEventTypeKeyDown && [Event isARepeat]) { Field->Clicked = True; }
          continue;
        }
      } break;

      default:
      {
      } break;
    }

    [NSApp sendEvent:Event];
  }

  [Pool drain];
  return EventFound;
}

#undef BindMacKeyCode

inline void
BonsaiSwapBuffers(os *Os)
{
  TIMED_FUNCTION();

  @autoreleasepool
  {
    [Os->GlContext flushBuffer];
  }
}

link_internal void
PlatformMakeRenderContextCurrent(os *Os)
{
  @autoreleasepool
  {
    [Os->GlContext makeCurrentContext];
  }
}

link_internal void
PlatformReleaseRenderContext(os *Os)
{
  @autoreleasepool
  {
    [NSOpenGLContext clearCurrentContext];
  }
}

link_internal const char *
PlatformGetEnvironmentVar(const char *VarName, memory_arena *Memory)
{
  const char* Result = getenv(VarName);
  return Result;
}

void
PlatformDebugStacktrace()
{
  void *StackSymbols[32];
  s32 SymbolCount = backtrace(StackSymbols, 32);
  backtrace_symbols_fd(StackSymbols, SymbolCount, STDERR_FILENO);
  return;
}

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
  Warn("Context switch tracing not supported on macOS!");
}
#endif
