
struct input_event
{
  b32 Clicked;
  b32 Pressed;

  // TODO(Jesse): Add this?
  // b32 Released;
};

struct input
{
  input_event Escape;

  input_event Enter;
  input_event Space;
  input_event Shift;
  input_event Ctrl;
  input_event Alt;
  input_event Delete;

  input_event F12;
  input_event F11;
  input_event F10;
  input_event F9;
  input_event F8;
  input_event F7;
  input_event F6;
  input_event F5;
  input_event F4;
  input_event F3;
  input_event F2;
  input_event F1;

  input_event RMB;
  input_event LMB;
  input_event MMB;

  input_event Q;
  input_event W;
  input_event E;
  input_event R;
  input_event T;
  input_event Y;
  input_event U;
  input_event I;
  input_event O;
  input_event P;

  input_event A;
  input_event S;
  input_event D;
  input_event F;
  input_event G;
  input_event H;
  input_event J;
  input_event K;
  input_event L;

  input_event Z;
  input_event X;
  input_event C;
  input_event V;
  input_event B;
  input_event N;
  input_event M;

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
        Input->(Member.name).Clicked = False;
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

