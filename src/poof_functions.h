// TODO(Jesse)(bug, poof): If I put this function in bonsai_stdlib/shader.cpp
// poof fails to call it when it gets to engine/shader.cpp
poof(
  func gen_shader_uniform_push(uniform_t)
  {
    shader_uniform *
    PushShaderUniform( memory_arena *Mem, const char *Name, uniform_t.name *Value)
    {
      shader_uniform *Uniform = PushShaderUniform(Mem, Name);
      Uniform->Type = ShaderUniform_(uniform_t.name.to_capital_case);
      Uniform->uniform_t.name.to_capital_case = Value;
      return Uniform;
    }

    shader_uniform *
    GetUniform(memory_arena *Mem, shader *Shader, uniform_t.name *Value, const char *Name)
    {
      shader_uniform *Uniform = PushShaderUniform(Mem, Name, Value);
      Uniform->ID = GetShaderUniform(Shader, Name);
      return Uniform;
    }
  }
);

// TODO(Jesse): This would be better if we could specifically target the anonymous union
// instead of just doing it for every union we find in the type. Probably it'll never
// be a problem, but probably it will be eventually ..
poof(
  func d_union_constructors(DUnionType)
  {
    DUnionType.map_members(M) {
      M.is_union? {

        M.map_members (ConstructorArgT)
        {
          link_internal DUnionType.name
          DUnionType.name.to_capital_case((ConstructorArgT.name) A)
          {
            DUnionType.name Result = {
              .Type = type_(ConstructorArgT.name),
              .(ConstructorArgT.name) = A
            };
            return Result;
          }

          gen_constructor(ConstructorArgT)
        }
      }
    }
  }
)

poof(
  func gen_default_equality_operator(Type)
  {
    link_internal b32
    operator==( Type.name E1, Type.name E2 )
    {
      b32 Reuslt = Type.map_members(M).sep(&&) { E1.(M.name) == E2.(M.name) };
      return Reuslt;
    }

    link_internal b32
    operator!=( Type.name E1, Type.name E2 )
    {
      b32 Reuslt = !(E1 == E2);
      return Reuslt;
    }
  }
)

poof(
  func gen_constructor(Type)
  {
    link_internal Type.name
    Type.name.to_capital_case( Type.map_members(M).sep(,) { M.type M.name.to_capital_case } )
    {
      Type.name Reuslt = {
        Type.map_members(M).sep(,) {
          .M.name = M.name.to_capital_case
        }
      };
      return Reuslt;
    }
  }
)

poof(
  func gen_hetero_vector_operator(type_datatype t1, type_datatype t2, type_poof_symbol Operator)
  {
    t1.member(0, (E)
    {
      E.is_array?
      {
        inline t1.name
        operator(Operator)( t1.name P1, t2.name P2 )
        {
          t1.name Result = {
          E.map_array(Index)
          {
            .(E.name)[Index] = P1.(E.name)[Index] Operator E.type( P2.(E.name)[Index] ),
          }
          };
          return Result;
        }
      }
    })
  }
)

poof(
  func gen_vector_infix_operator(type_datatype Type, type_poof_symbol Operator)
  {
    Type.member(0, (E)
    {
      E.is_array?
      {
        inline Type.name
        operator(Operator)( Type.name P1, Type.name P2 )
        {
          Type.name Result = {
          E.map_array(Index)
          {
            .(E.name)[Index] = P1.(E.name)[Index] Operator P2.(E.name)[Index],
          }
          };
          return Result;
        }
      }
    })
  }
)

poof(
  func gen_vector_infix_operators(type_datatype Type, type_datatype E, type_poof_symbol Operator)
  {
    inline Type.name
    operator(Operator)( Type.name P1, Type.name P2 )
    {
      Type.name Result = {
      E.map_array(Index)
      {
        .(E.name)[Index] = P1.(E.name)[Index] Operator P2.(E.name)[Index],
      }
      };
      return Result;
    }

    inline Type.name
    operator(Operator)( Type.name P1, E.type Scalar )
    {
      Type.name Result = {
      E.map_array(Index)
      {
        .(E.name)[Index] = P1.(E.name)[Index] Operator Scalar,
      }
      };
      return Result;
    }

    inline Type.name
    operator(Operator)( E.type Scalar, Type.name P1 )
    {
      Type.name Result = {
      E.map_array(Index)
      {
        .(E.name)[Index] = Scalar Operator P1.(E.name)[Index],
      }
      };
      return Result;
    }

  }
)

poof(
  func gen_vector_comparator(type_datatype Type, type_datatype E, type_poof_symbol Comparator)
  {
    inline b32
    operator(Comparator)( Type.name P1, Type.name P2 )
    {
      b32 Result = ( E.map_array(Index).sep(&&) { P1.(E.name)[Index] (Comparator) P2.(E.name)[Index] });
      return Result;
    }
  }
)

