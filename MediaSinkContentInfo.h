#pragma once

#include <wmcontainer.h>
#include "AudioEncoder.h"

class AsfContentInfoBuilder
{
public:
  AsfContentInfoBuilder();
  ~AsfContentInfoBuilder();

  void AddStreamSink(WORD streamNumber, AudioEncoderParameters* encoderParameters);
  void SetMetadataAsString(LPWSTR field, LPWSTR value);
  IMFASFContentInfo* ConstructMfAsfContentInfo();

//  IPropertyStore* GetEncoderConfigurationPropertyStore(WORD streamNumber);

private:
  IMFASFContentInfo* _mfAsfContentInfo;
  IMFASFProfile* _mfAsfProfile;
  IMFMetadataProvider* _mfMetadataProvider;
  IMFMetadata* _mfMetadata;
};
