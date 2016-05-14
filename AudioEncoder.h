#pragma once

#include <mfidl.h>

class AudioEncoder
{
public:
  static IMFMediaType* GetQualityBasedMediaType(int quality);

//  static IMFMediaType* GetConstantBitrateMediaType(int bitrate);
  ~AudioEncoder();

protected:
  AudioEncoder(IMFActivate* mfActivate);


private:
  IMFActivate* _mfActivate;
  IMFTransform* _mfTransform;
  IPropertyStore* _propertyStore;





};
