#define UI_FUNCTION_PROTO_NAMES  ui_render_params *Params
#define UI_FUNCTION_PROTO_DEFAULTS ui_render_params *Params = &DefaultUiRenderParams_Generic
#define UI_FUNCTION_INSTANCE_NAMES Params

#define UNPACK_UI_RENDER_PARAMS(Params)                                    \
     relative_position          Pos = Params->RelativePosition.Position;   \
  ui_element_reference   RelativeTo = Params->RelativePosition.RelativeTo; \
              ui_style      *FStyle = Params->FStyle;                      \
              ui_style      *BStyle = Params->BStyle;                      \
                    v2       Offset = Params->Offset;                      \
                    v4      Padding = Params->Padding;                     \
  auto   AlignFlags = Params->AlignFlags;                                  \
  auto   LayoutFlags = Params->LayoutFlags;                                \



#define DEBUG_MAX_UI_WINDOW_SLICES 1024.0f
#define DISABLE_CLIPPING RectMinMax(V2(f32_MIN), V2(f32_MAX) )
#define DISABLE_CLIPPING_MAX V2(f32_MAX)

#define OPEN_INDENT_FOR_TOGGLEABLE_REGION() (PushForceUpdateBasis(Ui, V2(20.f, 0.f)))
#define CLOSE_INDENT_FOR_TOGGLEABLE_REGION() (PushForceUpdateBasis(Ui, V2(-20.f, 0.f)))



struct ui_debug
{
  b8 OutlineUiValues;
  b8 OutlineUiButtons;
  b8 OutlineUiTables;
  b8 OutlineUiTableColumns;

  b8 DebugBreakOnElementClick;
  b8 DebugBreakUiCommand;

  b8 LogClickEvents;
};

link_weak ui_debug *GetUiDebug();

/******************************               ********************************/
/******************************   Rendering   ********************************/
/******************************               ********************************/

struct render_buffers_2d
{
  texture DebugTextureArray;
  s32 TextTextureUniform;

  shader UiShader;
  gpu_mapped_ui_buffer Buf;
};

enum window_layout_flags
{
  WindowLayoutFlag_None                    = 0,

  WindowLayoutFlag_Size_Dynamic            = (1 << 0),

  WindowLayoutFlag_StartupSize_InferHeight = (1 << 1),
  WindowLayoutFlag_StartupSize_InferWidth  = (1 << 2),
  WindowLayoutFlag_StartupSize_Infer       = (WindowLayoutFlag_StartupSize_InferHeight|WindowLayoutFlag_StartupSize_InferWidth),

  WindowLayoutFlag_Align_Right             = (1 << 3),
  WindowLayoutFlag_Align_Bottom            = (1 << 4),
  WindowLayoutFlag_Align_BottomRight       = (WindowLayoutFlag_Align_Right|WindowLayoutFlag_Align_Bottom),

  // NOTE(Jesse): Must have the infer flags such that the windows get bumped
  // below the default window layout.  I don't love this implicit behavior,
  // but it is what it is for now.
  WindowLayoutFlag_Default                 = (WindowLayoutFlag_Size_Dynamic|WindowLayoutFlag_StartupSize_Infer),

  // Set this to free the window layout from the hashtable after drawing
  WindowLayoutFlag_DeferFree               = (1 << 5),
};

struct window_layout
{
  ui_id HashtableKey;

  cs Title;

  // TODO(Jesse): Pack Minimized into flags somehow?
  b32 Minimized;
  u32 MinimizeIndex;

  s32 Flags; // window_layout_flags

  v2 Basis;   // Absolute offset from (0,0)
  v2 MaxClip; // Basis-relative maximum corner of the window
  v2 Scroll;  // Basis-relative offset of the content within the window

  // NOTE(Jesse): For resetting when we un-minimize
  s32 CachedFlags;
  v2  CachedBasis;
  v2  CachedMaxClip;
  v2  CachedScroll;


  u64 InteractionStackIndex;

  r32 zBackground;
  r32 zText;
  r32 zBorder;
  r32 zTitleBar;

  window_layout *NextHotWindow;
};


link_internal u64
Hash(window_layout *E)
{
  u64 Result = Hash(&E->HashtableKey);
  return Result;
}

typedef window_layout* window_layout_ptr;

/* poof(are_equal(window_layout)) */
/* #include <generated/are_equal_window_layout.h> */

link_internal b32
AreEqual(window_layout *Thing1, window_layout *Thing2)
{
  b32 Result = False;
  if (Thing1 && Thing2)
  {
      Result = AreEqual(Thing1->HashtableKey, Thing2->HashtableKey);
  }
  return Result;
}

