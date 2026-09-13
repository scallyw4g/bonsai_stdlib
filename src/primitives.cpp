
link_internal b32
AreEqual(base_ptr_relative_edit *Thing1, base_ptr_relative_edit *Thing2)
{
  Assert(Thing1 && Thing2);
  b32 Result = Thing1->LocalOffset == Thing2->LocalOffset;

  if (Result) {Assert(Thing1->Datatype == Thing2->Datatype);}
  return Result;
}
