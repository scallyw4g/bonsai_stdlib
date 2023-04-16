
#define TryCast(T, Ptr) ((Ptr)->Type == type_##T ? (&(Ptr)->T) : 0)
#define SafeCast(T, Ptr) (T*)(&(Ptr)->T); Assert((Ptr)->Type == type_##T)

#define SafeAccess(T, Ptr) SafeCast(T, Ptr)
#define SafeAccessPtr(T, Ptr) ((Ptr)->T); Assert((Ptr)->Type == type_##T)

#define ITERATE_OVER_AS(prefix, value_ptr)        \
  for (auto prefix##Iter = Iterator((value_ptr)); \
      IsValid(&prefix##Iter);                     \
      Advance(&prefix##Iter))

#define ITERATE_OVER(value_ptr)           \
  for (auto Iter = Iterator((value_ptr)); \
      IsValid(&Iter);                     \
      Advance(&Iter))

#define GET_ELEMENT(I) (&(I).At->Element)

#define SetBitfield(type, Dest, Value) do { \
  Assert( ((Dest)&(Value)) == 0 );          \
  (Dest) = (type)((Dest) | (Value));        \
} while (false)

#define UnsetBitfield(type, Dest, Value) do { \
  Assert( (Dest) & (Value) );                 \
  (Dest) = (type)((Dest) & ~(Value));         \
} while (false)

#define ToggleBitfieldValue(Dest, Value) \
      (Dest) = (Dest) & (Value) ?  ((u32)(Dest) & ~(u32)(Value)) : ((u32)(Dest) | (u32)(Value))

