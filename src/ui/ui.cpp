#define u32_COUNT_PER_QUAD (6)


poof(buffer(ui_toggle_button_handle))
#include <generated/buffer_ui_toggle_button_handle.h>

poof(hashtable_impl(ui_toggle))
#include <generated/hashtable_impl_ui_toggle.h>

poof(hashtable_get(ui_toggle, {ui_id}, {Id}))
#include <generated/hashtable_get_ui_toggle_31501_688856534.h>

poof(hashtable_get_ptr(ui_toggle, {ui_id}, {Id}))
#include <generated/hashtable_get_ptr_ui_toggle_31501_688856534.h>


enum ui_toggle_button_group_flags
{
  ToggleButtonGroupFlags_None = 0,
  ToggleButtonGroupFlags_RadioButtons = (1 << 0),
  ToggleButtonGroupFlags_DrawVertical = (1 << 1),
};

struct ui_toggle_button_group
{
  renderer_2d *Ui;

  ui_toggle_button_handle_buffer Buttons;

  ui_element_reference UiRef;
  ui_toggle_button_group_flags Flags;

  // This is a bitfield which indicates which enum values are toggled on.  Each
  // bit corresponds to the enum value index.
  u64 ToggleBits;

  b32 AnyElementClicked;
};

link_internal ui_element_reference
DrawToggleButtonGroup(ui_toggle_button_group *Group, UI_FUNCTION_PROTO_DEFAULTS);

link_internal ui_toggle_button_group
UiToggleButtonGroup(renderer_2d *Ui, ui_toggle_button_handle_buffer *Buttons, ui_toggle_button_group_flags Flags = ToggleButtonGroupFlags_None, UI_FUNCTION_PROTO_DEFAULTS)
{
  ui_toggle_button_group Result = {};
  Result.Ui = Ui;
  Result.Flags = Flags;
  Result.Buttons = *Buttons;

  Assert(Buttons->Count < bitsof(Result.ToggleBits));

  Result.UiRef = DrawToggleButtonGroup(&Result, UI_FUNCTION_INSTANCE_NAMES);

  return Result;
}

link_internal b32
ToggledOn(renderer_2d* Group, interactable_handle *Interaction)
{
  b32 Result = False;

  maybe_ui_toggle Maybe = GetById(&Group->ToggleTable, Interaction->Id);
  if (Maybe.Tag) { Result = Maybe.Value.ToggledOn; }

  return Result;
}

link_internal b32
ToggledOn(renderer_2d *Ui, ui_toggle_button_handle *Button)
{
  interactable_handle Handle = { Button->Id };
  b32 Result = ToggledOn(Ui, &Handle);
  return Result;
}

#if 0
link_internal b32
SelectionChanged(ui_toggle_button_group *ButtonGroup)
{
  b32 Result = ButtonGroup->SelectionChanged;
  return Result;
}
#endif

#if 0
link_internal b32
ToggledOn(renderer_2d *Ui, ui_toggle_button_group *Group, cs ButtonName)
{
  b32 Result = False;
  IterateOver(&Group->Buttons, Button, ButtonIndex)
  {
    if (StringsMatch(Button->Text, ButtonName))
    {
      Result = ToggledOn(Ui, Button);
      break;
    }
  }
  return Result;
}
#endif




/*******************************              *********************************/
/*******************************  DrawBounds  *********************************/
/*******************************              *********************************/



inline void
UpdateDrawBounds(layout *Layout, v2 LayoutRelativeTestP)
{
  // I know it's tempting.. but don't..
  /* if (LayoutRelativeTestP.x != 0.f || LayoutRelativeTestP.y != 0.f) */
  {
    v2 AbsP = Layout->Basis + LayoutRelativeTestP;
    while (Layout)
    {
      v2 LayoutRelP = AbsP - Layout->Basis;
      Layout->DrawBounds.Min = Min(LayoutRelP, Layout->DrawBounds.Min);
      Layout->DrawBounds.Max = Max(LayoutRelP, Layout->DrawBounds.Max);

      Layout = Layout->Prev;
    }
  }
}

inline void
UpdateDrawBounds(layout *Layout)
{
  UpdateDrawBounds(Layout, Layout->At);
}



/*****************************                ********************************/
/*****************************  Text Helpers  ********************************/
/*****************************                ********************************/


link_internal void
AdvanceSpaces(u32 N, layout *Layout, v2 FontSize)
{
  Layout->At.x += (N*FontSize.x);
  UpdateDrawBounds(Layout);
  return;
}

link_internal void AdvanceLayoutStackBy(v2 Delta, layout* Layout);


link_internal b32
WithinTolerance(f32 Epsilon, v3 A, v3 B)
{
  b32 Result = (A.x+Epsilon > B.x && A.x-Epsilon < B.x) &&
               (A.y+Epsilon > B.y && A.y-Epsilon < B.y) &&
               (A.z+Epsilon > B.z && A.z-Epsilon < B.z);
  return Result;
}

link_internal b32
WithinTolerance(f32 Epsilon, f32 A, f32 B)
{
  b32 Result = (A+Epsilon > B && A-Epsilon < B);
  return Result;
}

link_internal void
NewRow(layout *Layout)
{
  // NOTE(Jesse): This epsilon is INSANELY (in caps!) big.  Should find a
  // better way to deal with this.  It's a bit of a weird hack anyways..
  r32 Epsilon = 0.01f;
  while (Layout)
  {
#if 1
  // NOTE(Jesse): This is a special case for if we call NewRow() multiple
  // times in a row without drawing stuff.. ie:
  //
  // BufferSomeText();
  // NewRow();
  // NewRow();
  // BufferSomeText();
  //
  // It also handles if we draw a bunch of stuff but don't ever advance the
  // layout, and do advance the DrawBounds
  //
  r32 VerticalAdvance = 0.f;
  if (WithinTolerance(Epsilon, Layout->At.y, Layout->DrawBounds.Max.y)) { VerticalAdvance = Global_Font.Size.y; }

  Layout->At.x = Layout->Padding.Left;

  // NOTE(Jesse): This adds DrawBounds.Max.y such that if we draw stuff other
  // than text on the line we get proper advancement.  Expanded callgraph nodes
  // are a good example
  //
  // NOTE(Jesse): If we push a new line as the first thing we'd set the At to
  // inverted infinity max, so we have to check for that.  We also wanted to
  // advance a line, so we set the VerticalAdvance.
  //
  // NOTE(Jesse): This comparison should be exact.
  if (Layout->DrawBounds.Max.y == InvertedInfinityRectangle().Max.y)
  {
    VerticalAdvance = Global_Font.Size.y;
  }
  else
  {
    Layout->At.y = Layout->DrawBounds.Max.y;
  }

  Layout->At.y += VerticalAdvance;

  // TODO(Jesse): Do we actually want to call this here?  Probably not if we
  // just do a NewRow and not print anything?
  UpdateDrawBounds(Layout);
#else

  // NOTE(Jesse): This is misguided; when there are two tables next to
  // one-another the advance ends up counting both of them on the default
  // layout, which means the next element gets bumped down the sum of both
  // their heights, not the max height of either.
  Layout->At.x = 0.0f;
  AdvanceLayoutStackBy( V2(0.f, Global_Font.Size.y), Layout);

#endif

    Layout = Layout->Prev;
  }
}

link_internal v3
SelectColorState(render_state* RenderState, ui_style *Style)
{
  v3 Result = Style->Color;

  if (RenderState->Hover)
  {
    Result = Style->HoverColor;
  }
  if (RenderState->Pressed)
  {
    Result = Style->PressedColor;
  }
  if (RenderState->Clicked)
  {
    Result = Style->ClickedColor;
  }

  return Result;
}


link_internal void
BufferQuadUVs(textured_2d_geometry_buffer* Geo, rect2 UV, s32 Slice)
{
  // @streaming_ui_render_memory
  Assert(BufferHasRoomFor(Geo, u32_COUNT_PER_QUAD));

  v3 LeftTop     = V3(UV.Min.x, UV.Min.y, r32(Slice));
  v3 RightTop    = V3(UV.Max.x, UV.Min.y, r32(Slice));
  v3 RightBottom = V3(UV.Max.x, UV.Max.y, r32(Slice));
  v3 LeftBottom  = V3(UV.Min.x, UV.Max.y, r32(Slice));

  u32 StartingIndex = Geo->At;
  Geo->UVs[StartingIndex++] = LeftTop;
  Geo->UVs[StartingIndex++] = LeftBottom;
  Geo->UVs[StartingIndex++] = RightTop;

  Geo->UVs[StartingIndex++] = RightBottom;
  Geo->UVs[StartingIndex++] = RightTop;
  Geo->UVs[StartingIndex++] = LeftBottom;

  return;
}

link_internal rect2
UVsForFullyCoveredQuad(void)
{
  // Note(Jesse): These are weird compared to what you might expect because
  // OpenGL screen coordinates originate at the bottom left, but are inverted
  // in our app such that the origin is at the top-left
  // @inverted_screen_y_coordinate
  v2 LeftTop    = V2(0.0f, 1.0f);
  v2 RightBottom = V2(1.0f, 0.0f);

  rect2 Result = RectMinMax(LeftTop, RightBottom);
  return Result;
}

link_internal rect2
UVsForChar(u8 C)
{
  r32 OneOverSixteen = 1.0f/16.0f;

  // Note(Jesse): These are weird compared to what you might expect because
  // OpenGL screen coordinates originate at the bottom left, but are inverted
  // in our app such that the origin is at the top-left
  // @inverted_screen_y_coordinate
  v2 LeftBottom = GetUVForCharCode(C);
  v2 LeftTop  = LeftBottom + V2(0.0f, OneOverSixteen);
  v2 RightBottom  = LeftBottom + V2(OneOverSixteen, 0.0f);

  rect2 Result = RectMinMax(LeftTop, RightBottom);
  return Result;
}

template <typename T> link_internal void
BufferColorsDirect(T* Geo, v3 Color)
{
  // @streaming_ui_render_memory
  Assert(BufferHasRoomFor(Geo, u32_COUNT_PER_QUAD));

  v3* Colors = Geo->Colors;
  u32 StartingIndex = Geo->At;
  for ( u32 ColorIndex = StartingIndex;
        ColorIndex < StartingIndex + u32_COUNT_PER_QUAD + 1;
        ++ColorIndex)
  {
    Colors[ColorIndex] = Color;
  }

  return;
}

// TODO(Jesse): This is just wasteful .. BufferColorsDirect does this check!
template <typename T> link_internal void
BufferColors(renderer_2d *Group, T *Geo, v3 Color)
{
  // @streaming_ui_render_memory
  Assert(BufferHasRoomFor(Geo, u32_COUNT_PER_QUAD));
  BufferColorsDirect(Geo, Color);
}

#if 1
inline b32
RangeContainsInclusive(u64 Min, u64 N, u64 Max)
{
  b32 Result = (N >= Min && N <= Max);
  return Result;
}
#endif

inline b32
RangeContains(r32 Min, r32 N, r32 Max)
{
  b32 Result = (N >= Min && N < Max);
  return Result;
}

link_internal clip_result
ClipRect3AgainstRect2(v2 MinP, v2 Dim, r32 Z, rect2 *UV, rect2 Clip)
{
  // Note(Jesse): Z==0 | far-clip
  // Note(Jesse): Z==1 | near-clip
  clip_result Result = {
    .ClippedMin = MinP,
    .ClippedMax = MinP+Dim,
  };

  Assert(Z >= 0.0f && Z <= 1.0f);

  v2 MaxP = MinP+Dim;
  if ( Clip.Max.x <= MinP.x || Clip.Max.y <= MinP.y || MaxP.x < Clip.Min.x || MaxP.y < Clip.Min.y )
  {
    Result.ClipStatus = ClipStatus_FullyClipped;
  }
  else
  {
    r32 Left   = MinP.x;
    r32 Right  = Left+Dim.x;
    r32 Top    = MinP.y;
    r32 Bottom = Top+Dim.y;

    v3 LeftTop    = V3(Left, Top, Z);
    v3 RightTop   = V3(Right, Top, Z);
    v3 RightBottom = V3(Right, Bottom, Z);
    v3 LeftBottom  = V3(Left, Bottom, Z);

    if (RangeContains(Left, Clip.Max.x, Right))
    {
      r32 Total = Right - Left;
      r32 TotalClipped = Right - Clip.Max.x;
      Result.ClipRatio.Max.x = TotalClipped / Total;

      Result.ClippedMax.x = RightTop.x = RightBottom.x = Clip.Max.x;
      Result.ClipStatus = ClipStatus_PartialClipping;

      Assert(Result.ClipRatio.Max.x >= 0.0f && Result.ClipRatio.Max.x <= 1.0f);
    }

    if (RangeContains(Top, Clip.Max.y, Bottom))
    {
      r32 Total = Bottom - Top;
      r32 TotalClipped = Bottom - Clip.Max.y;
      Result.ClipRatio.Max.y = TotalClipped / Total;

      Result.ClippedMax.y = LeftBottom.y = RightBottom.y = Clip.Max.y;
      Result.ClipStatus = ClipStatus_PartialClipping;

      Assert(Result.ClipRatio.Max.y >= 0.0f && Result.ClipRatio.Max.y <= 1.0f);
    }

    if (RangeContains(Top, Clip.Min.y, Bottom))
    {
      r32 Total = Bottom - Top;
      r32 TotalClipped = Clip.Min.y - Top;
      Result.ClipRatio.Min.y = TotalClipped / Total;

      Result.ClippedMin.y = LeftTop.y = RightTop.y = Clip.Min.y;
      Result.ClipStatus = ClipStatus_PartialClipping;

      Assert(Result.ClipRatio.Min.y >= 0.0f && Result.ClipRatio.Min.y <= 1.0f);
    }

    if (UV)
    {
      v2 MinUvDiagonal = UV->Max - UV->Min;
      v2 MaxUvDiagonal = UV->Min - UV->Max;

      v2 MinUvModifier = MinUvDiagonal * Result.ClipRatio.Min;
      v2 MaxUvModifier = MaxUvDiagonal * Result.ClipRatio.Max;

      UV->Min += MinUvModifier;
      UV->Max += MaxUvModifier;
    }

    /* Result.Clip.Max = RightBottom.xy; */
  }

  return Result;
}


