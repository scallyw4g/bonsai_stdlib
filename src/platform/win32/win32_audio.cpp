#include <xaudio2.h>
#include <objbase.h>

typedef HRESULT (WINAPI *xaudio2_create_proc)(IXAudio2 **, UINT32, XAUDIO2_PROCESSOR);

void
PlatformShutdownAudio(platform *Plat)
{
  if (Plat == 0) { return; }
  audio *Audio = &Plat->Audio;

  for (u32 VoiceIndex = 0; VoiceIndex < Audio->VoicePoolCount; ++VoiceIndex)
  {
    if (Audio->VoicePool[VoiceIndex])
    {
      Audio->VoicePool[VoiceIndex]->DestroyVoice();
      Audio->VoicePool[VoiceIndex] = 0;
    }
  }
  Audio->VoicePoolCount = 0;
  Audio->NextVoiceIndex = 0;

  if (Audio->MasteringVoice)
  {
    Audio->MasteringVoice->DestroyVoice();
    Audio->MasteringVoice = 0;
  }

  if (Audio->Engine)
  {
    Audio->Engine->Release();
    Audio->Engine = 0;
  }

  if (Audio->ComInitialized)
  {
    CoUninitialize();
    Audio->ComInitialized = False;
  }

  if (Audio->XAudioDll)
  {
    CloseLibrary(Audio->XAudioDll);
    Audio->XAudioDll = 0;
  }

  Audio->Initialized = False;
}

b32
PlatformInitializeAudio(platform *Plat)
{
  if (Plat == 0)
  {
    Error("PlatformInitializeAudio called with null platform");
    return False;
  }

  audio *Audio = &Plat->Audio;
  if (Audio->Initialized) { return True; }

  HRESULT CoInitResult = CoInitializeEx(0, COINIT_MULTITHREADED);
  if (SUCCEEDED(CoInitResult))
  {
    Audio->ComInitialized = True;
  }
  else if (CoInitResult != RPC_E_CHANGED_MODE)
  {
    Error("CoInitializeEx failed while initializing XAudio2 (HRESULT=0x%lx)", (u32)CoInitResult);
    return False;
  }

  Audio->XAudioDll = OpenLibrary("xaudio2_9.dll");
  if (!Audio->XAudioDll)
  {
    Audio->XAudioDll = OpenLibrary("xaudio2_8.dll");
  }

  if (!Audio->XAudioDll)
  {
    Error("Could not load xaudio2 runtime (tried xaudio2_9.dll and xaudio2_8.dll)");
    PlatformShutdownAudio(Plat);
    return False;
  }

  xaudio2_create_proc XAudio2CreateProc = Cast(xaudio2_create_proc, GetProcFromLib(Audio->XAudioDll, "XAudio2Create"));
  if (!XAudio2CreateProc)
  {
    Error("Could not resolve XAudio2Create from XAudio2 runtime");
    PlatformShutdownAudio(Plat);
    return False;
  }

  HRESULT Hr = XAudio2CreateProc(&Audio->Engine, 0, XAUDIO2_DEFAULT_PROCESSOR);
  if (FAILED(Hr))
  {
    Error("XAudio2Create failed (HRESULT=0x%lx)", (u32)Hr);
    PlatformShutdownAudio(Plat);
    return False;
  }

  Hr = Audio->Engine->CreateMasteringVoice(&Audio->MasteringVoice);
  if (FAILED(Hr))
  {
    Error("IXAudio2::CreateMasteringVoice failed (HRESULT=0x%lx)", (u32)Hr);
    PlatformShutdownAudio(Plat);
    return False;
  }

  Audio->SourceVoiceFormat = {};
  Audio->SourceVoiceFormat.wFormatTag = WAVE_FORMAT_PCM;
  Audio->SourceVoiceFormat.nChannels = 2;
  Audio->SourceVoiceFormat.nSamplesPerSec = 44100;
  Audio->SourceVoiceFormat.wBitsPerSample = 16;
  Audio->SourceVoiceFormat.nBlockAlign = (Audio->SourceVoiceFormat.nChannels * Audio->SourceVoiceFormat.wBitsPerSample) / 8;
  Audio->SourceVoiceFormat.nAvgBytesPerSec = Audio->SourceVoiceFormat.nSamplesPerSec * Audio->SourceVoiceFormat.nBlockAlign;
  Audio->SourceVoiceFormat.cbSize = 0;

  for (u32 VoiceIndex = 0; VoiceIndex < PLATFORM_AUDIO_VOICE_POOL_COUNT; ++VoiceIndex)
  {
    IXAudio2SourceVoice *Voice = 0;
    Hr = Audio->Engine->CreateSourceVoice(&Voice, &Audio->SourceVoiceFormat, 0, XAUDIO2_DEFAULT_FREQ_RATIO, 0, 0, 0);
    if (FAILED(Hr))
    {
      Error("IXAudio2::CreateSourceVoice failed for pool voice %u (HRESULT=0x%lx)", VoiceIndex, (u32)Hr);
      PlatformShutdownAudio(Plat);
      return False;
    }

    Audio->VoicePool[VoiceIndex] = Voice;
    ++Audio->VoicePoolCount;
  }

  Audio->Initialized = True;
  return True;
}

