
struct framebuffer
{
  u32 ID;
  u32 Attachments;
};


struct rtt_framebuffer
{
  framebuffer FBO;
      texture DestTexture;
};

poof(static_cursor(rtt_framebuffer, {3}))
#include <generated/static_cursor_rokjL8Dl.h>

poof(circular_buffer_h(rtt_framebuffer, {static_cursor_3}))
#include <generated/circular_buffer_h_Tg6yrcq1.h>

link_internal void
BindFramebuffer(rtt_framebuffer *Framebuffer);

link_internal framebuffer
GenFramebuffer();

link_internal void
SetDrawBuffers(framebuffer *FBO);

link_internal b32
InitializeRenderToTextureFramebuffer(rtt_framebuffer *Framebuffer, v2i Dim, cs DebugTextureName);

link_internal void
FramebufferTexture(framebuffer *FBO, texture *Tex);

link_internal void
FramebufferDepthTexture(texture *Tex);

link_internal void
FramebufferTexture(framebuffer *FBO, texture *Tex);