template <typename T> link_internal void
BufferQuadDirect(T *Geo, v2 MinP, v2 Dim, r32 Z, v2 *ScreenDim)
{
  Assert(BufferHasRoomFor(Geo, u32_COUNT_PER_QUAD));

  if (ScreenDim->x > 0 && ScreenDim->y > 0)
  {
    r32 Left   = MinP.x;
    r32 Right  = Left+Dim.x;
    r32 Top    = MinP.y;
    r32 Bottom = Top+Dim.y;

    v3 LeftTop    = V3(Left, Top, Z);
    v3 RightTop   = V3(Right, Top, Z);
    v3 RightBottom = V3(Right, Bottom, Z);
    v3 LeftBottom  = V3(Left, Bottom, Z);


    #define TO_NDC(P) ((P * ToNDC) - 1.0f)
    v3 ToNDC = 2.0f/V3(ScreenDim->x, ScreenDim->y, 1.0f);

    // Native OpenGL screen coordinates are {0,0} at the bottom-left corner. This
    // maps the origin to the top-left of the screen.
    // @inverted_screen_y_coordinate
    v3 InvertYZ = V3(1.0f, -1.0f, -1.0f);

    v3 *Dest = Geo->Verts;
    u32 StartingIndex = Geo->At;
    Dest[StartingIndex++] = InvertYZ * TO_NDC(LeftTop);
    Dest[StartingIndex++] = InvertYZ * TO_NDC(LeftBottom);
    Dest[StartingIndex++] = InvertYZ * TO_NDC(RightTop);

    Dest[StartingIndex++] = InvertYZ * TO_NDC(RightBottom);
    Dest[StartingIndex++] = InvertYZ * TO_NDC(RightTop);
    Dest[StartingIndex++] = InvertYZ * TO_NDC(LeftBottom);
    #undef TO_NDC
  }
}

link_internal clip_result
BufferTexturedQuad( renderer_2d *Group,
                    s32 TextureSlice,
                    v2 MinP,
                    v2 Dim,
                    rect2 UV,
                    v3 Color,
                    r32 Z,
                    rect2  Clip,
                    rect2 *ClipOptional )
{
  textured_2d_geometry_buffer* Geo = &Group->TextGroup->Geo;

  rect2 WindowClip = RectMinMax(V2(0), *Group->ScreenDim);
  clip_result Result = ClipRect3AgainstRect2(MinP, Dim, Z, &UV, WindowClip);
  if (Result.ClipStatus != ClipStatus_FullyClipped)
  {
    Result = ClipRect3AgainstRect2(Result.ClippedMin, Result.ClippedMax - Result.ClippedMin, Z, &UV, Clip);
    if (Result.ClipStatus != ClipStatus_FullyClipped && ClipOptional)
    {
      Result = ClipRect3AgainstRect2(Result.ClippedMin, Result.ClippedMax-Result.ClippedMin, Z, &UV, *ClipOptional);
    }
  }

  switch (Result.ClipStatus)
  {
    case ClipStatus_NoClipping:
    case ClipStatus_PartialClipping:
    {
      // @streaming_ui_render_memory
      Assert(BufferHasRoomFor(Geo, u32_COUNT_PER_QUAD));

      BufferQuadDirect(Geo, Result.ClippedMin, Result.ClippedMax-Result.ClippedMin, Z, Group->ScreenDim);
      BufferQuadUVs(Geo, UV, TextureSlice);
      BufferColors(Group, Geo, Color);
      Geo->At += u32_COUNT_PER_QUAD;
    } break;

    case ClipStatus_FullyClipped:
    {
    } break;

    InvalidDefaultCase;
  }


  return Result;
}

link_internal void
DrawTexturedQuadImmediate( renderer_2d *Group,
                           texture *Tex,
                           v2 MinP, v2 Dim, rect2 UV, v3 Color, r32 Z,
                           rect2 Clip, rect2 *ClipOptional )
{
}

link_internal clip_result
BufferUntexturedQuad(renderer_2d *Group, untextured_2d_geometry_buffer *Geo,
                     v2 MinP, v2 Dim, v3 Color, r32 Z, rect2 Clip)
{
  // @streaming_ui_render_memory
  Assert(BufferHasRoomFor(Geo, u32_COUNT_PER_QUAD));

  clip_result Result = ClipRect3AgainstRect2(MinP, Dim, Z, 0, Clip);
  switch (Result.ClipStatus)
  {
    case ClipStatus_NoClipping:
    case ClipStatus_PartialClipping:
    {
      BufferQuadDirect(Geo, Result.ClippedMin, Result.ClippedMax-Result.ClippedMin, Z, Group->ScreenDim);
      BufferColors(Group, Geo, Color);
      Geo->At += u32_COUNT_PER_QUAD;
    } break;

    case ClipStatus_FullyClipped:
    {
    } break;

    InvalidDefaultCase;
  }

  return Result;
}

link_internal clip_result
BufferUntexturedQuad(renderer_2d *Group, untextured_2d_geometry_buffer *Geo,
                         rect2 Rect, v3 Color, r32 Z, rect2 Clip)
{
  v2 MinP = Rect.Min;
  v2 Dim = Rect.Max - Rect.Min;
  clip_result Result = BufferUntexturedQuad(Group, Geo, MinP, Dim, Color, Z, Clip);
  return Result;
}

link_internal void
BufferChar(renderer_2d *Group, u8 Char, v2 MinP, v2 FontSize, v3 Color, r32 Z, rect2 ClipWindow, rect2 *ClipOptional)
{
  rect2 UV = UVsForChar(Char);

  // Lightly text gets a dark shadow, dark text gets a light shadow
  v3 ShadowColor = V3(0.1f);
  if (Length(Color) < 0.6f)
  {
    ShadowColor = V3(0.9f);
  }

  v2 ShadowOffset = 0.075f*FontSize;
  BufferTexturedQuad( Group, DebugTextureArraySlice_Font,
                      MinP+ShadowOffset, FontSize, UV, ShadowColor, Z, ClipWindow, ClipOptional);

  BufferTexturedQuad( Group, DebugTextureArraySlice_Font,
                      MinP, FontSize, UV, Color, Z, ClipWindow, ClipOptional);
}

#if 0
link_internal void
BufferChar(renderer_2d *Group, u8 Char, v2 MinP, v2 FontSize, u32 Color, r32 Z, rect2 ClipWindow, rect2 *ClipOptional)
{
  v3 ColorVector = GetColorData(Color);
  BufferChar(Group, Char, MinP, FontSize, ColorVector, Z, ClipWindow, ClipOptional);
}
#endif

link_internal void
BufferBorder(renderer_2d *Group, rect2 Rect, v3 Color, r32 Z, rect2 Clip, v4 Thickness = V4(1))
{
  v2 TopLeft     = Rect.Min;
  v2 BottomRight = Rect.Max;
  v2 TopRight    = V2(Rect.Max.x, Rect.Min.y);
  v2 BottomLeft  = V2(Rect.Min.x, Rect.Max.y);

  rect2 TopRect    = RectMinMax(TopLeft ,    TopRight    + V2(0, Thickness.Top));
  rect2 BottomRect = RectMinMax(BottomLeft,  BottomRight - V2(0, Thickness.Bottom));
  rect2 LeftRect   = RectMinMax(TopLeft ,    BottomLeft  + V2(Thickness.Left, 0));
  rect2 RightRect  = RectMinMax(TopRight,    BottomRight - V2(Thickness.Right, 0));

  BufferUntexturedQuad(Group, &Group->Geo, TopRect,    Color, Z, Clip);
  BufferUntexturedQuad(Group, &Group->Geo, LeftRect,   Color, Z, Clip);
  BufferUntexturedQuad(Group, &Group->Geo, RightRect,  Color, Z, Clip);
  BufferUntexturedQuad(Group, &Group->Geo, BottomRect, Color, Z, Clip);
}

link_internal void
BufferBorder(renderer_2d *Group, interactable* PickerListInteraction, v3 Color, r32 Z, rect2 Clip, v4 Thickness = V4(1))
{
  rect2 Bounds = RectMinMax(PickerListInteraction->MinP, PickerListInteraction->MaxP);
  BufferBorder(Group, Bounds, Color, Z, Clip, Thickness);
  return;
}

link_internal void
AdvanceLayoutStackBy(v2 Delta, layout* Layout)
{
  while (Layout)
  {
    Layout->At += Delta;
    UpdateDrawBounds(Layout);
    Layout = Layout->Prev;
  }
}

link_internal void
BufferValue(counted_string Text, v2 AbsAt, renderer_2d *Group, layout* Layout, v3 Color, ui_style* Style, r32 Z, rect2 ClipWindow, rect2 *ClipOptional, text_render_params Params, b32 DoBuffering = True)
{
  r32 xDelta = 0;
  /* v2 MinP = GetAbsoluteAt(Layout) + V2(xDelta, 0); */

  for ( u32 CharIndex = 0;
            CharIndex < Text.Count;
          ++CharIndex )
  {
    v2 AbsMinP = AbsAt + V2(xDelta, 0);

    if (DoBuffering)
    {
      BufferChar(Group, (u8)Text.Start[CharIndex], AbsMinP, Style->Font.Size, Color, Z, ClipWindow, ClipOptional);
    }

    xDelta += Style->Font.Size.x;
  }

  if ( (Params & TextRenderParam_NoAdvanceLayout) == False)
  {
    AdvanceLayoutStackBy(V2(xDelta, 0), Layout);
    v2 MaxP = Layout->At + V2(0, Style->Font.Size.y);
    UpdateDrawBounds(Layout, MaxP);
  }

  if (GetUiDebug && GetUiDebug()->OutlineUiValues)
  {
    v2 StartingP = AbsAt;
    v2 EndingP = AbsAt + V2(xDelta, Style->Font.Size.y);

    rect2 AbsBounds = RectMinMax(StartingP, EndingP);
    BufferBorder(Group, AbsBounds, V3(0, 0, 1), Z, DISABLE_CLIPPING);

    if (GetUiDebug && GetUiDebug()->DebugBreakOnElementClick)
    {
      if ( IsInsideRect(AbsBounds, *Group->MouseP) && Group->Input->LMB.Clicked ) { RuntimeBreak(); }
    }
  }

  return;
}

link_internal void
BufferValue(counted_string Text, renderer_2d *Group, render_state* RenderState, ui_style* Style, rect2 ClipOptional, text_render_params RenderParams, b32 DoBuffering = True)
{
  layout* Layout = RenderState->Layout;

  r32 Z      = GetZ(zDepth_Text, RenderState->Window);
  v3 Color   = SelectColorState(RenderState, Style);
  v2 AbsAt   = GetAbsoluteAt(Layout);
  rect2 Clip = RenderState->ClipRect; //GetAbsoluteClip(RenderState->Window);

  rect2 *ClipOptionalPtr = 0;
  if (Area(ClipOptional) > 0.f)
  {
    ClipOptional += GetAbsoluteAt(Layout);
    ClipOptionalPtr = &ClipOptional;
  }


  // NOTE(Jesse): This is weird, but it's how I wanted the phrasing on the API
  // side, so we have this double-negative check here.
  /* if ((RenderParams & TextRenderParam_NoScroll) == 0) */
  {
    /* AbsAt -= Scroll; */
  }

  if (RenderParams & TextRenderParam_DisableClipping)
  {
    Clip = DISABLE_CLIPPING;
    ClipOptionalPtr = 0;
  }

  BufferValue(Text, AbsAt, Group, Layout, Color, Style, Z, Clip, ClipOptionalPtr, RenderParams, DoBuffering);
}

link_internal void
BufferTextAt(renderer_2d *Group, v2 BasisP, counted_string Text, v2 FontSize, v3 Color, r32 Z, rect2 Clip, b32 DoBuffering = True)
{
  if (DoBuffering)
  {
    for ( u32 CharIndex = 0;
        CharIndex < Text.Count;
        CharIndex++ )
    {
      v2 MinP = BasisP + V2(FontSize.x*CharIndex, 0);
      BufferChar(Group, (u8)Text.Start[CharIndex], MinP, FontSize, Color, Z, Clip, 0);
    }
  }
}



// TODO(Jesse, id: 104, tags: cleanup, robustness): Test this actually gets respected!!
#define INVALID_RENDER_COMMAND_INDEX (u32_MAX)

link_internal u32
PushUiRenderCommand(renderer_2d *Group, ui_render_command* Command)
{
  ui_render_command_buffer *CommandBuffer = Group->CommandBuffer;

  u32 Result = CommandBuffer->CommandCount;

  if (CommandBuffer->CommandCount < MAX_UI_RENDER_COMMAND_COUNT)
  {
    CommandBuffer->Commands[CommandBuffer->CommandCount++] = *Command;
  }
  else
  {
    // TODO(Jesse, id: 105, tags: id_104, cleanup, robustness): Test this actually gets respected!!
    Result = INVALID_RENDER_COMMAND_INDEX;
    Error("Exhausted RenderCommandBuffer Space!");
  }

  return Result;
}