poof(
  func gen_vector_mut_infix_operators(type_datatype Type, type_datatype E, type_poof_symbol Operator)
  {
    inline Type.name &
    operator(Operator)( Type.name &P1, Type.name P2 )
    {
      E.map_array(Index) {
        P1.(E.name)[Index] (Operator) P2.(E.name)[Index];
      }
      return P1;
    }

    inline Type.name &
    operator(Operator)( Type.name &P1, E.type Scalar )
    {
      E.map_array(Index) {
        P1.(E.name)[Index] (Operator) Scalar;
      }
      return P1;
    }
  }
)

poof(
  func gen_vector_operators(Type)
  {
    Type.member(0, (E)
    {
      E.is_array?
      {
        link_internal void
        DeepCopy((Type.name) *Src, (Type.name) *Dest)
        {
          *Dest = *Src;
        }

        gen_vector_comparator(Type, E, {==})

        // NOTE(Jesse): This is for contaiers that call AreEqual()
        link_internal b32
        AreEqual((Type.name) V1, (Type.name) V2)
        {
          b32 Result = V1 == V2;
          return Result;
        }

        // NOTE(Jesse): Can't gen != because the condition welding it together
        // is not &&, it's ||
        //
        /* gen_vector_comparator(Type, {!=}) */
        inline b32
        operator!=( Type.name P1, Type.name P2 )
        {
          b32 Result = !(P1 == P2);
          return Result;
        }

        gen_vector_comparator(Type, E, {<})

        gen_vector_comparator(Type, E, {<=})

        gen_vector_comparator(Type, E, {>})

        gen_vector_comparator(Type, E, {>=})

        gen_vector_infix_operators(Type, E, {+})

        gen_vector_infix_operators(Type, E, {-})

        gen_vector_infix_operators(Type, E, {*})

        gen_vector_infix_operators(Type, E, {/})

        gen_vector_mut_infix_operators(Type, E, {+=})

        gen_vector_mut_infix_operators(Type, E, {-=})

        gen_vector_mut_infix_operators(Type, E, {*=})

        gen_vector_mut_infix_operators(Type, E, {/=})
      }
      {
        poof_error { (Type.name).member(0) was not an array.  Got name((E.name)) type((E.type)). }
      }
    })
  }
)

poof(
  func gen_hetero_vector_operators(t1, t2)
  {
    t1.member(0, (m0)
    {
      m0.is_array?
      {
        gen_hetero_vector_operator(t1, t2, {+})
        gen_hetero_vector_operator(t1, t2, {-})
        gen_hetero_vector_operator(t1, t2, {*})
        gen_hetero_vector_operator(t1, t2, {/})
      }
      {
        poof_error { (t1.name).member(0) was not an array.  Got name((m0.name)) type((m0.type)). }
      }
    })
  }
)

poof(
  func gen_vector_lerp(vec_t)
  {
    inline vec_t.name
    Lerp(r32 t, vec_t.name P1, vec_t.name P2)
    {
      Assert(t<=1);
      Assert(t>=0);
      vec_t.name Result = (1.0f-t)*P1 + t*P2;
      return Result;
    }
  }
)

/* poof( */
/*   data_func get_vec_base_type(vec_t) -> type_datatype */
/*   { */
/*     vec_t.member(E) (base_array) { */
/*       base_array.is_array? */
/*       { */
/*         return base_array.type */
/*       } */
/*       { */
/*         poof_error { (Type.name).member(0) was not an array.  Got name((base_array.name)) type((base_array.type)). } */
/*       } */
/*     } */
/*   } */
/* ) */

/* poof( */
/*   func gen_vector_area(vec_t) */
/*   { */
/*     vec_t.get_vec_base_type (vec_base_t) */
/*     { */
/*       inline vec_base_t */
/*       Area( vec_t.name Vec ) */
/*       { */
/*         Assert(A.x > 0); */
/*         Assert(A.y > 0); */
/*         vec_base_t Result = A.x * A.y; */
/*         return Result; */
/*       } */
/*     } */
/*   } */
/* ) */

poof(
  func gen_vector_area(vec_t)
  {
    vec_t.member(0, (base_array)
    {
      base_array.is_array?
      {
        inline base_array.type
        Area( vec_t.name Vec )
        {
          Assert(Vec.x > 0);
          Assert(Vec.y > 0);
          base_array.type Result = base_array.map_array(Index).sep(*) { Vec.(base_array.name)[Index] };
          return Result;
        }
      }
      {
        poof_error { (Type.name).member(0) was not an array.  Got name((base_array.name)) type((base_array.type)). }
      }
    })
  }
)

