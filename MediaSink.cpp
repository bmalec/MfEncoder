#include "stdafx.h"
#include <wmcontainer.h>
#include <mfapi.h>
#include <Mferror.h>
#include <wmcodecdsp.h>
#include <propvarutil.h>
#include "Globals.h"
#include "MediaSink.h"

const INT32 VIDEO_WINDOW_MSEC = 3000;



MediaSink::MediaSink(IMFActivate *mfActivate)
{
  _mfActivate = mfActivate;
}



MediaSink::~MediaSink()
{
  if (_mfActivate)
    _mfActivate->Release();
}

IMFActivate* MediaSink::GetActivationObject()
{
  return _mfActivate;
}


static void SetBooleanProperty(IPropertyStore *propertyStore, PROPERTYKEY key, bool value)
{
  PROPVARIANT propVar;

  InitPropVariantFromBoolean(value, &propVar);
  HRESULT hr = propertyStore->SetValue(key, propVar);
  hr = S_OK;
}


static void SetUint32Property(IPropertyStore *propertyStore, PROPERTYKEY key, UINT32 value)
{
  PROPVARIANT propVar;

  InitPropVariantFromUInt32(value, &propVar);
  HRESULT hr = propertyStore->SetValue(key, propVar);
  hr = S_OK;
}


//-------------------------------------------------------------------
//  SetEncodingProperties
//  Create a media source from a URL.
//
//  guidMT:  Major type of the stream, audio or video
//  pProps:  A pointer to the property store in which 
//           to set the required encoding properties.
//-------------------------------------------------------------------

static HRESULT SetEncodingProperties(const GUID guidMT, IPropertyStore* pProps)
{
  if (!pProps)
  {
    return E_INVALIDARG;
  }

  if (EncodingMode == NONE)
  {
    return MF_E_NOT_INITIALIZED;
  }

  HRESULT hr = S_OK;

  PROPVARIANT var;

  switch (EncodingMode)
  {
  case CBR:
    // Set VBR to false.
    hr = InitPropVariantFromBoolean(FALSE, &var);
    if (FAILED(hr))
    {
      goto done;
    }

    hr = pProps->SetValue(MFPKEY_VBRENABLED, var);
    if (FAILED(hr))
    {
      goto done;
    }

    // Set the video buffer window.
    if (guidMT == MFMediaType_Video)
    {
      hr = InitPropVariantFromInt32(VIDEO_WINDOW_MSEC, &var);
      if (FAILED(hr))
      {
        goto done;
      }

      hr = pProps->SetValue(MFPKEY_VIDEOWINDOW, var);
      if (FAILED(hr))
      {
        goto done;
      }
    }
    break;

  case VBR:
    //Set VBR to true.
    hr = InitPropVariantFromBoolean(TRUE, &var);
    if (FAILED(hr))
    {
      goto done;
    }

    hr = pProps->SetValue(MFPKEY_VBRENABLED, var);
    if (FAILED(hr))
    {
      goto done;
    }

    // Number of encoding passes is 1.

    hr = InitPropVariantFromInt32(1, &var);
    if (FAILED(hr))
    {
      goto done;
    }

    hr = pProps->SetValue(MFPKEY_PASSESUSED, var);
    if (FAILED(hr))
    {
      goto done;
    }

    // Set the quality level.

    if (guidMT == MFMediaType_Audio)
    {
      hr = InitPropVariantFromUInt32(50, &var);
      if (FAILED(hr))
      {
        goto done;
      }

      hr = pProps->SetValue(MFPKEY_DESIRED_VBRQUALITY, var);
      if (FAILED(hr))
      {
        goto done;
      }
    }
    else if (guidMT == MFMediaType_Video)
    {
      hr = InitPropVariantFromUInt32(95, &var);
      if (FAILED(hr))
      {
        goto done;
      }

      hr = pProps->SetValue(MFPKEY_VBRQUALITY, var);
      if (FAILED(hr))
      {
        goto done;
      }
    }
    break;

  default:
    hr = E_UNEXPECTED;
    break;
  }

done:
  PropVariantClear(&var);
  return hr;
}




//-------------------------------------------------------------------
//  GetOutputTypeFromWMAEncoder
//  Gets a compatible output type from the Windows Media audio encoder.
//
//  ppAudioType: Receives a pointer to the media type.
//-------------------------------------------------------------------