link_internal void
PushDebugCommand(renderer_2d *Group)
{
  ui_render_command Command = {
    .Type = type_ui_render_command_debug,
  };

  PushUiRenderCommand(Group, &Command);

  // NOTE(Jesse): This doesn't really work because the thing that toggles DebugBreakUiCommand happens last in the UI ..
  /* if (GetUiDebug && GetUiDebug()->DebugBreakUiCommand) { RuntimeBreak(); } */
}

link_internal void
PushNewRow(renderer_2d *Group)
{
  ui_render_command Command = {
    .Type = type_ui_render_command_new_row
  };

  PushUiRenderCommand(Group, &Command);

  return;
}

link_internal void
Text(renderer_2d* Group, counted_string String, ui_style *Style = &DefaultStyle, text_render_params RenderParams = TextRenderParam_Default, v2 Offset = {}, rect2 Clip = {})
{
  ui_render_command Command = {
    .Type = type_ui_render_command_text,
    .ui_render_command_text = {
      .Layout = {},
      .Style = *Style,
      .String = String,
      .Offset = Offset,
      .Clip = Clip,
      .Params = RenderParams,
    }
  };

  PushUiRenderCommand(Group, &Command);

  return;
}

link_internal u32
StartColumn(renderer_2d *Group, ui_style* Style = 0, v4 Padding = V4(0), column_render_params Params = ColumnRenderParam_RightAlign)
{
  ui_render_command Command = {
    .Type = type_ui_render_command_column_start,
    .ui_render_command_column_start = {
      .Layout = {},
      .Style  = Style? *Style : DefaultStyle,
      .Width = 0.f,
      .MaxWidth = 0.f,
      .Params = Params,
    }
  };

  Command.ui_render_command_column_start.Layout.Padding = Padding;

  Assert(Command.ui_render_command_column_start.Layout.DrawBounds.Min == InvertedInfinityRectangle().Min);
  Assert(Command.ui_render_command_column_start.Layout.DrawBounds.Max == InvertedInfinityRectangle().Max);

  u32 Result = PushUiRenderCommand(Group, &Command);
  return Result;
}

link_internal void
EndColumn(renderer_2d* Group, u32 StartCommandIndex)
{
  ui_render_command Command = {
    .Type = type_ui_render_command_column_end,
    .ui_render_command_column_end.StartCommandIndex = StartCommandIndex,
  };

  PushUiRenderCommand(Group, &Command);

  return;
}

link_internal void
PushColumn(renderer_2d *Group, counted_string String, ui_style* Style = &DefaultStyle, v4 Padding = DefaultColumnPadding, column_render_params Params = ColumnRenderParam_RightAlign)
{
  u32 StartIndex = StartColumn(Group, Style, Padding, Params);
    Text(Group, String, Style);
  EndColumn(Group, StartIndex);
}

link_internal void
PushColumn(renderer_2d *Group, counted_string String, column_render_params Params)
{
  u32 StartIndex = StartColumn(Group, &DefaultStyle, DefaultColumnPadding, Params);
    Text(Group, String, &DefaultStyle);
  EndColumn(Group, StartIndex);
}

link_internal void
PushTextAt(renderer_2d *Group, counted_string Text, v2 At, rect2 Clip, font *Font = &Global_Font)
{
  ui_render_command Command = {
    .Type = type_ui_render_command_text_at,

    .ui_render_command_text_at.Text    = Text,
    .ui_render_command_text_at.At      = At,
    .ui_render_command_text_at.Clip    = Clip,
    /* .ui_render_command_text_at.Font    = Font, */
  };

  PushUiRenderCommand(Group, &Command);

  return;
}

link_internal void
PushTooltip(renderer_2d *Group, counted_string Text)
{
  PushTextAt(Group, Text, *Group->MouseP+V2(14, -7), DISABLE_CLIPPING);
  return;
}

link_internal void
PushTexturedQuad(renderer_2d *Group, texture *Texture, s32 TextureSlice, v2 Dim, z_depth zDepth, v3 Tint = V3(1), quad_render_params Params = QuadRenderParam_Default )
{
  Assert(Texture->Slices > 1);
  ui_render_command Command = {
    .Type = type_ui_render_command_textured_quad,

    .ui_render_command_textured_quad.Params = Params,
    .ui_render_command_textured_quad.TextureSlice = TextureSlice,
    .ui_render_command_textured_quad.Texture = Texture,
    .ui_render_command_textured_quad.QuadDim = Dim,
    .ui_render_command_textured_quad.zDepth = zDepth,
    .ui_render_command_textured_quad.Tint = Tint,
    .ui_render_command_textured_quad.IsDepthTexture = Texture->IsDepthTexture,
    .ui_render_command_textured_quad.HasAlphaChannel = (Texture->Channels == 4),
    .ui_render_command_textured_quad.Layout =
    {
      .DrawBounds = InvertedInfinityRectangle(),
    }
  };

  PushUiRenderCommand(Group, &Command);
}

#if 1
link_internal void
PushTexturedQuad(renderer_2d *Group, texture *Texture, v2 Dim, z_depth zDepth, v3 Tint = V3(1), quad_render_params Params = QuadRenderParam_Default )
{
  Assert(Texture->Slices == 1);
  ui_render_command Command = {
    .Type = type_ui_render_command_textured_quad,

    .ui_render_command_textured_quad.Params = Params,
    .ui_render_command_textured_quad.TextureSlice = -1,
    .ui_render_command_textured_quad.Texture = Texture,
    .ui_render_command_textured_quad.QuadDim = Dim,
    .ui_render_command_textured_quad.zDepth = zDepth,
    .ui_render_command_textured_quad.Tint = Tint,
    .ui_render_command_textured_quad.IsDepthTexture = Texture->IsDepthTexture,
    .ui_render_command_textured_quad.HasAlphaChannel = (Texture->Channels == 4),
    .ui_render_command_textured_quad.Layout =
    {
      .DrawBounds = InvertedInfinityRectangle(),
    }
  };

  PushUiRenderCommand(Group, &Command);

  return;
}
#endif

link_internal void
PushTexturedQuadColumn(renderer_2d *Group, texture *Texture, s32 TextureSlice, v2 Dim, z_depth zDepth, v3 Tint = V3(1), quad_render_params Params = QuadRenderParam_Default )
{
  Assert(TextureSlice >= 0);
  u32 Start = StartColumn(Group);
    PushTexturedQuad(Group, Texture, TextureSlice, Dim, zDepth, Tint, Params);
  EndColumn(Group, Start);
}

link_internal void
PushUntexturedQuadAt(renderer_2d* Group, v2 Offset, v2 QuadDim, z_depth zDepth, ui_style *Style = 0 )
{
  ui_render_command Command =
  {
    .Type = type_ui_render_command_untextured_quad_at,
    .ui_render_command_untextured_quad_at =
    {
      .QuadDim = QuadDim,
      .zDepth  = zDepth,
      .Style = Style? *Style : DefaultStyle,
      .Layout  =
      {
        .At         = Offset,
        .DrawBounds = InvertedInfinityRectangle(),
      }
    }
  };

  PushUiRenderCommand(Group, &Command);
}

link_internal void
PushUntexturedQuad(renderer_2d* Group, v2 Offset, v2 QuadDim, z_depth zDepth, ui_style *Style = 0, v4 Padding = V4(0), quad_render_params Params = QuadRenderParam_Default )
{
  ui_render_command Command = {
    .Type = type_ui_render_command_untextured_quad,

    .ui_render_command_untextured_quad =
    {
      .QuadDim = QuadDim,
      .Params  = Params,
      .zDepth  = zDepth,
      .Style   = Style? *Style : DefaultStyle,
      .Layout  =
      {
        .Basis   = Offset,
        .Padding = Padding,
        .DrawBounds = InvertedInfinityRectangle(),
      },
    }
  };

  PushUiRenderCommand(Group, &Command);

  return;
}

link_internal void
PushButtonEnd(renderer_2d *Group)
{
  ui_render_command Command = {
    .Type = type_ui_render_command_button_end,
  };
  PushUiRenderCommand(Group, &Command);
  return;
}

link_internal interactable_handle
PushButtonStart(renderer_2d *Group, ui_id InteractionId, ui_style* Style = 0)
{
  ui_render_command Command = {
    .Type = type_ui_render_command_button_start,
    .ui_render_command_button_start.ID = InteractionId,
    .ui_render_command_button_start.Style = Style ? *Style : DefaultStyle,
  };

  PushUiRenderCommand(Group, &Command);

  interactable_handle Handle = {
    .Id = InteractionId
  };

  return Handle;
}

link_internal ui_element_reference
PushTableStart(renderer_2d* Group, relative_position Position = Position_None,  ui_element_reference RelativeTo = {}, v2 Offset = {}, ui_style *Style = &DefaultStyle, v4 Padding = {})
{
  ui_render_command Command = {
    .Type = type_ui_render_command_table_start,

    .ui_render_command_table_start.RelativeTo = RelativeTo,
    .ui_render_command_table_start.Position = Position,
    .ui_render_command_table_start.Style = *Style,
    .ui_render_command_table_start.Layout =
    {
      .Basis   = Offset,
      .Padding = Padding,
      .DrawBounds = InvertedInfinityRectangle(),
    }
  };

  u32 ElementIndex = PushUiRenderCommand(Group, &Command);

  ui_element_reference Result = {
    .Index = ElementIndex,
  };

  return Result;
}

link_internal void
PushTableEnd(renderer_2d *Group)
{
  ui_render_command Command = {
    .Type = type_ui_render_command_table_end,
  };

  PushUiRenderCommand(Group, &Command);

  return;
}

link_internal void
PushRelativeBorder(renderer_2d *Group, v2 Dim, v3 Color, v4 Thickness = V4(1))
{
  ui_render_command Command = {
    .Type = type_ui_render_command_rel_border,
    .ui_render_command_rel_border.Dim = Dim,
    .ui_render_command_rel_border.Color = Color,
    .ui_render_command_rel_border.Thickness = Thickness,
  };

  PushUiRenderCommand(Group, &Command);
}
link_internal void
PushBorder(renderer_2d *Group, rect2 AbsoluteBounds, v3 Color, v4 Thickness = V4(1))
{
  ui_render_command Command = {
    .Type = type_ui_render_command_abs_border,
    .ui_render_command_abs_border.Bounds = AbsoluteBounds,
    .ui_render_command_abs_border.Color = Color,
    .ui_render_command_abs_border.Thickness = Thickness,
  };

  PushUiRenderCommand(Group, &Command);
}

link_internal void
PushResetDrawBounds(renderer_2d *Group)
{
  ui_render_command Command = {
    .Type = type_ui_render_command_reset_draw_bounds,
  };
  PushUiRenderCommand(Group, &Command);
  return;
}

link_internal void
PushForceUpdateBasis(renderer_2d *Group, v2 Offset)
{
  ui_render_command Command = {
    .Type = type_ui_render_command_force_update_basis,
    .ui_render_command_force_update_basis.Offset = Offset
  };
  PushUiRenderCommand(Group, &Command);
  return;
}

link_internal void
PushForceAdvance(renderer_2d *Group, v2 Offset)
{
  ui_render_command Command = {
    .Type = type_ui_render_command_force_advance,
    .ui_render_command_force_advance.Offset = Offset,
  };
  PushUiRenderCommand(Group, &Command);
  return;
}

link_internal rect2
GetDrawBounds(counted_string String, ui_style *Style)
{
  r32 xMax = (String.Count * Style->Font.Size.x);
  rect2 Result =  RectMinMax(V2(0.f, 0.f), V2(xMax, Style->Font.Size.y));
  return Result;
}

link_internal void
PushBorderlessWindowStart( renderer_2d *Group, window_layout *Window, v2 WindowMaxClip = V2(f32_MAX))
{
  rect2 AbsWindowBounds = RectMinDim(Window->Basis, WindowMaxClip);
  /* rect2 ClipRect = RectMinMax(AbsWindowBounds.Min + V2(0, Global_TitleBarHeight), AbsWindowBounds.Max); */
  rect2 ClipRect = RectMinMax(AbsWindowBounds.Min, AbsWindowBounds.Max);

  ui_render_command Command = {
    .Type = type_ui_render_command_window_start,

    .ui_render_command_window_start = {
      .Window = Window,
      .ClipRect = ClipRect,
      .Layout = {
        .Basis = Window->Basis,
        .DrawBounds = InvertedInfinityRectangle(),
      }
    }
  };

  PushUiRenderCommand(Group, &Command);
}

