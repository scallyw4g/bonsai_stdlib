
poof(block_array(rtt_framebuffer, {8}))
#include <generated/block_array_brTXSu6y.h>

poof(circular_buffer_c(rtt_framebuffer, {static_cursor_3}))
#include <generated/circular_buffer_c_vvQpAEsJ.h>


framebuffer
GenFramebuffer()
{
  framebuffer Framebuffer = {};
  GetGL()->GenFramebuffers(1, &Framebuffer.ID);
  return Framebuffer;
}

link_internal void
ClearFramebuffer(rtt_framebuffer *Framebuffer)
{
  auto GL = GetGL();
  GL->BindFramebuffer(GL_FRAMEBUFFER, Framebuffer->FBO.ID);
  GL->Clear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}

link_internal b32
InitializeRenderToTextureFramebuffer(rtt_framebuffer *Framebuffer, v2i Dim, cs DebugTextureName)
{
  auto GL = GetGL();

  Framebuffer->FBO = GenFramebuffer();
  GL->BindFramebuffer(GL_FRAMEBUFFER, Framebuffer->FBO.ID);

  Framebuffer->DestTexture = MakeTexture_RGBA(Dim, Cast(v4*, 0), DebugTextureName, 1, TextureStorageFormat_RGBA32F);

  FramebufferTexture(&Framebuffer->FBO, &Framebuffer->DestTexture);
  SetDrawBuffers(&Framebuffer->FBO);

  b32 Result = CheckAndClearFramebuffer();
  return Result;
}

link_internal void
DeallocateRenderToTextureFramebuffer(rtt_framebuffer *Framebuffer)
{
  // NOTE(Jesse): This works, I just want it to crash because the case where I was
  // using it is now invalid; we're using a freelist instead.
  NotImplemented;

  auto GL = GetGL();

  GL->DeleteTextures(1, &Framebuffer->DestTexture.ID);
  GL->DeleteFramebuffers(1, &Framebuffer->FBO.ID);

  *Framebuffer = {};
}

void
SetDrawBuffers(framebuffer *FBO)
{
  // TODO(Jesse): Should we just do a stack allocation here with the max?
  //
  u32 *Attachments = Allocate(u32, GetTranArena(), FBO->Attachments);
  for (u32 AttIndex = 0; AttIndex < FBO->Attachments; ++AttIndex)
  {
    Attachments[AttIndex] =  GL_COLOR_ATTACHMENT0 + AttIndex;
  }

  GetGL()->DrawBuffers((s32)FBO->Attachments, Attachments);
}

link_internal void
FramebufferDepthTexture(texture *Tex)
{
  GetGL()->FramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, Tex->ID, 0);
}

link_internal void
FramebufferTexture(framebuffer *FBO, texture *Tex)
{
  Assert(Tex->ID != INVALID_TEXTURE_HANDLE);
  u32 Attachment = FBO->Attachments++;
  GetGL()->FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + Attachment, GL_TEXTURE_2D, Tex->ID, 0);
}

link_internal void
BindFramebuffer(rtt_framebuffer *Framebuffer)
{
  GetGL()->BindFramebuffer(GL_FRAMEBUFFER, Framebuffer->FBO.ID);
}

