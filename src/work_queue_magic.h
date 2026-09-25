
poof(
  func asyncify_function_h(func_t)
  {
    struct (func_t.name.to_snake_case)_async_params poof(@async_function_params)
    {
      func_t.value ? { func_t.value* Result; }
      func_t.map(arg) 
      {
        arg;
      }
    };
  }
)


poof(
  func asyncify_function_c(func_t)
  {
    link_internal work_queue_task
    (func_t.name)_Task(
        work_queue *Queue,
        func_t.map(arg).sep(,) { arg }                     /// Closure args
        func_t.value? { , func_t.value* FuncResultDest } ) /// Func result pointer (optional)
    {
      (func_t.name.to_snake_case)_async_params Params =
      {
        func_t.value?   {  FuncResultDest, }
        func_t.map(arg) { arg.name, }
      };

      work_queue_task Result = WorkQueueEntryAsyncFunction(Queue, &Params);
      return Result;
    }

    link_internal void
    (func_t.name)_Async(
        work_queue *Queue,
        func_t.map(arg).sep(,) { arg }
        func_t.value? { , func_t.value* Result } )
    {
      auto Task = (func_t.name)_Task( Queue,
        func_t.map(arg).sep(,) { arg.name }
        func_t.value? { , Result }
      );
      SubmitSingleTask(Queue, &Task);
    }

    link_internal void
    ExecFunction((func_t.name.to_snake_case)_async_params *Params)
    {
      func_t.value? { auto Result = } func_t.name((func_t.map(arg).sep(,) { Params->(arg.name) }));
      func_t.value? { if (Params->Result) { *Params->Result = Result; } }
    }
  }
)

poof(
  for_datatypes(func)
  func (func_t)
  {
    func_t.has_tag(async)?
    {
      asyncify_function_h(func_t)
    }
  }
)
#include <generated/poof_builtin.for_datatypes$$SN5pK3PT.h>

// Generate tagged_union for async functions
//
enum async_function_call_type
{
  poof(
    for_datatypes(struct) @code_fragment
    func (struct_t)
    {
      struct_t.has_tag(async_function_params)?
      {
        type_(struct_t.name),
      }
    }
  )
#include <generated/poof_builtin.for_datatypes$$fOKTiPYO.h>
};

#if 0
struct work_queue_task_async_function_call
{
  async_function_call_type Type;
  union
  {
    poof(
      for_datatypes(struct) @code_fragment
      func (struct_t)
      {
        struct_t.has_tag(async_function_params)?
        {
          struct_t.name struct_t.name;
        }
      }
    )
#include <generated/poof_builtin.for_datatypes$$gEjNYfI5.h>
  };
};

#else
struct work_queue_task_async_function_call
{
  async_function_call_type Type;
  union
  {
    compile_shader_pair_async_params compile_shader_pair_async_params;
  };
};

#endif

#if 0
poof(
  func gen_work_queue_task_async_function_call()
  {
    struct work_queue_task_async_function_call
    {
      async_function_call_type Type;
      union
      {
        for_datatypes(struct) @code_fragment
        func (struct_t)
        {
          struct_t.has_tag(async_function_params)?
          {
            struct_t.name struct_t.name;
          }
        }
      };
    };
  }
)

poof(gen_work_queue_task_async_function_call())
#include <generated/gen_work_queue_task_async_function_call$$rJefEXhD.h>

poof(string_and_value_tables(async_function_call_type))
#include <generated/string_and_value_tables$async_function_call_type$hJOrda0k.h>
#endif

