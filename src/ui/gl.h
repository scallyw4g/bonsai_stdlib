
link_internal void
ClearFramebuffer(framebuffer FBO)
{
  GL.BindFramebuffer(GL_FRAMEBUFFER, FBO.ID);
  GL.Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  GL.BindFramebuffer(GL_FRAMEBUFFER, 0);
}

link_internal void
ClearFramebuffers(render_entity_to_texture_group *Group)
{
  ClearFramebuffer(Group->GameGeoFBO);
}

