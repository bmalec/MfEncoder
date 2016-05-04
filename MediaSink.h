#pragma once

#include <mfidl.h>
#include "MediaSource.h"
// #include <mfreadwrite.h>

class MediaSink
{
  /*
  struct MetadataKeyValuePair
  {
    wchar_t Key[40];
    wchar_t *Value;
  };
  */


private:
  IMFActivate *_mfActivate;
//  IMFMediaSource* _mfMediaSource;
//  MetadataKeyValuePair *_metadata;
//  int _metadataItemCount;
//  void LoadMetadataFromSource();

protected:
  MediaSink(IMFActivate* mfActivate);

public:
  static MediaSink *Create(const wchar_t *url, MediaSource* source);
  ~MediaSink();

  IMFActivate* GetActivationObject();

  //  IMFMediaType *GetCurrentMediaType();
//  IMFMediaSource* GetMFMediaSource();
  //  wchar_t *GetMetadataValue(wchar_t *metadataKey);
};

