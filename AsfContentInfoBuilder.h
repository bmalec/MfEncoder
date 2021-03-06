#pragma once

#include <wmcontainer.h>
#include "AudioEncoder.h"

class AsfContentInfoBuilder
{
public:
  AsfContentInfoBuilder();
  ~AsfContentInfoBuilder();

  void AddStreamSink(WORD streamNumber, AudioEncoderParameters* encoderParameters);
  void SetMetadataAsString(PCWSTR field, PCWSTR value);
  IMFASFContentInfo* ConstructMfAsfContentInfo();

private:
  IMFASFContentInfo* _mfAsfContentInfo;
  IMFASFProfile* _mfAsfProfile;
  IMFMetadataProvider* _mfMetadataProvider;
  IMFMetadata* _mfMetadata;
};