poof(
  func gen_common_vector(vec_t)
  {
    vec_t.member(0, (base_array)
    {
      base_array.is_array?
      {
        inline base_array.type
        LengthSq( vec_t.name Vec )
        {
          base_array.type Result = base_array.map_array(Index).sep(+) { Vec.(base_array.name)[Index]*Vec.(base_array.name)[Index] };
          return Result;
        }

        inline r32
        Length( vec_t.name Vec )
        {
          r32 Result = (r32)sqrt((r64)LengthSq(Vec));
          return Result;
        }

        inline vec_t.name
        Max( vec_t.name A, vec_t.name B )
        {
          vec_t.name Result;
          base_array.map_array(Index)
          {
            Result.(base_array.name)[Index] = Max( A.(base_array.name)[Index], B.(base_array.name)[Index] );
          }
          return Result;
        }

        inline vec_t.name
        Min( vec_t.name A, vec_t.name B )
        {
          vec_t.name Result;
          base_array.map_array(Index)
          {
            Result.(base_array.name)[Index] = Min( A.(base_array.name)[Index], B.(base_array.name)[Index] );
          }
          return Result;
        }

        inline vec_t.name
        Abs( vec_t.name Vec )
        {
          vec_t.name Result;
          base_array.map_array(Index)
          {
            Result.(base_array.name)[Index] = ((base_array.type))Abs( Vec.(base_array.name)[Index] );
          }
          return Result;
        }


        inline vec_t.name
        GetSign( vec_t.name Vec )
        {
          vec_t.name Result;
          base_array.map_array(Index)
          {
            Result.(base_array.name)[Index] = GetSign( Vec.(base_array.name)[Index] );
          }
          return Result;
        }


        inline vec_t.name
        Bilateral( vec_t.name Vec )
        {
          vec_t.name Result;
          base_array.map_array(Index)
          {
            Result.(base_array.name)[Index] = Bilateral( Vec.(base_array.name)[Index] );
          }
          return Result;
        }

        inline vec_t.name
        ClampNegative( vec_t.name V )
        {
          vec_t.name Result = V;
          base_array.map_array(Index)
          {
            if ( V.base_array.name[Index] > base_array.type(0) ) Result.base_array.name[Index] = base_array.type(0);
          }
          return Result;
        }

        inline vec_t.name
        ClampPositive( vec_t.name V )
        {
          vec_t.name Result = V;
          base_array.map_array(Index)
          {
            if ( V.base_array.name[Index] < base_array.type(0) ) Result.base_array.name[Index] = base_array.type(0);
          }
          return Result;
        }


      }
      {
        poof_error { (Type.name).member(0) was not an array.  Got name((base_array.name)) type((base_array.type)). }
      }
    })
  }
)

poof(
  func gen_vector_normalize(vec_t)
  {
    vec_t.member(0, (base_array)
    {
      base_array.is_array?
      {
        inline vec_t.name
        Normalize( vec_t.name Vec, r32 Length)
        {
          if (Length == 0.f) return {};
          vec_t.name Result = Vec/Length;
          return Result;
        }

        inline vec_t.name
        Normalize( vec_t.name Vec )
        {
          vec_t.name Result = Normalize(Vec, Length(Vec));
          return Result;
        }
      }
      {
        poof_error { (Type.name).member(0) was not an array.  Got name((base_array.name)) type((base_array.type)). }
      }
    })
  }
)

poof(
  func index_of(Type)
  {
    link_internal umm
    IndexOf((Type.name)_buffer *Buf, (Type.name) Element)
    {
      umm Result = Buf->Count;

      for (u32 ElementIndex = 0;
          ElementIndex < Buf->Count;
          ++ElementIndex)
      {
        if (AreEqual(Buf->Start[ElementIndex], Element))
        {
          Result = ElementIndex;
          break;
        }
      }

      return Result;
    }
  }
)

poof(
  func are_equal(Type)
  {
    link_internal b32
    AreEqual((Type.name) *Thing1, (Type.name) *Thing2)
    {
      b32 Result = MemoryIsEqual((u8*)Thing1, (u8*)Thing2, sizeof( (Type.name) ) );
      return Result;
    }
  }
)

poof(
  func tuple(type_poof_symbol Types)
  {
    struct tuple_(Types.map.sep(_) (T) {(T.name)})
    {
      Types.map (T, Index) {
        (T.name) E(Index);
      }
    };

    /* poof_bind( TupleType <- { tuple_(Types.map.sep(_) (T) {(T.name)} } )*/
    /* gen_constructor(TupleType) */

    link_internal tuple_(Types.map.sep(_) (T) {(T.name)})
    Tuple( Types.map.sep(,) (T, Index) { (T.name) E(Index)} )
    {
      /* TupleType Result = */
      tuple_(Types.map.sep(_) (T, Index) {(T.name)}) Result =
      {
        Types.map (T, Index)
        {
          .E(Index) = E(Index),
        }
      };
      return Result;
    }

  }
)

  // TODO(Jesse): Replace this with stream?  Probably
