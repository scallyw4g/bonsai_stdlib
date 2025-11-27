

link_internal b32
Hover(renderer_2d* Group, ui_id *Id, v2 *Offset_out = 0)
{
  b32 Result = Group->Hover.ID == *Id;

  if (Result && Offset_out)
  {
    v2 MouseP = *Group->MouseP;
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
  b32 Result = Group->Clicked.ID == Handle->Id;
  return Result;
}

link_internal b32
Clicked(renderer_2d *Group, ui_id Id)
{
  b32 Result = Group->Clicked.ID == Id;
  return Result;
}

link_internal b32
Clicked(renderer_2d *Group, ui_id *Id)
{
  b32 Result = Group->Clicked.ID == *Id;
  return Result;
}


/* link_internal b32 */
/* Released(renderer_2d *Group, interactable_handle *Handle) */
/* { */
/*   b32 Result = Group->Released.ID == Handle->Id; */
/*   return Result; */
/* } */

link_internal b32
Pressed(renderer_2d* Group, interactable_handle *Handle, v2 *Offset_out = 0)
{
  b32 Result = Group->Pressed.ID == Handle->Id;

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
  if ( !IsValid(&Group->Pressed.ID) &&
        MouseButtonClicked && Hover(Group, Interaction))
  {
    Group->Pressed.ID = Interaction->ID;
    Result = True;

    if (GetUiDebug)
    {
      if (GetUiDebug()->LogClickEvents)
      {
        auto Id = &Interaction->ID;
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
  ui_id CurrentInteraction = Group->Pressed.ID;
  b32 CurrentInteractionMatches = CurrentInteraction == Interaction->ID;
  b32 MouseDepressed = Group->Input->LMB.Pressed || Group->Input->RMB.Pressed;

  b32 Result = False;
  if (MouseDepressed && CurrentInteractionMatches)
  {
    Result = True;
  }
  else if (MouseDepressed && !IsValid(&CurrentInteraction) && Hover(Group, Interaction))
  {
    Group->Pressed.ID = Interaction->ID;
    Result = True;
  }

  return Result;
}

