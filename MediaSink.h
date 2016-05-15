#pragma once

#include <mfidl.h>
#include "MediaSinkContentInfo.h"

class MediaSink
{
private:
  IMFMediaSink* _mfMediaSink;
  MediaSinkContentInfo* _mediaSinkContentInfo;

protected:
  MediaSink(IMFMediaSink* mediaSink, MediaSinkContentInfo* mediaSinkContentInfo);

public:
  static MediaSink *Create(const wchar_t *url, MediaSinkContentInfo* contentInfo);

  ~MediaSink();

  IMFMediaSink* GetMFMediaSink();
  IMFStreamSink* GetMFStreamSinkByIndex(DWORD id);
  IPropertyStore* GetEncoderConfigurationPropertyStore(WORD streamNumber);
  IMFTopologyNode* CreateTopologyOutputNode(WORD streamNumber);
  IMFTopologyNode* CreateTopologyTransformNode(WORD streamNumber);




};


