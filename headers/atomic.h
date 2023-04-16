
inline void*
AtomicReplace( volatile void **Source, void *Exchange)
{
  b32 Replaced = False;
  void *Value = 0;

  while (!Replaced)
  {
    Value = (void*)*Source;
    Replaced = AtomicCompareExchange( Source, Exchange, Value );
  }

  return Value;
}
