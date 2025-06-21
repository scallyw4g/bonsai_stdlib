
struct input_event
{
  b32 Clicked;
  b32 Pressed;

  // TODO(Jesse): Add this?
  // b32 Released;
};

struct input
{
  input_event Enter;     poof(@win32_keycode(VK_RETURN))
  input_event Escape;    poof(@win32_keycode(VK_ESCAPE))

  input_event Alt;       poof(@win32_keycode(VK_MENU))
  input_event Ctrl;      poof(@win32_keycode(VK_CONTROL))
  input_event Shift;     poof(@win32_keycode(VK_SHIFT))

  input_event Backspace; poof(@win32_keycode(VK_BACK))
  input_event Delete;    poof(@win32_keycode(VK_DELETE))

  input_event F12; poof(@win32_keycode(VK_F12))
  input_event F11; poof(@win32_keycode(VK_F11))
  input_event F10; poof(@win32_keycode(VK_F10))
  input_event F9;  poof(@win32_keycode(VK_F9))
  input_event F8;  poof(@win32_keycode(VK_F8))
  input_event F7;  poof(@win32_keycode(VK_F7))
  input_event F6;  poof(@win32_keycode(VK_F6))
  input_event F5;  poof(@win32_keycode(VK_F5))
  input_event F4;  poof(@win32_keycode(VK_F4))
  input_event F3;  poof(@win32_keycode(VK_F3))
  input_event F2;  poof(@win32_keycode(VK_F2))
  input_event F1;  poof(@win32_keycode(VK_F1))

  input_event Dot;    poof(@glyph('.') @win32_keycode(VK_OEM_PERIOD))
  input_event Minus;  poof(@glyph('-') @win32_keycode(VK_OEM_MINUS))
  input_event FSlash; poof(@glyph('/') @win32_keycode(VK_OEM_2))

  input_event Space;      poof(@glyph(' ') @win32_keycode(VK_SPACE))
  input_event Underscore; poof(@glyph('_'))

  input_event N0;  poof(@glyph('0') @win32_keycode(0x30))
  input_event N1;  poof(@glyph('1') @win32_keycode(0x31))
  input_event N2;  poof(@glyph('2') @win32_keycode(0x32))
  input_event N3;  poof(@glyph('3') @win32_keycode(0x33))
  input_event N4;  poof(@glyph('4') @win32_keycode(0x34))
  input_event N5;  poof(@glyph('5') @win32_keycode(0x35))
  input_event N6;  poof(@glyph('6') @win32_keycode(0x36))
  input_event N7;  poof(@glyph('7') @win32_keycode(0x37))
  input_event N8;  poof(@glyph('8') @win32_keycode(0x38))
  input_event N9;  poof(@glyph('9') @win32_keycode(0x39))

  input_event A; poof(@glyph('a') @win32_keycode(0x41))
  input_event B; poof(@glyph('b') @win32_keycode(0x42))
  input_event C; poof(@glyph('c') @win32_keycode(0x43))
  input_event D; poof(@glyph('d') @win32_keycode(0x44))
  input_event E; poof(@glyph('e') @win32_keycode(0x45))
  input_event F; poof(@glyph('f') @win32_keycode(0x46))
  input_event G; poof(@glyph('g') @win32_keycode(0x47))
  input_event H; poof(@glyph('h') @win32_keycode(0x48))
  input_event I; poof(@glyph('i') @win32_keycode(0x49))
  input_event J; poof(@glyph('j') @win32_keycode(0x4A))
  input_event K; poof(@glyph('k') @win32_keycode(0x4B))
  input_event L; poof(@glyph('l') @win32_keycode(0x4C))
  input_event M; poof(@glyph('m') @win32_keycode(0x4D))
  input_event N; poof(@glyph('n') @win32_keycode(0x4E))
  input_event O; poof(@glyph('o') @win32_keycode(0x4F))
  input_event P; poof(@glyph('p') @win32_keycode(0x50))
  input_event Q; poof(@glyph('q') @win32_keycode(0x51))
  input_event R; poof(@glyph('r') @win32_keycode(0x52))
  input_event S; poof(@glyph('s') @win32_keycode(0x53))
  input_event T; poof(@glyph('t') @win32_keycode(0x54))
  input_event U; poof(@glyph('u') @win32_keycode(0x55))
  input_event V; poof(@glyph('v') @win32_keycode(0x56))
  input_event W; poof(@glyph('w') @win32_keycode(0x57))
  input_event X; poof(@glyph('x') @win32_keycode(0x58))
  input_event Y; poof(@glyph('y') @win32_keycode(0x59))
  input_event Z; poof(@glyph('z') @win32_keycode(0x5A))



