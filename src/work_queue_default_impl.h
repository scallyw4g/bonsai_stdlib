#ifdef BONSAI_STDLIB_USE_CUSTOM_THREADPOOL
#error "Included work_queue_default_impl.h when BONSAI_STDLIB_USE_CUSTOM_THREADPOOL was defined"
#endif

link_internal void
AllocateJobsArray(platform *Plat, s32 TotalJobs) {}

link_internal void
HandleJob(work_queue_job *Job, thread_local_state *Thread, application_api *AppApi);