poof(maybe(window_layout))
#include <generated/maybe_window_layout.h>

poof(maybe(window_layout_ptr))
#include <generated/maybe_window_layout_ptr.h>

poof(hashtable_struct(window_layout))
#include <generated/hashtable_struct_window_layout.h>

#define INTERACTION_ALWAYS_ON_TOP (u64_MAX)

struct ui_toggle
{
  ui_id Id;
  b32 ToggledOn;
};

poof(are_equal(ui_toggle))
#include <generated/are_equal_ui_toggle.h>

struct ui_toggle_button_handle
{
  cs Text;
  cs Tooltip;
  ui_id Id;
  u32 Value; // NOTE(Jesse): This is typically the associated enum value
};

typedef void (*modal_callback)(void*);

/* // 0x3FFFFF == 22 set bits == 4,194,303 in decimal */
/* CAssert(0x3FFFFF == 0b1111111111111111111111); */
/* #define UiId(window, base, mod) u64( (u64(base)&0x3FFFFF) | ( (u64(mod)&0x3FFFFF) << 22 | (u64(window)&0x3FFFFF) << 44)) */

#define UiMaskAndCastPointer(p) u32(u64(p)&0xffffffff)

link_internal ui_id
UiId(const char *Label)
{
  ui_id Result = {0, 0, 0, UiMaskAndCastPointer(Label)};
  return Result;
}

link_internal ui_id
UiId(void *Window, void *Interaction, void *Element, void *Index)
{
  ui_id Result = {UiMaskAndCastPointer(Index), UiMaskAndCastPointer(Window), UiMaskAndCastPointer(Interaction), UiMaskAndCastPointer(Element)};
  return Result;
}

link_internal ui_id
UiId(void *Window, void *Interaction, void *Element)
{
  ui_id Result = {0, UiMaskAndCastPointer(Window), UiMaskAndCastPointer(Interaction), UiMaskAndCastPointer(Element)};
  return Result;
}

link_internal ui_id
UiId(window_layout *Window, void *Interaction, void *Element)
{
  ui_id Result = {0, UiMaskAndCastPointer(Window), UiMaskAndCastPointer(Interaction), UiMaskAndCastPointer(Element)};
  return Result;
}

link_internal ui_id
UiId(window_layout *Window, const char *Interaction, u32 Element)
{
  return UiId(Window, Cast(void*, Interaction), (void*)u64(Element));
}

link_internal ui_id
UiId(window_layout *Window, u64 Interaction, u64 Element)
{
  return UiId(Window, Cast(void*, Interaction), (void*)Element);
}

link_internal ui_id
UiId(window_layout *Window, const char *Interaction, u64 Element)
{
  return UiId(Window, Cast(void*, Interaction), (void*)Element);
}

link_internal ui_id
UiId(window_layout *Window, const char *Interaction, void *Element)
{
  return UiId(Window, Cast(void*, Interaction), Element);
}

link_internal ui_toggle_button_handle
UiToggle(cs Text, cs Tooltip, ui_id Id, u32 Value)
{
  ui_toggle_button_handle Result = { Text, Tooltip, Id, Value };
  return Result;
}

typedef ui_toggle* ui_toggle_ptr;

// TODO(Jesse): Move this?
link_internal umm
Hash(umm *Value) { return *Value; }

link_internal umm
Hash(ui_toggle *Toggle)
{
  return Hash(&Toggle->Id);
}

poof(maybe(ui_toggle))
#include <generated/maybe_ui_toggle.h>

poof(maybe(ui_toggle_ptr))
#include <generated/maybe_ui_toggle_ptr.h>

poof(hashtable_struct(ui_toggle))
#include <generated/hashtable_struct_ui_toggle.h>


struct text_box_edit_state
{
  ui_id Id;
  char *Text;
  umm   TextBufferLen;
};


struct textured_quad_render_pass
poof(
    @vert_source_file(STDLIB_SHADER_PATH "FullPassthrough.vertexshader")
    @frag_source_file(STDLIB_SHADER_PATH "SimpleTexture.fragmentshader")
  )
{
  shader Program;
  shader_uniform Uniforms[4];

  b32 IsDepthTexture;    poof(@uniform)
  b32 HasAlphaChannel;   poof(@uniform)
  s32 TextureSlice;      poof(@uniform)
   v3 Tint;              poof(@uniform)
};


struct ui_render_pass
{
};

