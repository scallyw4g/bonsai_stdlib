
#include <evntrace.h>
#include <evntcons.h>
#include <strsafe.h>
#include <tdh.h>

void Bonsai_ETWEventCallback(PEVENT_RECORD Event)
{
  EVENT_HEADER *Header = &Event->EventHeader;

  u8 ProcessorNumber = Event->BufferContext.ProcessorNumber;
  u32 ThreadID = Header->ThreadId;
  s64 CycleTime = Header->TimeStamp.QuadPart;

  s32 ProviderIsSystem = IsEqualGUID(Header->ProviderId, SystemTraceControlGuid);

  /* DebugLine("Event { (%d) (%u)(%u)(%lu) }", ProviderIsSystem, (u32)ProcessorNumber, ThreadID, CycleTime); */
}

static TRACEHANDLE OpenTraceHandle;

void
DoRandomAssLoggingIFoundOnStackOverflow()
{
#define MAX_GUID_SIZE 39

  DWORD status = ERROR_SUCCESS;
  PROVIDER_ENUMERATION_INFO* penum = NULL;    // Buffer that contains provider information
  PROVIDER_ENUMERATION_INFO* ptemp = NULL;
  DWORD bufSize = 0;                       // Size of the penum buffer
  HRESULT hr = S_OK;                          // Return value for StringFromGUID2
  WCHAR StringGuid[MAX_GUID_SIZE];
  DWORD RegisteredMOFCount = 0;
  DWORD RegisteredManifestCount = 0;

  // Retrieve the required buffer size.

  status = TdhEnumerateProviders(penum, &bufSize);

  // Allocate the required buffer and call TdhEnumerateProviders. The list of 
  // providers can change between the time you retrieved the required buffer 
  // size and the time you enumerated the providers, so call TdhEnumerateProviders
  // in a loop until the function does not return ERROR_INSUFFICIENT_BUFFER.

  while (ERROR_INSUFFICIENT_BUFFER == status)
  {
      ptemp = (PROVIDER_ENUMERATION_INFO*)realloc(penum, bufSize);
      if (NULL == ptemp)
      {
          wprintf(L"Allocation failed (size=%lu).\n", bufSize);
          goto cleanup;
      }

      penum = ptemp;
      ptemp = NULL;

      status = TdhEnumerateProviders(penum, &bufSize);
  }

  if (ERROR_SUCCESS != status)
  {
      wprintf(L"TdhEnumerateProviders failed with %lu.\n", status);
  }
  else
  {
      // Loop through the list of providers and print the provider's name, GUID, 
      // and the source of the information (MOF class or instrumentation manifest).

      b32 FoundKernelLogger = False;
      for (DWORD i = 0; i < penum->NumberOfProviders; i++)
      {
          hr = StringFromGUID2(penum->TraceProviderInfoArray[i].ProviderGuid, StringGuid, ARRAYSIZE(StringGuid));

          if (FAILED(hr))
          {
              wprintf(L"StringFromGUID2 failed with 0x%x\n", hr);
              goto cleanup;
          }

          const size_t cSize = sizeof(KERNEL_LOGGER_NAME);
          wchar_t wc[cSize];
          mbstowcs (wc, KERNEL_LOGGER_NAME, cSize);

          if ( wcscmp( (LPWSTR)((PBYTE)(penum)+penum->TraceProviderInfoArray[i].ProviderNameOffset), wc) == 0 )
          {
            FoundKernelLogger = True;
          }

          /* wprintf(L"Provider name: %s\nProvider GUID: %s\nSource: %s\n\n", */
          /*     (LPWSTR)((PBYTE)(penum)+penum->TraceProviderInfoArray[i].ProviderNameOffset), */
          /*     StringGuid, */
          /*     (penum->TraceProviderInfoArray[i].SchemaSource) ? L"WMI MOF class" : L"XML manifest"); */

          (penum->TraceProviderInfoArray[i].SchemaSource) ? RegisteredMOFCount++ : RegisteredManifestCount++;
      }

      if (FoundKernelLogger)
      {
        Info("Found kernel logger!");
      }
      DebugLine(CSz("\nThere are %d registered providers; %lu are registered via MOF class and\n%lu are registered via a manifest.\n"),
          penum->NumberOfProviders,
          RegisteredMOFCount,
          RegisteredManifestCount);
  }

cleanup:

  if (penum)
  {
      free(penum);
      penum = NULL;
  }
}

link_internal b32
CloseAnyExistingTrace(EVENT_TRACE_PROPERTIES *EventTracingProps)
{
  b32 Result = False;

  auto ControlTraceResult = ControlTrace(0, KERNEL_LOGGER_NAME, EventTracingProps, EVENT_TRACE_CONTROL_STOP);
  if (ControlTraceResult == ERROR_SUCCESS || ControlTraceResult == ERROR_WMI_INSTANCE_NOT_FOUND)
  {
    Result = True;
  }
  return Result;
}

