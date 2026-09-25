
poof(block_array_c(u32, {8}))
#include <generated/block_array_c$u32.688856411$umSpxLL6.h>

poof(hashtable_impl(u32));
#include <generated/hashtable_impl$u32$86NfpxtK.h>

poof(hashtable_get(u32, {u32}, {Value}));
#include <generated/hashtable_get$u32.688719149.715384584$6rxohQIv.h>


poof(block_array_c(base_ptr_relative_edit, {8}))
#include <generated/block_array_c$base_ptr_relative_edit.688856411$YKhss0Oq.h>

link_internal void
MaybePushChangeRecord(
   base_ptr_relative_edit_block_array *ChangeRecords,
                               primitive_type  Datatype,
                                          u64  Value,
                                          u64  ValuePtr )
{
  if (ChangeRecords)
  {
    base_ptr_relative_edit Record = {};
    Record.Datatype  = Datatype;
    Record.Value     = Value;

    Assert(ChangeRecords->BasePtr);
    Assert(ChangeRecords->BasePtr <= ValuePtr);
    Record.LocalOffset = SafeTruncateToU32(ValuePtr - ChangeRecords->BasePtr);

    Push(ChangeRecords, &Record);
  }
}