/* poof(buffer(window_layout)) */
/* #include <generated/buffer_window_layout.h> */
struct input;
struct ui_render_command_buffer;
struct renderer_2d
{
  render_buffers_2d *TextGroup;
  gpu_mapped_ui_buffer SolidQuadGeometryBuffer;

  textured_quad_render_pass TexturedQuadRenderPass;
  gpu_mapped_ui_buffer CustomQuadGeometryBuffer;

  texture SpriteTextureArray;

  u64 InteractionStackTop;

     v2 *MouseP;
     v2 *MouseDP;
     v2 *ScreenDim;
  input *Input;

      ui_toggle_hashtable ToggleTable;
  window_layout_hashtable WindowTable;



#define MAX_MINIMIZED_WINDOWS 64
  window_layout *MinimizedWindowBuffer[MAX_MINIMIZED_WINDOWS];
  window_layout *HighestWindow; // NOTE(Jesse): Highest in terms of InteractionStackIndex

  interactable Hover;
  interactable Clicked;
  interactable Pressed;
  b32 RequestedForceCapture;

  text_box_edit_state TextEdit;

  ui_render_command_buffer *CommandBuffer;

  memory_arena RenderCommandArena;
  memory_arena UiToggleArena;
  memory_arena WindowTableArena;

#define RANDOM_COLOR_COUNT 128
  v3 DebugColors[RANDOM_COLOR_COUNT];

  v3_cursor *ColorPalette;
};

typedef render_buffers_2d debug_text_render_group;
typedef renderer_2d debug_ui_render_group;

struct layout
{
  v2 Basis;

  // Relative to Basis
  v2 At;

  // Relative to Basis
  rect2 DrawBounds = InvertedInfinityRectangle();

  v4 Padding;

  layout* Prev;
};

struct table_info
{
  u32 ColumnCount;
  u32 RowCount;
  r32 *ColumnWidths;

  u32 OnePastTableEnd;
};

struct render_state
{
  window_layout*  Window;
  u32 WindowStartCommandIndex;

  rect2 ClipRect = DISABLE_CLIPPING;

  layout* Layout;

  b32 Hover;
  b32 Pressed;
  b32 Clicked;
};









/***************************                **********************************/
/*************************** Command Buffer **********************************/
/***************************                **********************************/



enum ui_texture_slice
{
  UiTextureSlice_White,
  UiTextureSlice_Font,
  UiTextureSlice_Count,
};

enum ui_element_alignment_flags
{
  UiElementAlignmentFlag_LeftAlign  = 0,
  UiElementAlignmentFlag_RightAlign = (1 << 0),
};

enum ui_element_layout_flags
{
  UiElementLayoutFlag_NoAdvance       = 0,
  UiElementLayoutFlag_AdvanceLayout   = (1 << 0),
  UiElementLayoutFlag_AdvanceClip     = (1 << 1),
  UiElementLayoutFlag_DisableClipping = (1 << 2),

  UiElementLayoutFlag_Default = (UiElementLayoutFlag_AdvanceLayout|UiElementLayoutFlag_AdvanceClip),
};

/* enum ui_element_layout_flags */
/* { */
/*   TextRenderParam_Default         = 0, */
/*   TextRenderParam_NoAdvanceLayout = (1 << 0), */
/*   TextRenderParam_DisableClipping = (1 << 1), */
/* }; */


enum button_params
{
  ButtonParam_Undefined    = 0,
  /* ButtonParam_DiscardButtonDrawBounds = (1 << 0), */
  ButtonParam_ToggleButton = (1 << 1),
};

enum relative_position
{
  Position_None,

  Position_LeftOf,
  Position_RightOf,
  Position_Above,
  Position_Below,
};

enum z_depth
{
  zDepth_Background,
  zDepth_TitleBar,
  zDepth_Text,
  zDepth_Border,

  zDepth_GlobalModal,
};






struct font
{
  v2 Size;
};

inline font
MakeFont(v2 Size)
{
  font Result = {};
  Result.Size = Size;
  return Result;
}

/* debug_global f32 Global_DefaultFontScale = 1.0f; */
/* debug_global f32 Global_DefaultFontScale = 0.75f; */
/* debug_global f32 Global_DefaultFontScale = 0.6f; */
/* debug_global f32 Global_DefaultFontScale = 0.5f; */
debug_global f32 Global_DefaultFontScale = 0.45f;

debug_global v2 Global_DefaultFontSize = V2(26, 34);

// TODO(Jesse, id: 77, tags: font, cleanup): Axe this!
debug_global font Global_Font = {
  .Size = Global_DefaultFontSize * Global_DefaultFontScale,
};