static HRESULT GetOutputTypeFromWMAEncoder(IMFMediaType** ppAudioType)
{
  if (!ppAudioType)
  {
    return E_POINTER;
  }

  //We need to find a suitable output media type
  //We need to create the encoder to get the available output types
  //and discard the instance.

  IMFActivate **transformActivationObjs;
  UINT32 transformCount;

  MFT_REGISTER_TYPE_INFO regTypeInfo;

  regTypeInfo.guidMajorType = MFMediaType_Audio;
  regTypeInfo.guidSubtype = MFAudioFormat_WMAudioV9;

  // Look for an encoder.

  HRESULT hr = MFTEnumEx(MFT_CATEGORY_AUDIO_ENCODER, MFT_ENUM_FLAG_TRANSCODE_ONLY, nullptr, &regTypeInfo, &transformActivationObjs, &transformCount);

  if ((hr != S_OK) || (transformCount < 1))
    return hr;

  // Regardless how many activation objects returned, just instantiate the first one
  // (would I want to instantiate another?  Why?  Which one?)

  IMFActivate *activationObj = *transformActivationObjs;
  IMFTransform *mfEncoder;
  wchar_t transformName[128];
  UINT32 nameLen;

  activationObj->GetString(MFT_FRIENDLY_NAME_Attribute, transformName, sizeof(transformName), &nameLen);

  hr = activationObj->ActivateObject(IID_PPV_ARGS(&mfEncoder));

  IPropertyStore *propertyStore;

  hr = mfEncoder->QueryInterface(IID_PPV_ARGS(&propertyStore));

  SetBooleanProperty(propertyStore, MFPKEY_VBRENABLED, true);
  SetBooleanProperty(propertyStore, MFPKEY_CONSTRAIN_ENUMERATED_VBRQUALITY, true);
  SetUint32Property(propertyStore, MFPKEY_DESIRED_VBRQUALITY, 50);

  // this fails with a not_implemented error  hr = propertyStore->Commit();

  // enumerate output types and try to find the appropriate one for our purposes

  UINT32 channelCount;
  UINT32 samplesPerSecond;
  UINT32 bitsPerSample;

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

    if ((channelCount == 2) && (samplesPerSecond == 44100) && (bitsPerSample == 16))
    {
      *ppAudioType = mediaType;
      //      (*ppAudioType)->AddRef();
      break;
    }
    else
    {
      mediaType->Release();
    }

  }

  propertyStore->Release();

  // release all the stupid activation pointers (because COM was such a GREAT idea)

  for (UINT32 i = 0; i < transformCount; i++)
  {
    IMFActivate *temp = *(transformActivationObjs + i);
    temp->Release();
  }

  // free the stupid activation array object (because COM was such an f'ing great idea)
  // (did I ever mention I think COM was just... stupid?)

  CoTaskMemFree(transformActivationObjs);

  //done:
  //  SafeRelease(&pProps);
  //  SafeRelease(&pType);
  //  SafeRelease(&pMFT);
  //CoTaskMemFree(pMFTCLSIDs);
  return hr;
}




//-------------------------------------------------------------------
//  CreateAudioStream
//  Create an audio stream and add it to the profile.
//
//  pProfile: A pointer to the ASF profile.
//  wStreamNumber: Stream number to assign for the new stream.
//-------------------------------------------------------------------

static HRESULT CreateAudioStream(IMFASFProfile* pProfile, WORD wStreamNumber)
{
  if (!pProfile)
  {
    return E_INVALIDARG;
  }
  if (wStreamNumber < 1 || wStreamNumber > 127)
  {
    return MF_E_INVALIDSTREAMNUMBER;
  }

  IMFMediaType* pAudioType = nullptr;
  IMFASFStreamConfig* pAudioStream = nullptr;

  //Create an output type from the encoder
  HRESULT hr = GetOutputTypeFromWMAEncoder(&pAudioType);
  if (FAILED(hr))
  {
    goto done;
  }

  //Create a new stream with the audio type
  hr = pProfile->CreateStream(pAudioType, &pAudioStream);
  if (FAILED(hr))
  {
    goto done;
  }

  //Set stream number
  hr = pAudioStream->SetStreamNumber(wStreamNumber);
  if (FAILED(hr))
  {
    goto done;
  }

  //Add the stream to the profile
  hr = pProfile->SetStream(pAudioStream);
  if (FAILED(hr))
  {
    goto done;
  }

  wprintf_s(L"Audio Stream created. Stream Number: %d.\n", wStreamNumber);

done:
  pAudioStream->Release();
  pAudioType->Release();
  return hr;
}



MediaSink *MediaSink::Create(const wchar_t *filename, MediaSource* source)
{
  HRESULT hr;
  IMFASFProfile *mfAsfProfile = nullptr;
  IMFPresentationDescriptor *mfPresentationDescriptor = nullptr;
  IMFStreamDescriptor *mfStreamDescriptor;
  IMFMediaTypeHandler *mfMediaTypeHandler;
  IMFMediaType *mfMediaType;
  DWORD streamDescriptorCount;
  BOOL selected;
  GUID majorType;
  IMFActivate *mfActivate;


  hr = MFCreateASFProfile(&mfAsfProfile);
  mfPresentationDescriptor = source->GetPresentationDescriptor();
  hr = mfPresentationDescriptor->GetStreamDescriptorCount(&streamDescriptorCount);
  hr = mfPresentationDescriptor->GetStreamDescriptorByIndex(0, &selected, &mfStreamDescriptor);

  hr = mfStreamDescriptor->GetMediaTypeHandler(&mfMediaTypeHandler);
  hr = mfMediaTypeHandler->GetMediaTypeByIndex(0, &mfMediaType);

  hr = mfMediaType->GetMajorType(&majorType);

  hr = CreateAudioStream(mfAsfProfile, 1);

  IMFASFContentInfo *mfAsfContentInfo;
  IPropertyStore *contentInfoProperties;

  hr = MFCreateASFContentInfo(&mfAsfContentInfo);

  //Get stream's encoding property
  hr = mfAsfContentInfo->GetEncodingConfigurationPropertyStore(1, &contentInfoProperties);


  //Set the stream-level encoding properties
  hr = SetEncodingProperties(majorType, contentInfoProperties);

  hr = mfAsfContentInfo->GetEncodingConfigurationPropertyStore(0, &contentInfoProperties);

  PROPVARIANT var;
  PropVariantInit(&var);

  var.vt = VT_BOOL;
  var.boolVal = VARIANT_TRUE;

  hr = contentInfoProperties->SetValue(MFPKEY_ASFMEDIASINK_AUTOADJUST_BITRATE, var);

  //Initialize with the profile
  hr = mfAsfContentInfo->SetProfile(mfAsfProfile);

  //Create the activation object for the  file sink
  hr = MFCreateASFMediaSinkActivate(filename, mfAsfContentInfo, &mfActivate);

  return new MediaSink(mfActivate);

}

