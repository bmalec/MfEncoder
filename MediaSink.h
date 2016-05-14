#pragma once

#include <mfidl.h>
#include "MediaSinkContentInfo.h"

class MediaSink
{
private:
  IMFMediaSink* _mfMediaSink;

protected:
  MediaSink(IMFMediaSink* mediaSink);

public:
  static MediaSink *Create(const wchar_t *url, MediaSinkContentInfo* contentInfo);

  ~MediaSink();

  IMFMediaSink* GetMFMediaSink();
  IMFStreamSink* GetMFStreamSinkByIndex(DWORD id);

};