link_internal void
SetupPropsForContextSwitchEventTracing(EVENT_TRACE_PROPERTIES *EventTracingProps, u32 BufferSize)
{
  ZeroMemory(EventTracingProps, BufferSize);

  EventTracingProps->Wnode.BufferSize = BufferSize;
  EventTracingProps->Wnode.Guid = SystemTraceControlGuid;          // Tell it to profile kernel events
  EventTracingProps->Wnode.ClientContext = 3;
  EventTracingProps->Wnode.Flags = WNODE_FLAG_TRACED_GUID;         // NOTE(Jesse): Apparently this has to appear here, but I have no idea what it does

  EventTracingProps->BufferSize = 4; // Size (in KB) of the buffer ETW allocates for events.  4 (4kb) is minimum, 16384 (16mb) is maximum
  EventTracingProps->MinimumBuffers = 1;
  EventTracingProps->MaximumBuffers = 1;

  // Seems to me like we should use this, but Casey says use real_time_mode.. so
  // that's what we're going to do, for now.
  /* EventTracingProps.LogFileMode = EVENT_TRACE_BUFFERING_MODE; */
  // EventTracingProps.FlushTimer = 1; // Not used for circular in-memory sessions

  // Use default if it's not set
  /* EventTracingProps->FlushTimer = 1; */

  // Seems like we might need these flags too?
  /* EventTracingProps->LogFileMode = EVENT_TRACE_REAL_TIME_MODE | EVENT_TRACE_SYSTEM_LOGGER_MODE; */
  /* EventTracingProps->EnableFlags = EVENT_TRACE_FLAG_THREAD | EVENT_TRACE_FLAG_CSWITCH; */

  EventTracingProps->LogFileMode = EVENT_TRACE_REAL_TIME_MODE;
  EventTracingProps->EnableFlags = EVENT_TRACE_FLAG_CSWITCH;

  EventTracingProps->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);
  StringCbCopy(((char*)EventTracingProps + EventTracingProps->LoggerNameOffset), sizeof(KERNEL_LOGGER_NAME), KERNEL_LOGGER_NAME);
  //
  // Supposedly unused if we specify EVENT_TRACE_REAL_TIME_MODE
  /* EventTracingProps->LogFileNameOffset = sizeof(EVENT_TRACE_PROPERTIES) + sizeof(KERNEL_LOGGER_NAME); */
  /* StringCbCopy(((char*)EventTracingProps + EventTracingProps->LogFileNameOffset), sizeof(LOG_FILE_NAME), LOG_FILE_NAME); */
}

static DWORD WINAPI
Win32TracingThread(LPVOID Parameter)
{
  u32 BufferSize = sizeof(EVENT_TRACE_PROPERTIES) + sizeof(KERNEL_LOGGER_NAME);
  EVENT_TRACE_PROPERTIES *EventTracingProps = (EVENT_TRACE_PROPERTIES*)malloc(BufferSize);

  SetupPropsForContextSwitchEventTracing(EventTracingProps, BufferSize);

  // We have to first shut down anyone using this tracer because there's a
  // system-wide limit of _one_ consumer of it.  If we opened a trace and
  // crashed, it's still open, and our call to StartTrace will fail.
  Info("Stopping existing Kernel Trace Session if it exists");
  if (CloseAnyExistingTrace(EventTracingProps))
  {
    // NOTE(Jesse): ControlTrace() modifies the EVENT_TRACE_PROPERTIES struct,
    // so we have to zero it and fill it out again.
    SetupPropsForContextSwitchEventTracing(EventTracingProps, BufferSize);

    Info("Starting Trace");
    TRACEHANDLE TraceHandle = {};
    auto StartTraceResult = StartTraceA(&TraceHandle, KERNEL_LOGGER_NAME, EventTracingProps);

    if ( StartTraceResult == ERROR_SUCCESS )
    {
      EVENT_TRACE_LOGFILE Logfile = {};

      Logfile.LoggerName = (char*)KERNEL_LOGGER_NAME;
      Logfile.ProcessTraceMode = (PROCESS_TRACE_MODE_REAL_TIME | PROCESS_TRACE_MODE_EVENT_RECORD | PROCESS_TRACE_MODE_RAW_TIMESTAMP);
      Logfile.EventRecordCallback = Bonsai_ETWEventCallback;

      Info("OpenTrace");
      OpenTraceHandle = OpenTrace(&Logfile);
      if (OpenTraceHandle != INVALID_PROCESSTRACE_HANDLE)
      {
        Info("Started Context Switch tracing");
        Ensure( ProcessTrace(&OpenTraceHandle, 1, 0, 0) == ERROR_SUCCESS );
      }
      else
      {
        u64 SpecificErrorCode = GetLastError();
        Error("Opening Trace : error number (%u)", SpecificErrorCode);
      }
    }
    else
    {
      Error("Starting Trace : error number (%u)", StartTraceResult);
    }
  }
  else
  {
    Error("Closing Existing tracing sessions failed.");
  }

  DebugLine("Exiting ProcessTrace thread");
  return(0);
}

void
Platform_EnableContextSwitchTracing()
{
  /* Assert(Logfile.IsKernelTrace == TRUE); */

  DWORD ThreadID;
  Info("Creating tracing thread");
  HANDLE ThreadHandle = CreateThread(0, 0, Win32TracingThread, 0, 0, &ThreadID);
}

