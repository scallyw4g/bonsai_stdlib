/* link_internal b32 */
/* Reallocate(char_cursor *Dest, umm Increment) */
/* { */
/*   b32 Result = False; */
/*   umm CurrentSize = TotalSize(Dest); */
/*   if (Reallocate((u8*)Dest->Start, Dest->Memory, CurrentSize, CurrentSize+Increment)) */
/*   { */
/*     Dest->End += Increment; */
/*     Result = True; */
/*   } */

/*   return Result; */
/* } */

