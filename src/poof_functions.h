#if BONSAI_INTERNAL
#define ENSURE_OWNED_BY_THREAD(ThingPointer) Assert((ThingPointer)->OwnedByThread == ThreadLocal_ThreadIndex)
#define OWNED_BY_THREAD_MEMBER() s32 OwnedByThread = INVALID_THREAD_LOCAL_THREAD_INDEX
#define OWNED_BY_THREAD_MEMBER_INIT() .OwnedByThread = ThreadLocal_ThreadIndex,
#else 
#define ENSURE_OWNED_BY_THREAD(...)
#define OWNED_BY_THREAD_MEMBER(...)
#define OWNED_BY_THREAD_MEMBER_INIT(...)
#endif

// TODO(Jesse)(bug, poof): If I put this function in bonsai_stdlib/shader.cpp
// poof fails to call it when it gets to engine/shader.cpp
poof(
  func set_shader_uniform(uniform_t)
  {
    b32
    SetShaderUniform(shader *Shader, u32 Index, uniform_t.name *Value, const char *Name)
    {
      Assert(Index < Shader->Uniforms.Count);

      shader_uniform *Uniform = Shader->Uniforms.Start + Index;

      Uniform->Type = ShaderUniform_(uniform_t.name.to_capital_case);
      Uniform->uniform_t.name.to_capital_case = Value;
      Uniform->Name = Name;

      Uniform->ID = GetShaderUniform(Shader, Name);

      return Uniform->ID != INVALID_SHADER_UNIFORM;
    }
  }
);

// TODO(Jesse): This would be better if we could specifically target the anonymous union
// instead of just doing it for every union we find in the type. Probably it'll never
// be a problem, but probably it will be eventually ..
//
// TODO(Jesse): This is insanely ugly.. can we make these  better?
poof(
  func d_union_constructors(DUnionType)
  {
    DUnionType.map_members(M) {
      M.is_union? {

        M.map_members (UnionMemberT)
        {
          link_internal DUnionType.name
          DUnionType.name.to_capital_case( (UnionMemberT.name) A DUnionType.map_members(m_inner) { m_inner.is_named(Type)?  { } { m_inner.is_union?  { } {, (m_inner.type) (m_inner.name)} } } )
          {
            DUnionType.name Result = {
              .Type = type_(UnionMemberT.name),
              .(UnionMemberT.name) = A,

              DUnionType.map_members(m_inner).sep(,) { m_inner.is_named(Type)?  { } { m_inner.is_union?  { } { .(m_inner.name) = (m_inner.name) } } }

            };
            return Result;
          }

        }
      }
    }
  }
)

