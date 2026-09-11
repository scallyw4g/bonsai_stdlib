
link_internal b32
AreEqual(primitive_value_changed_record *Thing1, primitive_value_changed_record *Thing2)
{
  Assert(Thing1 && Thing2);
  b32 Result = Thing1->LocalOffset == Thing2->LocalOffset;

  if (Result) {Assert(Thing1->Datatype == Thing2->Datatype);}
  return Result;
}