poof(

  func buffer_builder(Type)
  {
    struct (Type.name)_buffer_builder
    {
      (Type.name)_stream Chunks;
    };

    link_internal void
    Append( (Type.name)_buffer_builder* Builder, (Type.name) E)
    {
      Push(&Builder->Chunks, E);
    }

    struct (Type.name)_buffer
    {
      umm Count;
      (Type.name) *E;
    };

    link_internal (Type.name)_buffer
    (Type.name.to_capital_case)Buffer(umm TotalElements, memory_arena *PermMemory)
    {
      (Type.name)_buffer Result = {};
      Result.Count = TotalElements;
      Result.E = Allocate( (Type.name), PermMemory, TotalElements);
      return Result;
    }

    link_internal (Type.name)_buffer
    Finalize( (Type.name)_buffer_builder *Builder, memory_arena *PermMemory)
    {
      TIMED_FUNCTION();

      u32 TotalElements = 0;

      // TODO(Jesse): Keep track of # of chunks?
      ITERATE_OVER(&Builder->Chunks)
      {
        ++TotalElements;
      }

      auto Result = (Type.name.to_capital_case)Buffer(TotalElements, PermMemory);

      u32 ElementIndex = 0;
      ITERATE_OVER(&Builder->Chunks)
      {
        auto At = GET_ELEMENT(Iter);
        Result.E[ElementIndex] = *At;
        Assert(ElementIndex < Result.Count);
        ++ElementIndex;
      }
      Assert(ElementIndex == Result.Count);

      return Result;
    }
  }

)




// TODO(Jesse): Should the hashes this operates on not be 64-bit?
poof( func hashtable(Type) { (hashtable_struct(Type)) (hashtable_impl(Type)) })

poof(
  func hashtable_struct(Type)
  {
    struct (Type.name)_linked_list_node
    {
      (Type.name) Element;
      (Type.name)_linked_list_node *Next;
    };

    struct (Type.name)_hashtable
    {
      umm Size;
      (Type.name)_linked_list_node **Elements;
    };
  }
);

poof(
  func hashtable_impl(Type)
  {
    link_internal (Type.name)_linked_list_node *
    Allocate_(Type.name)_linked_list_node(memory_arena *Memory)
    {
      (Type.name)_linked_list_node *Result = Allocate( (Type.name)_linked_list_node, Memory, 1);
      return Result;
    }

    link_internal (Type.name)_hashtable
    Allocate_(Type.name)_hashtable(umm ElementCount, memory_arena *Memory)
    {
      (Type.name)_hashtable Result = {};
      Result.Elements = Allocate( (Type.name)_linked_list_node*, Memory, ElementCount);
      Result.Size = ElementCount;
      return Result;
    }

    link_internal (Type.name)_linked_list_node *
    GetHashBucket(umm HashValue, (Type.name)_hashtable *Table)
    {
      Assert(Table->Size);
      (Type.name)_linked_list_node *Result = Table->Elements[HashValue % Table->Size];
      return Result;
    }

    link_internal Type.name *
    GetFirstAtBucket(umm HashValue, (Type.name)_hashtable *Table)
    {
      (Type.name)_linked_list_node *Bucket = GetHashBucket(HashValue, Table);
      (Type.name) *Result = &Bucket->Element;
      return Result;
    }

    link_internal Type.name *
    Insert((Type.name)_linked_list_node *Node, (Type.name)_hashtable *Table)
    {
      Assert(Table->Size);
      umm HashValue = Hash(&Node->Element) % Table->Size;
      (Type.name)_linked_list_node **Bucket = Table->Elements + HashValue;
      while (*Bucket) Bucket = &(*Bucket)->Next;
      *Bucket = Node;
      return &Bucket[0]->Element;
    }

    link_internal (Type.name)*
    Insert((Type.name) Element, (Type.name)_hashtable *Table, memory_arena *Memory)
    {
      (Type.name)_linked_list_node *Bucket = Allocate_(Type.name)_linked_list_node(Memory);
      Bucket->Element = Element;
      Insert(Bucket, Table);
      return &Bucket->Element;
    }
  }
)

poof(
  func hashtable_get(Type, type_poof_symbol key_type, type_poof_symbol key_name)
  {
    maybe_(Type.name)
    GetBy(key_name)( (Type.name)_hashtable *Table, key_type key_name )
    {
      maybe_(Type.name) Result = {};

      auto *Bucket = GetHashBucket(umm(Hash(&key_name)), Table);
      while (Bucket)
      {
        auto E = &Bucket->Element;

        if (AreEqual(E->key_name, key_name))
        {
          Result.Tag = Maybe_Yes;
          Result.Value = *E;
          break;
        }
        else
        {
          Bucket = Bucket->Next;
        }
      }

      return Result;


    }
  }
);

