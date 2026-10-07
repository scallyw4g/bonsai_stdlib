
poof(
  func asyncify_function_closure_params(func_t)
  {
    /// Generate a struct to capture closure arguments
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
  func async_function_prototypes(func_t)
  {
    link_internal work_queue_task 
    (func_t.name)_Task( work_queue *Queue func_t.map(arg) {, arg } func_t.value? { , func_t.value* FuncResultDest } );

    link_internal void
    (func_t.name)_Async( work_queue *Queue func_t.map(arg) {, arg } func_t.value? { , func_t.value *Result } );
  }
)

poof(
  func asyncify_function_c(func_t)
  {
    /// Generate a helper function to capture closure args and wrap it in a
    /// task for submission to work queues
    link_internal work_queue_task
    (func_t.name)_Task(
        work_queue *Queue
        func_t.map(arg) {, arg }                           /// Closure args
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

    link_internal work_queue_job *
    (func_t.name)_Job(
        work_queue *Queue
        func_t.map(arg) {, arg }                           /// Closure args
        func_t.value? { , func_t.value* FuncResultDest }   /// Func result pointer (optional)
        , u32 AwaitCount = 0 )
    {
      (func_t.name.to_snake_case)_async_params Params =
      {
        func_t.value?   {  FuncResultDest, }
        func_t.map(arg) { arg.name, }
      };

      work_queue_task Task = WorkQueueEntryAsyncFunction(Queue, &Params);
      work_queue_job *Result = ReserveWorkQueueJob(GetPlatform(), AwaitCount, 0);

      PushTask(Result, &Task);

      return Result;
    }

    /// Generate the Async function definition
    /// Returns the task index
    link_internal global_job_index
    (func_t.name)_Async(
        work_queue *Queue
        func_t.map(arg) {, arg }
        func_t.value? { , func_t.value *Result } 
        , u32 AwaitCount = 0
      )
    {
      /// Call helper to initialize the job
      auto Job = (func_t.name)_Job(
        Queue
        func_t.map(arg) {, arg.name }
        func_t.value? { , Result }
        , AwaitCount
      );

      SubmitJob(Queue, Job);
      return Job->Index;
    }

    /// Execute the function from the captured closure
    link_internal void
    ExecFunction((func_t.name.to_snake_case)_async_params *Params)
    {
      func_t.value? { auto Result = } func_t.name((func_t.map(arg).sep(,) { Params->(arg.name) }));
      func_t.value? { if (Params->Result) { *Params->Result = Result; } }
    }
  }
)