  input_event RMB;
  input_event LMB;
  input_event MMB;

  s32 MouseWheelDelta;
};

struct hotkeys
{
  b32 Debug_ToggleMenu;
  b32 Debug_ToggleProfiling;

  b32 Debug_TriangulateIncrement;
  b32 Debug_TriangulateDecrement;

  b32 Left;
  b32 Right;
  b32 Forward;
  b32 Backward;
};

inline v3
GetOrthographicInputs(hotkeys *Hotkeys)
{
  v3 Right = V3(1,0,0);
  v3 Forward = V3(0,1,0);

  v3 UpdateDir = V3(0,0,0);

  if ( Hotkeys->Forward )
    UpdateDir += Forward;

  if ( Hotkeys->Backward )
    UpdateDir -= Forward;

  if ( Hotkeys->Right )
    UpdateDir += Right;

  if ( Hotkeys->Left )
    UpdateDir -= Right;

  UpdateDir = Normalize(UpdateDir);

  return UpdateDir;
}

link_internal void
BindHotkeysToInput(hotkeys *Hotkeys, input *Input)
{

  if (Input->Shift.Pressed) {
    if (Input->Minus.Clicked)
    {
      Input->Minus = {};
      Input->Underscore = {True, True};
    }
  }

#if BONSAI_INTERNAL
  if (Input->F1.Clicked) { Hotkeys->Debug_ToggleMenu         = True; }
  if (Input->F2.Clicked) { Hotkeys->Debug_ToggleProfiling    = True; }

  /* Hotkeys->Debug_TriangulateDecrement = Input->F5.Clicked; */
  /* Hotkeys->Debug_TriangulateIncrement = Input->F6.Clicked; */
#endif

  Hotkeys->Left = Input->A.Pressed;
  Hotkeys->Right = Input->D.Pressed;
  Hotkeys->Forward = Input->W.Pressed;
  Hotkeys->Backward = Input->S.Pressed;

  /* Hotkeys->Player_Fire = Input->Space.Clicked; */
  /* Hotkeys->Player_Proton = Input->Shift.Clicked; */
}

link_internal void
ClearClickedFlags(input *Input)
{
poof(
  func (input InputDef)
  {
    InputDef.map_members (Member)
    {
      Member.is_type(input_event)?
      {
        Member.is_array?
        {
          Member.map_array(Index)
          {
            Input->(Member.name)[Index].Clicked = False;
          }
        }
        {
          Input->(Member.name).Clicked = False;
        }
      }
    }
  }
)
#include <generated/anonymous_function_input_HLGbqwh0.h>

  return;
}

link_internal void
ResetInputForFrameStart(input *Input, hotkeys *Hotkeys)
{
  if (Input) { Input->MouseWheelDelta = 0; ClearClickedFlags(Input); }
  if (Hotkeys) { Clear(Hotkeys); }
}

// TODO(Jesse, globals_cleanup): Put this on stdlib ..?
global_variable r64 Global_LastDebugTime = 0;
r32 GetDt()
{
  r64 ThisTime = GetHighPrecisionClock();
  r64 Result = ThisTime - Global_LastDebugTime;
  Global_LastDebugTime = ThisTime;
  return r32(Result);
}
