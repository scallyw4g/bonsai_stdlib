
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