poof(
  func dunion_debug_print_prototype(DUnion)
  {
    link_internal void DebugPrint( (DUnion.type) *Struct, u32 Depth = 0);
    link_internal void DebugPrint( (DUnion.type) Struct, u32 Depth = 0);
  }
)

poof(
  func dunion_debug_print(DUnion)
  {
    dunion_debug_print_prototype(DUnion)

    DUnion.map_members (M)
    {
      M.is_union?
      {
        M.map_members (UnionMember)
        {
          debug_print_struct(UnionMember)
        }
      }
    }

    link_internal void
    DebugPrint( (DUnion.type) Struct, u32 Depth)
    {
      DebugPrint("DUnion.type {\n", Depth);

      switch(Struct.Type)
      {
        DUnion.map_members (M)
        {
          M.is_union?
          {
            M.map_members (UnionMember)
            {
              case type_(UnionMember.type):
              {
                DebugPrint(Struct.(UnionMember.name), Depth+4);
              } break;
            }
          }
        }

        default : { DebugPrint("default while printing ((DUnion.type)) ((DUnion.name)) ", Depth+4); DebugLine("Type(%d)", Struct.Type); } break;
      }
      DebugPrint("}\n", Depth);
    }

    link_internal void
    DebugPrint( (DUnion.type) *Struct, u32 Depth)
    {
      if (Struct)
      {
        DebugPrint(*Struct, Depth);
      }
    }
  }
)

poof(
  func staticbuffer(Type, type_poof_symbol StaticCount, type_poof_symbol TypeName )
  {
    struct TypeName
    {
      Type.name Start[StaticCount];
    };

    link_inline (Type.name)*
    GetPtr( TypeName *Buf, umm Index)
    {
      Type.name *Result = {};
      if ( Index < umm((StaticCount)) )
      {
        Result = Buf->Start+Index;
      }
      return Result;
    }

    link_inline (Type.name)
    Get( TypeName *Buf, umm Index)
    {
      Assert(Index >= 0);
      Assert(Index < umm((StaticCount)));
      Type.name Result = Buf->Start[Index];
      return Result;
    }

    link_internal umm
    AtElements( TypeName  *Buf)
    {
      return StaticCount;
    }

    link_internal umm
    TotalElements( TypeName *Buf)
    {
      return StaticCount;
    }


  }
)

poof(
  func deep_copy(Type)
  {
    /* link_internal void */
    /* DeepCopy( Type.name *Src, Type.name *Dest) */
    /* { */
    /*   Assert(TotalElements(Src) <= TotalElements(Dest)); */
    /*   IterateOver(Src, Element, ElementIndex) */
    /*   { */
    /*     DeepCopy(Element, Dest->Start+ElementIndex); */
    /*   } */
    /* } */
    link_internal void
    DeepCopy((Type.name)_cursor *Src, (Type.name)_cursor *Dest)
    {
      umm SrcAt = AtElements(Src);
      Assert(SrcAt <= TotalElements(Dest));

      IterateOver(Src, Element, ElementIndex)
      {
        DeepCopy(Element, Dest->Start+ElementIndex);
      }

      Dest->At = Dest->Start+SrcAt;
      Assert(Dest->At <= Dest->End);
    }
  }
)

poof(
  func buffer(Type)
  {
    struct (Type.name)_buffer
    {
      Type.name *Start;
      umm Count;
    };

    link_internal (Type.name)_buffer
    (Type.name.to_capital_case)Buffer(umm ElementCount, memory_arena* Memory)
    {
      (Type.name)_buffer Result = {};

      if (ElementCount)
      {
        Result.Start = Allocate( (Type.name), Memory, ElementCount );
        Result.Count = ElementCount;
      }
      else
      {
        Warn("Attempted to allocate (Type.name)_buffer of 0 length.");
      }

      return Result;
    }

    link_inline umm
    CurrentCount((Type.name)_buffer *Buf)
    {
      umm Result = Buf->Count;
      return Result;
    }

    link_inline (Type.name) *
    Get((Type.name)_buffer *Buf, u32 Index)
    {
      Assert(Index < Buf->Count);
      Type.name *Result = Buf->Start + Index;
      return Result;
    }
  }
)

