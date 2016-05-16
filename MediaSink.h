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
  static MediaSink *Create(const wchar_t *url, IMFASFContentInfo* asfContentInfo);

  ~MediaSink();

  IMFMediaSink* GetMFMediaSink();
  IMFStreamSink* GetMFStreamSinkByIndex(DWORD id);
//  IPropertyStore* GetEncoderConfigurationPropertyStore(WORD streamNumber);
  IMFTopologyNode* CreateTopologyOutputNode(WORD streamNumber);
  IMFTopologyNode* CreateTopologyTransformNode(WORD streamNumber);
  IMFTransform* GetAudioEncoderForStream(WORD streamNumber);
  void UpdatePostEncodeStreamSinkProperties(WORD streamNumber, IPropertyStore* encoderProperties);




};


