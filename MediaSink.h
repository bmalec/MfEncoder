#pragma once

#include <mfidl.h>
#include "MediaSource.h"
#include "MediaSinkContentInfo.h"
#include "Parameters.h"

class MediaSink
{
private:
//  IMFActivate* _mfActivate;
  IMFMediaSink* _mfMediaSink;


protected:
  MediaSink(IMFMediaSink* mediaSink);

public:
  static MediaSink *Create(const wchar_t *url, MediaSource* source, Parameters* params);
  static MediaSink *Create(const wchar_t *url, IMFMediaType* mfMediaType, Parameters* params);
  static MediaSink *Create(const wchar_t *url, MediaSinkContentInfo* contentInfo);

  ~MediaSink();

//  void Activate();

//   IMFActivate* GetActivationObject();
  IMFMediaSink* GetMFMediaSink();
  IMFStreamSink* GetMFStreamSinkByIndex(DWORD id);

};


