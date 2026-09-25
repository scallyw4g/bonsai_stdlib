#ifndef BONSAI_SHADER_PATH
#define BONSAI_SHADER_PATH "shaders/"
#endif

#ifndef STDLIB_SHADER_PATH
#define STDLIB_SHADER_PATH "external/bonsai_stdlib/shaders/"
#endif


#ifndef BONSAI_NO_AVX
#include <bonsai_stdlib/src/perlin.cpp>
#endif

#include <bonsai_stdlib/src/assert.cpp>
#include <bonsai_stdlib/src/memory_arena.cpp>
#include <bonsai_stdlib/src/vector.cpp>
#include <bonsai_stdlib/src/debug_print.cpp>  // TODO(Jesse, id: 91, tags: cleanup, metaprogramming, format_counted_string): Jettison this .. Can it be metaprogrammed?
#include <bonsai_stdlib/src/counted_string.cpp>
#include <bonsai_stdlib/src/sort.cpp>
#include <bonsai_stdlib/src/rect.cpp>
#include <bonsai_stdlib/src/primitive_containers.cpp>
#include <bonsai_stdlib/src/primitives.cpp>
#include <bonsai_stdlib/src/platform.cpp>
#include <bonsai_stdlib/src/thread.cpp>
#include <bonsai_stdlib/src/string_builder.cpp>
#include <bonsai_stdlib/src/ansi_stream.cpp>
#include <bonsai_stdlib/src/binary_parser.cpp>
#include <bonsai_stdlib/src/colors.cpp>
#include <bonsai_stdlib/src/bitmap.cpp>
#include <bonsai_stdlib/src/matrix.cpp>
#include <bonsai_stdlib/src/heap_memory.cpp>
#include <bonsai_stdlib/src/xml.cpp>
#include <bonsai_stdlib/src/file.cpp>
#include <bonsai_stdlib/src/filesystem.cpp>

// NOTE(Jesse): Must match defines in header.glsl
#define VERTEX_POSITION_LAYOUT_LOCATION    0

// NOTE(Jesse): Normals and UVs are mutually exclusive, so we use slot 1 for either/or;
// We never draw 3D geometry with UVs and 2D UI doesn't need normals
#define VERTEX_NORMAL_LAYOUT_LOCATION      1
#define VERTEX_UV_LAYOUT_LOCATION          1

#define VERTEX_COLOR_LAYOUT_LOCATION       2
#define VERTEX_TRANS_EMISS_LAYOUT_LOCATION 3


#include <bonsai_stdlib/src/gl.cpp>
#include <bonsai_stdlib/src/texture.cpp>
#include <bonsai_stdlib/src/shader.cpp>
#include <bonsai_stdlib/src/2d_render_utils.cpp>
#include <bonsai_stdlib/src/ui/gl.cpp>

#include <bonsai_stdlib/src/heap_allocator.cpp>

#include <bonsai_stdlib/src/gpu_mapped_buffer.cpp>
#include <bonsai_stdlib/src/gpu_heap_allocator.cpp>
#include <bonsai_stdlib/src/framebuffer.cpp>

#include <bonsai_stdlib/src/to_string.cpp>

#include <bonsai_stdlib/src/texture_cursor.cpp>
#include <bonsai_stdlib/src/ui/interactable.cpp>
#include <bonsai_stdlib/src/ui/ui.cpp>
#include <bonsai_stdlib/src/debug_ui.cpp>

#include <bonsai_stdlib/src/c_token.cpp>
#include <bonsai_stdlib/src/c_parser.cpp>

#include <bonsai_stdlib/src/initialize.cpp>

#if BONSAI_DEBUG_SYSTEM_API
#include <bonsai_debug/debug.cpp>
#endif

#include <bonsai_stdlib/src/work_queue_magic.cpp>
#include <bonsai_stdlib/src/work_queue.cpp>


// @bonsai_stdlib_use_custom_threadpool
#if BONSAI_STDLIB_USE_CUSTOM_THREADPOOL
#error "not implemented"
#else
#include <bonsai_stdlib/src/work_queue_default_impl.cpp>
#endif