debug_global r32 Global_FontKerningTweak = -2.5f;

// TODO(Jesse, tags: font, cleanup): Axe this!
debug_global font Global_SmallFont =  {
  .Size = Global_DefaultFontSize * Global_DefaultFontScale*0.75f,
};

global_variable r32 Global_TitleBarPadding = Global_Font.Size.y*0.2f;
global_variable r32 Global_TitleBarHeight = Global_Font.Size.y + (Global_TitleBarPadding*2.f);

struct ui_element_reference
{
  u32 Index;
};

struct ui_relative_position_reference
{
     relative_position Position;
  ui_element_reference RelativeTo;
};

struct ui_style
{
  v3 Color;

  v3 HoverColor;
  v3 PressedColor;
  v3 ClickedColor;

  font Font; // TODO(Jesse): Move into ui_render_params
};

struct ui_render_params
{
  ui_relative_position_reference RelativePosition;

  ui_style *FStyle; // foreground
  ui_style *BStyle; // background

  v2 Offset;
  v4 Padding;

  ui_element_alignment_flags  AlignFlags;
  ui_element_layout_flags     LayoutFlags;
};

link_internal ui_style UiStyleFromLightestColor(v3 Color, font *Font = &Global_Font);
link_internal ui_style FlatUiStyle(v3 Color, font *Font = &Global_Font);

debug_global v4 DatastructureIndent = V4(Global_Font.Size.x*2, 0, 0, 0);
debug_global v4 DefaultDatastructurePadding = V4(5, 5, 0, 0);

debug_global v4 DefaultColumnPadding       = V4( 3, 3, 30, 3);

debug_global v4 DefaultButtonPadding       = V4(10, 3, 10, 3);
debug_global v4 DefaultCheckboxPadding     = DefaultButtonPadding;
debug_global v4 DefaultToggleButtonPadding = DefaultButtonPadding;

debug_global v4 DefaultGenericPadding           = V4( 3, 3, 3, 3);
debug_global v4 DefaultGenericVerticalPadding   = V4( 0, 3, 0, 3);
debug_global v4 DefaultGenericHorizontalPadding = V4( 3, 0, 3, 0);
debug_global v4 DefaultZeroPadding              = V4( 0, 0, 0, 0);
/* debug_global v4 DefaultColumnPadding         = V4(0); */
/* debug_global v4 DefaultButtonPadding         = V4(15); */
/* debug_global v4 DefaultButtonPadding         = V4(0); */


global_variable v2 Global_ResizeHandleDim = V2(15);

#define UI_WINDOW_BORDER_DEFAULT_WIDTH     (V4(2.f))

#define UI_SATURATE_FACTOR (1.95f)
#define UI_MUTE_FACTOR (0.8f)

/* #define UI_WINDOW_BEZEL_DEFAULT_COLOR      (V3(0.25f,  0.07f, 0.25f)) */
/* #define UI_WINDOW_BEZEL_DEFAULT_COLOR      (V3(93.f,  51.f, 107.f)/255.f) */
#define UI_WINDOW_BEZEL_DEFAULT_COLOR      (V3(42.f,  24.f, 48.f)/255.f)
#define UI_WINDOW_BACKGROUND_DEFAULT_COLOR (V3(0.07f, 0.01f, 0.08f))
#define UI_COLOR_DEFAULT_SELECTED          (V3( 0.7f,  1.0f, 0.7f ))

#define UI_WINDOW_BEZEL_DEFAULT_COLOR_MUTED      (UI_MUTE_FACTOR*UI_WINDOW_BEZEL_DEFAULT_COLOR)
#define UI_WINDOW_BACKGROUND_DEFAULT_COLOR_MUTED (UI_MUTE_FACTOR*UI_WINDOW_BACKGROUND_DEFAULT_COLOR)
#define UI_COLOR_DEFAULT_SELECTED_MUTED          (UI_MUTE_FACTOR*UI_COLOR_DEFAULT_SELECTED)

#define UI_WINDOW_BEZEL_DEFAULT_COLOR_SATURATED      (UI_SATURATE_FACTOR*UI_WINDOW_BEZEL_DEFAULT_COLOR)
#define UI_WINDOW_BACKGROUND_DEFAULT_COLOR_SATURATED (UI_SATURATE_FACTOR*UI_WINDOW_BACKGROUND_DEFAULT_COLOR)
#define UI_COLOR_DEFAULT_SELECTED_SATURATED          (UI_SATURATE_FACTOR*UI_COLOR_DEFAULT_SELECTED)


