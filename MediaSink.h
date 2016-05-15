#pragma once

#include <mfidl.h>
#include <wmcontainer.h>
//#include "MediaSinkContentInfo.h"

class MediaSink
{
private:
  IMFMediaSink* _mfMediaSink;
  IMFASFContentInfo* _mfAsfContentInfo;
//  AsfContentInfoBuilder* _mediaSinkContentInfo;

protected:
  MediaSink(IMFMediaSink* mediaSink, IMFASFContentInfo* asfContentInfo);

  IMFMediaType* GetMediaTypeForStream(WORD streamNumber);

public:
  static MediaSink *Create(const wchar_t *url, IMFASFContentInfo* asfContentInfo);

  ~MediaSink();

  IMFMediaSink* GetMFMediaSink();
  IMFStreamSink* GetMFStreamSinkByIndex(DWORD id);
  IPropertyStore* GetEncoderConfigurationPropertyStore(WORD streamNumber);
  IMFTopologyNode* CreateTopologyOutputNode(WORD streamNumber);
  IMFTopologyNode* CreateTopologyTransformNode(WORD streamNumber);




};


