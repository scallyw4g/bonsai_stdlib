
template <typename element_t, typename stream_t>inline element_t *
Push(element_t Element, stream_t *Array)
{
  Assert( Array->At < Array->End );
  element_t *Result = Array->At;
  *Array->At++ = Element;
  return Result;
}

template <typename element_t, typename cursor_t> inline element_t
Pop(cursor_t *Cursor)
{
  Assert( Cursor->At > Cursor->Start );
  Cursor->At--;
  element_t Element = *Cursor->At;
  return Element;
}

template <typename stream_t> inline umm
CurrentCount(stream_t* Stream)
{
  umm Result = (umm)(Stream->At - Stream->Start);
  return Result;
}

template <typename stream_t> inline umm
Count(stream_t* Stream)
{
  umm Result = (umm)(Stream->End - Stream->Start);
  return Result;
}

template <typename stream_t> inline void
TruncateToCurrentElements(stream_t* Stream)
{
  Assert(Stream->At <= Stream->End);
  Stream->End = Stream->At;
}

template <typename stream_t> inline void
Rewind(stream_t* Stream)
{
  Stream->At = Stream->Start;
}

template <typename stream_t> inline void
FastForward(stream_t* Stream)
{
  if (Stream->Start < Stream->End)
  {
    Stream->At = Stream->End -1;
  }
}

template <typename T> b32
BufferHasRoomFor(T *Buffer, u32 Count)
{
  b32 Result = (Buffer->At + Count) <= Buffer->End;
  return Result;
}