#define UI_COLOR_DEFAULT               (V3(0.95f))
#define UI_COLOR_DEFAULT_BLURRED       (V3(0.35f))
#define UI_COLOR_DEFAULT_DISABLED      (V3(0.4f, 0.3f, 0.3f))
#define UI_COLOR_DEFAULT_ERROR         (V3(0.8f, 0.3f, 0.1f))
#define UI_COLOR_DEFAULT_WARNING       (V3(0.6f, 0.3f, 0.1f))

#define UI_HOVER_HIGHLIGHT_DISABLED    (V3(-1.f))

debug_global ui_style DefaultStyle         = UiStyleFromLightestColor(UI_COLOR_DEFAULT);
debug_global ui_style DefaultSelectedStyle = UiStyleFromLightestColor(UI_COLOR_DEFAULT_SELECTED);
debug_global ui_style DefaultBlurredStyle  = UiStyleFromLightestColor(UI_COLOR_DEFAULT_BLURRED);
debug_global ui_style DefaultDisabledStyle = UiStyleFromLightestColor(UI_COLOR_DEFAULT_DISABLED);
debug_global ui_style DefaultErrorStyle    = UiStyleFromLightestColor(UI_COLOR_DEFAULT_ERROR);
debug_global ui_style DefaultWarningStyle  = UiStyleFromLightestColor(UI_COLOR_DEFAULT_WARNING);


debug_global ui_style DefaultBackgroundStyle       = UiStyleFromLightestColor(UI_WINDOW_BACKGROUND_DEFAULT_COLOR);
debug_global ui_style DefaultWindowBezelStyle      = UiStyleFromLightestColor(UI_WINDOW_BEZEL_DEFAULT_COLOR_SATURATED);
debug_global ui_style SaturatedWindowBezelStyle    = UiStyleFromLightestColor(UI_WINDOW_BEZEL_DEFAULT_COLOR_SATURATED);
debug_global ui_style DefaultWindowBackgroundStyle = UiStyleFromLightestColor(UI_WINDOW_BACKGROUND_DEFAULT_COLOR);
debug_global ui_style DefaultButtonBackgroundStyle = UiStyleFromLightestColor(UI_WINDOW_BACKGROUND_DEFAULT_COLOR_SATURATED);

debug_global ui_style Global_DefaultCheckboxForeground = UiStyleFromLightestColor(UI_WINDOW_BEZEL_DEFAULT_COLOR_SATURATED);
debug_global ui_style Global_DefaultCheckboxBackground = UiStyleFromLightestColor(UI_WINDOW_BEZEL_DEFAULT_COLOR_MUTED);

debug_global ui_style Global_DefaultSuccessStyle = UiStyleFromLightestColor(V3(0.1f, 0.9f, 0.1f));;
debug_global ui_style Global_DefaultWarnStyle    = UiStyleFromLightestColor(V3(1.f,  0.5f, 0.2f));
debug_global ui_style Global_DefaultErrorStyle   = UiStyleFromLightestColor(V3(1.f,  0.2f, 0.1f));

global_variable ui_render_params DefaultUiRenderParams_Button =
{
  {},
  &DefaultStyle,
  &DefaultButtonBackgroundStyle,
  {},
  DefaultButtonPadding,
  UiElementAlignmentFlag_LeftAlign,
  UiElementLayoutFlag_Default,
};


global_variable ui_render_params DefaultUiRenderParams_ButtonSelected =
{
  {},
  &DefaultSelectedStyle,
  &DefaultButtonBackgroundStyle,
  {},
  DefaultButtonPadding,
  UiElementAlignmentFlag_LeftAlign,
  UiElementLayoutFlag_Default,
};

global_variable ui_render_params DefaultUiRenderParams_Checkbox =
{
  {},
  &DefaultStyle,
  &DefaultButtonBackgroundStyle,
  {},
  DefaultCheckboxPadding,
  UiElementAlignmentFlag_LeftAlign,
  UiElementLayoutFlag_Default,
};

global_variable ui_render_params DefaultUiRenderParams_Column =
{
  {},
  &DefaultStyle,
  &DefaultBackgroundStyle,
  {},
  DefaultColumnPadding,
  UiElementAlignmentFlag_LeftAlign,
  UiElementLayoutFlag_Default,
};

global_variable ui_render_params DefaultUiRenderParams_Generic =
{
  {},
  &DefaultStyle,
  &DefaultBackgroundStyle,
  {},
  DefaultGenericPadding,
  UiElementAlignmentFlag_LeftAlign,
  UiElementLayoutFlag_Default,
};

