struct layout;
struct window_layout;

struct ui_id
{
  u32 _Reserved;
  u32 WindowBits;
  u32 InteractionBits;
  u32 ElementBits;

  /* operator bool(); */
};

link_internal b32
IsValid(ui_id *Id)
{
  b32 Reuslt = (Id->WindowBits | Id->InteractionBits | Id->ElementBits) != 0;
  return Reuslt;
}

link_internal u64
Hash(ui_id *Id)
{
  // TODO(Jesse)(hash): Is this any good?
  u64 Result = 654378024321 ^ (Id->WindowBits | (Id->InteractionBits << 31)) ^ (Id->ElementBits << 15);
  return Result;
}

poof(gen_default_equality_operator(ui_id))
#include <generated/gen_default_equality_operator_ui_id.h>

/* poof(are_equal(ui_id)) */
/* #include <generated/are_equal_ui_id.h> */

link_internal b32
AreEqual(ui_id &Thing1, ui_id &Thing2)
{
  b32 Result = Thing1 == Thing2;
  return Result;
}

link_internal b32
AreEqual(ui_id *Thing1, ui_id *Thing2)
{
  b32 Result = *Thing1 == *Thing2;
  return Result;
}


struct interactable_handle
{
  ui_id Id;
};

struct interactable
{
  ui_id ID;
  v2 MinP;
  v2 MaxP;

  window_layout* Window;
};

struct button_interaction_result
{
  b32 Pressed;
  b32 Clicked;
  b32 Hover;

  interactable Interaction;
};

link_internal interactable
Interactable(v2 MinP, v2 MaxP, ui_id ID, window_layout *Window)
{
  interactable Result = {};
  Result.MinP = MinP;
  Result.MaxP = MaxP;
  Result.ID = ID;
  Result.Window = Window;

  return Result;
}

link_internal interactable
Interactable(rect2 Rect, ui_id ID, window_layout *Window)
{
  interactable Result = Interactable(Rect.Min, Rect.Max, ID, Window);
  return Result;
}

v2 GetAbsoluteAt(layout* Layout);

link_internal interactable
StartInteractable(layout* Layout, ui_id ID, window_layout *Window)
{
  v2 StartingAt = GetAbsoluteAt(Layout);
  interactable Result = Interactable(StartingAt, StartingAt, ID, Window);
  return Result;
}

link_internal rect2
Rect2(s32 Flood)
{
  rect2 Result = RectMinMax(V2(Flood), V2(Flood));
  return Result;
}

link_internal rect2
Rect2(interactable Interaction)
{
  rect2 Result = RectMinMax(Interaction.MinP, Interaction.MaxP);
  return Result;
}

link_internal rect2
Rect2(interactable *Interaction)
{
  rect2 Result = Rect2(*Interaction);
  return Result;
}