// TODO(Jesse)(immediate): Rewrite this in terms of a cursor.
poof(
  func generate_stack(Type)
  {
    struct (Type.name)_stack
    {
      Type.name *Start;
      umm At;
      umm End;
    };

    link_internal (Type.name)_stack
    (Type.name.to_capital_case)Stack(umm ElementCount, memory_arena* Memory)
    {
      Type.name *Start = ((Type.name)*)PushStruct(Memory, sizeof((Type.name))*ElementCount, 1, 0);
      (Type.name)_stack Result = {
        .Start = Start,
        .End = ElementCount,
        .At = 0,
      };
      return Result;
    }

    link_internal umm
    CurrentCount((Type.name)_stack *Cursor)
    {
      umm Result = Cursor->At;
      return Result;
    }

    link_internal s32
    LastIndex((Type.name)_stack *Cursor)
    {
      s32 Result = s32(Cursor->At)-1;
      return Result;
    }

    link_internal (Type.name)
    Get((Type.name)_stack *Cursor, umm ElementIndex)
    {
      Assert(ElementIndex < Cursor->At);
      Type.name Result = Cursor->Start[ElementIndex];
      return Result;
    }

    link_internal void
    Set((Type.name)_stack *Cursor, umm ElementIndex, (Type.name) Element)
    {
      umm CurrentElementCount = Cursor->At;
      Assert (ElementIndex <= CurrentElementCount);

      Cursor->Start[ElementIndex] = Element;
      if (ElementIndex == CurrentElementCount)
      {
        Cursor->At++;
      }
    }

    link_internal Type.name *
    Push((Type.name)_stack *Cursor, (Type.name) Element)
    {
      Assert( Cursor->At < Cursor->End );
      Type.name *Result = Cursor->Start+Cursor->At;
      Cursor->Start[Cursor->At++] = Element;
      return Result;
    }

    link_internal Type.name
    Pop((Type.name)_stack *Cursor)
    {
      Assert( Cursor->At > 0 );
      Type.name Result = Cursor->Start[LastIndex(Cursor)];
      Cursor->At--;
      return Result;
    }

    link_internal b32
    RemoveUnordered((Type.name)_stack *Cursor, (Type.name) Query)
    {
      b32 Result = False;
      StackIterator(ElementIndex, Cursor)
      {
        Type.name Element = Get(Cursor, ElementIndex);
        if (AreEqual(Element, Query))
        {
          b32 IsLastIndex = LastIndex(Cursor) == s32(ElementIndex);
          Type.name Tmp = Pop(Cursor);

          if (IsLastIndex) { Assert(AreEqual(Tmp, Query)); }
          else { Set(Cursor, ElementIndex, Tmp); }
          Result = True;
        }
      }
      return Result;
    }
  }
)

poof(
  func generate_cursor(Type)
  {
    (generate_cursor_struct(Type))
    (generate_cursor_functions(Type))
  }
)
poof(
  func generate_cursor_struct(Type)
  {
    struct (Type.name)_cursor
    {
      Type.name *Start;
      // TODO(Jesse)(immediate): For the love of fucksakes change these to indices
      Type.name *At;
      Type.name *End;
    };

  }
)

poof(
  func generate_cursor_functions(Type)
  {
    link_internal (Type.name)_cursor
    (Type.name.to_capital_case)Cursor(umm ElementCount, memory_arena* Memory)
    {
      Type.name *Start = ((Type.name)*)PushStruct(Memory, sizeof((Type.name))*ElementCount, 1, 0);
      (Type.name)_cursor Result = {
        .Start = Start,
        .End = Start+ElementCount,
        .At = Start,
      };
      return Result;
    }

    link_internal (Type.name)*
    GetPtr((Type.name)_cursor *Cursor, umm ElementIndex)
    {
      Type.name *Result = {};
      if (ElementIndex < AtElements(Cursor)) {
        Result = Cursor->Start+ElementIndex;
      }
      return Result;
    }

    link_internal (Type.name)*
    GetPtrUnsafe((Type.name)_cursor *Cursor, umm ElementIndex)
    {
      Type.name *Result = {};
      if (ElementIndex < TotalElements(Cursor)) {
        Result = Cursor->Start+ElementIndex;
      }
      return Result;
    }

    link_internal (Type.name)
    Get((Type.name)_cursor *Cursor, umm ElementIndex)
    {
      Assert(ElementIndex < CurrentCount(Cursor));
      Type.name Result = Cursor->Start[ElementIndex];
      return Result;
    }

    link_internal void
    Set((Type.name)_cursor *Cursor, umm ElementIndex, (Type.name) Element)
    {
      umm CurrentElementCount = CurrentCount(Cursor);
      Assert (ElementIndex <= CurrentElementCount);

      Cursor->Start[ElementIndex] = Element;
      if (ElementIndex == CurrentElementCount)
      {
        Cursor->At++;
      }
    }

    link_internal (Type.name)*
    Advance((Type.name)_cursor *Cursor)
    {
      Type.name * Result = {};
      if ( Cursor->At < Cursor->End ) { Result = Cursor->At++; }
      return Result;
    }

    link_internal Type.name *
    Push((Type.name)_cursor *Cursor, (Type.name) Element)
    {
      Assert( Cursor->At < Cursor->End );
      Type.name *Result = Cursor->At;
      *Cursor->At++ = Element;
      return Result;
    }

    link_internal Type.name
    Pop((Type.name)_cursor *Cursor)
    {
      Assert( Cursor->At > Cursor->Start );
      Type.name Result = Cursor->At[-1];
      Cursor->At--;
      return Result;
    }

    link_internal s32
    LastIndex((Type.name)_cursor *Cursor)
    {
      s32 Result = s32(CurrentCount(Cursor))-1;
      return Result;
    }

    link_internal b32
    Remove((Type.name)_cursor *Cursor, (Type.name) Query)
    {
      b32 Result = False;
      CursorIterator(ElementIndex, Cursor)
      {
        Type.name Element = Get(Cursor, ElementIndex);
        if (AreEqual(Element, Query))
        {
          b32 IsLastIndex = LastIndex(Cursor) == s32(ElementIndex);
          Type.name Tmp = Pop(Cursor);

          if (IsLastIndex) { Assert(AreEqual(Tmp, Query)); }
          else { Set(Cursor, ElementIndex, Tmp); }
          Result = True;
        }
      }
      return Result;
    }


    link_internal b32
    ResizeCursor((Type.name)_cursor *Cursor, umm Count, memory_arena *Memory)
    {
      umm CurrentSize = TotalSize(Cursor);

      TruncateToElementCount(Cursor, Count);
      umm NewSize = TotalSize(Cursor);

      Assert(NewSize/sizeof((Type.name)) == Count);

      /* Info("Attempting to reallocate CurrentSize(%u), NewSize(%u)", CurrentSize, NewSize); */
      Ensure(Reallocate((u8*)Cursor->Start, Memory, CurrentSize, NewSize));
      return 0;
    }

  }
)