global_variable ui_render_params DefaultUiRenderParams_GenericHorizontal =
{
  {},
  &DefaultStyle,
  &DefaultBackgroundStyle,
  {},
  DefaultGenericHorizontalPadding,
  {},
  UiElementLayoutFlag_Default,
};


global_variable ui_render_params DefaultUiRenderParams_Blank =
{
  {},
  &DefaultStyle,
  &DefaultBackgroundStyle,
  {},
  DefaultZeroPadding,
  {},
  UiElementLayoutFlag_Default,
};


// NOTE(Jesse): We use blank styling because the toolbar is just a container
// .. it's got buttons inside that already have their own padding/spacing.
global_variable ui_render_params DefaultUiRenderParams_Toolbar = DefaultUiRenderParams_Blank;


link_internal r32
CharHeights(u32 N)
{
  r32 Result = Global_Font.Size.y * N;
  return Result;
}

link_internal r32
CharWidths(u32 N)
{
  r32 Result = Global_Font.Size.x * N;
  return Result;
}

link_internal void
SetGlobalFontScale(r32 Scale)
{
  Global_Font.Size = Global_DefaultFontSize * Scale;
  Global_SmallFont.Size = Global_DefaultFontSize * Scale * 0.75f;

  DatastructureIndent = V4(Global_Font.Size.x*2, 0, 0, 0);

  Global_TitleBarPadding = Global_Font.Size.y*0.2f;
  Global_TitleBarHeight = Global_Font.Size.y + (Global_TitleBarPadding*2.f);

  DefaultStyle         = UiStyleFromLightestColor(V3(1));
  DefaultSelectedStyle = UiStyleFromLightestColor(V3(.6f, 1.f, .6f));
  DefaultBlurredStyle  = UiStyleFromLightestColor(V3(.25f, .25f, .25f));

  Global_DefaultSuccessStyle = UiStyleFromLightestColor(V3(0.f, 1.f, 0.f));;
  Global_DefaultWarnStyle    = UiStyleFromLightestColor(V3(1.f, .5f, .2f));
  Global_DefaultErrorStyle   = UiStyleFromLightestColor(V3(1.f, 0.25f, 0.f));

  Info("Font Scale (%f) Font Size (%f, %f)", double(Scale), (double)Global_Font.Size.x, (double)Global_Font.Size.y);
}










struct ui_render_command_rel_border
{
  window_layout* Window;
  v2 Dim;
  v3 Color;
  v4 Thickness;
  z_depth zDepth;
};

struct ui_render_command_abs_border
{
  window_layout* Window;
  rect2 Bounds;
  v3 Color;
  v4 Thickness;
  rect2 ClipRect;
};

struct ui_render_command_window_start
{
  layout Layout;
  rect2 ClipRect;
  window_layout* Window;
};

struct ui_render_command_window_end
{
  window_layout* Window;
};

struct ui_render_command_column_start
{
  layout Layout;
  ui_style Style;
  r32 Width;
  r32 MaxWidth;
  ui_element_alignment_flags      AlignFlags;
  /* ui_element_layout_flags LayoutFlags; */ // NOTE(Jesse): Asking for a column that doesn't advance the layout is kinda just nonsense ..?
};

struct ui_render_command_column_end
{
  u32 StartCommandIndex;
};

struct ui_render_command_text
{
  layout Layout;
  ui_style Style;
  counted_string String;
  v2 Offset;
  rect2 Clip;
  ui_element_layout_flags Params;
};

struct ui_render_command_text_at
{
  counted_string Text;
  v2 At;
  rect2 Clip;
  /* font Font; */
};

typedef void (shader_setup_callback)(void*);

struct ui_render_command_untextured_quad
{
  layout Layout;
  ui_style Style;
  v2 QuadDim;
  z_depth zDepth;
  ui_element_layout_flags Params;

  shader_setup_callback *ShaderSetupCallback;
  void *ShaderSetupArgs;

  // @render_command_layout_engine_crutch
  r32 LayoutZ;
  rect2 LayoutClip;
};

struct ui_render_command_untextured_quad_at
{
  v2 QuadDim;
  z_depth zDepth;
  ui_element_layout_flags Params;
  ui_style Style;
  layout Layout;
};

struct ui_render_command_textured_quad
{
  ui_element_layout_flags Params;

  s32 TextureSlice;
  texture *Texture;

  b32 IsDepthTexture;
  b32 HasAlphaChannel;

