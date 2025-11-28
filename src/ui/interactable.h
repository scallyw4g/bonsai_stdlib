struct layout;
struct window_layout;

union ui_id
poof(@do_editor_ui)
{
  struct
  {
    u32 WindowBits;
    u32 InteractionBits;
    u32 ElementBits;
    u32 HashBits;
  };
  u32 E[4];
};

link_internal b32
IsValid(ui_id *Id)
{
  b32 Reuslt = (Id->E[0] | Id->E[1] | Id->E[2] | Id->E[3]) != 0;
  return Reuslt;
}

link_internal u64
Hash(ui_id *Id)
{
  u64 Result =
    ChrisWellonsIntegerHash_lowbias32(Id->E[0]) +
    ChrisWellonsIntegerHash_lowbias32(Id->E[1]) +
    ChrisWellonsIntegerHash_lowbias32(Id->E[2]) +
    ChrisWellonsIntegerHash_lowbias32(Id->E[3]) ;
  return Result;
}


link_internal b32
operator==( ui_id E1, ui_id E2 )
{
  b32 Result = 
    E1.E[0] == E2.E[0] &&
    E1.E[1] == E2.E[1] &&
    E1.E[2] == E2.E[2] &&
    E1.E[3] == E2.E[3];
  return Result;
}

link_internal b32
operator!=( ui_id E1, ui_id E2 )
{
  b32 Reuslt = !(E1 == E2);
  return Reuslt;
}

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
poof(@do_editor_ui)
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

