#pragma once

#include <mfidl.h>
#include <wmcontainer.h>

class MediaSink
{
private:
  IMFMediaSink* _mfMediaSink;
  IMFASFContentInfo* _mfAsfContentInfo;

protected:
  MediaSink(IMFMediaSink* mediaSink, IMFASFContentInfo* asfContentInfo);

  IMFMediaType* GetMediaTypeForStream(WORD streamNumber);

public:
  static MediaSink *Create(PCWSTR url, IMFASFContentInfo* asfContentInfo);

  ~MediaSink();

  IMFTopologyNode* CreateTopologyOutputNode(WORD streamNumber);
  IMFTopologyNode* CreateTopologyTransformNode(WORD streamNumber);
  IMFTransform* GetAudioEncoderForStream(WORD streamNumber);
  void UpdatePostEncodeStreamSinkProperties(WORD streamNumber, IPropertyStore* encoderProperties);




};


