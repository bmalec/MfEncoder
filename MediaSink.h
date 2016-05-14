#pragma once

#include <mfidl.h>
#include "MediaSource.h"
#include "MediaSinkContentInfo.h"
#include "Parameters.h"

class MediaSink
{
private:
  IMFMediaSink* _mfMediaSink;

protected:
  MediaSink(IMFMediaSink* mediaSink);

public:
  static MediaSink *Create(const wchar_t *url, MediaSource* source, Parameters* params);
  static MediaSink *Create(const wchar_t *url, IMFMediaType* mfMediaType, Parameters* params);
  static MediaSink *Create(const wchar_t *url, MediaSinkContentInfo* contentInfo);

  ~MediaSink();

  IMFMediaSink* GetMFMediaSink();
  IMFStreamSink* GetMFStreamSinkByIndex(DWORD id);

};


