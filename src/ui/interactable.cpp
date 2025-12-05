

link_internal b32
Hover(renderer_2d* Group, ui_id *Id, v2 *Offset_out = 0)
{
  b32 Result = Group->Hover.Id == *Id;
  if (Result && Offset_out)
  {
    v2 MouseP = *Group->MouseP;
    // TODO(Jesse): wtf are we accessing Pressed here for .. shouldn't it be Hover?!
    v2 RelativeOffset = MouseP - Group->Pressed.MinP;
    *Offset_out = RelativeOffset;
  }

  return Result;
}

link_internal b32
Hover(renderer_2d *Group, interactable_handle *Handle, v2 *Offset_out = 0)
{
  b32 Result = Hover(Group, &Handle->Id);
  return Result;
}

link_internal b32
Clicked(renderer_2d *Group, interactable_handle *Handle)
{
  if (IsValid(&Group->Clicked.Id)) { Assert(Group->Input->LMB.Clicked || Group->Input->RMB.Clicked); }
  b32 Result = Group->Clicked.Id == Handle->Id;
  return Result;
}

link_internal b32
Clicked(renderer_2d *Group, ui_id Id)
{
  if (IsValid(&Group->Clicked.Id)) { Assert(Group->Input->LMB.Clicked || Group->Input->RMB.Clicked); }
  b32 Result = Group->Clicked.Id == Id;
  return Result;
}

link_internal b32
Clicked(renderer_2d *Group, ui_id *Id)
{
  if (IsValid(&Group->Clicked.Id)) { Assert(Group->Input->LMB.Clicked || Group->Input->RMB.Clicked); }
  b32 Result = Group->Clicked.Id == *Id;
  return Result;
}

link_internal b32
RClicked(renderer_2d *Group, ui_id *Id)
{
  if (IsValid(&Group->Clicked.Id)) { Assert(Group->Input->LMB.Clicked || Group->Input->RMB.Clicked); }
  b32 Result = Group->Input->RMB.Clicked && (Group->Clicked.Id == *Id);
  return Result;
}

link_internal b32
LClicked(renderer_2d *Group, ui_id *Id)
{
  if (IsValid(&Group->Clicked.Id)) { Assert(Group->Input->LMB.Clicked || Group->Input->RMB.Clicked); }
  b32 Result = Group->Input->LMB.Clicked && (Group->Clicked.Id == *Id);
  return Result;
}


/* link_internal b32 */
/* Released(renderer_2d *Group, interactable_handle *Handle) */
/* { */
/*   b32 Result = Group->Released.Id == Handle->Id; */
/*   return Result; */
/* } */

link_internal b32
Pressed(renderer_2d* Group, interactable_handle *Handle, v2 *Offset_out = 0)
{
  b32 Result = Group->Pressed.Id == Handle->Id;

  if (Result && Offset_out)
  {
    v2 MouseP = *Group->MouseP;
    v2 RelativeOffset = MouseP - Group->Pressed.MinP;
    *Offset_out = RelativeOffset;
  }

  return Result;
}





link_internal b32
Hover(renderer_2d* Group, interactable *Interaction)
{
  v2 MouseP = *Group->MouseP;

  b32 HotWindowMatchesInteractionWindow = (Group->HighestWindow == Interaction->Window);
  b32 Result = HotWindowMatchesInteractionWindow && IsInsideRect(Rect2(Interaction), MouseP);

  if (Interaction->Window)
  {
    Result = Result && IsInsideRect( GetBounds(Interaction->Window), MouseP );
  }

  return Result;
}

link_internal b32
Clicked(renderer_2d* Group, interactable *Interaction)
{
  b32 MouseButtonClicked = Group->Input->LMB.Clicked || Group->Input->RMB.Clicked;

  b32 Result = False;
  if ( !IsValid(&Group->Pressed.Id) &&
        MouseButtonClicked && Hover(Group, Interaction))
  {
    Group->Pressed.Id = Interaction->Id;
    Result = True;

    if (GetUiDebug)
    {
      if (GetUiDebug()->LogClickEvents)
      {
        auto Id = &Interaction->Id;
        Info("Click (%d)(%d)(%d)(%d)",  Id->WindowBits, Id->InteractionBits, Id->ElementBits, Id->HashBits);
      }
    }
  }

  return Result;
}

link_internal b32
Clicked(renderer_2d* Group, interactable Interaction)
{
  b32 Result = Clicked(Group, &Interaction);
  return Result;
}

link_internal b32
Pressed(renderer_2d* Group, interactable *Interaction)
{
  b32 CurrentInteractionMatches = Group->Pressed.Id == Interaction->Id;
  b32 MouseDepressed = Group->Input->LMB.Pressed || Group->Input->RMB.Pressed;

  b32 Result = False;
  if (MouseDepressed && CurrentInteractionMatches)
  {
    Result = True;
  }
  else if (MouseDepressed && Hover(Group, Interaction) && !IsValid(&Group->Pressed.Id))
  {
    // NOTE(Jesse): This is an invariant that (logically)
    // must be true if we didn't have a valid Pressed id
    b32 MouseClicked = Group->Input->LMB.Clicked || Group->Input->RMB.Clicked;
    Assert(MouseClicked);

    Group->Pressed.Id = Interaction->Id;
    Result = True;
  }

  return Result;
}

