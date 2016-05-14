#pragma once

#include <wmcontainer.h>
#include "AudioEncoder.h"

class MediaSinkContentInfo
{
public:
  MediaSinkContentInfo();
  ~MediaSinkContentInfo();

  void AddStreamSink(WORD streamNumber, AudioEncoderParameters* encoderParameters);
  IMFASFContentInfo* GetMfAsfContentInfoObject();
  
private:
  IMFASFContentInfo* _mfAsfContentInfo;
  IMFASFProfile* _mfAsfProfile;
};
