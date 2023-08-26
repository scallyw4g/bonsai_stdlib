

link_internal b32
Hover(debug_ui_render_group* Group, interactable_handle *Interaction)
{
  b32 Result = Group->HoverInteraction.Interaction.ID == Interaction->Id;
  return Result;
}

link_internal b32
Clicked(debug_ui_render_group* Group, interactable_handle *Interaction)
{
  b32 Result = Group->ClickedInteraction.Interaction.ID == Interaction->Id;
  return Result;
}

link_internal b32
Pressed(debug_ui_render_group* Group, interactable_handle *Interaction)
{
  b32 Result = Group->PressedInteraction.Interaction.ID == Interaction->Id;
  return Result;
}

link_internal b32
Hover(debug_ui_render_group* Group, interactable *Interaction)
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
Clicked(debug_ui_render_group* Group, interactable *Interaction)
{
  b32 MouseButtonClicked = Group->Input->LMB.Clicked || Group->Input->RMB.Clicked;

  b32 Result = False;
  if ( !Group->PressedInteraction.Interaction.ID &&
       MouseButtonClicked && Hover(Group, Interaction))
  {
    Group->PressedInteraction.Interaction.ID = Interaction->ID;
    Result = True;
  }

  return Result;
}

link_internal b32
Clicked(debug_ui_render_group* Group, interactable Interaction)
{
  b32 Result = Clicked(Group, &Interaction);
  return Result;
}

link_internal b32
Pressed(debug_ui_render_group* Group, interactable *Interaction)
{
  umm CurrentInteraction = Group->PressedInteraction.Interaction.ID;
  b32 CurrentInteractionMatches = CurrentInteraction == Interaction->ID;
  b32 MouseDepressed = Group->Input->LMB.Pressed || Group->Input->RMB.Pressed;

  b32 Result = False;
  if (MouseDepressed && CurrentInteractionMatches)
  {
    Result = True;
  }
  else if (MouseDepressed && !CurrentInteraction && Hover(Group, Interaction))
  {
    Group->PressedInteraction.Interaction.ID = Interaction->ID;
    Result = True;
  }

  return Result;
}


link_internal v2
GetElementRelativeOffset(renderer_2d *Ui, interactable_handle *Handle)
{
  return V2(0);
}
