#pragma once

#include <mfidl.h>
#include "MediaSource.h"
#include "MediaSink.h"

class AudioEncoderParameters
{
  enum class CompressionFamily {Lossless, ConstantBitRate, QualityBasedVariableBitRate};

public:
  static AudioEncoderParameters* CreateQualityBasedVbrParameters(UINT32 qualityLevel, UINT32 channelCount, UINT32 samplesPerSecond, UINT32 bitsPerSample);
  static AudioEncoderParameters* CreateLosslessEncoderParameters(UINT32 channelCount, UINT32 samplesPerSecond, UINT32 bitsPerSample);

  BOOL IsLossless();
  BOOL IsQualityBasedVbr();
  UINT32 GetQualityLevel();
  UINT32 GetChannelCount();
  UINT32 GetSamplesPerSecond();
  UINT32 GetBitsPerSample();

protected:
  AudioEncoderParameters(CompressionFamily compressionFamily, UINT32 qualityLevel, UINT32 channelCount, UINT32 samplesPerSecond, UINT32 bitsPerSample);

private:
  CompressionFamily _compressionFamily;
  UINT32 _qualityLevel;
  UINT32 _channelCount;
  UINT32 _samplesPerSecond;
  UINT32 _bitsPerSample;
};


class AudioEncoder
{
public:
  static IMFMediaType* GetEncoderMediaType(AudioEncoderParameters* encoderParameters);
  static void SetEncoderPropertiesForQualityBasedVbr(IPropertyStore* propertyStore, UINT32 quality);
  static void Encode(MediaSource* mediaSource, MediaSink* mediaSink, AudioEncoderParameters* encoderParameters);

private:
  IMFActivate* _mfActivate;
  IMFTransform* _mfTransform;
  IPropertyStore* _propertyStore;
};