link_internal void
PushWindowStartInternal( renderer_2d *Group,
                         window_layout *Window,
                         cs TitleText,
                         cs MinimizedIcon,
                         ui_id ResizeHandleInteractionId,
                         ui_id MinimizeInteractionId,
                         ui_id TitleBarInteractionId,
                         v2 WindowResizeHandleMin,
                         v2 WindowResizeHandleDim,
                         v2 MinimizeButtonOffset,
                         v2 WindowBasis,
                         v2 WindowMaxClip,
                         v2 WindowScroll )
{
  rect2 AbsWindowBounds = RectMinDim(WindowBasis, WindowMaxClip);
  rect2 ClipRect = RectMinMax(AbsWindowBounds.Min + V2(0, Global_TitleBarHeight), AbsWindowBounds.Max);

  ui_render_command Command = {
    .Type = type_ui_render_command_window_start,

    .ui_render_command_window_start = {
      .Window = Window,
      .ClipRect = ClipRect,
      .Layout = {
        .Basis = WindowBasis,
        .DrawBounds = InvertedInfinityRectangle(),
      }
    }
  };

  PushUiRenderCommand(Group, &Command);

  ui_style TitleBarStyle = UiStyleFromLightestColor(UI_WINDOW_BEZEL_DEFAULT_COLOR);

  // NOTE(Jesse): Must come first to take precedence over the title bar when clicking
  PushButtonStart(Group, ResizeHandleInteractionId);
    PushUntexturedQuadAt(Group, WindowResizeHandleMin, WindowResizeHandleDim, zDepth_Border, &TitleBarStyle);
  PushButtonEnd(Group);

  PushForceAdvance(Group, V2(Global_TitleBarPadding));

  Text(Group, TitleText, &DefaultStyle, TextRenderParam_DisableClipping );

  if (!Window->Minimized)
  {
    PushButtonStart(Group, MinimizeInteractionId);
      Text(Group, MinimizedIcon, &DefaultStyle, TextRenderParam_DisableClipping, MinimizeButtonOffset );
    PushButtonEnd(Group);
  }

  PushButtonStart(Group, TitleBarInteractionId);
    PushUntexturedQuadAt(Group, WindowBasis, V2(WindowMaxClip.x, Global_TitleBarHeight), zDepth_TitleBar, &TitleBarStyle);
  PushButtonEnd(Group);

  PushBorder(Group, AbsWindowBounds, TitleBarStyle.HoverColor, UI_WINDOW_BORDER_DEFAULT_WIDTH);

  ui_style BackgroundStyle = UiStyleFromLightestColor(UI_WINDOW_BACKGROUND_DEFAULT_COLOR);
  PushUntexturedQuadAt(Group, WindowBasis, WindowMaxClip, zDepth_Background, &BackgroundStyle);
  PushNewRow(Group);

/*   PushResetDrawBounds(Group); */

  PushForceUpdateBasis(Group, V2(UI_WINDOW_BORDER_DEFAULT_WIDTH.Left, UI_WINDOW_BORDER_DEFAULT_WIDTH.Top)*2.f);
  PushForceUpdateBasis(Group, WindowScroll);

  PushResetDrawBounds(Group);
}

link_internal void
UnminimizeWindow(renderer_2d *Group, window_layout *Window)
{
  Window->Minimized = False;

  Group->MinimizedWindowBuffer[Window->MinimizeIndex] = 0;
  Window->MinimizeIndex = 0;

  Window->Flags = Window->CachedFlags;
  Window->Basis = Window->CachedBasis;
  Window->MaxClip = Window->CachedMaxClip;
  Window->Scroll = Window->CachedScroll;
}

link_internal void
PushWindowStart(renderer_2d *Group, window_layout *Window)
{
  TIMED_FUNCTION();

  v2 ResizeHandleDim = V2(20);
  /* counted_string TitleText = FCS(CSz("%S (%u) (%.1f, %.1f)"), Window->Title, Window->InteractionStackIndex, double(Window->Scroll.x), double(Window->Scroll.y) ); */
  counted_string TitleText = Window->Title;
  counted_string MinimizedIcon = CSz(" _ ");
  rect2 TitleRect = GetDrawBounds(TitleText, &DefaultStyle);
  rect2 MinimizeRect = GetDrawBounds(MinimizedIcon, &DefaultStyle);

  ui_id TitleBarInteractionId = UiId(Window, Cast(void*, "WindowTitleBar"), 0);
  interactable_handle TitleBarHandle = { .Id = TitleBarInteractionId };


  ui_id MinimizeInteractionId = UiId(Window, Cast(void*, "WindowMinimizeInteraction"), 0);
  interactable_handle MinimizeButtonHandle = { .Id = MinimizeInteractionId };

  if (Window->Minimized && Clicked(Group, &TitleBarHandle))
  {
    UnminimizeWindow(Group, Window);
  }
  else if (Pressed(Group, &TitleBarHandle))
  {
    Window->Basis -= *Group->MouseDP; // TODO(Jesse, id: 107, tags: cleanup, speed): Can we compute this with MouseP to avoid a frame of input delay?
    Window->Flags &= ~(WindowLayoutFlag_StartupAlign_Right|WindowLayoutFlag_StartupAlign_Bottom);
  }
  else if (!Window->Minimized && Clicked(Group, &MinimizeButtonHandle))
  {
    Window->Minimized = True;

    for (u32 MinimizeIndex = 0; MinimizeIndex < MAX_MINIMIZED_WINDOWS; ++MinimizeIndex)
    {
      window_layout **Slot = Group->MinimizedWindowBuffer + MinimizeIndex;
      if (*Slot == 0)
      {
        *Slot = Window;
        Window->MinimizeIndex = MinimizeIndex;
        break;
      }
    }

    Window->CachedFlags = Window->Flags;
    Window->CachedBasis = Window->Basis;
    Window->CachedMaxClip = Window->MaxClip;
    Window->CachedScroll = Window->Scroll;

    rect2 MinimizedTitleBarBounds = RectMinDim({}, V2(TitleRect.Max.x, Global_TitleBarHeight));

    v2 WindowOffsetFromCornerOfScreen = V2(20);
    v2 WindowDim  = MinimizedTitleBarBounds.Max + V2(ResizeHandleDim.x, 0) + V2(20, 0);
    v2 WindowBasis = V2(Group->ScreenDim->x - WindowDim.x - WindowOffsetFromCornerOfScreen.x, (Window->MinimizeIndex * Global_TitleBarHeight) + WindowOffsetFromCornerOfScreen.y );

    Window->Flags = WindowLayoutFlag_StartupAlign_Right;
    Window->Basis = WindowBasis;
    Window->MaxClip = WindowDim;
  }


  ui_id ResizeHandleInteractionId = UiId(Window, Cast(void*, "WindowResizeWidget"), Window);
  interactable_handle ResizeHandle = { .Id = ResizeHandleInteractionId };

  v2 TitleBounds = V2(Window->Title.Count*Global_Font.Size.x, Global_Font.Size.y);
  Window->MaxClip = Max(TitleBounds, Window->MaxClip);

  if (Pressed(Group, &ResizeHandle))
  {
    Window->Flags &= ~(WindowLayoutFlag_DynamicSize|WindowLayoutFlag_StartupAlign_Right|WindowLayoutFlag_StartupAlign_Bottom);

    v2 AbsoluteTitleBounds = Window->Basis + TitleBounds;
    v2 TestMaxClip = *Group->MouseP - Window->Basis;

    if (Group->MouseP->x > AbsoluteTitleBounds.x )
    {
      Window->MaxClip.x = Max(TitleBounds.x, TestMaxClip.x);
    }
    else
    {
      Window->MaxClip.x = TitleBounds.x;
    }

    if (Group->MouseP->y > AbsoluteTitleBounds.y )
    {
      Window->MaxClip.y = Max(TitleBounds.y, TestMaxClip.y);
    }
    else
    {
      Window->MaxClip.y = TitleBounds.y;
    }
  }

  Window->MaxClip.x = Max(Window->MaxClip.x, TitleRect.Max.x + MinimizeRect.Max.x + MinimizeRect.Max.x + 50);

  f32 KeepOnTheScreenThreshold = 30.f;
  Window->Basis = Max(V2(-(Window->MaxClip.x-KeepOnTheScreenThreshold), 0.f), Window->Basis);
  Window->Basis = Min(*Group->ScreenDim-V2(KeepOnTheScreenThreshold), Window->Basis);

  PushWindowStartInternal( Group,
                           Window,
                           TitleText,
                           MinimizedIcon,
                           ResizeHandleInteractionId,
                           MinimizeInteractionId,
                           TitleBarInteractionId,
                           GetAbsoluteMaxClip(Window)-ResizeHandleDim,
                           ResizeHandleDim,
                           V2(Window->MaxClip.x-TitleRect.Max.x-50, 0),
                           Window->Basis,
                           Window->MaxClip,
                           Window->Scroll );
}


link_internal void
PushWindowEnd(renderer_2d *Group, window_layout *Window)
{
  ui_render_command EndCommand = {};
  EndCommand.Type = type_ui_render_command_window_end;
  EndCommand.ui_render_command_window_end.Window = Window;
  PushUiRenderCommand(Group, &EndCommand);
  return;
}



/*********************************           *********************************/
/*********************************  Buttons  *********************************/
/*********************************           *********************************/



link_internal button_interaction_result
ButtonInteraction(renderer_2d* Group, rect2 Bounds, ui_id InteractionId, window_layout *Window, ui_style *Style)
{
  button_interaction_result Result = {};

  interactable Interaction = Interactable(Bounds, InteractionId, Window);
  Result.Interaction = Interaction;

  if (GetUiDebug && GetUiDebug()->OutlineUiButtons)
  {
    BufferBorder(Group, Rect2(Interaction), V3(1,0,0), 1.0f, DISABLE_CLIPPING);

    if (GetUiDebug && GetUiDebug()->DebugBreakOnElementClick)
    {
      if ( IsInsideRect(Rect2(Interaction), *Group->MouseP) && Group->Input->LMB.Clicked ) { RuntimeBreak(); }
    }
  }

  if (Hover(Group, &Interaction))
  {
    Result.Hover = True;
    Style->Color = Style->HoverColor;
  }

  if (Clicked(Group, &Interaction))
  {
    Result.Clicked = True;
    Style->Color = Style->ClickedColor;
  }

  if (Pressed(Group, &Interaction))
  {
    Result.Pressed = True;
    Style->Color = Style->ClickedColor;
  }

  /* if (Style->IsActive && !Result.Pressed) */
  /* { */
  /*   Style->Color = Style->ActiveColor; */
  /* } */

  return Result;
}

link_internal b32
Button(renderer_2d* Group, counted_string ButtonName, ui_id ButtonId, ui_style* Style = &DefaultStyle, v4 Padding = DefaultButtonPadding, column_render_params ColumnParams = ColumnRenderParam_RightAlign)
{
  // TODO(Jesse, id: 108, tags: cleanup, potential_bug): Do we have to pass the style to both of these functions, and is that a good idea?
  interactable_handle Button = PushButtonStart(Group, ButtonId, Style);
    PushColumn(Group, ButtonName, Style, Padding, ColumnParams);
  PushButtonEnd(Group);

  b32 Result = Clicked(Group, &Button);
  return Result;
}

link_internal b32
ToggleButton(renderer_2d* Group, cs ButtonNameOn, cs ButtonNameOff, ui_id InteractionId, ui_style* Style = &DefaultStyle, v4 Padding = DefaultButtonPadding, column_render_params ColumnParams = ColumnRenderParam_RightAlign)
{
  interactable_handle Handle = {
    .Id = InteractionId
  };

  b32 Result = ToggledOn(Group, &Handle);

  if (Result && Style == &DefaultStyle)
  {
    Style = &DefaultSelectedStyle;
  }

  ui_render_command StartCommand = {
    .Type = type_ui_render_command_button_start,
    .ui_render_command_button_start.ID = InteractionId,
    .ui_render_command_button_start.Style = Style ? *Style : DefaultStyle,
    .ui_render_command_button_start.Params = ButtonParam_ToggleButton,
  };

  PushUiRenderCommand(Group, &StartCommand);

  cs NameToUse = Result ? ButtonNameOn : ButtonNameOff;
  PushColumn(Group, NameToUse, Style, Padding, ColumnParams);

  ui_render_command EndCommand = {
    .Type = type_ui_render_command_button_end,
  };
  PushUiRenderCommand(Group, &EndCommand);

  return Result;
}



/**************************                      *****************************/
/************************** Composite Structures *****************************/
/**************************                      *****************************/



link_internal void
PushSliderBar(debug_ui_render_group *Group, r32 PercFilled, v3 FColor, v3 BColor, r32 BarWidth, r32 *BarHeight = &Global_Font.Size.y)
{
  PercFilled = Clamp01(PercFilled);

  v2 BackgroundQuadDim = V2(BarWidth, *BarHeight);
  v2 SliderQuadDim = V2(1.f, *BarHeight);

  r32 SliderOffset = (BackgroundQuadDim.x*PercFilled) - BackgroundQuadDim.x - (SliderQuadDim.x/2.f);

  ui_style Style = UiStyleFromLightestColor(BColor);
  PushUntexturedQuad(Group, V2(0), BackgroundQuadDim, zDepth_TitleBar, &Style);

  Style = UiStyleFromLightestColor(FColor);
  PushUntexturedQuad(Group, V2(SliderOffset, 0.f), SliderQuadDim, zDepth_TitleBar, &Style, V4(0), QuadRenderParam_NoAdvance);
}

link_internal void
PushBargraph(debug_ui_render_group *Group, r32 PercFilled, v3 FColor, v3 BColor, r32 BarWidth, r32 *BarHeight = &Global_Font.Size.y)
{
  PercFilled = Clamp01(PercFilled);

  v2 BackgroundQuadDim = V2(BarWidth, *BarHeight);
  v2 ShadedQuadDim = BackgroundQuadDim * V2(PercFilled, 1);

  v2 UnshadedQuadDim = V2(BackgroundQuadDim.x - ShadedQuadDim.x, BackgroundQuadDim.y);

  ui_style Style = UiStyleFromLightestColor(FColor);
  PushUntexturedQuad(Group, V2(0), ShadedQuadDim, zDepth_TitleBar, &Style);

  Style = UiStyleFromLightestColor(BColor);
  PushUntexturedQuad(Group, V2(0), UnshadedQuadDim, zDepth_TitleBar, &Style);
}