poof(
  for_datatypes(struct)
  func (struct_t)
  {
    struct_t.has_tag(async_function_params)?
    {
      struct struct_t.name;
      link_internal work_queue_task
      WorkQueueEntryAsyncFunction( work_queue *Queue, (struct_t.name) *Params )
      {
        work_queue_task Result = {};
        Result.Queue = Queue;
        Result.Type = type_work_queue_task_async_function_call;
        Result.work_queue_task_async_function_call.Type = type_(struct_t.name);
        Result.work_queue_task_async_function_call.(struct_t.name) = *Params;
        return Result;
      }
    }
  }
)
#include <generated/poof_builtin.for_datatypes$$Xs04c1ly.h>

poof(
  for_datatypes(func)
  func (func_t)
  {
    func_t.has_tag(async)?
    {
      asyncify_function_c(func_t)
    }
  }
)
#include <generated/poof_builtin.for_datatypes$$Xst5mK32.h>



link_internal void
DispatchAsyncFunctionCall(work_queue_task_async_function_call *WrappedTask)
{
  NotImplemented;
  /* tswitch(WrappedTask) */
  /* { */
  /*   poof( */
  /*     func (async_function_call_type tag_t) @code_fragment */
  /*     { */
  /*       tag_t.map(tag_v) */
  /*       { */
  /*         { */
  /*           tmatch( tag_v.name.strip_single_prefix, WrappedTask, FuncParams ); */
  /*           ExecFunction(FuncParams); */
  /*         } break; */
  /*       } */
  /*     } */
  /*   ) */
/* #include <generated/poof_func.anonymous$async_function_call_type$xS6OHMBZ.h> */
  /* } */
}

link_internal work_queue_job *
GetJobFromGlobal(platform *Plat, global_job_index GlobalJobIndex)
{
  work_queue_job *Result = StripVolatile(work_queue_job*, Plat->Jobs+GlobalJobIndex.Index);
  return Result;
}

link_internal work_queue_task *
PeekNextTask(work_queue_job *Job)
{
  work_queue_task* Result = TryGetPtr(&Job->Tasks, Job->NextTaskIndex);
  return Result;
}

link_internal work_queue_task *
PopNextTask(work_queue_job *Job)
{
  work_queue_task* Result = GetPtr(&Job->Tasks, Job->NextTaskIndex++);
  return Result;
}

link_internal void
PushTask(work_queue_job *Job, work_queue_task *Task)
{
  Push(&Job->Tasks, Task);
}

// @assert_job_queue
//
link_internal void
SubmitJob( work_queue *Queue, work_queue_job *Job )
{
  Assert(Queue);

  TIMED_FUNCTION();

  platform *Plat = GetPlatform();

  AcquireFutex(&Queue->EnqueueFutex);

  while (QueueIsFull(Queue))
  {
    b32 HighPriorityMode = False;
    if (Plat->HighPriorityModeFutex.SignalValue != FUTEX_UNSIGNALLED_VALUE)
    {
      UnsignalFutex(&Plat->HighPriorityModeFutex);
      HighPriorityMode = True;
    }

    Perf("Queue full!");
    SleepMs(1);

    if (HighPriorityMode) { SignalFutex(&Plat->HighPriorityModeFutex); }
  }

  FullBarrier;

  Queue->JobIndices[Queue->EnqueueIndex] = Job->Index;

  u32 NewIndex = GetNextQueueIndex(Queue->EnqueueIndex);
  Assert(NewIndex != Queue->DequeueIndex); // QueueIsFull check
  AtomicExchange(&Queue->EnqueueIndex, NewIndex);

  FullBarrier;

  ReleaseFutex(&Queue->EnqueueFutex);
}

link_internal void
SubmitSingleTask( work_queue *Queue, work_queue_task *Entry, b32 PerfTrackJob)
{
  TIMED_FUNCTION();

  Assert(Queue);
  // @assert_job_queue
  Assert(Entry->Queue == Queue);

  // TODO(Jesse): Pass in Platform
  work_queue_job *Job = ReserveWorkQueueJob(GetPlatform(), PerfTrackJob);
  PushTask(Job, Entry);
  SubmitJob(Queue, Job);
}



