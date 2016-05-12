#include "stdafx.h"
#include <wmcontainer.h>
#include <mfapi.h>
#include <Mferror.h>
#include <wmcodecdsp.h>
#include <propvarutil.h>
#include "Parameters.h"
#include "Util.h"
#include "MediaSink.h"




/*
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
*/


//-------------------------------------------------------------------
//  GetOutputTypeFromWMAEncoder
//  Gets a compatible output type from the Windows Media audio encoder.
//
//  ppAudioType: Receives a pointer to the media type.
//-------------------------------------------------------------------

static IMFMediaType* GetOutputTypeFromWMAEncoder(Parameters* params)
{
  IMFMediaType* result;

  //We need to find a suitable output media type
  //We need to create the encoder to get the available output types
  //and discard the instance.

  IMFActivate **transformActivationObjs;
  UINT32 transformCount;

  MFT_REGISTER_TYPE_INFO regTypeInfo;

  regTypeInfo.guidMajorType = MFMediaType_Audio;
  regTypeInfo.guidSubtype = (params->Quality < 100) ? MFAudioFormat_WMAudioV8 : MFAudioFormat_WMAudio_Lossless;

  // Look for an encoder.

  HRESULT hr = MFTEnumEx(MFT_CATEGORY_AUDIO_ENCODER, MFT_ENUM_FLAG_TRANSCODE_ONLY, nullptr, &regTypeInfo, &transformActivationObjs, &transformCount);

  if ((hr != S_OK) || (transformCount < 1))
    return nullptr;

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

  SetBooleanPropertyStoreValue(propertyStore, MFPKEY_VBRENABLED, TRUE);
  SetBooleanPropertyStoreValue(propertyStore, MFPKEY_CONSTRAIN_ENUMERATED_VBRQUALITY, TRUE);
  SetUint32PropertyStoreValue(propertyStore, MFPKEY_DESIRED_VBRQUALITY, params->Quality);

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
      result = mediaType;
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

  return result;
}


static void SetContentInfoMetadata(IMFASFContentInfo* mfAsfContentInfo, MediaSource* mediaSource, Parameters* parameters)
{
  // please work

  IMFMetadataProvider* mfMetadataProvider;
  IMFMetadata* mfMetadata;
  PROPVARIANT prop;


  HRESULT hr = mfAsfContentInfo->QueryInterface(IID_IMFMetadataProvider, (void**)&mfMetadataProvider);
  hr = mfMetadataProvider->GetMFMetadata(NULL, 0, 0, &mfMetadata);

  wchar_t *value;

  value = parameters->Album;

  if (!value)
  {
    value = mediaSource->GetMetadataValue(L"WM/AlbumTitle");
  }

  if (value)
  {
    InitPropVariantFromString(value, &prop);
    mfMetadata->SetProperty(L"WM/AlbumTitle", &prop);
  }

  value = parameters->Artist;

  if (!value)
  {
    value = mediaSource->GetMetadataValue(L"Author");
  }

  if (value)
  {
    InitPropVariantFromString(value, &prop);
    mfMetadata->SetProperty(L"Author", &prop);
  }

  value = parameters->Artist;

  if (!value)
  {
    value = mediaSource->GetMetadataValue(L"WM/AlbumArtist");
  }

  if (value)
  {
    InitPropVariantFromString(value, &prop);
    mfMetadata->SetProperty(L"WM/AlbumArtist", &prop);
  }

  value = parameters->Genre;

  if (!value)
  {
    value = mediaSource->GetMetadataValue(L"WM/Genre");
  }

  if (value)
  {
    InitPropVariantFromString(value, &prop);
    mfMetadata->SetProperty(L"WM/Genre", &prop);
  }

  value = parameters->Title;

  if (!value)
  {
    value = mediaSource->GetMetadataValue(L"Title");
  }

  if (value)
  {
    InitPropVariantFromString(value, &prop);
    mfMetadata->SetProperty(L"Title", &prop);
  }

  value = parameters->TrackNumber;

  if (!value)
  {
    value = mediaSource->GetMetadataValue(L"WM/TrackNumber");
  }

  if (value)
  {
    InitPropVariantFromString(value, &prop);
    mfMetadata->SetProperty(L"WM/TrackNumber", &prop);
  }

  value = parameters->Year;

  if (!value)
  {
    value = mediaSource->GetMetadataValue(L"WM/Year");
  }

  if (value)
  {
    InitPropVariantFromString(value, &prop);
    mfMetadata->SetProperty(L"WM/Year", &prop);
  }

  if (mfMetadata) mfMetadata->Release();
  if (mfMetadataProvider) mfMetadataProvider->Release();
}




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





