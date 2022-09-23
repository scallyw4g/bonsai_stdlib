#define ReadBarrier  asm volatile("" ::: "memory"); _mm_lfence()
#define WriteBarrier asm volatile("" ::: "memory"); _mm_sfence()
#define FullBarrier  asm volatile("" ::: "memory"); _mm_sfence(); _mm_lfence()

link_internal u32 GetWorkerThreadCount();
link_internal u32 GetTotalThreadCount();
link_internal void WaitForWorkerThreads(volatile u32 *WorkerThreadsWaiting);

typedef THREAD_MAIN_RETURN (*thread_main_callback_type)(void*);

