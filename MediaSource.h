#pragma once

#include <mfidl.h>
#include "StreamDescriptor.h"
// #include <mfreadwrite.h>

class MediaSource
{
  struct MetadataKeyValuePair
  {
    wchar_t Key[40];
    wchar_t *Value;
  };


private:
  IMFMediaSource* _mfMediaSource;
  IMFPresentationDescriptor* _mfPresentationDescriptor;
  MetadataKeyValuePair *_metadata;
  int _metadataItemCount;
  DWORD _streamDescriptorCount;

  void LoadMetadataFromSource();

protected:
  MediaSource(IMFMediaSource *mfMediaSource);

public:
  static MediaSource *Open(const wchar_t *url);
  ~MediaSource();

  IMFPresentationDescriptor* GetPresentationDescriptor();
  DWORD GetStreamDescriptorCount();
  GUID GetMajorType();
  StreamDescriptor* GetStreamDescriptorByIndex(DWORD index);

//  IMFMediaType *GetCurrentMediaType();
  IMFMediaSource* GetMFMediaSource();
//  wchar_t *GetMetadataValue(wchar_t *metadataKey);
};
