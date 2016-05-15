#pragma once

#include <mfidl.h>

class MediaSource
{
  struct MetadataKeyValuePair
  {
    PWSTR Key;
    PWSTR Value;
  };


private:
  IMFMediaSource* _mfMediaSource;
  IMFPresentationDescriptor* _mfPresentationDescriptor;
  MetadataKeyValuePair *_metadata;
  ULONG _metadataPropertyCount;

  void LoadMetadataFromSource();

protected:
  MediaSource(IMFMediaSource *mfMediaSource);

public:
  static MediaSource *Open(const wchar_t *url);
  ~MediaSource();

  IMFPresentationDescriptor* GetPresentationDescriptor();

  IMFTopologyNode* CreateTopologySourceNode();

  IMFMediaSource* GetMFMediaSource();
  wchar_t *GetMetadataValue(wchar_t *metadataKey);
};
