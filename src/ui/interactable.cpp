

link_internal b32
Hover(renderer_2d* Group, interactable_handle *Interaction, v2 *Offset_out = 0)
{
  b32 Result = Group->Hover.ID == Interaction->Id;

  if (Result && Offset_out)
  {
    v2 MouseP = *Group->MouseP;
    v2 RelativeOffset = MouseP - Group->Pressed.MinP;
    *Offset_out = RelativeOffset;
  }

  return Result;
}

link_internal b32
Clicked(renderer_2d* Group, interactable_handle *Interaction)
{
  b32 Result = Group->Clicked.ID == Interaction->Id;
  return Result;
}

link_internal b32
Pressed(renderer_2d* Group, interactable_handle *Interaction, v2 *Offset_out = 0)
{
  b32 Result = Group->Pressed.ID == Interaction->Id;

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
  if ( !Group->Pressed.ID &&
       MouseButtonClicked && Hover(Group, Interaction))
  {
    Group->Pressed.ID = Interaction->ID;
    Result = True;
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
  umm CurrentInteraction = Group->Pressed.ID;
  b32 CurrentInteractionMatches = CurrentInteraction == Interaction->ID;
  b32 MouseDepressed = Group->Input->LMB.Pressed || Group->Input->RMB.Pressed;

  b32 Result = False;
  if (MouseDepressed && CurrentInteractionMatches)
  {
    Result = True;
  }
  else if (MouseDepressed && !CurrentInteraction && Hover(Group, Interaction))
  {
    Group->Pressed.ID = Interaction->ID;
    Result = True;
  }

  return Result;
}

