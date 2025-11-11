
struct framebuffer
{
  u32 ID;
  u32 Attachments;
};


struct rtt_framebuffer
{
  framebuffer Framebuffer;
      texture DestTexture;
};

poof(static_cursor(rtt_framebuffer, {3}))
#include <generated/static_cursor_rokjL8Dl.h>

poof(circular_buffer_h(rtt_framebuffer_static_cursor_3))
#include <generated/circular_buffer_h_Tg6yrcq1.h>