  layout Layout;
  // TODO(Jesse): Add this
  /* ui_style Style; */
  v2 QuadDim;
  z_depth zDepth;
  v3 Tint;

  // NOTE(Jesse): Written to by the layout engine, not for use by external code
  // At the moment, only valid for TexturedQuadSource_Discrete
  //
  // @render_command_layout_engine_crutch
  r32 LayoutZ;
  rect2 LayoutClip;
};


struct ui_render_command_button_start
{
  ui_id ID;
  ui_style BStyle;
  button_params Params;
};

struct ui_render_command_button_end
{
};

struct ui_render_command_table_start
{
  layout Layout;
  ui_style Style;

  relative_position Position;
  ui_element_reference RelativeTo;

  table_info TableInfo;
};

struct ui_render_command_table_end
{
  u32 TableStartIndex; // NOTE(Jesse): Set by PreprocessTable
};

struct ui_render_command_force_advance
{
  v2 Offset;
};

struct ui_render_command_force_update_basis
{
  v2 Offset;
};



poof(
  d_union ui_render_command
  {
    ui_render_command_window_start
    ui_render_command_window_end

    ui_render_command_button_start
    ui_render_command_button_end

    ui_render_command_table_start
    ui_render_command_table_end

    ui_render_command_column_start
    ui_render_command_column_end

    ui_render_command_text
    ui_render_command_text_at

    ui_render_command_textured_quad
    ui_render_command_untextured_quad
    ui_render_command_untextured_quad_at

    ui_render_command_rel_border
    ui_render_command_abs_border

    ui_render_command_force_advance
    ui_render_command_force_update_basis

    ui_render_command_new_row           enum_only
    ui_render_command_reset_draw_bounds enum_only

    ui_render_command_debug             enum_only
  }
)
#include <generated/d_union_ui_render_command.h>





#define MAX_UI_RENDER_COMMAND_COUNT (4096*4096)

struct ui_render_command_buffer
{
  u32 CommandCount;
  ui_render_command Commands[MAX_UI_RENDER_COMMAND_COUNT];
};







/******************************               ********************************/
/******************************      Misc     ********************************/
/******************************               ********************************/


enum quad_shaping_op
{
  QuadShapingOp_None,
  QuadShapingOp_Circle,
};

enum clip_status
{
  ClipStatus_NoClipping,
  ClipStatus_PartialClipping,
  ClipStatus_FullyClipped
};

struct clip_result
{
  clip_status ClipStatus;

  v2 ClippedMin;
  v2 ClippedMax;

  rect2 ClipRatio;
};

struct window_sort_params
{
  u32 Count;
  u64 LowestInteractionStackIndex;

  sort_key* SortKeys;
};

struct find_button_start_result
{
  u32 Index;
  ui_render_command_button_start* Command;
};

struct find_command_result
{
  ui_render_command* Command;
  u32 Index;
};







typedef b32 (*file_traversal_filter_function)(file_traversal_node*);

struct filtered_file_traversal_helper_params
{
  window_layout *Window;
  file_traversal_filter_function FilterFunction;
};







// FIXME(Jesse): Pretty sure this is unnecessary if we just draw the shadow first
// @shadow_epsilon
#define DEBUG_FONT_SHADOW_EPSILON (0.0000001f)

link_internal r32
GetZ(z_depth zDepth, window_layout* Window)
{
  // @shadow_epsilon
  r32 Result = DEBUG_FONT_SHADOW_EPSILON;

  if (Window)
  {
    switch (zDepth)
    {
      case zDepth_Background:
      {
        Result = Window->zBackground;
      } break;
      case zDepth_Text:
      {
        Result = Window->zText;
      } break;
      case zDepth_TitleBar:
      {
        Result = Window->zTitleBar;
      } break;
      case zDepth_Border:
      {
        Result = Window->zBorder;
      } break;

      case zDepth_GlobalModal:
      {
        Result = 1.f;
      } break;

    }
  }
  else
  {
    if (zDepth == zDepth_GlobalModal)
    {
      Result = 1.f;
    }
  }

  return Result;
}

link_internal ui_style
FlatUiStyle(v3 Color, font *Font)
{
  ui_style Style  = {
    .Color        = Color,
    .HoverColor   = Color,
    .PressedColor = Color,
    .ClickedColor = Color,
    .Font         = *Font,
  };
  return Style;
}