link_internal b32
Clicked(ui_toggle_button_group *Group, cs ButtonName)
{
  renderer_2d *Ui = Group->Ui;

  b32 Result = False;
  IterateOver(&Group->Buttons, Button, ButtonIndex)
  {
    if (StringsMatch(Button->Text, ButtonName))
    {
      interactable_handle Handle = {Button->Id};
      Result = Clicked(Group->Ui, &Handle);
      break;
    }
  }
  return Result;
}

link_internal void
UnsetAllTogglesExcluding(ui_toggle_button_group *Group, umm ExcludeIndex)
{
  Group->ToggleBits = 0;
  IterateOver(&Group->Buttons, InnerButton, InnerButtonIndex)
  {
    if (InnerButtonIndex == ExcludeIndex) continue;
    maybe_ui_toggle_ptr Maybe = GetPtrById(&Group->Ui->ToggleTable, InnerButton->Id);
    if (Maybe.Tag) { Maybe.Value->ToggledOn = False; }
  }
}

link_internal void
SetRadioButton(ui_toggle_button_group *Group, maybe_ui_toggle_ptr MaybeToggle, ui_toggle_button_handle *ToggleHandle, b32 Value)
{
  UnsetAllTogglesExcluding(Group, umm_MAX);

  if (MaybeToggle.Tag)
  {
    MaybeToggle.Value->ToggledOn = Value;
  }
  else
  {
    ui_toggle E = {ToggleHandle->Id, True};
    Insert(E, &Group->Ui->ToggleTable, &Group->Ui->UiToggleArena);
  }
}

link_internal void
SetRadioButton(ui_toggle_button_group *Group, ui_toggle_button_handle *ToggleHandle, b32 Value)
{
  Assert(Group->Flags & ToggleButtonGroupFlags_RadioButtons);
  maybe_ui_toggle_ptr MaybeInputToggle = GetPtrById(&Group->Ui->ToggleTable, ToggleHandle->Id);
  SetRadioButton(Group, MaybeInputToggle, ToggleHandle, Value);
}

link_internal void
ToggleRadioButton(ui_toggle_button_group *Group, ui_toggle_button_handle *ToggleHandle)
{
  Assert(Group->Flags & ToggleButtonGroupFlags_RadioButtons);
  maybe_ui_toggle_ptr MaybeInputToggle = GetPtrById(&Group->Ui->ToggleTable, ToggleHandle->Id);
  u32 ToggleState = True;
  if (MaybeInputToggle.Tag) { ToggleState = !MaybeInputToggle.Value->ToggledOn; }
  SetRadioButton(Group, MaybeInputToggle, ToggleHandle, ToggleState);
}

link_internal ui_element_reference
DrawToggleButtonGroup(ui_toggle_button_group *Group, UI_FUNCTION_PROTO_NAMES)
{
  renderer_2d *Ui = Group->Ui;
  ui_toggle_button_handle_buffer *ButtonBuffer = &Group->Buttons;

  // Reset this every frame; it's ephermeral.
  //
  // TODO(Jesse): These aren't stored .. we should asesert this ..???
  Group->ToggleBits = 0;

  ui_element_reference Result = PushTableStart(Ui, UI_FUNCTION_INSTANCE_NAMES);
    IterateOver(ButtonBuffer, UiButton, ButtonIndex)
    {
      interactable_handle ButtonHandle = {UiButton->Id};

      // Unset toggle bits and unset all the currently set toggle fields in the toggle table
      if ( BitfieldIsSet(Group->Flags, ToggleButtonGroupFlags_RadioButtons)
           && Clicked(Ui, &ButtonHandle) )
      {
        UnsetAllTogglesExcluding(Group, ButtonIndex);
      }

      ui_style *ThisStyle = ToggledOn(Ui, UiButton) ? &DefaultSelectedStyle : Style;
      if (ToggleButton(Ui, UiButton->Text, UiButton->Text, UiButton->Id, ThisStyle, DefaultToggleButtonPadding))
      {
        Group->ToggleBits |= (1 << ButtonIndex);
      }

      if (Clicked(Ui, &ButtonHandle))
      {
        Group->AnyElementClicked = True;
      }

      if (Group->Flags & ToggleButtonGroupFlags_DrawVertical)
      {
        PushNewRow(Ui);
      }
    }
  PushNewRow(Ui);
  PushTableEnd(Ui);
  return Result;
}

link_internal maybe_file_traversal_node
DrawFileNodes(renderer_2d *Ui,  file_traversal_node Node, filtered_file_traversal_helper_params *Params)
{
  maybe_file_traversal_node Result = {};

  if (Params->FilterFunction(&Node))
  {
    v4 Pad = V4(10, 0, 10, 0);
    switch (Node.Type)
    {
      InvalidCase(FileTraversalType_None);

      case FileTraversalType_File:
      {
        // NOTE(Jesse): The Node strings (Name and Dir) are transient allocated so
        // we have to hash the string to have a stable identifier across frames.
        interactable_handle FileButton = PushButtonStart(Ui, UiId(Params->Window, "DrawFileNodes", Hash(Node.Name) ^ Hash(Node.Dir)) );
          PushColumn(Ui, CSz(" "), &DefaultStyle, Pad);
          PushColumn(Ui, Node.Name);
          PushNewRow(Ui);
        PushButtonEnd(Ui);

        if (Clicked(Ui, &FileButton)) { Result.Tag = Maybe_Yes; Result.Value = Node; }
      } break;

      case FileTraversalType_Dir:
      {
        PushColumn(Ui, CSz("+"), &DefaultStyle, Pad);
        PushColumn(Ui, Node.Name);
        PushNewRow(Ui);
      } break;
    }
  }

  return Result;
}


/**************************                      *****************************/
/**************************  Command Processing  *****************************/
/**************************                      *****************************/