//-------------------------------------------------------------------
//  SetStreamEncodingProperties
//  Create a media source from a URL.
//
//  guidMT:  Major type of the stream, audio or video
//  pProps:  A pointer to the property store in which 
//           to set the required encoding properties.
//-------------------------------------------------------------------

static void SetStreamEncodingProperties(Parameters* params, IPropertyStore* propertyStore)
{
  if (!params) throw std::exception("Null parameter: params");
  if (!propertyStore) throw std::exception("Null parameter: propertyStore");

  if (params->Quality > 0)
  {
    // Quality has been specified, so use quality-based VBR
    SetBooleanPropertyStoreValue(propertyStore, MFPKEY_VBRENABLED, TRUE);

    // Number of encoding passes is 1.
    SetInt32PropertyStoreValue(propertyStore, MFPKEY_PASSESUSED, 1);

    // Set the quality level.
    SetUint32PropertyStoreValue(propertyStore, MFPKEY_DESIRED_VBRQUALITY, params->Quality);
  }
  else
  {
    // quality not specified, use constant bitrate
    SetBooleanPropertyStoreValue(propertyStore, MFPKEY_VBRENABLED, FALSE);
  }
}







//-------------------------------------------------------------------
//  CreateAudioStream
//  Create an audio stream and add it to the profile.
//
//  pProfile: A pointer to the ASF profile.
//  wStreamNumber: Stream number to assign for the new stream.
//-------------------------------------------------------------------

static void CreateAudioStream(IMFASFProfile* mfAsfProfile, Parameters* params, WORD wStreamNumber)
{
  if (!mfAsfProfile) throw std::exception("Null parameter: pProfile");
  if (wStreamNumber < 1 || wStreamNumber > 127) throw std::exception("Invalid stream number");

  IMFMediaType* mfMediaType = nullptr;
  IMFASFStreamConfig* mfAsfStreamConfig = nullptr;

  //Create an output type from the encoder
  mfMediaType = GetOutputTypeFromWMAEncoder(params);

  HRESULT hr;

  do
  {
    //Create a new stream with the audio type
    if (!SUCCEEDED(hr = mfAsfProfile->CreateStream(mfMediaType, &mfAsfStreamConfig)))
      break;

    //Set stream number
    if (!SUCCEEDED(hr = mfAsfStreamConfig->SetStreamNumber(wStreamNumber)))
      break;

    //Add the stream to the profile
    hr = mfAsfProfile->SetStream(mfAsfStreamConfig);
  } while (0);

  if (mfAsfStreamConfig) mfAsfStreamConfig->Release();
  if (mfMediaType) mfMediaType->Release();

  if (FAILED(hr))
    throw std::exception("Error creating audio stream");
}



MediaSink* MediaSink::Create(const wchar_t *filename, MediaSource* source, Parameters* params)
{
  HRESULT hr;
  IMFASFProfile *mfAsfProfile = nullptr;
  IMFActivate *mfActivate;


  hr = MFCreateASFProfile(&mfAsfProfile);

  CreateAudioStream(mfAsfProfile, params, 1);

  IMFASFContentInfo *mfAsfContentInfo;
  IPropertyStore *contentInfoProperties;

  hr = MFCreateASFContentInfo(&mfAsfContentInfo);

  //Get stream's encoding property
  hr = mfAsfContentInfo->GetEncodingConfigurationPropertyStore(1, &contentInfoProperties);

  //Set the stream-level encoding properties
  SetStreamEncodingProperties(params, contentInfoProperties);

  // TODO: we should release contentInfoProperties here, right????

  hr = mfAsfContentInfo->GetEncodingConfigurationPropertyStore(0, &contentInfoProperties);

  PROPVARIANT var;
  PropVariantInit(&var);

  var.vt = VT_BOOL;
  var.boolVal = VARIANT_TRUE;

  hr = contentInfoProperties->SetValue(MFPKEY_ASFMEDIASINK_AUTOADJUST_BITRATE, var);

  //Initialize with the profile
  hr = mfAsfContentInfo->SetProfile(mfAsfProfile);

  // Set MediaSink metadata

  IMFMetadataProvider* mfMetadataProvider;
  IMFMetadata* mfMetadata;

  hr = mfAsfContentInfo->QueryInterface(IID_IMFMetadataProvider, (void**)&mfMetadataProvider);
  hr = mfMetadataProvider->GetMFMetadata(NULL, 0, 0, &mfMetadata);

  SetContentInfoMetadata(mfAsfContentInfo, source, params);
  
  //Create the activation object for the  file sink
  hr = MFCreateASFMediaSinkActivate(filename, mfAsfContentInfo, &mfActivate);

  if (mfAsfContentInfo) mfAsfContentInfo->Release();

  return new MediaSink(mfActivate);

}