link_internal ui_style
UiStyleFromLightestColor(v3 Color, font *Font)
{
  ui_style Style  = {
    .Color        = Color,
    .HoverColor   = Color*1.1f,
    .PressedColor = Color*1.2f,
    .ClickedColor = Color*1.2f,
    /* .ActiveColor  = V3(.85f, 1.f, .85f), */

    .Font         = *Font,

    /* .IsActive     = False, */
  };

  return Style;
}

global_variable v2 DefaultWindowSize = V2(1800, 800);
global_variable f32 DefaultWindowTopOffset = 200.f;
global_variable f32 DefaultWindowSideOffset = 12.f;

link_internal v2
DefaultWindowBasis(v2 ScreenDim, v2 WindowDim = DefaultWindowSize)
{
  v2 Basis = V2(DefaultWindowSideOffset, ScreenDim.y - WindowDim.y - DefaultWindowSideOffset);
  return Basis;
}

link_internal v2
AlignRightWindowBasis(v2 ScreenDim, v2 WindowDim = DefaultWindowSize)
{
  v2 Basis = V2(ScreenDim.x - WindowDim.x - DefaultWindowSideOffset, DefaultWindowSideOffset);
  return Basis;
}

link_internal window_layout
WindowLayout(const char* Title, v2 Basis, v2 MaxClip = DefaultWindowSize, window_layout_flags Flags = WindowLayoutFlag_Default)
{
  local_persist u32 NextWindowStackIndex = 0;

  window_layout Window = {};
  Window.Flags = Flags;
  Window.Basis = Basis;
  Window.MaxClip = MaxClip;
  Window.Title = CS(Title);
  Window.InteractionStackIndex = NextWindowStackIndex++;

  return Window;
}

link_internal window_layout
WindowLayout(const char* Title, window_layout_flags Flags = WindowLayoutFlag_None)
{
  return WindowLayout(Title, {}, {}, window_layout_flags(Flags|WindowLayoutFlag_Default));
}

v2
GetAbsoluteAt(layout *Layout)
{
  v2 Result = Layout ? Layout->Basis + Layout->At : V2(0);
  return Result;
}

v2
GetAbsoluteDrawBoundsMin(layout *Layout)
{
  v2 Result = Layout ? Layout->Basis + Layout->DrawBounds.Min : V2(0);
  return Result;
}

v2
GetAbsoluteDrawBoundsMax(layout *Layout)
{
  v2 Result = Layout ? Layout->Basis + Layout->DrawBounds.Max : V2(0);
  return Result;
}

rect2
GetAbsoluteDrawBounds(layout *Layout)
{
  rect2 Result = RectMinMax( GetAbsoluteDrawBoundsMin(Layout), GetAbsoluteDrawBoundsMax(Layout) );
  return Result;
}

inline v2
GetScroll(window_layout *Window)
{
  v2 Result = Window? Window->Scroll : V2(0);
  return Result;
}

inline rect2
GetAbsoluteClip(window_layout *Window)
{
  rect2 Result = Window? RectMinMax(Window->Basis + V2(0, Global_TitleBarHeight), Window->MaxClip+Window->Basis) : DISABLE_CLIPPING;
  return Result;
}

inline v2
GetAbsoluteMaxClip(window_layout *Window)
{
  v2 Result = Window? Window->MaxClip + Window->Basis : DISABLE_CLIPPING_MAX;
  return Result;
}

rect2
GetBounds(window_layout* Window)
{
  rect2 Result = RectMinMax(Window->Basis, GetAbsoluteMaxClip(Window));
  return Result;
}

link_internal rect2
GetWindowBounds(window_layout *Window)
{
  v2 TopLeft = Window->Basis;
  v2 BottomRight = GetAbsoluteMaxClip(Window);
  rect2 Result = RectMinMax(TopLeft, BottomRight);
  return Result;
}

link_internal v2
GetDim(window_layout *Window)
{
  v2 Result = Window->MaxClip;
  return Result;
}


link_internal void DrawUi(renderer_2d *Group, ui_render_command_buffer *CommandBuffer);

link_internal clip_result BufferTexturedQuad( renderer_2d *Group, ui_geometry_buffer *Geo, v2  MinP, v2 Dim, v3 Color, r32 Z, rect2 Clip);
link_internal clip_result BufferTexturedQuad( renderer_2d *Group, ui_geometry_buffer *Geo, rect2, v3 Color, r32 Z, rect2 Clip);
link_internal clip_result BufferTexturedQuad( renderer_2d *Group, s32  TextureSlice, v2  MinP, v2  Dim, rect2  UV, v3  Color, r32  Z, rect2  Clip, rect2 *ClipOptional );
