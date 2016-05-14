#pragma once

#include <wmcontainer.h>
#include "AudioEncoder.h"

class MediaSinkContentInfo
{
public:
  MediaSinkContentInfo();
  ~MediaSinkContentInfo();

  void AddStreamSink(WORD streamNumber, AudioEncoderParameters* encoderParameters);
  void SetMetadataAsString(LPWSTR field, LPWSTR value);
  IMFASFContentInfo* GetMfAsfContentInfoObject();

private:
  IMFASFContentInfo* _mfAsfContentInfo;
  IMFASFProfile* _mfAsfProfile;
  IMFMetadataProvider* _mfMetadataProvider;
  IMFMetadata* _mfMetadata;
};