#define GetCommandAs(TypeName, CommandBuffer, CommandIndex)                     \
  &(GetCommand((CommandBuffer), (CommandIndex))->ui_render_command_##TypeName); \
  do {ui_render_command* TempCommand = GetCommand((CommandBuffer), (CommandIndex)); \
  Assert(TempCommand->Type == type_ui_render_command_##TypeName);} while (false)


#define RenderCommandAs(TypeName, Command)                                  \
  (ui_render_command_##TypeName *)&(Command)->ui_render_command_##TypeName; \
  Assert((Command)->Type == type_ui_render_command_##TypeName)


link_internal void
PushLayout(layout** Dest, layout* Layout)
{
  /* Layout->DrawBounds = InvertedInfinityRectangle(); */

  Assert(!Layout->Prev);
  Layout->Prev = *Dest;
  *Dest = Layout;

/*   if (Layout != &GlobalLightPosition */
  /* Assert(Layout->At == V2(0)); */
  /* Info("%f %f", double(Layout->At.x), r64(Layout->At.y)); */

  // NOTE(Jesse): We have to do this such that the padding we're about to advance
  // by gets accounted for (or, rather, the basis we're starting at is accurate).
  // If we do not, the padding space does not get included in the final draw bounds
  UpdateDrawBounds(Layout);

  v2 Pad = V2(Layout->Padding.Left, Layout->Padding.Top);
  AdvanceLayoutStackBy(Pad, Layout);
}

link_internal layout*
PopLayout(layout** Layout)
{
  layout* PoppedLayout = *Layout;

  v4 Padding = PoppedLayout->Padding;
  v2 ClipTest = PoppedLayout->DrawBounds.Max + V2(0, Padding.Bottom);
  UpdateDrawBounds(PoppedLayout, ClipTest);

  v2 Advance = V2(Padding.Right, -Padding.Top);
  AdvanceLayoutStackBy(Advance, PoppedLayout);

  layout* PrevLayout = PoppedLayout->Prev;
  PoppedLayout->Prev = 0;
  *Layout = PrevLayout;

  return PoppedLayout;
}

link_internal ui_render_command*
GetCommand(ui_render_command_buffer* CommandBuffer, u32 CommandIndex)
{
  ui_render_command* Command = 0;
  if (CommandIndex < CommandBuffer->CommandCount)
    { Command = CommandBuffer->Commands+CommandIndex; }
  return Command;
}

link_internal window_layout*
GetHighestWindow(renderer_2d* Group, ui_render_command_buffer* CommandBuffer)
{
  TIMED_FUNCTION();

  u64 HighestInteractionStackIndex = 0;
  window_layout* HighestWindow = 0;

  for (u32 CommandIndex = 0;
      CommandIndex < CommandBuffer->CommandCount;
      ++CommandIndex)
  {
    ui_render_command *Command = GetCommand(CommandBuffer, CommandIndex);
    switch(Command->Type)
    {
      case type_ui_render_command_window_start:
      {
        ui_render_command_window_start* WindowStart = RenderCommandAs(window_start, Command);
        window_layout *TestWindow = WindowStart->Window;

        b32 InsideWindowBounds = IsInsideRect(GetWindowBounds(TestWindow), *Group->MouseP);
        b32 FoundNewHighestStackIndex = HighestInteractionStackIndex <= TestWindow->InteractionStackIndex;
        if ( InsideWindowBounds && FoundNewHighestStackIndex )
        {
          HighestWindow = TestWindow;
          HighestInteractionStackIndex = TestWindow->InteractionStackIndex;
        }

      } break;

      default: {} break;
    }
  }

  b32 Clicked = (Group->Input->LMB.Clicked || Group->Input->RMB.Clicked);
  if (Clicked && HighestWindow)
  {
    HighestWindow->InteractionStackIndex = ++Group->InteractionStackTop;
  }

  return HighestWindow;
}

link_internal void
ProcessButtonStart(renderer_2d* Group, render_state* RenderState, ui_id ButtonId)
{
  if (ButtonId == Group->Hover.ID)
  {
    Group->Hover = {};
    RenderState->Hover = True;
  }
  if (ButtonId == Group->Clicked.ID)
  {
    Group->Clicked = {};;
    RenderState->Clicked = True;
  }
  if (ButtonId == Group->Pressed.ID)
  {
    // Intentionally reset to 0 outside of this bonsai_function, because it's
    // dependant on the mouse buttons being released.
    // TODO(Jesse, id: 109, tags: cleanup, robustness): Reset this in here?
    RenderState->Pressed = True;
  }

  return;
}

link_internal button_interaction_result
ProcessButtonEnd(renderer_2d *Group, ui_id InteractionId, render_state* RenderState, rect2 AbsButtonBounds, ui_style* Style)
{
  Assert(IsValid(&InteractionId));

  button_interaction_result Button = ButtonInteraction( Group,
                                                        AbsButtonBounds,
                                                        InteractionId,
                                                        RenderState->Window,
                                                        Style);

  if (Button.Hover)
  {
    Group->Hover = Button.Interaction;
  }

  if (Button.Clicked)
  {
    Group->Clicked = Button.Interaction;
  }

  if (Button.Pressed)
  {
    Group->Pressed = Button.Interaction;
  }

  RenderState->Hover = False;
  RenderState->Pressed = False;
  RenderState->Clicked = False;

  return Button;
}

link_internal void
ProcessTexturedQuadPush(renderer_2d* Group, ui_render_command_textured_quad *Command, render_state* RenderState)
{
  rect2 Clip = RenderState->ClipRect;
  v2 MinP    = GetAbsoluteAt(RenderState->Layout);
  v2 Dim     = Command->QuadDim;
  r32 Z      = GetZ(Command->zDepth, RenderState->Window);

  // There's a second pass that draws all discrete textures
  Command->Clip = Clip;
  Command->Z = Z;

  if (Command->Texture == 0) { BufferValue(CSz("(null texture)"), MinP, Group, RenderState->Layout, V3(1.f, 0.55f, 0.1f), &DefaultStyle, Z, Clip, 0, TextRenderParam_NoAdvanceLayout); }

  if (Command->Params & QuadRenderParam_AdvanceClip)
  {
    UpdateDrawBounds(RenderState->Layout, RenderState->Layout->At);
    UpdateDrawBounds(RenderState->Layout, RenderState->Layout->At + Dim);
  }

  if (Command->Params & QuadRenderParam_AdvanceLayout)
  {
    AdvanceLayoutStackBy(V2(Dim.x, 0), RenderState->Layout);
  }
}

link_internal void
ProcessUntexturedQuadAtPush(renderer_2d* Group, ui_render_command_untextured_quad_at *Command, render_state* RenderState, b32 DoBuffering = True)
{
  rect2 Clip = RenderState->ClipRect; //GetAbsoluteClip(RenderState->Window);
  v2 MinP    = GetAbsoluteAt(&Command->Layout);
  v2 Dim     = Command->QuadDim;
  v3 Color   = SelectColorState(RenderState, &Command->Style);
  r32 Z      = GetZ(Command->zDepth, RenderState->Window);

  if (DoBuffering)
  {
    BufferUntexturedQuad(Group, &Group->Geo, MinP, Dim, Color, Z, Clip);
  }

  UpdateDrawBounds(&Command->Layout, MinP);
  UpdateDrawBounds(&Command->Layout, MinP + Dim);

  return;
}

link_internal void
ProcessUntexturedQuadPush(renderer_2d* Group, ui_render_command_untextured_quad *Command, render_state* RenderState)
{
  rect2 Clip = RenderState->ClipRect;
  v2 MinP    = GetAbsoluteAt(RenderState->Layout);
  v2 Dim     = Command->QuadDim;
  v3 Color   = SelectColorState(RenderState, &Command->Style);
  r32 Z      = GetZ(Command->zDepth, RenderState->Window);

  BufferUntexturedQuad(Group, &Group->Geo, MinP, Dim, Color, Z, Clip);

  if (Command->Params & QuadRenderParam_AdvanceClip)
  {
    UpdateDrawBounds(RenderState->Layout, RenderState->Layout->At);
    UpdateDrawBounds(RenderState->Layout, RenderState->Layout->At + Dim);
  }

  if (Command->Params & QuadRenderParam_AdvanceLayout)
  {
    AdvanceLayoutStackBy(V2(Dim.x, 0), RenderState->Layout);
  }

  return;
}

link_internal window_sort_params
GetWindowSortParams(ui_render_command_buffer *CommandBuffer)
{
  window_sort_params Result = {};

  {
    u32 CommandIndex = 0;
    ui_render_command *Command = GetCommand(CommandBuffer, CommandIndex++);
    while (Command)
    {
      switch(Command->Type)
      {
        case type_ui_render_command_window_start:
        {
          ++Result.Count;
          ui_render_command_window_start* WindowStart = RenderCommandAs(window_start, Command);
          Result.LowestInteractionStackIndex = Min(Result.LowestInteractionStackIndex, WindowStart->Window->InteractionStackIndex);
        } break;
        default : {} break;
      }

      Command = GetCommand(CommandBuffer, CommandIndex++);
    }
  }


  Result.SortKeys = Allocate(sort_key, GetTranArena(), Result.Count);

  {
    u32 AtKey = 0;
    u32 CommandIndex = 0;
    ui_render_command *Command = GetCommand(CommandBuffer, CommandIndex++);
    while (Command)
    {
      switch(Command->Type)
      {
        case type_ui_render_command_window_start:
        {
          ui_render_command_window_start* WindowStart = RenderCommandAs(window_start, Command);
          Result.SortKeys[AtKey++] = { .Index = CommandIndex-1, .Value = WindowStart->Window->InteractionStackIndex };
        } break;
        default : {} break;
      }

      Command = GetCommand(CommandBuffer, CommandIndex++);
    }
  }

  return Result;
}



link_internal void
SetWindowZDepths(ui_render_command_buffer *CommandBuffer)
{
  window_sort_params WindowSortParams = GetWindowSortParams(CommandBuffer);

  BubbleSort(WindowSortParams.SortKeys, WindowSortParams.Count);

  r64 SliceInterval = 1.0/(r64)WindowSortParams.Count;
  SliceInterval -= SliceInterval*0.0001;

  for (u32 SortKeyIndex = 0;
      SortKeyIndex < WindowSortParams.Count;
      ++SortKeyIndex)
  {
    u32 CommandIndex = (u32)WindowSortParams.SortKeys[SortKeyIndex].Index;
    window_layout* Window = CommandBuffer->Commands[CommandIndex].ui_render_command_window_start.Window;

    r64 Basis = 1.0 - (SliceInterval*(r64)SortKeyIndex) - SliceInterval;
    Window->zBackground = (r32)(Basis);
    Window->zTitleBar   = (r32)(Basis + (SliceInterval*0.1));
    Window->zText       = (r32)(Basis + (SliceInterval*0.2));
    Window->zBorder     = (r32)(Basis + (SliceInterval*0.3));
    Assert(Window->zBackground <= 1.0f);
    Assert(Window->zBackground >= 0.0f);

    Assert(Window->zText <= 1.0f);
    Assert(Window->zText >= 0.0f);

    Assert(Window->zTitleBar <= 1.0f);
    Assert(Window->zTitleBar >= 0.0f);

    Assert(Window->zBorder <= 1.0f);
    Assert(Window->zBorder >= 0.0f);
  }

  return;
}

link_internal find_command_result
FindPreviousCommand(ui_render_command_buffer* CommandBuffer, ui_render_command_type CommandType, u32 StartingIndex)
{
  Assert(StartingIndex < CommandBuffer->CommandCount);

  find_command_result Result = {};
  for (u32 CommandIndex = StartingIndex;
      ;
      --CommandIndex)
  {
    ui_render_command* Command = GetCommand(CommandBuffer, CommandIndex);
    if (Command->Type == CommandType)
    {
        Result.Command = Command;
        Result.Index = CommandIndex;
        break;
    }

    if (CommandIndex == 0) { break; }
  }

  return Result;
}

link_internal u32
FindPreviousButtonStart(ui_render_command_buffer* CommandBuffer, u32 StartingIndex)
{
  u32 Result = FindPreviousCommand(CommandBuffer, type_ui_render_command_button_start, StartingIndex).Index;
  return Result;
}

link_internal void
ResetAllLayouts(ui_render_command_buffer* CommandBuffer)
{
  for (u32 CommandIndex = 0;
           CommandIndex < CommandBuffer->CommandCount;
         ++CommandIndex)
  {
    ui_render_command* Command = GetCommand(CommandBuffer, CommandIndex);
    switch(Command->Type)
    {

      poof(
        func (ui_render_command RenderCommandDef)
        {
          RenderCommandDef.map_members(Member)
          {
            Member.is_union?
            {
              Member.map_members(UMember)
              {
                UMember.contains_type(layout)?
                {
                  case type_(UMember.type):
                  {
                    Command->(UMember.name).Layout.At = {};
                    Command->(UMember.name).Layout.DrawBounds = InvertedInfinityRectangle();
                  } break;
                }
              }
            }
          }
        }
      )
#include <generated/anonymous_ui_render_command_nKuoMe2B.h>

      default: {} break;
    }

    continue;
  }
}

link_internal rect2
FindAbsoluteDrawBoundsBetween(ui_render_command_buffer* CommandBuffer, u32 FirstCommand, u32 OnePastLastCommand)
{
  Assert(FirstCommand < CommandBuffer->CommandCount);
  Assert(OnePastLastCommand <= CommandBuffer->CommandCount);

  rect2 Result = InvertedInfinityRectangle();

  for (u32 CommandIndex = FirstCommand;
      CommandIndex < OnePastLastCommand;
      ++CommandIndex)
  {
    ui_render_command* Command = GetCommand(CommandBuffer, CommandIndex);
    switch(Command->Type)
    {

      poof(
        func (ui_render_command RenderCommandDef)
        {
          RenderCommandDef.map_members(Member)
          {
            Member.is_union?
            {
              Member.map_members(UMember)
              {
                UMember.contains_type(layout)?
                {
                  case type_(UMember.type):
                  {
                    Result.Max = Max(Result.Max, GetAbsoluteDrawBoundsMax(&Command->(UMember.name).Layout));
                    Result.Min = Min(Result.Min, GetAbsoluteDrawBoundsMin(&Command->(UMember.name).Layout));
                  } break;
                }
              }
            }
          }
        }
      )
#include <generated/anonymous_function_ui_render_command_RuTTrHiW.h>

      default: {} break;
    }

    continue;
  }

  return Result;
}

link_internal rect2
FindRelativeDrawBoundsBetween(ui_render_command_buffer* CommandBuffer, v2 RelativeTo, u32 FirstCommand, u32 OnePastLastCommand)
{
  rect2 Result = FindAbsoluteDrawBoundsBetween(CommandBuffer, FirstCommand, OnePastLastCommand);

  Result.Min -= RelativeTo;
  Result.Max -= RelativeTo;

  return Result;
}


// This is a three-part function
//
// 1. Count max number of columns while tallying up the width of each column
//
// 2. Tally up the max width for each column
//
// 3. Write the max width to each column command in the table
//

link_internal table_info
PreprocessTable(renderer_2d *Ui, render_state *RenderState, ui_render_command_buffer* CommandBuffer, u32 StartingIndex)
{
  table_info Result = {};

  Result.OnePastTableEnd = StartingIndex;

  {
    u32 NextCommandIndex = StartingIndex;
    u32 NextColumnIndex = 0;

    b32 FoundEnd = False;
    ui_render_command* Command = GetCommand(CommandBuffer, NextCommandIndex++);

    u32 SubTableCount = 0;

    r32 *CurrentWidth = 0;
    while (Command && !FoundEnd)
    {
      switch(Command->Type)
      {
          case type_ui_render_command_column_start:
          {
            if (SubTableCount == 0)
            {
            ++NextColumnIndex;
              Result.ColumnCount = Max(Result.ColumnCount, NextColumnIndex);
              Assert(NextColumnIndex <= u16_MAX);
            }

            ui_render_command_column_start* TypedCommand = RenderCommandAs(column_start, Command);
            CurrentWidth = &TypedCommand->Width;
            *CurrentWidth += TypedCommand->Layout.Padding.Left + TypedCommand->Layout.Padding.Right;
          } break;

          case type_ui_render_command_untextured_quad:
          {
            // TODO(Jesse): Should this not be represented?  The assert in here fires sometimes (when opening entities)
            /* if (SubTableCount == 0) */
            /* { */
            /*   ui_render_command_untextured_quad* TypedCommand = RenderCommandAs(untextured_quad, Command); */
            /*   Assert(CurrentWidth); */
            /*   *CurrentWidth += TypedCommand->QuadDim.x; */
            /* } */
          } break;

          case type_ui_render_command_textured_quad:
          {
            if (SubTableCount == 0)
            {
              ui_render_command_textured_quad* TypedCommand = RenderCommandAs(textured_quad, Command);
              Assert(CurrentWidth);
              *CurrentWidth += TypedCommand->QuadDim.x;
            }
          } break;

          case type_ui_render_command_text:
          {
            if (CurrentWidth)
            {
              if (SubTableCount == 0)
              {
                ui_render_command_text* TypedCommand = RenderCommandAs(text, Command);
                Assert(CurrentWidth);
                *CurrentWidth += GetDrawBounds(TypedCommand->String, &TypedCommand->Style).Max.x;
              }
            }
          } break;

          case type_ui_render_command_new_row:
          {
            if (SubTableCount == 0)
            {
              CurrentWidth = 0;
              NextColumnIndex = 0;
            ++Result.RowCount;
            }
          } break;

          case type_ui_render_command_table_start:
          {
            ++SubTableCount;
          } break;

          case type_ui_render_command_table_end:
          {
            ui_render_command_table_end* TypedCommand = RenderCommandAs(table_end, Command);
            TypedCommand->TableStartIndex = StartingIndex-1;
            if (SubTableCount)
            {
              --SubTableCount;
            }
            else
            {
              FoundEnd = True;
              Result.OnePastTableEnd = NextCommandIndex;
            }
          } break;

          default: {} break;
      }

      Command = GetCommand(CommandBuffer, NextCommandIndex++);
    }
  }

  Result.ColumnWidths = Allocate(r32, GetTranArena(), Result.ColumnCount);

  {
    u32 SubTableCount = 0;

    u32 NextColumnIndex = 0;
    for (u32 CommandIndex = StartingIndex;
             CommandIndex < Result.OnePastTableEnd;
           ++CommandIndex)
    {
      ui_render_command* Command = GetCommand(CommandBuffer, CommandIndex);
      switch(Command->Type)
      {
          case type_ui_render_command_column_start:
          {
            if (SubTableCount == 0)
            {
              ui_render_command_column_start* TypedCommand = RenderCommandAs(column_start, Command);

              Assert(NextColumnIndex < Result.ColumnCount);
              Result.ColumnWidths[NextColumnIndex] = Max(Result.ColumnWidths[NextColumnIndex], TypedCommand->Width);
              ++NextColumnIndex;
            }
          } break;

          case type_ui_render_command_new_row:
          {
            if (SubTableCount == 0)
            {
              NextColumnIndex = 0;
            }
          } break;

          case type_ui_render_command_table_start:
          {
            ++SubTableCount;
          } break;

          case type_ui_render_command_table_end:
          {
            if (SubTableCount)
            {
              --SubTableCount;
            }
            else
            {
              Assert(CommandIndex == Result.OnePastTableEnd-1);
            }
          } break;

          default: {} break;
      }

      continue;
    }
  }

  {
    u32 SubTableCount = 0;
    u32 NextColumnIndex = 0;

    for (u32 CommandIndex = StartingIndex;
        CommandIndex < Result.OnePastTableEnd;
        ++CommandIndex)
    {
      ui_render_command* Command =  GetCommand(CommandBuffer, CommandIndex);
      switch(Command->Type)
      {
          case type_ui_render_command_column_start:
          {
            if (SubTableCount == 0)
            {
              ui_render_command_column_start* TypedCommand = RenderCommandAs(column_start, Command);

              Assert(NextColumnIndex < Result.ColumnCount);
              TypedCommand->MaxWidth = Result.ColumnWidths[NextColumnIndex];
              ++NextColumnIndex;
            }
          } break;

          case type_ui_render_command_new_row:
          {
            if (SubTableCount == 0)
            {
              NextColumnIndex = 0;
            }
          } break;

          case type_ui_render_command_table_start:
          {
            ++SubTableCount;
          } break;

          case type_ui_render_command_table_end:
          {
            if (SubTableCount)
            {
              --SubTableCount;
            }
            else
            {
              Assert(CommandIndex == Result.OnePastTableEnd-1);
            }
          } break;

          default: {} break;
      }

      continue;
    }
  }


  return Result;
}

link_internal void
FlushCommandBuffer(renderer_2d *Group, render_state *RenderState, ui_render_command_buffer *CommandBuffer, layout *DefaultLayout)
{
  TIMED_FUNCTION();

  u32 NextCommandIndex = 0;
  ui_render_command *Command = GetCommand(CommandBuffer, NextCommandIndex++);
  while (Command)
  {
    switch(Command->Type)
    {
      InvalidCase(type_ui_render_command_noop);

      case type_ui_render_command_debug:
      {
        if (GetUiDebug && GetUiDebug()->DebugBreakUiCommand) { RuntimeBreak(); }
      } break;

      case type_ui_render_command_window_start:
      {
        Assert(LengthSq(DefaultLayout->Padding.xy) == 0);
        Assert(LengthSq(DefaultLayout->Padding.zw) == 0);

        Assert(RenderState->Layout == DefaultLayout);
        PopLayout(&RenderState->Layout);


        Assert(!RenderState->Window);
        ui_render_command_window_start* TypedCommand = RenderCommandAs(window_start, Command);
        RenderState->WindowStartCommandIndex = NextCommandIndex-1;

        PushLayout(&RenderState->Layout, &TypedCommand->Layout);
        RenderState->Window = TypedCommand->Window;
        RenderState->ClipRect = TypedCommand->ClipRect;
      } break;

      case type_ui_render_command_window_end:
      {
        ui_render_command_window_end* TypedCommand = RenderCommandAs(window_end, Command);

        if (TypedCommand->Window->Flags & WindowLayoutFlag_StartupSize_InferHeight)
        {
          TypedCommand->Window->Flags = (TypedCommand->Window->Flags&(~WindowLayoutFlag_StartupSize_InferHeight));
          TypedCommand->Window->MaxClip.y = RenderState->Layout->DrawBounds.Max.y;
          TypedCommand->Window->Basis.y = DefaultLayout->DrawBounds.Max.y + DefaultWindowSideOffset;
        }

        if (TypedCommand->Window->Flags & WindowLayoutFlag_StartupSize_InferWidth)
        {
          TypedCommand->Window->Flags = (TypedCommand->Window->Flags&(~WindowLayoutFlag_StartupSize_InferWidth));
          TypedCommand->Window->MaxClip.x = RenderState->Layout->DrawBounds.Max.x;
          TypedCommand->Window->Basis.x = DefaultWindowSideOffset;
        }

        if (TypedCommand->Window->Flags & WindowLayoutFlag_StartupAlign_Right)
        {
          TypedCommand->Window->Basis.x = Group->ScreenDim->x - TypedCommand->Window->MaxClip.x - DefaultWindowSideOffset;
        }

        if (TypedCommand->Window->Flags & WindowLayoutFlag_StartupAlign_Bottom)
        {
          TypedCommand->Window->Basis.y = Group->ScreenDim->y - TypedCommand->Window->MaxClip.y - DefaultWindowSideOffset;
        }

        if (TypedCommand->Window->Flags & WindowLayoutFlag_DynamicSize)
        {
          TypedCommand->Window->MaxClip = RenderState->Layout->DrawBounds.Max;
        }


        Assert(TypedCommand->Window == RenderState->Window);
        RenderState->Window = 0;
        PopLayout(&RenderState->Layout);
        Assert(RenderState->Layout == 0);

        RenderState->WindowStartCommandIndex = 0;

        PushLayout(&RenderState->Layout, DefaultLayout);
        Assert(RenderState->Layout == DefaultLayout);

        RenderState->ClipRect = DISABLE_CLIPPING;
      } break;

      case type_ui_render_command_table_start:
      {
        ui_render_command_table_start* TypedCommand = RenderCommandAs(table_start, Command);
        TypedCommand->TableInfo = PreprocessTable(Group, RenderState, CommandBuffer, NextCommandIndex);

        u32 OnePastTableEnd = TypedCommand->TableInfo.OnePastTableEnd;
        if (OnePastTableEnd)
        {

          switch(TypedCommand->Position)
          {
            case Position_None:
            {
              r32 BasisX = RenderState->Layout->Basis.x;
              r32 BasisY = GetAbsoluteAt(RenderState->Layout).y;
              TypedCommand->Layout.Basis = V2(BasisX, BasisY);
            } break;

            // NOTE(Jesse): Not Implemented
            InvalidCase(Position_Above);
            InvalidCase(Position_LeftOf);

            case Position_RightOf:
            {
              ui_render_command_table_start* RelativeTable = GetCommandAs(table_start, CommandBuffer, TypedCommand->RelativeTo.Index);
              TypedCommand->Layout.Basis += V2(GetAbsoluteDrawBoundsMax(&RelativeTable->Layout).x, RelativeTable->Layout.Basis.y );
            } break;

            case Position_Below:
            {
              ui_render_command_table_start* RelativeTable = GetCommandAs(table_start, CommandBuffer, TypedCommand->RelativeTo.Index);
              TypedCommand->Layout.Basis += V2(RelativeTable->Layout.Basis.x, GetAbsoluteDrawBoundsMax(&RelativeTable->Layout).y);
            } break;
          }

          PushLayout(&RenderState->Layout, &TypedCommand->Layout);
        }
        else
        {
          Error("No type_ui_render_command_table_end detected, aborting render.");
          NextCommandIndex = CommandBuffer->CommandCount;
        }
      } break;

      case type_ui_render_command_table_end:
      {
        ui_render_command_table_end *TypedCommand = RenderCommandAs(table_end, Command);

        layout *Popped = PopLayout(&RenderState->Layout);

        if (GetUiDebug && GetUiDebug()->OutlineUiTables)
        {
          rect2 AbsDrawBounds = GetAbsoluteDrawBounds(Popped);
          BufferBorder(Group, AbsDrawBounds, V3(0,0,1), 0.9f, DISABLE_CLIPPING);

          if (IsInsideRect(AbsDrawBounds, *Group->MouseP))
          {
            ui_render_command_table_start *StartCommand = GetCommandAs(table_start, CommandBuffer, TypedCommand->TableStartIndex);

            r32 At = 0;
            r32 Height = Max(1u, StartCommand->TableInfo.RowCount) * Global_Font.Size.y;
            RangeIterator_t(u32, ColumnIndex, StartCommand->TableInfo.ColumnCount)
            {
              r32 Width = StartCommand->TableInfo.ColumnWidths[ColumnIndex];
              v2 Pad = V2(StartCommand->Layout.Padding.Left, StartCommand->Layout.Padding.Top);
              rect2 Bounds = RectMinDim(AbsDrawBounds.Min + V2(At, 0) + Pad, V2(Width, Height));
              BufferBorder(Group, Bounds, Normalize(V3(0,1,1)), 0.9f, DISABLE_CLIPPING);
              At += Width;
            }

            if (GetUiDebug && GetUiDebug()->DebugBreakOnElementClick && Group->Input->LMB.Clicked )
            {
              RuntimeBreak();
            }

          }
        }

        // NOTE(Jesse): This is a bug, not a feature.  If this is here there's no way to have
        // tables inline next to one-another, except by using position, which is annoying.
        //
        // In addition, I was seeing some wacky positioning when doing this that I didn't
        // totally hunt down, but I was able to get the positioning to behave when I removed
        // this, so suffice to say there are a few reasons this is a bug...
        /* NewRow(RenderState->Layout); */
      } break;

      case type_ui_render_command_column_start:
      {
        ui_render_command_column_start* TypedCommand = RenderCommandAs(column_start, Command);
        TypedCommand->Layout.Basis += GetAbsoluteAt(RenderState->Layout);

        PushLayout(&RenderState->Layout, &TypedCommand->Layout);

        v2 Advance = {};
        if (TypedCommand->Params & ColumnRenderParam_RightAlign)
        {
          Advance = V2(TypedCommand->MaxWidth - TypedCommand->Width, 0);
          AdvanceLayoutStackBy(Advance, RenderState->Layout);
        }
      } break;

      case type_ui_render_command_column_end:
      {
        ui_render_command_column_end *TypedCommand = RenderCommandAs(column_end, Command);

        ui_render_command *StartCommandBase = CommandBuffer->Commands + TypedCommand->StartCommandIndex;
        ui_render_command_column_start *StartCommand = RenderCommandAs(column_start, StartCommandBase);

        v2 Advance = {};
        if (StartCommand->Params == ColumnRenderParam_LeftAlign)
        {
          Advance = V2(StartCommand->MaxWidth - StartCommand->Width, 0);
          AdvanceLayoutStackBy(Advance, RenderState->Layout);
        }

#if 1
        if (GetUiDebug && GetUiDebug()->OutlineUiTableColumns)
        {
          rect2 AbsDrawBounds = GetAbsoluteDrawBounds(RenderState->Layout);
          BufferBorder(Group, AbsDrawBounds, Normalize(V3(1,0,1)), 0.9f, DISABLE_CLIPPING);

          if (GetUiDebug()->DebugBreakOnElementClick)
          {
            if ( IsInsideRect(AbsDrawBounds, *Group->MouseP) && Group->Input->LMB.Clicked ) { RuntimeBreak(); }
          }
        }
#endif

        PopLayout(&RenderState->Layout);
      } break;

      case type_ui_render_command_text:
      {
        ui_render_command_text* TypedCommand = RenderCommandAs(text, Command);
        TypedCommand->Layout.Basis += GetAbsoluteAt(RenderState->Layout) + TypedCommand->Offset;

        PushLayout(&RenderState->Layout, &TypedCommand->Layout);
        BufferValue(TypedCommand->String, Group, RenderState, &TypedCommand->Style, TypedCommand->Clip, TypedCommand->Params);
        PopLayout(&RenderState->Layout);
      } break;

      case type_ui_render_command_text_at:
      {
        ui_render_command_text_at* TextCommand = RenderCommandAs(text_at, Command);
        BufferTextAt(Group, TextCommand->At, TextCommand->Text, Global_Font.Size, V3(1), 1.0f, TextCommand->Clip);
      } break;

      case type_ui_render_command_textured_quad:
      {
        ui_render_command_textured_quad* TypedCommand = RenderCommandAs(textured_quad, Command);
        TypedCommand->Layout.Basis += GetAbsoluteAt(RenderState->Layout);

        PushLayout(&RenderState->Layout, &TypedCommand->Layout);
        ProcessTexturedQuadPush(Group, TypedCommand, RenderState);
        PopLayout(&RenderState->Layout);
      } break;

      case type_ui_render_command_untextured_quad:
      {
        ui_render_command_untextured_quad* TypedCommand = RenderCommandAs(untextured_quad, Command);
        TypedCommand->Layout.Basis += GetAbsoluteAt(RenderState->Layout);

        PushLayout(&RenderState->Layout, &TypedCommand->Layout);
        ProcessUntexturedQuadPush(Group, TypedCommand, RenderState);
        PopLayout(&RenderState->Layout);
      } break;

      case type_ui_render_command_untextured_quad_at:
      {
        ui_render_command_untextured_quad_at* TypedCommand = RenderCommandAs(untextured_quad_at, Command);
        ProcessUntexturedQuadAtPush(Group, TypedCommand, RenderState);
      } break;

      case type_ui_render_command_new_row:
      {
        NewRow(RenderState->Layout);
      } break;

      case type_ui_render_command_button_start:
      {
        ui_render_command_button_start* TypedCommand = RenderCommandAs(button_start, Command);
        ProcessButtonStart(Group, RenderState, TypedCommand->ID);
      } break;

      case type_ui_render_command_button_end:
      {
        u32 ButtonStartIndex = FindPreviousButtonStart(CommandBuffer, NextCommandIndex-1);
        rect2 AbsDrawBounds = FindAbsoluteDrawBoundsBetween(CommandBuffer, ButtonStartIndex, NextCommandIndex);

        ui_render_command *ButtonCmd = CommandBuffer->Commands+ButtonStartIndex;
        ui_render_command_button_start* ButtonStart = RenderCommandAs(button_start, ButtonCmd);

        button_interaction_result ButtonResult = ProcessButtonEnd(Group, ButtonStart->ID, RenderState, AbsDrawBounds, &ButtonStart->Style);

        if (ButtonStart->Params & ButtonParam_ToggleButton)
        {
          if (ButtonResult.Clicked)
          {
            maybe_ui_toggle_ptr Maybe = GetPtrById(&Group->ToggleTable, ButtonStart->ID);
            if (Maybe.Tag)
            {
              ui_toggle* E = Maybe.Value;
              E->ToggledOn = !E->ToggledOn;
            }
            else
            {
              ui_toggle E = {ButtonStart->ID, True};
              Insert(E, &Group->ToggleTable, &Group->UiToggleArena);
            }
          }
        }
      } break;

      case type_ui_render_command_rel_border:
      {
        ui_render_command_rel_border* Border = RenderCommandAs(rel_border, Command);
        BufferBorder(Group, RectMinDim(GetAbsoluteAt(RenderState->Layout), Border->Dim), Border->Color, GetZ(zDepth_Border, RenderState->Window), DISABLE_CLIPPING, Border->Thickness);
      } break;
      case type_ui_render_command_abs_border:
      {
        ui_render_command_abs_border* Border = RenderCommandAs(abs_border, Command);
        BufferBorder(Group, Border->Bounds, Border->Color, GetZ(zDepth_Border, RenderState->Window), DISABLE_CLIPPING, Border->Thickness);
      } break;

      case type_ui_render_command_force_advance:
      {
        ui_render_command_force_advance* TypedCommand = RenderCommandAs(force_advance, Command);
        AdvanceLayoutStackBy(TypedCommand->Offset, RenderState->Layout);
      } break;

      case type_ui_render_command_force_update_basis:
      {
        ui_render_command_force_update_basis* TypedCommand = RenderCommandAs(force_update_basis, Command);
        /* AdvanceLayoutStackBy(TypedCommand->Offset, RenderState->Layout); */
        RenderState->Layout->Basis += TypedCommand->Offset;
        /* RenderState->Layout->DrawBounds.Min -= TypedCommand->Offset; */
        /* RenderState->Layout->DrawBounds.Max -= TypedCommand->Offset; */
        /* AdvanceLayoutStackBy(TypedCommand->Offset, RenderState->Layout); */
        AdvanceLayoutStackBy(V2(0), RenderState->Layout);
        /* UpdateDrawBounds(RenderState->Layout); */

      } break;

      case type_ui_render_command_reset_draw_bounds:
      {
        RenderState->Layout->DrawBounds = InvertedInfinityRectangle();
      } break;
    }

    Command = GetCommand(CommandBuffer, NextCommandIndex++);
  }
}

link_internal void
DrawUi(renderer_2d *Group, ui_render_command_buffer *CommandBuffer)
{
  TIMED_FUNCTION();

  layout DefaultLayout = {};
  DefaultLayout.DrawBounds = InvertedInfinityRectangle();

  render_state RenderState = { .Layout = &DefaultLayout, .ClipRect = DISABLE_CLIPPING };

  SetWindowZDepths(CommandBuffer);

  FlushCommandBuffer(Group, &RenderState, CommandBuffer, &DefaultLayout);

  Group->SolidGeoCountLastFrame = Group->Geo.At;
  Group->TextGeoCountLastFrame = Group->TextGroup->Geo.At;
  DrawUiBuffers(Group, Group->ScreenDim); // Draws text and solid UI buffers

  // Draw textured quads
  u32 NextCommandIndex = 0;
  ui_render_command *Command = GetCommand(CommandBuffer, NextCommandIndex++);
  while (Command)
  {
    switch(Command->Type)
    {
      InvalidCase(type_ui_render_command_noop);

      case type_ui_render_command_window_start:
      case type_ui_render_command_window_end:
      case type_ui_render_command_table_start:
      case type_ui_render_command_table_end:
      case type_ui_render_command_column_start:
      case type_ui_render_command_column_end:
      case type_ui_render_command_text:
      case type_ui_render_command_text_at:
      case type_ui_render_command_untextured_quad:
      case type_ui_render_command_untextured_quad_at:
      case type_ui_render_command_new_row:
      case type_ui_render_command_button_start:
      case type_ui_render_command_button_end:
      case type_ui_render_command_rel_border:
      case type_ui_render_command_abs_border:
      case type_ui_render_command_force_advance:
      case type_ui_render_command_force_update_basis:
      case type_ui_render_command_reset_draw_bounds:
      case type_ui_render_command_debug:
        { break; }

      case type_ui_render_command_textured_quad:
      {
        ui_render_command_textured_quad* TypedCommand = RenderCommandAs(textured_quad, Command);

        {
          v2 MinP    = GetAbsoluteDrawBoundsMin(&TypedCommand->Layout);
          v2 Dim     = TypedCommand->QuadDim;
          r32 Z      = TypedCommand->Z;
          rect2 Clip = TypedCommand->Clip;

          if (TypedCommand->Texture)
          {
            Assert(Group->TextGroup->Geo.At == 0);

            shader *Shader = &Group->TexturedQuadShader;
            GL.UseProgram(Shader->ID);

            if (TypedCommand->TextureSlice < 0)
            {
              Assert(TypedCommand->Texture->Slices == 1);
              BindUniform(Shader, "Texture", TypedCommand->Texture, 0);
              GL.BindTexture(GL_TEXTURE_2D_ARRAY, 0);
            }
            else
            {
              Assert(TypedCommand->Texture->Slices > 1);
              BindUniform(Shader, "TextureArray", TypedCommand->Texture, 0);
              GL.BindTexture(GL_TEXTURE_2D, 0);
            }

            BindUniform(Shader, "IsDepthTexture",  TypedCommand->IsDepthTexture  );
            BindUniform(Shader, "HasAlphaChannel", TypedCommand->HasAlphaChannel );
            BindUniform(Shader, "TextureSlice",    TypedCommand->TextureSlice   );
            BindUniform(Shader, "Tint",            &TypedCommand->Tint   );

            // NOTE(Jesse):  We're not passing a 3D or texture array to the shader here, so we have to use 0 as the slice
            // TODO(Jesse): This looks like it should actually work for 3D texture arrays too ..?
            BufferTexturedQuad(Group, TypedCommand->TextureSlice, MinP, Dim, UVsForFullyCoveredQuad(), V3(1, 0, 0), Z, Clip, 0);

            Group->SolidGeoCountLastFrame += Group->Geo.At;
            Group->TextGeoCountLastFrame  += Group->TextGroup->Geo.At;


#if 1
            GL.Enable(GL_BLEND);
            GL.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
              DrawUiBuffer(Group->TextGroup, &Group->TextGroup->Geo, Group->ScreenDim);
            GL.Disable(GL_BLEND);
#else
            Group->TextGroup->Geo.At = 0;
#endif

            /* GL.ActiveTexture(GL_TEXTURE0); */
            GL.BindTexture(GL_TEXTURE_2D, 0);
            GL.BindTexture(GL_TEXTURE_2D_ARRAY, 0);

            AssertNoGlErrors;
          }
          else
          {
            // we already drew "(null texture)" text in the previous pass
          }
        }

      } break;
    }

    Command = GetCommand(CommandBuffer, NextCommandIndex++);
  }
}

link_internal v2
BasisBelow(window_layout* Window, v2 WindowSpacing = V2(0, 50))
{
  v2 Result = V2(Window->Basis.x, GetAbsoluteMaxClip(Window).y) + WindowSpacing;
  return Result;
}

link_internal v2
BasisRightOf(window_layout* Window, v2 WindowSpacing = V2(50, 0))
{
  v2 Result = V2(GetAbsoluteMaxClip(Window).x, Window->Basis.y) + WindowSpacing;
  return Result;
}

link_internal void
AllocateAndInitGeoBuffer(textured_2d_geometry_buffer *Geo, u32 ElementCount, memory_arena *DebugArena)
{
  Geo->Verts  = Allocate(v3, DebugArena, ElementCount);
  Geo->Colors = Allocate(v3, DebugArena, ElementCount);
  Geo->UVs    = Allocate(v3, DebugArena, ElementCount);

  Geo->End = ElementCount;
  Geo->At = 0;
}

link_internal void
AllocateAndInitGeoBuffer(untextured_2d_geometry_buffer *Geo, u32 ElementCount, memory_arena *DebugArena)
{
  Geo->Verts = Allocate(v3, DebugArena, ElementCount);
  Geo->Colors = Allocate(v3, DebugArena, ElementCount);

  Geo->End = ElementCount;
  Geo->At = 0;
  return;
}

link_internal texture LoadBitmap(const char* FilePath, u32 SliceCount, memory_arena *Arena);

link_internal b32
InitRenderer2D(renderer_2d *Renderer, heap_allocator *Heap, memory_arena *PermMemory, v2 *MouseP, v2 *MouseDP, v2 *ScreenDim, input *Input, b32 Headless = False)
{
  b32 Result = True;

  Renderer->TextGroup     = Allocate(render_buffers_2d, PermMemory, 1);
  Renderer->CommandBuffer = Allocate(ui_render_command_buffer, PermMemory, 1);

  // TODO(Jesse, memory): Instead of allocate insanely massive buffers (these are ~400x overkill)
  // we should have a system that streams blocks of memory in as-necessary
  // @streaming_ui_render_memory
  u32 ElementCount = (u32)Megabytes(2);
  AllocateAndInitGeoBuffer(&Renderer->TextGroup->Geo, ElementCount, PermMemory);
  AllocateAndInitGeoBuffer(&Renderer->Geo, ElementCount, PermMemory);

  Renderer->ToggleTable = Allocate_ui_toggle_hashtable(1024, PermMemory);


  if (Headless == False)
  {
    auto TextGroup = Renderer->TextGroup;
    TextGroup->DebugTextureArray = LoadBitmap("texture_atlas_0.bmp", DebugTextureArraySlice_Count, GetTranArena());
    GL.GenBuffers(1, &TextGroup->SolidUIVertexBuffer);
    GL.GenBuffers(1, &TextGroup->SolidUIColorBuffer);
    GL.GenBuffers(1, &TextGroup->SolidUIUVBuffer);

    TextGroup->Text2DShader = LoadShaders( CSz(STDLIB_SHADER_PATH "TextVertexShader.vertexshader"), CSz(STDLIB_SHADER_PATH "TextVertexShader.fragmentshader") );

    TextGroup->TextTextureUniform = GL.GetUniformLocation(TextGroup->Text2DShader.ID, "TextTextureSampler");

    Renderer->TextGroup->SolidUIShader = LoadShaders( CSz(STDLIB_SHADER_PATH "SimpleColor.vertexshader"), CSz(STDLIB_SHADER_PATH "SimpleColor.fragmentshader") );

    // Generic shader that gets reused to draw simple textured quads
    Renderer->TexturedQuadShader = MakeFullTextureShader(0, PermMemory);

    AssertNoGlErrors;
  }


  random_series Entropy = {54623153};
  for (u32 ColorIndex = 0; ColorIndex < RANDOM_COLOR_COUNT; ++ColorIndex)
  {
    Renderer->DebugColors[ColorIndex] = RandomV3(&Entropy);
  }

  Assert(MouseP);
  Assert(MouseDP);
  Assert(ScreenDim);
  Assert(Input);

  Renderer->MouseP    = MouseP;
  Renderer->MouseDP   = MouseDP;
  Renderer->ScreenDim = ScreenDim;
  Renderer->Input     = Input;

  return Result;
}

link_internal void
UiFrameBegin(renderer_2d *Ui)
{
  Ui->RequestedForceCapture = False;

  Assert(Ui->Input);
  Assert(Ui->MouseP);
  Assert(Ui->MouseDP);
  Assert(Ui->ScreenDim->x > 0.f);
  Assert(Ui->ScreenDim->y > 0.f);

  input *Input = Ui->Input;
  if ( ! (Input->LMB.Pressed || Input->RMB.Pressed) ) { Ui->Pressed = {}; }
}

global_variable interactable Global_GenericWindowInteraction = {
   // TODO(Jesse): Come up with a less nonsense way of doing this!  Cannot use
   // a string constant because with multiple DLLs there are multiple of them!
   // Could do a comptime string hash if poof supported that..

  .ID = {0, 0, 0, 4208142317, }, // 420blazeit  Couldn't spell faggot.. rip.
};

global_variable interactable Global_ViewportInteraction = {
   // TODO(Jesse): Come up with a less nonsense way of doing this!  Cannot use
   // a string constant because with multiple DLLs there are multiple of them!
   // Could do a comptime string hash if poof supported that..

  .ID = {0, 0, 4208142317, 0}, // 420blazeit  Couldn't spell faggot.. rip.
};

link_internal b32
UiHoveredMouseInput(renderer_2d *Ui)
{
  b32 MouseWasHoveringOverWindow = Ui->HighestWindow != 0;
  b32 Result = MouseWasHoveringOverWindow;
  return Result;
}

link_internal b32
UiCapturedMouseInput(renderer_2d *Ui)
{
  b32 PressedIdWasNotViewportId = ( IsValid(&Ui->Pressed.ID) &&
                                             Ui->Pressed.ID != Global_ViewportInteraction.ID );
  b32 ForceCapture = Ui->RequestedForceCapture;
  b32 Result = ForceCapture || PressedIdWasNotViewportId;
  return Result;
}

link_internal b32
UiInteractionWasViewport(renderer_2d *Ui)
{
  b32 InteractionWasViewport = ( IsValid(&Ui->Pressed.ID) &&
                                          Ui->Pressed.ID == Global_ViewportInteraction.ID );
  b32 Result = InteractionWasViewport;
  return Result;
}

link_internal void
UiFrameEnd(renderer_2d *Ui)
{
  input *Input = Ui->Input;

  Ui->HighestWindow = GetHighestWindow(Ui, Ui->CommandBuffer);

  if (Ui->HighestWindow)
  {
    if (Input->Ctrl.Pressed)
    {
      Ui->HighestWindow->Scroll.x += Input->MouseWheelDelta;
    }
    else
    {
      Ui->HighestWindow->Scroll.y += Input->MouseWheelDelta;
    }
  }

  DrawUi(Ui, Ui->CommandBuffer);

  if (Input->LMB.Pressed || Input->RMB.Pressed)
  {
    if ( UiHoveredMouseInput(Ui) )
    {
      if (IsValid(&Ui->Pressed.ID))
      {
        // Do nothing, we recorded a specific interaction
      }
      else
      {
        // Set the interaction to a generic one so we know to not do things
        // like update the game camera
        Ui->Pressed = Global_GenericWindowInteraction;
      }
    }
    else if (UiCapturedMouseInput(Ui))
    {
      // If we force captured the input we didn't necessarily click on a UI
      // element.  Otherwise, we should have a valid ID
      if (Ui->RequestedForceCapture)
      {
      }
      else
      {
        Assert(IsValid(&Ui->Pressed.ID));
      }
    }
    else
    {
      // We clicked outside the UI, therefore we must be interacting with the
      // viewport
      Ui->Pressed = Global_ViewportInteraction;
    }

  }

  Ui->CommandBuffer->CommandCount = 0;
  if (GetUiDebug) { GetUiDebug()->DebugBreakUiCommand = False; }
}