poof(
  func d_union_all_constructors(type)
  {
    d_union_constructors(type)

    type.map_members(M)
    {
      M.is_union?
      {
        M.map_members (ConstructorArgT)
        {
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
    Type.name.to_capital_case( Type.map_members(M).sep(,) { M.type M.is_pointer?{*}M.is_array?{*}  M.name.to_capital_case } )
    {
      Type.name Reuslt = {
        Type.map_members(M).sep(,) {
          M.is_array?
          {
            .M.name = {}
          }
          {
            .M.name = M.name.to_capital_case
          }
        }
      };

        Type.map_members(M)
        {
          M.is_array?
          {
            RangeIterator(Index, s32((M.tag_value(array_length))))
            {
              Reuslt.M.name[Index] = M.name.to_capital_case[Index];
            }
          }
        }
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
          t1.name Result;
          E.map_array(Index)
          {
            Result.(E.name)[Index] = Cast((E.type), P1.(E.name)[Index] Operator Cast((E.type), P2.(E.name)[Index]));
          }
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
      Type.name Result = {{
      E.map_array(Index)
      {
        Cast((E.type), P1.(E.name)[Index] Operator P2.(E.name)[Index]),
      }
      }};
      return Result;
    }

    inline Type.name
    operator(Operator)( Type.name P1, E.type Scalar )
    {
      Type.name Result = {{
      E.map_array(Index)
      {
        Cast((E.type), P1.(E.name)[Index] Operator Scalar),
      }
      }};
      return Result;
    }

    inline Type.name
    operator(Operator)( E.type Scalar, Type.name P1 )
    {
      Type.name Result = {{
      E.map_array(Index)
      {
        Cast((E.type), Scalar Operator P1.(E.name)[Index]),
      }
      }};
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
  func vector_interpolation_functions(vec_t)
  {
    inline vec_t.name
    Lerp(r32 t, vec_t.name P1, vec_t.name P2)
    {
      Assert(t<=1);
      Assert(t>=0);
      vec_t.name Result = (1.0f-t)*P1 + t*P2;
      return Result;
    }

    // https://paulbourke.net/miscellaneous/interpolation/
    //
    link_internal vec_t.name
    CosineInterpolate( f32 t, vec_t.name y1, vec_t.name y2 )
    {
       f32 t2 = (1.f-Cos(t*PI32))/2.f;
       return(y1*(1.f-t2)+y2*t2);
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
          r32 Result = (r32)SquareRoot(LengthSq(Vec));
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

        inline vec_t.name
        Clamp01( vec_t.name V )
        {
          vec_t.name Result = V;
          base_array.map_array(Index)
          {
            if ( V.base_array.name[Index] < base_array.type(0) ) Result.base_array.name[Index] = base_array.type(0);
            if ( V.base_array.name[Index] > base_array.type(1) ) Result.base_array.name[Index] = base_array.type(1);
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
      if (Thing1 && Thing2)
      {
        Type.contains_type(cs)?
        {
          b32 Result = True;
          Type.map(member)
          {
            member.is_function?
            {
            }
            {
              Result &= AreEqual(Thing1->member.name, Thing2->member.name);
            }
          }
        }
        {
          b32 Result = MemoryIsEqual((u8*)Thing1, (u8*)Thing2, sizeof( (Type.name) ) );
        }
        return Result;
      }
      else
      {
        return (Thing1 == Thing2);
      }
    }

    link_internal b32
    AreEqual((Type.name) Thing1, (Type.name) Thing2)
    {
      Type.contains_type(cs)?
      {
        b32 Result = True;
        Type.map(member)
        {
          member.is_function?
          {
          }
          {
            Result &= AreEqual(Thing1.member.name, Thing2.member.name);
          }
        }
      }
      {
        b32 Result = MemoryIsEqual((u8*)&Thing1, (u8*)&Thing2, sizeof( (Type.name) ) );
      }
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
      b32 Tombstoned;
      (Type.name) Element;
      (Type.name)_linked_list_node *Next;
    };

    struct (Type.name)_hashtable
    {
      umm Size;
      (Type.name)_linked_list_node **Elements;
      /* OWNED_BY_THREAD_MEMBER() */
    };
  }
);

poof(
  func hashtable_impl(Type)
  {
    link_internal b32 AreEqual((Type.name)_linked_list_node *Node1, (Type.name)_linked_list_node *Node2 );

    link_internal (Type.name)_linked_list_node *
    Allocate_(Type.name)_linked_list_node(memory_arena *Memory)
    {
      (Type.name)_linked_list_node *Result = Allocate( (Type.name)_linked_list_node, Memory, 1);
      return Result;
    }

    link_internal (Type.name)_hashtable
    Allocate_(Type.name)_hashtable(umm ElementCount, memory_arena *Memory)
    {
      (Type.name)_hashtable Result = {
        .Elements = Allocate( (Type.name)_linked_list_node*, Memory, ElementCount),
        .Size = ElementCount,
        /* OWNED_BY_THREAD_MEMBER_INIT() */
      };
      return Result;
    }

    link_internal (Type.name)_linked_list_node *
    GetHashBucket(umm HashValue, (Type.name)_hashtable *Table)
    {
      /* ENSURE_OWNED_BY_THREAD(Table); */

      Assert(Table->Size);
      (Type.name)_linked_list_node *Result = Table->Elements[HashValue % Table->Size];
      return Result;
    }

    link_internal Type.name *
    GetFirstAtBucket(umm HashValue, (Type.name)_hashtable *Table)
    {
      /* ENSURE_OWNED_BY_THREAD(Table); */

      (Type.name)_linked_list_node *Bucket = GetHashBucket(HashValue, Table);
      (Type.name) *Result = &Bucket->Element;
      return Result;
    }

    link_internal (Type.name)_linked_list_node**
    GetMatchingBucket((Type.name) Element, (Type.name)_hashtable *Table, memory_arena *Memory)
    {
      umm HashValue = Hash(&Element) % Table->Size;
      (Type.name)_linked_list_node **Bucket = Table->Elements + HashValue;
      while (*Bucket)
      {
        if (AreEqual(&Bucket[0]->Element, &Element)) { break; }
        Bucket = &(*Bucket)->Next;
      }
      return Bucket;
    }

    link_internal Type.name *
    Insert((Type.name)_linked_list_node *Node, (Type.name)_hashtable *Table)
    {
      /* ENSURE_OWNED_BY_THREAD(Table); */

      Assert(Table->Size);
      umm HashValue = Hash(&Node->Element) % Table->Size;
      (Type.name)_linked_list_node **Bucket = Table->Elements + HashValue;
      while (*Bucket)
      {
        /* Assert(!AreEqual(*Bucket, Node)); */
        Bucket = &(*Bucket)->Next;
      }
      *Bucket = Node;
      return &Bucket[0]->Element;
    }

    link_internal (Type.name)*
    Insert((Type.name) Element, (Type.name)_hashtable *Table, memory_arena *Memory)
    {
      /* ENSURE_OWNED_BY_THREAD(Table); */

      (Type.name)_linked_list_node *Bucket = Allocate_(Type.name)_linked_list_node(Memory);
      Bucket->Element = Element;
      Insert(Bucket, Table);
      return &Bucket->Element;
    }

    link_internal (Type.name)*
    Upsert((Type.name) Element, (Type.name)_hashtable *Table, memory_arena *Memory)
    {
      umm HashValue = Hash(&Element) % Table->Size;
      (Type.name)_linked_list_node **Bucket = Table->Elements + HashValue;
      while (*Bucket)
      {
        if (AreEqual(&Bucket[0]->Element, &Element)) { break; }
        Bucket = &(*Bucket)->Next;
      }

      if (*Bucket && Bucket[0]->Tombstoned == False)
      {
        Bucket[0]->Element = Element;
      }
      else
      {
        Insert(Element, Table, Memory);
      }

      return &Bucket[0]->Element;
    }


    //
    // Iterator impl.
    //

    struct (Type.name)_hashtable_iterator
    {
      umm HashIndex;
      (Type.name)_hashtable *Table;
      (Type.name)_linked_list_node *Node;
    };

    link_internal (Type.name)_hashtable_iterator
    operator++( (Type.name)_hashtable_iterator &Iterator )
    {
      if (Iterator.Node)
      {
        Iterator.Node = Iterator.Node->Next;
      }
      else
      {
        Assert (Iterator.HashIndex < Iterator.Table->Size );
        Iterator.Node = Iterator.Table->Elements[++Iterator.HashIndex];
      }

      return Iterator;
    }

    link_internal b32
    operator<( (Type.name)_hashtable_iterator I0, (Type.name)_hashtable_iterator I1)
    {
      b32 Result = I0.HashIndex < I1.HashIndex;
      return Result;
    }

    link_inline (Type.name)_hashtable_iterator
    ZerothIndex((Type.name)_hashtable *Hashtable)
    {
      (Type.name)_hashtable_iterator Iterator = {};
      Iterator.Table = Hashtable;
      Iterator.Node = Hashtable->Elements[0];
      return Iterator;
    }

    link_inline (Type.name)_hashtable_iterator
    AtElements((Type.name)_hashtable *Hashtable)
    {
      (Type.name)_hashtable_iterator Result = { Hashtable->Size, 0, 0 };
      return Result;
    }

    link_inline Type.name *
    GetPtr((Type.name)_hashtable *Hashtable, (Type.name)_hashtable_iterator Iterator)
    {
      Type.name *Result = {};
      if (Iterator.Node)
      {
        Result = &Iterator.Node->Element;
      }
      return Result;
    }

    link_inline Type.name *
    TryGetPtr((Type.name)_hashtable *Hashtable, (Type.name)_hashtable_iterator Iterator)
    {
      return GetPtr(Hashtable, Iterator);
    }

  }
)

poof(
  func hashtable_get(Type, type_poof_symbol key_type, type_poof_symbol key_name)
  {
    (Type.name)_linked_list_node*
    GetBucketBy(key_name)( (Type.name)_hashtable *Table, key_type Query )
    {
      /* ENSURE_OWNED_BY_THREAD(Table); */

      (Type.name)_linked_list_node* Result = {};

      auto *Bucket = GetHashBucket(umm(Hash(&Query)), Table);
      while (Bucket)
      {
        auto E = &Bucket->Element;

        Type.is_primitive?
        {
        if (Bucket->Tombstoned == False && AreEqual(*E, Query))
        }
        {
        if (Bucket->Tombstoned == False && AreEqual(E->key_name, Query))
        }
        {
          Result = Bucket;
          break;
        }
        else
        {
          Bucket = Bucket->Next;
        }
      }

      return Result;
    }

    maybe_(Type.name)
    GetBy(key_name)( (Type.name)_hashtable *Table, key_type Query )
    {
      /* ENSURE_OWNED_BY_THREAD(Table); */

      maybe_(Type.name) Result = {};

      (Type.name)_linked_list_node *Bucket = GetBucketBy(key_name)(Table, Query);
      if (Bucket)
      {
        Result.Tag = Maybe_Yes;
        Result.Value = Bucket->Element;
      }

      return Result;
    }

    /// TODO(Jesse): Remove memory from here.
    link_internal b32
    Tombstone((key_type) Key, (Type.name)_hashtable *Table, memory_arena *Memory)
    {
      b32 Result = False;
      (Type.name)_linked_list_node *Bucket = GetBucketBy(key_name)(Table, Key);
      if (Bucket)
      {
        Assert(Bucket->Tombstoned == False);
        Bucket->Tombstoned = True;
        Result = True;
      }
      return Result;
    }

    link_internal b32
    Drop( (Type.name)_hashtable *Table, (key_type) Key )
    {
      return Tombstone(Key, Table, 0);
    }
  }
);

poof(
  func hashtable_get_ptr(Type, type_poof_symbol key_type, type_poof_symbol key_name)
  {
    maybe_(Type.name)_ptr
    GetPtrBy(key_name)( (Type.name)_hashtable *Table, key_type Query )
    {
      /* ENSURE_OWNED_BY_THREAD(Table); */

      maybe_(Type.name)_ptr Result = {};

      auto *Bucket = GetHashBucket(umm(Hash(&Query)), Table);
      while (Bucket)
      {
        auto E = &Bucket->Element;

        if (Bucket->Tombstoned == False && AreEqual(E->key_name, Query))
        {
          Result.Tag = Maybe_Yes;
          Result.Value = E;
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
  func hashtable_to_buffer(type)
  {
    (type.name)_buffer
    ToBuffer( (type.name)_hashtable *Table, memory_arena *Memory )
    {
      umm Count = 0;
      RangeIterator_t(umm, Index, Table->Size)
      {
        (type.name)_linked_list_node *E = Table->Elements[Index];
        if (E) Count++;
      }

      (type.name)_buffer Result = (type.name.to_capital_case)Buffer(Count, Memory);

      Count = 0;
      RangeIterator_t(umm, Index, Table->Size)
      {
        (type.name)_linked_list_node *E = Table->Elements[Index];
        if (E) { Result.Start[Count] = E->Element; Count++; }
      }
      Assert(Count == Result.Count);

      return Result;
    }

  }
)





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
    /* dunion_debug_print_prototype(DUnion) */

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
    DebugPrint( (DUnion.name) *Struct, u32 Depth)
    {
      DebugPrint("DUnion.name {\n", Depth);

      if (Struct)
      {
        switch(Struct->Type)
        {
          DUnion.map_members (M)
          {
            M.is_union?
            {
              M.map_members (UnionMember)
              {
                case type_(UnionMember.type):
                {
                  DebugPrint(&Struct->(UnionMember.name), Depth+4);
                } break;
              }
            }
          }

          default : { DebugPrint("default while printing ((DUnion.type)) ((DUnion.name)) ", Depth+4); DebugLine("Type(%d)", Struct->Type); } break;
        }
      }
      else
      {
        DebugPrint("(null)", Depth);
      }
      DebugPrint("}\n", Depth);
    }

    link_internal void
    DebugPrint( (DUnion.name) Struct, u32 Depth)
    {
      DebugPrint(&Struct, Depth);
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

    link_inline (Type.name)*
    TryGetPtr( TypeName *Buf, umm Index)
    {
      return GetPtr(Buf, Index);
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
    DeepCopy((Type.name) *Src, (Type.name) *Dest)
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
  func buffer_h(Type, count_type)
  {
    struct (Type.name)_buffer
    {
      count_type.name Count;
      Type.name *Start; poof(@array_length(Element->Count))
    };

    link_internal (Type.name)_buffer
    (Type.name.to_capital_case)Buffer( count_type.name ElementCount, memory_arena* Memory);

    link_internal (Type.name)_buffer
    (Type.name.to_capital_case)Buffer( Type.name *Start, count_type.name ElementCount)
    {
      (Type.name)_buffer Result = {ElementCount, Start};
      return Result;
    }

    link_inline count_type.name
    LastIndex((Type.name)_buffer *Buf)
    {
      count_type.name Result = Buf->Count > 0? Buf->Count-1 : 0;
      return Result;
    }

    link_inline count_type.name
    ZerothIndex((Type.name)_buffer *Buf)
    {
      count_type.name Result = 0;
      return Result;
    }

    link_inline count_type.name
    AtElements((Type.name)_buffer *Buf)
    {
      count_type.name Result = Buf->Count;
      return Result;
    }

    link_inline count_type.name
    TotalElements((Type.name)_buffer *Buf)
    {
      count_type.name Result = Buf->Count;
      return Result;
    }

    link_inline count_type.name
    CurrentCount((Type.name)_buffer *Buf)
    {
      count_type.name Result = Buf->Count;
      return Result;
    }

    link_inline (Type.name) *
    GetPtr((Type.name)_buffer *Buf, count_type.name Index)
    {
      Type.name *Result = 0;
      if (Index < Buf->Count) { Result = Buf->Start + Index; }
      return Result;
    }

    link_inline (Type.name) *
    TryGetPtr((Type.name)_buffer *Buf, count_type.name Index)
    {
      return GetPtr(Buf, Index);
    }

    link_inline (Type.name) *
    Get((Type.name)_buffer *Buf, count_type.name Index)
    {
      Type.name *Result = GetPtr(Buf, Index);
      return Result;
    }
  }
)
poof(
  func buffer_c(Type, count_type)
  {
    link_internal (Type.name)_buffer
    (Type.name.to_capital_case)Buffer( count_type.name ElementCount, memory_arena* Memory)
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
  }
)

// TODO(Jesse): Remove
poof(
  func buffer_t(Type, count_type)
  {
    buffer_h(Type, count_type)
  }
)

poof(
  func buffer(Type)
  {
    buffer_h(Type, umm)
    buffer_c(Type, umm)
  }
)

// TODO(Jesse)(immediate): Rewrite this in terms of a cursor.
poof(
  func generate_stack(Type, type_poof_symbol ExtraMembers)
  {
    struct (Type.name)_stack
    {
      Type.name *Start;
      umm At;
      umm End;
      (ExtraMembers)
    };

    link_internal (Type.name)_stack
    (Type.name.to_capital_case)Stack(umm ElementCount, memory_arena* Memory)
    {
      Type.name *Start = ((Type.name)*)PushStruct(Memory, sizeof((Type.name))*ElementCount, 1, 0);
      (Type.name)_stack Result = {};
      Result.Start = Start;
      Result.End = ElementCount;
      Result.At = 0;
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
      /* OWNED_BY_THREAD_MEMBER(); */
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
      (Type.name)_cursor Result = {};
      Result.Start = Start;
      Result.End = Start+ElementCount;
      Result.At = Start;
      return Result;
    }

    link_internal (Type.name)*
    GetPtr((Type.name)_cursor *Cursor, umm ElementIndex)
    {
      /* ENSURE_OWNED_BY_THREAD(Cursor); */

      Type.name *Result = {};
      if (ElementIndex < AtElements(Cursor)) { Result = Cursor->Start+ElementIndex; }
      return Result;
    }

    link_internal (Type.name)*
    TryGetPtr((Type.name)_cursor *Cursor, umm ElementIndex)
    {
      return GetPtr(Cursor, ElementIndex);
    }

    link_internal (Type.name)*
    GetPtrUnsafe((Type.name)_cursor *Cursor, umm ElementIndex)
    {
      /* ENSURE_OWNED_BY_THREAD(Cursor); */

      Type.name *Result = {};
      if (ElementIndex < TotalElements(Cursor)) { Result = Cursor->Start+ElementIndex; }
      return Result;
    }

    link_internal (Type.name)
    Get((Type.name)_cursor *Cursor, umm ElementIndex)
    {
      /* ENSURE_OWNED_BY_THREAD(Cursor); */

      Assert(ElementIndex < CurrentCount(Cursor));
      Type.name Result = Cursor->Start[ElementIndex];
      return Result;
    }

    link_internal void
    Set((Type.name)_cursor *Cursor, umm ElementIndex, (Type.name) Element)
    {
      /* ENSURE_OWNED_BY_THREAD(Cursor); */

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
      /* ENSURE_OWNED_BY_THREAD(Cursor); */

      Type.name * Result = {};
      if ( Cursor->At < Cursor->End ) { Result = Cursor->At++; }
      return Result;
    }

    link_internal Type.name *
    Push((Type.name)_cursor *Cursor, (Type.name) Element)
    {
      /* ENSURE_OWNED_BY_THREAD(Cursor); */

      Assert( Cursor->At < Cursor->End );
      Type.name *Result = Cursor->At;
      *Cursor->At++ = Element;
      return Result;
    }

    link_internal Type.name
    Pop((Type.name)_cursor *Cursor)
    {
      /* ENSURE_OWNED_BY_THREAD(Cursor); */

      Assert( Cursor->At > Cursor->Start );
      Type.name Result = Cursor->At[-1];
      Cursor->At--;
      return Result;
    }

    link_internal s32
    LastIndex((Type.name)_cursor *Cursor)
    {
      /* ENSURE_OWNED_BY_THREAD(Cursor); */

      s32 Result = s32(CurrentCount(Cursor))-1;
      return Result;
    }

    link_internal Type.name*
    LastElement((Type.name)_cursor *Cursor)
    {
      Type.name *Result = {};
      s32 I = LastIndex(Cursor);
      if (I > -1) { Result = Cursor->Start + I; }
      return Result;
    }

    link_internal b32
    Remove((Type.name)_cursor *Cursor, (Type.name) Query)
    {
      /* ENSURE_OWNED_BY_THREAD(Cursor); */

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
      /* ENSURE_OWNED_BY_THREAD(Cursor); */

      umm CurrentSize = TotalSize(Cursor);

      TruncateToElementCount(Cursor, Count);
      umm NewSize = TotalSize(Cursor);

      Assert(NewSize/sizeof((Type.name)) == Count);

      /* Info("Attempting to reallocate CurrentSize(%u), NewSize(%u)", CurrentSize, NewSize); */
      Ensure(Reallocate((u8*)Cursor->Start, Memory, CurrentSize, NewSize));
      return 0;
    }

    link_internal void
    Unshift( (Type.name)_cursor *Cursor )
    {
      /* ENSURE_OWNED_BY_THREAD(Cursor); */
      umm Count = AtElements(Cursor);
      if (Count)
      {
        for (umm Index = 1; Index < Count; ++Index)
        {
          Cursor->Start[Index-1] = Cursor->Start[Index];
        }

        // NOTE(Jesse): This is actually correct, even though it doesn't look
        // like it at first glance.  At is OnePastLast, so decrementing and
        // then clearing overwrites the last value that was set.
        Cursor->At--;
        *Cursor->At = {};
      }
    }

  }
)

poof(
  func generate_string_table(EnumType)
  {
    link_internal counted_string
    ToStringPrefixless((EnumType.name) Type)
    {
      counted_string Result = {};
      switch (Type)
      {
        EnumType.map_values (EnumValue)
        {
          case EnumValue.name: { Result = CSz("EnumValue.name.strip_all_prefix"); } break;
        }

        EnumType.has_tag(bitfield)?
        {
          // TODO(Jesse): This is pretty barf and we could do it in a single allocation,
          // but the metaprogram might have to be a bit fancier..
          default:
          {
            u32 CurrentFlags = u32(Type);

            u32 BitsSet = CountBitsSet_Kernighan(CurrentFlags);
            switch(BitsSet)
            {
              case 0: // We likely passed 0 into this function, and the enum didn't have a 0 value
              case 1: // The value we passed in was outside the range of the valid enum values
              {
                Result = FSz("(invalid value (%d))", CurrentFlags);
              } break;

              default:
              {
                u32 FirstValue = UnsetLeastSignificantSetBit(&CurrentFlags);
                Result = ToStringPrefixless((EnumType.name)(FirstValue));

                while (CurrentFlags)
                {
                  u32 Value = UnsetLeastSignificantSetBit(&CurrentFlags);
                  cs Next = ToStringPrefixless((EnumType.name)(Value));
                  Result = FSz("%S | %S", Result, Next);
                }
              } break;
            }
          } break;
        }
      }
      /* if (Result.Start == 0) { Info("Could not convert value(%d) to (EnumType.name)", Type); } */
      return Result;
    }

    link_internal counted_string
    ToString((EnumType.name) Type)
    {
      counted_string Result = {};
      switch (Type)
      {
        EnumType.map_values (EnumValue)
        {
          case EnumValue.name: { Result = CSz("EnumValue.name"); } break;
        }

        EnumType.has_tag(bitfield)?
        {
          // TODO(Jesse): This is pretty barf and we could do it in a single allocation,
          // but the metaprogram might have to be a bit fancier..
          default:
          {
            u32 CurrentFlags = u32(Type);

            u32 FirstValue = UnsetLeastSignificantSetBit(&CurrentFlags);
            Result = ToString((EnumType.name)(FirstValue));

            while (CurrentFlags)
            {
              u32 Value = UnsetLeastSignificantSetBit(&CurrentFlags);
              cs Next = ToString((EnumType.name)(Value));
              Result = FSz("%S | %S", Result, Next);
            }
          } break;
        }
      }
      /* if (Result.Start == 0) { Info("Could not convert value(%d) to (EnumType.name)", Type); } */
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
      Assert(Stream->Memory);

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

    link_internal (Type.name)_stream
    (Type.name.to_capital_case)Stream(memory_arena *Memory)
    {
      (Type.name)_stream Result = {};
      Result.Memory = Memory;
      return Result;
    }

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
        poof_error { Unable to generate debug_print; (T) was not a struct or enum. }
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
    link_internal void DebugPrint( StructDef.name *RuntimeStruct, u32 Depth = 0)
    {
      /* if (Depth == 0) */
      {
        DebugPrint("StructDef.name ", Depth);
      }

      if (RuntimeStruct)
      {
        DebugPrint("{\n", Depth);
        StructDef.map_members (Member)
        {
          Member.is_defined?
          {
            Member.name?
            {
              Member.is_compound?
              {
                DebugPrint("Member.type Member.name {\n", Depth+2);
                DebugPrint(&RuntimeStruct->(Member.name), Depth+4);
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
                  DebugPrint(&RuntimeStruct->(Member.name), 1);
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
      else
      {
        DebugPrint(" = (null)\n", Depth);
      }

    }

    link_internal void DebugPrint( StructDef.name RuntimePtr, u32 Depth = 0)
    {
      DebugPrint(&RuntimePtr, Depth);
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


poof(
  func block_array_h(element_t, type_poof_symbol n_elements_per_block, type_poof_symbol extra_members)
  {
    @var block_array_t (element_t.name)_block_array
    @var block_t       (element_t.name)_block
    @var index_t       (element_t.name)_block_array_index

    struct block_t
    {
      /* u32 Index; */
      umm At;
      element_t.name Elements[n_elements_per_block];
    };

    struct index_t
    {
      umm Index; /// TODO(Jesse): Should this be u32?
      /* block_t *Block; */
      /* u32 BlockIndex; */
      /* u32 ElementIndex; */
    };

    struct block_array_t
    {
      block_t **BlockPtrs; poof(@array_length(Element->BlockCount))
          u32   BlockCount;
          u32   ElementCount;
      memory_arena *Memory; poof(@no_serialize)
      extra_members
    };

    link_internal block_array_t
    block_array_t.to_capital_case(memory_arena *Memory)
    {
      block_array_t Result = {};
      Result.Memory = Memory;
      return Result;
    }

    are_equal(index_t)

    typedef (element_t.name)_block_array (element_t.name)_paged_list;

    link_internal index_t
    operator++( index_t &I0 )
    {
      I0.Index++;
      return I0;
    }

    link_internal b32
    operator<( index_t I0, index_t I1 )
    {
      b32 Result = I0.Index < I1.Index;
      return Result;
    }

    link_internal b32
    operator==( index_t I0, index_t I1 )
    {
      b32 Result = I0.Index == I1.Index;
      return Result;
    }

    link_inline umm
    GetIndex( index_t *Index)
    {
      umm Result = Index->Index;
      return Result;
    }

    /// TODO(Jesse): Remove this globally
    link_internal index_t
    ZerothIndex( block_array_t *Arr )
    {
      return {};
    }

    link_internal index_t
    Capacity( block_array_t *Arr )
    {
      index_t Result = {Arr->BlockCount * n_elements_per_block};
      return Result;
    }

    link_internal index_t
    AtElements( block_array_t *Arr )
    {
      index_t Result = {Arr->ElementCount};
      return Result;
    }

    /// TODO(Jesse): Remove this globally.  It used to be necessary but is superfluous now.
    link_internal umm
    TotalElements( block_array_t *Arr )
    {
      umm Result = AtElements(Arr).Index;
      return Result;
    }

    /// TODO(Jesse): Remove?
    link_internal index_t
    LastIndex( block_array_t *Arr )
    {
      index_t Result = {};
      umm Count = AtElements(Arr).Index;
      if (Count) Result.Index = Count-1;
      return Result;
    }

    link_internal umm
    Count( block_array_t *Arr )
    {
      auto Result = AtElements(Arr).Index;
      return Result;
    }

    link_internal block_t *
    GetBlock( block_array_t *Arr, index_t Index )
    {
      umm BlockIndex   = Index.Index / n_elements_per_block;
      Assert(BlockIndex < Arr->BlockCount);
      block_t *Block = Arr->BlockPtrs[BlockIndex];
      return Block;
    }

    link_internal element_t.name element_t.is_pointer?{}{*}
    GetPtr( block_array_t *Arr, index_t Index )
    {
      Assert(Arr->BlockPtrs);
      Assert(Index.Index < Capacity(Arr).Index);

      block_t *Block = GetBlock(Arr, Index);

      umm ElementIndex = Index.Index % n_elements_per_block;
      element_t.name element_t.is_pointer?{}{*}Result = element_t.is_pointer?{*}(Block->Elements + ElementIndex);
      return Result;
    }

    link_internal element_t.name element_t.is_pointer?{}{*}
    TryGetPtr((element_t.name)_block_array *Arr, index_t Index)
    {
      element_t.name element_t.is_pointer?{}{*} Result = {};
      if (Arr->BlockPtrs && Index.Index < Capacity(Arr).Index)
      {
        Result = GetPtr(Arr, Index);
      }
      return Result;
    }


    link_internal element_t.name element_t.is_pointer?{}{*}
    GetPtr( block_array_t *Arr, umm Index )
    {
      index_t I = {Index};
      return GetPtr(Arr, I);
    }

    link_internal element_t.name element_t.is_pointer?{}{*}
    TryGetPtr((element_t.name)_block_array *Arr, umm Index)
    {
      element_t.name element_t.is_pointer?{}{*} Result = {};
      if (Arr->BlockPtrs && Index < AtElements(Arr).Index)
      {
        index_t I = {Index};
        Result = GetPtr(Arr, I);
      }
      return Result;
    }
  }
)

#define INVALID_BLOCK_ARRAY_INDEX umm_MAX

poof(
  func block_array_c(element_t, type_poof_symbol n_elements_per_block)
  {
    @var block_array_t (element_t.name)_block_array
    @var block_t       (element_t.name)_block
    @var index_t       (element_t.name)_block_array_index

    /* link_internal block_t * */
    /* Allocate_(element_t.name)_block(memory_arena *Memory) */
    /* { */
    /*   block_t *Result = Allocate( block_t, Memory, 1); */
    /*   return Result; */
    /* } */

    link_internal cs
    CS( index_t Index )
    {
      return FSz("(%u)", Index.Index);
    }

    link_internal element_t.name element_t.is_pointer?{}{*}
    Set( block_array_t *Arr,
         element_t.name element_t.is_pointer?{}{*}Element,
         index_t Index )
    {
      Assert(Arr->BlockPtrs);
      Assert(Index.Index < Capacity(Arr).Index);
      block_t *Block = GetBlock(Arr, Index);
      umm ElementIndex = Index.Index % n_elements_per_block;
      auto Slot = Block->Elements+ElementIndex;
      *Slot = element_t.is_pointer?{}{*}Element;
      return element_t.is_pointer?{*}{}Slot;
    }

    link_internal void
    NewBlock( block_array_t *Arr )
    {
      block_t  *NewBlock     = Allocate( block_t , Arr->Memory,                 1);
      block_t **NewBlockPtrs = Allocate( block_t*, Arr->Memory, Arr->BlockCount+1);

      RangeIterator_t(u32, BlockI, Arr->BlockCount)
      {
        NewBlockPtrs[BlockI] = Arr->BlockPtrs[BlockI];
      }

      NewBlockPtrs[Arr->BlockCount] = NewBlock;

      /// NOTE(Jesse): We leak the old array of block pointers here .. it would
      /// be better to allocate them on a global heap and free ..?
      Arr->BlockPtrs = NewBlockPtrs;
      Arr->BlockCount += 1;
    }

    link_internal void
    RemoveUnordered( block_array_t *Array, index_t Index)
    {
      auto LastElement = GetPtr(Array, LastIndex(Array));
      Set(Array, LastElement, Index);
      Array->ElementCount -= 1;
    }

    link_internal void
    RemoveOrdered( block_array_t *Array, index_t IndexToRemove)
    {
      Assert(IndexToRemove.Index < Array->ElementCount);

      element_t.name element_t.is_pointer?{}{*}Prev = {};

      index_t Max = AtElements(Array);
      RangeIteratorRange_t(umm, Index, Max.Index, IndexToRemove.Index)
      {
        element_t.name element_t.is_pointer?{}{*}E = GetPtr(Array, Index);

        if (Prev)
        {
          *Prev = *E;
        }

        Prev = E;
      }

      Array->ElementCount -= 1;
    }

    link_internal void
    RemoveOrdered( block_array_t *Array, element_t.name element_t.is_pointer?{}{*}Element )
    {
      IterateOver(Array, E, I)
      {
        if (E == Element)
        {
          RemoveOrdered(Array, I);
          break;
        }
      }
    }

    link_internal index_t
    Find( block_array_t *Array, element_t.name element_t.is_pointer?{}{*}Query)
    {
      index_t Result = {INVALID_BLOCK_ARRAY_INDEX};
      IterateOver(Array, E, Index)
      {
        if ( E == Query)
        {
          Result = Index;
          break;
        }
      }
      return Result;
    }

    link_internal b32
    IsValid((element_t.name)_block_array_index *Index)
    {
      index_t Test = {INVALID_BLOCK_ARRAY_INDEX};
      b32 Result = (AreEqual(Index, &Test) == False);
      return Result;
    }

    link_internal element_t.name element_t.is_pointer?{}{*}
    Push( block_array_t *Array, element_t.name element_t.is_pointer?{}{*}Element)
    {
      Assert(Array->Memory);

      if (AtElements(Array) == Capacity(Array))
      {
        NewBlock(Array);
      }

      element_t.name element_t.is_pointer?{}{*}Result = Set(Array, Element, AtElements(Array));

      Array->ElementCount += 1;

      return Result;
    }

    link_internal element_t.name element_t.is_pointer?{}{*}
    Push( block_array_t *Array )
    {
      element_t.name Element = {};
      auto Result = Push(Array, element_t.is_pointer?{}{&}Element);
      return Result;
    }

    link_internal void
    Shift( block_array_t *Array, element_t.name element_t.is_pointer?{}{*}Element )
    {
      Assert(Array->Memory);
      element_t.name element_t.is_pointer?{}{*}Prev = {};

      // Alocate a new thingy
      Push(Array);

      auto End = AtElements(Array);
      RangeIteratorReverse(Index, s32(End.Index))
      {
        auto E = GetPtr(Array, umm(Index));
        if (Prev) { *Prev = *E; }
        Prev = E;
      }

      *Prev = *Element;
    }
  }
)

poof(
  func flatten_block_array(element_t)
  {
    link_internal (element_t.name)_buffer
    Flatten((element_t.name)_block_array *Array, memory_arena *Memory)
    {
      // TODO(Jesse): This is MAJOR barf
      auto At = AtElements(Array);
      auto Count = GetIndex(&At);
      (element_t.name)_buffer Result = (element_t.name.to_capital_case)Buffer(Count, Memory);
      IterateOver(Array, Element, ElementIndex)
      {
        // NOTE(Jesse): UGGGGGGHHH
        umm Index = GetIndex(&ElementIndex);
        Result.Start[Index] = *Element;
      }
      return Result;
    }
  }
)

// nocheckin -- fix poof here.
poof(
  func block_array(type, type_poof_symbol n_elements_per_block)
  {
    (block_array_h( type, {8}, {} ))
    (block_array_c( type, {8} ))
  }
)

poof(
  func draw_element_union(union_type)
  {
    link_internal void
    Draw(renderer_2d *Ui, union_type.name *Union, ui_style* Style = &DefaultStyle, v4 Padding = DefaultDatastructurePadding, column_render_params Params = ColumnRenderParam_LeftAlign)
    {
      cs AsString = union_type.member(E, (element_member) {
        FSz("element_member.map_array(index).sep() {(?) }", element_member.map_array(index).sep(,) { Union->E[index] });
      })
      PushColumn(Ui, AsString, Style, Padding, Params);
      PushNewRow(Ui);
    }
  }
)

poof(
  func draw_tagged_union(d_union_type)
  {
    d_union_type.map_members (union_member)
    {
      union_member.is_union?
      {
        union_member.map_members(sub_type)
        {
          (draw_datastructure(sub_type))
        }
      }
    }

    link_internal void
    Draw(renderer_2d *Ui, d_union_type.name *DUnion, ui_style* Style = &DefaultStyle, v4 Padding = DefaultDatastructurePadding, column_render_params Params = ColumnRenderParam_LeftAlign)
    {
      switch (DUnion->Type)
      {
        d_union_type.map_members(anon_union) {
          anon_union.is_union? {

            InvalidCase(type_(d_union_type.name)_noop);

            anon_union.map_members (union_member)
            {
              case type_(union_member.name):
              {
                PushColumn(Ui, CSz("union_member.name {"), Style, Padding, Params);
                PushNewRow(Ui);

                Draw(Ui, &DUnion->(union_member.name), Style, Padding + DatastructureIndent, Params);

                PushColumn(Ui, CSz("}"), Style, Padding, Params);
                PushNewRow(Ui);
              } break;
            }
          }
        }
      }
    }
  }
)

poof(
  func draw_datastructure(type)
  {
    link_internal void
    Draw(renderer_2d *Ui, type.name *Element, ui_style* Style = &DefaultStyle, v4 Padding = DefaultDatastructurePadding, column_render_params Params = ColumnRenderParam_LeftAlign)
    {
      type.map_members (member)
      {
        member.is_enum?
        {
          PushColumn(Ui, CSz("member.name : "), Style, Padding, Params);
          PushColumn(Ui, ToString(Element->(member.name)), Style, DefaultDatastructurePadding, Params);
        }
        {
          member.is_primitive?
          {
            PushColumn(Ui, CSz("member.name : "), Style, Padding, Params);
            PushColumn(Ui, CS(Element->(member.name)), Style, DefaultDatastructurePadding, Params);
          }
          {
            member.is_union?
            {
              PushColumn(Ui, CSz(" -- skipping union_member -- "));
              /* cs ButtonNameOff = CSz(" + member.name"); */
            }
            {
              {
                cs ButtonNameOn = CSz(" - member.name : {");
                cs ButtonNameOff = CSz(" + member.name");
                if (ToggleButton(Ui, ButtonNameOn, ButtonNameOff, umm(ButtonNameOn.Start) ^ umm(Element), Style, Padding, Params ))
                {
                  PushNewRow(Ui);

                  Draw(Ui, &Element->(member.name), Style, Padding + DatastructureIndent, Params);

                  PushColumn(Ui, CSz("}"), Style, Padding, Params);
                }
              }
            }
          }
        }
        PushNewRow(Ui);
      }
    }
  }
)

poof(
  func freelist_allocator(type)
  {
    struct (type.name)_freelist
    {
       (type.name) *First;
      memory_arena *Memory;
    };

    link_internal type.name *
    GetOrAllocate((type.name)_freelist *Freelist)
    {
      type.name *Result = Freelist->First;

      if (Result)
      {
        Freelist->First = Result->Next;
      }
      else
      {
        Result = Allocate( (type.name), Freelist->Memory, 1 );
      }

      return Result;
    }

    link_internal void
    Free((type.name)_freelist *Freelist, type.name *Element)
    {
      Element->Next = Freelist->First;
      Freelist->First = Element;
    }
  }
)