poof(
  func generate_string_table(EnumType)
  {
    link_internal counted_string
    ToString( (EnumType.name) Type)
    {
      counted_string Result = {};
      switch (Type)
      {
        EnumType.map_values (EnumValue)
        {
          case EnumValue.name: { Result = CSz("EnumValue.name"); } break;
        }
      }
      return Result;
    }
  }
)

poof(
  func generate_value_table(EnumType)
  {
    link_internal (EnumType.name)
    (EnumType.name.to_capital_case)(counted_string S)
    {
      EnumType.name Result = {};

      EnumType.map_values(TEnumV)
      {
        if (StringsMatch(S, CSz("TEnumV.name"))) { return TEnumV.name; }
      }

      return Result;
    }
  }
)



poof(
  func generate_stream_chunk_struct(Type)
  {
    struct (Type.name)_stream_chunk
    {
      (Type.name) Element;
      (Type.name)_stream_chunk* Next;
    };
  }
)

poof(
  func generate_stream_push(Type)
  {
    link_internal (Type.name) *
    Push((Type.name)_stream* Stream, (Type.name) Element)
    {
      if (Stream->Memory == 0)
      {
        Stream->Memory = AllocateArena();
      }

      /* (Type.name)_stream_chunk* NextChunk = AllocateProtection((Type.name)_stream_chunk*), Stream->Memory, 1, False) */
      (Type.name)_stream_chunk* NextChunk = ((Type.name)_stream_chunk*)PushStruct(Stream->Memory, sizeof((Type.name)_stream_chunk), 1, 0);
      NextChunk->Element = Element;

      if (!Stream->FirstChunk)
      {
        Assert(!Stream->LastChunk);
        Stream->FirstChunk = NextChunk;
        Stream->LastChunk = NextChunk;
      }
      else
      {
        Stream->LastChunk->Next = NextChunk;
        Stream->LastChunk = NextChunk;
      }

      Assert(NextChunk->Next == 0);
      Assert(Stream->LastChunk->Next == 0);

      Stream->ChunkCount += 1;

      Type.name *Result = &NextChunk->Element;
      return Result;
    }
  }
)

poof(
  func generate_stream_struct(Type)
  {
    struct (Type.name)_stream
    {
      memory_arena *Memory;
      (Type.name)_stream_chunk* FirstChunk;
      (Type.name)_stream_chunk* LastChunk;
      umm ChunkCount;
    };

    link_internal void
    Deallocate((Type.name)_stream *Stream)
    {
      Stream->LastChunk = 0;
      Stream->FirstChunk = 0;
      VaporizeArena(Stream->Memory);
    }
  }
)

poof(
  func generate_stream_iterator(Type)
  {
    struct (Type.name)_iterator
    {
      (Type.name)_stream* Stream;
      (Type.name)_stream_chunk* At;
    };

    link_internal (Type.name)_iterator
    Iterator((Type.name)_stream* Stream)
    {
      (Type.name)_iterator Iterator = {
        .Stream = Stream,
        .At = Stream->FirstChunk
      };
      return Iterator;
    }

    link_internal b32
    IsValid((Type.name)_iterator* Iter)
    {
      b32 Result = Iter->At != 0;
      return Result;
    }

    link_internal void
    Advance((Type.name)_iterator* Iter)
    {
      Iter->At = Iter->At->Next;
    }

    link_internal b32
    IsLastElement((Type.name)_iterator* Iter)
    {
      b32 Result = Iter->At->Next == 0;
      return Result;
    }

  }
)

