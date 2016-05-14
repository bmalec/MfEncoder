#include "stdafx.h"
#include <mfapi.h>
#include <wmcodecdsp.h>
#include <Mferror.h>
#include <stdexcept>
#include "Util.h"
#include "AudioEncoder.h"


AudioEncoderParameters::AudioEncoderParameters(CompressionFamily compressionFamily, int qualityLevel, int channelCount, int samplesPerSecond, int bitsPerSample)
{
  _compressionFamily = compressionFamily;
  _qualityLevel = qualityLevel;
  _channelCount = channelCount;
  _samplesPerSecond = samplesPerSecond;
  _bitsPerSample = bitsPerSample;
}

AudioEncoderParameters* AudioEncoderParameters::CreateQualityBasedVbrParameters(int qualityLevel, int channelCount, int samplesPerSecond, int bitsPerSample)
{
  // TODO add parameter validation

  return new AudioEncoderParameters(CompressionFamily::QualityBasedVariableBitRate, qualityLevel, channelCount, samplesPerSecond, bitsPerSample);
}


BOOL AudioEncoderParameters::IsLossless()
{
  return _compressionFamily == CompressionFamily::Lossless;
}


BOOL AudioEncoderParameters::IsQualityBasedVbr()
{
  return _compressionFamily == CompressionFamily::QualityBasedVariableBitRate;
}


int AudioEncoderParameters::GetQualityLevel()
{
  return _qualityLevel;
}


int AudioEncoderParameters::GetChannelCount()
{
  return _channelCount;
}

int AudioEncoderParameters::GetBitsPerSample()
{
  return _bitsPerSample;
}


int AudioEncoderParameters::GetSamplesPerSecond()
{
  return _samplesPerSecond;
}




AudioEncoder::AudioEncoder(IMFActivate* mfActivate)
{

  _mfActivate = mfActivate;

  if (!SUCCEEDED(_mfActivate->ActivateObject(IID_PPV_ARGS(&_mfTransform))))
    throw std::exception("Unable to activate audio encoder");

  if (!SUCCEEDED(_mfTransform->QueryInterface(IID_PPV_ARGS(&_propertyStore))))
    throw std::exception("Unable to access audio encoder property store");






}


AudioEncoder::~AudioEncoder()
{
  _propertyStore->Release();
  _mfTransform->Release();
  _mfActivate->ShutdownObject();
  _mfActivate->Release();
}


IMFMediaType* AudioEncoder::GetEncoderMediaType(AudioEncoderParameters* encoderParameters)
{
  IMFMediaType* result;

  //We need to find a suitable output media type
  //We need to create the encoder to get the available output types
  //and discard the instance.

  IMFActivate **transformActivationObjs;
  UINT32 transformCount;

  MFT_REGISTER_TYPE_INFO regTypeInfo;

  regTypeInfo.guidMajorType = MFMediaType_Audio;
  regTypeInfo.guidSubtype = (encoderParameters->IsLossless()) ? MFAudioFormat_WMAudio_Lossless : MFAudioFormat_WMAudioV8;

  // Look for an encoder.

  HRESULT hr = MFTEnumEx(MFT_CATEGORY_AUDIO_ENCODER, MFT_ENUM_FLAG_TRANSCODE_ONLY, nullptr, &regTypeInfo, &transformActivationObjs, &transformCount);

  if ((hr != S_OK) || (transformCount < 1))
    throw std::exception("No Media Foundation encoders found");

  // Regardless how many activation objects returned, just instantiate the first one
  // (would I want to instantiate another?  Why?  Which one?)

  IMFActivate *activationObj = *transformActivationObjs;
  IMFTransform *mfEncoder;
  wchar_t transformName[128];
  UINT32 nameLen;

  activationObj->GetString(MFT_FRIENDLY_NAME_Attribute, transformName, sizeof(transformName), &nameLen);

   hr = activationObj->ActivateObject(IID_PPV_ARGS(&mfEncoder));

   IPropertyStore *propertyStore = nullptr;  // TODO when can this be released safely?

   if (encoderParameters->IsQualityBasedVbr())
   {
     hr = mfEncoder->QueryInterface(IID_PPV_ARGS(&propertyStore));

     SetEncoderPropertyStoreValuesForQualityBasedVbr(propertyStore, encoderParameters->GetQualityLevel());
   }

  // this fails with a not_implemented error  hr = propertyStore->Commit();

  // enumerate output types and try to find the appropriate one for our purposes

  UINT32 channelCount;
  UINT32 samplesPerSecond;
  UINT32 bitsPerSample;

  int desiredChannelCount = encoderParameters->GetChannelCount();
  int desiredSamplesPerSecond = encoderParameters->GetSamplesPerSecond();
  int desiredBitsPerSample = encoderParameters->GetBitsPerSample();

  DWORD index = 0;

  while (true)
  {
    IMFMediaType *mediaType;

    hr = mfEncoder->GetOutputAvailableType(0, index++, &mediaType);

    if (hr == MF_E_NO_MORE_TYPES)
      break;

    // Get the AM_MEDIA_TYPE structure from the media type, in case we want to need
    // to differentiate between Standard and Pro WMA codecs in the future...

    AM_MEDIA_TYPE *amMediaType;
    mediaType->GetRepresentation(AM_MEDIA_TYPE_REPRESENTATION, (LPVOID *)&amMediaType);
    WAVEFORMATEX *waveFormat = (WAVEFORMATEX *)amMediaType->pbFormat;

    // there's only a few things we're interested in with the output type, so only bother grabbing those values

    hr = mediaType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &channelCount);
    hr = mediaType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &samplesPerSecond);
    hr = mediaType->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &bitsPerSample);

    if ((channelCount == desiredChannelCount) && (samplesPerSecond == desiredSamplesPerSecond) && (bitsPerSample == desiredBitsPerSample))
    {
      result = mediaType;
      break;
    }
    else
    {
      mediaType->Release();
    }

  }

  propertyStore->Release();
  mfEncoder->Release();
  activationObj->ShutdownObject();

  // release all the stupid activation pointers (because COM was such a GREAT idea)

  for (UINT32 i = 0; i < transformCount; i++)
  {
    IMFActivate *temp = *(transformActivationObjs + i);
    temp->Release();
  }

  // free the stupid activation array object (because COM was such an f'ing great idea)
  // (did I ever mention I think COM was just... stupid?)

  CoTaskMemFree(transformActivationObjs);

  return result;
}



void AudioEncoder::SetEncoderPropertyStoreValuesForQualityBasedVbr(IPropertyStore* propertyStore, int quality)
{
  SetBooleanPropertyStoreValue(propertyStore, MFPKEY_VBRENABLED, TRUE);
  SetBooleanPropertyStoreValue(propertyStore, MFPKEY_CONSTRAIN_ENUMERATED_VBRQUALITY, TRUE);
  SetUint32PropertyStoreValue(propertyStore, MFPKEY_DESIRED_VBRQUALITY, quality);
}