b32
PlatformPlaySoundBuffer(platform *Plat,
                        u8 *Data,
                        u32 DataByteCount,
                        u16 FormatTag,
                        u16 ChannelCount,
                        u32 SamplesPerSecond,
                        u32 BytesPerSecond,
                        u16 BlockAlign,
                        u16 BitsPerSample)
{
  if (Plat == 0)
  {
    Error("PlatformPlaySoundBuffer called with null platform");
    return False;
  }

  audio *Audio = &Plat->Audio;
  if (Audio->Initialized == False)
  {
    Error("PlatformPlaySoundBuffer called before PlatformInitializeAudio");
    return False;
  }

  if (Data == 0 || DataByteCount == 0)
  {
    Error("PlatformPlaySoundBuffer called with empty buffer");
    return False;
  }

  if (FormatTag != Audio->SourceVoiceFormat.wFormatTag ||
      ChannelCount != Audio->SourceVoiceFormat.nChannels ||
      SamplesPerSecond != Audio->SourceVoiceFormat.nSamplesPerSec ||
      BytesPerSecond != Audio->SourceVoiceFormat.nAvgBytesPerSec ||
      BlockAlign != Audio->SourceVoiceFormat.nBlockAlign ||
      BitsPerSample != Audio->SourceVoiceFormat.wBitsPerSample)
  {
    Error("PlatformPlaySoundBuffer format mismatch; expected %u-ch %uHz %u-bit PCM", Audio->SourceVoiceFormat.nChannels, Audio->SourceVoiceFormat.nSamplesPerSec, Audio->SourceVoiceFormat.wBitsPerSample);
    return False;
  }

  if (Audio->VoicePoolCount == 0)
  {
    Error("PlatformPlaySoundBuffer called with empty voice pool");
    return False;
  }

  u32 VoiceIndex = Audio->NextVoiceIndex++ % Audio->VoicePoolCount;
  IXAudio2SourceVoice *Voice = Audio->VoicePool[VoiceIndex];
  if (Voice == 0)
  {
    Error("PlatformPlaySoundBuffer found null source voice at index %u", VoiceIndex);
    return False;
  }

  HRESULT Hr = Voice->Stop(0);
  if (FAILED(Hr)) { Error("IXAudio2SourceVoice::Stop failed (HRESULT=0x%lx)", (u32)Hr); return False; }

  Hr = Voice->FlushSourceBuffers();
  if (FAILED(Hr)) { Error("IXAudio2SourceVoice::FlushSourceBuffers failed (HRESULT=0x%lx)", (u32)Hr); return False; }

  XAUDIO2_BUFFER Buffer = {};
  Buffer.pAudioData = Data;
  Buffer.AudioBytes = DataByteCount;
  Buffer.Flags = XAUDIO2_END_OF_STREAM;

  Hr = Voice->SubmitSourceBuffer(&Buffer);
  if (FAILED(Hr)) { Error("IXAudio2SourceVoice::SubmitSourceBuffer failed (HRESULT=0x%lx)", (u32)Hr); return False; }

  Hr = Voice->Start(0);
  if (FAILED(Hr)) { Error("IXAudio2SourceVoice::Start failed (HRESULT=0x%lx)", (u32)Hr); return False; }

  return True;
}
