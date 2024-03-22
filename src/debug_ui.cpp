r32
GetXOffsetForHorizontalBar(u64 StartCycleOffset, u64 FrameTotalCycles, r32 TotalGraphWidth)
{
  r32 XOffset = ((r32)StartCycleOffset/(r32)FrameTotalCycles)*TotalGraphWidth;
  return XOffset;
}

link_internal void
PushCycleBar(debug_ui_render_group* Group, cycle_range* Range, cycle_range* Frame, r32 TotalGraphWidth, r32 BarHeight, r32 yOffset, ui_style *Style, v4 Padding = V4(0), cs Name = CSz(""))
{
  Assert(Frame->StartCycle <= Range->StartCycle);

  r32 FramePerc = (r32)Range->TotalCycles / (r32)Frame->TotalCycles;

  r32 BarWidth = FramePerc*TotalGraphWidth;

  if (BarWidth > 0.15f)
  {
    v2 BarDim = V2(BarWidth, BarHeight);

    u64 StartCycleOffset = Range->StartCycle - Frame->StartCycle;
    r32 xOffset = GetXOffsetForHorizontalBar(StartCycleOffset, Frame->TotalCycles, TotalGraphWidth);

    v2 Offset = V2(xOffset, yOffset);

    PushUntexturedQuad(Group, Offset, BarDim, zDepth_Text, Style, Padding, UiElementLayoutFlag_AdvanceClip);
    ui_style NameStyle = DefaultStyle;
    NameStyle.Font = Global_SmallFont;
    if (Name.Count) { Text(Group, Name, &NameStyle, UiElementLayoutFlag_NoAdvance, Offset, RectMinDim({}, BarDim)); }
  }

  return;
}