poof(
  func generate_stream_getters(InputTypeDef)
  {
    InputTypeDef.map_members (Member)
    {
      link_internal InputTypeDef.type
      GetBy(Member.name)( (Member.Type) Needle, (InputTypeDef.type)_stream *Haystack)
      {
        // TODO : Implement matching!
      }
    }
  }
)

poof(
  func generate_stream_compact(InputTypeDef)
  {
    link_internal (InputTypeDef.name)_buffer
    Compact((InputTypeDef.name)_stream *Stream, memory_arena *PermMemory)
    {
      (InputTypeDef.name)_buffer Result = {};
      if (Stream->ChunkCount)
      {
        Result = (InputTypeDef.name.to_capital_case)Buffer(Stream->ChunkCount, PermMemory);
        /* DebugLine("compact %u", Result.Count); */

        u32 Index = 0;
        ITERATE_OVER(Stream)
        {
          (InputTypeDef.name) *Spot = GET_ELEMENT(Iter);
          Result.Start[Index] = *Spot;

          ++Index;
        }

        Deallocate(Stream);
      }

      return Result;
    }
  }
)


poof(
  func generate_stream(Type)
  {
    (generate_stream_chunk_struct(Type))
    (generate_stream_struct(Type))
    (generate_stream_iterator(Type))
    (generate_stream_push(Type))
  }
)



poof(
  func string_and_value_tables(Def)
  {
    (generate_string_table(Def))
    (generate_value_table(Def))
  }
)

poof(
  func stream_and_cursor(Def)
  {
    (generate_cursor_struct(Def))
    (generate_cursor_functions(Def))
    (generate_stream(Def))
  }
)

poof(
  polymorphic_func void DebugPrint( @TypeDef RuntimeValue, u32 Depth)
  {
    DebugPrint("TypeDef.name: ", Depth);
    TypeDef.is_enum?
    {
      TypeDef.map_values (ValueDef)
      {
        DebugPrint("ValueDef.name ValueDef.value", Depth+1);
        DebugPrint(ToString(RuntimeValue), Depth+1);
      }
    }

    TypeDef.is_struct?
    {
      TypeDef.map_members (MemberDef)
      {
        DebugPrint("MemberDef.type MemberDef.name: ", Depth);
        DebugPrint(RuntimeValue.(MemberDef.name), Depth+1);
        DebugPrint("\n");
      }
    }
  }
)

poof(
  func debug_print(T)
  {
    T.is_struct?
    {
      debug_print_struct(T)
    }
    {
      T.is_enum?
      {
        debug_print_enum(T)
      }
      {
        poof_error { Unable to generate debug_print for (T) }
      }
    }

  }

)

poof(
  func debug_print_enum(TEnum)
  {
    link_internal void DebugPrint( TEnum.name RuntimeValue, u32 Depth)
    {
      switch (RuntimeValue)
      {
        TEnum.map_values (TValue)
        {
          case TValue.name:
          {
            DebugPrint("TValue.name", Depth);
          } break;
        }
      }
    }
  }
)

poof(
  func debug_print_struct(StructDef)
  {
    link_internal void DebugPrint( StructDef.name RuntimeStruct, u32 Depth = 0)
    {
      /* if (Depth == 0) */
      {
        DebugPrint("StructDef.name {\n", Depth);
      }

      StructDef.map_members (Member)
      {
        Member.is_defined?
        {
          Member.name?
          {
            Member.is_compound?
            {
              DebugPrint("Member.type Member.name {\n", Depth+2);
              DebugPrint(RuntimeStruct.(Member.name), Depth+4);
              DebugPrint("}\n", Depth+2);
            }
            {
              Member.is_function?
              {
                DebugPrint("Member.type Member.name = {function};", Depth+2);
              }
              // primitive
              {
                DebugPrint("Member.type Member.name =", Depth+2);
                DebugPrint(RuntimeStruct.(Member.name), 1);
                DebugPrint(";\n");
              }
            }
          }
          // NOTE(Jesse): an anonymous struct or union
          {
            DebugPrint("Member.type Member.name\n", Depth+2);
          }
        }
        // NOTE(Jesse): found no definition for this type.. probably an OS type
        {
          DebugPrint("poof_undefined_type ((Member.type) (Member.name))\n", Depth+2);
        }
      }

      /* if (Depth == 0) */
      {
        DebugPrint("}\n", Depth);
      }
    }

    link_internal void DebugPrint( StructDef.name *RuntimePtr, u32 Depth = 0)
    {
      if (RuntimePtr) { DebugPrint(*RuntimePtr, Depth); }
      else { DebugPrint("ptr(0)\n", Depth); }
    }
  }
)


poof(
  func maybe(Type)
  {
    struct maybe_(Type.name)
    {
      maybe_tag Tag;
      Type.name Value;
    };
  }
)
