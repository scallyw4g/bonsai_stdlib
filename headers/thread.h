#define ReadBarrier  asm volatile("" ::: "memory"); _mm_lfence()
#define WriteBarrier asm volatile("" ::: "memory"); _mm_sfence()
#define FullBarrier  asm volatile("" ::: "memory"); _mm_sfence(); _mm_lfence()

global_variable volatile b32 MainThreadBlocksWorkerThreads;
global_variable volatile u32 WorkerThreadsWaiting;

link_internal u32 GetWorkerThreadCount();
link_internal u32 GetTotalThreadCount();
link_internal void SuspendWorkerThreads();

typedef THREAD_MAIN_RETURN (*thread_main_callback_type)(void*);

