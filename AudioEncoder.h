#pragma once

#include <mfidl.h>

class AudioEncoderParameters
{
  enum class CompressionFamily {Lossless, ConstantBitRate, QualityBasedVariableBitRate};

public:
  static AudioEncoderParameters* CreateQualityBasedVbrParameters(int qualityLevel, int channelCount, int samplesPerSecond, int bitsPerSample);
  static AudioEncoderParameters* CreateLosslessEncoderParameters(int channelCount, int samplesPerSecond, int bitsPerSample);


  BOOL IsLossless();
  BOOL IsQualityBasedVbr();
  int GetQualityLevel();
  int GetChannelCount();
  int GetSamplesPerSecond();
  int GetBitsPerSample();

protected:
  AudioEncoderParameters(CompressionFamily compressionFamily, int qualityLevel, int channelCount, int samplesPerSecond, int bitsPerSample);

private:
  CompressionFamily _compressionFamily;
  int _qualityLevel;
  int _channelCount;
  int _samplesPerSecond;
  int _bitsPerSample;
};


class AudioEncoder
{
public:
  static IMFMediaType* GetEncoderMediaType(AudioEncoderParameters* encoderParameters);
  static void SetEncoderPropertyStoreValuesForQualityBasedVbr(IPropertyStore* propertyStore, int quality);

//  static IMFMediaType* GetConstantBitrateMediaType(int bitrate);
  ~AudioEncoder();

protected:
  AudioEncoder(IMFActivate* mfActivate);


private:
  IMFActivate* _mfActivate;
  IMFTransform* _mfTransform;
  IPropertyStore* _propertyStore;





};