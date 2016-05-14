#include "stdafx.h"
#include <windows.h>
#include <propvarutil.h>
#include <stdexcept>
#include "MediaSinkContentInfo.h"


MediaSinkContentInfo::MediaSinkContentInfo()
{
  HRESULT hr;

  _mfAsfContentInfo = nullptr;
  _mfAsfProfile = nullptr;
  _mfMetadataProvider = nullptr;
  _mfMetadata = nullptr;

  do
  {
    if (!SUCCEEDED(hr = MFCreateASFContentInfo(&_mfAsfContentInfo)))
      break;

    if (!SUCCEEDED(hr = MFCreateASFProfile(&_mfAsfProfile)))
      break;

    if (!SUCCEEDED(hr = _mfAsfContentInfo->QueryInterface(IID_IMFMetadataProvider, (void**)&_mfMetadataProvider)))
      break;

    if (!SUCCEEDED(hr = _mfMetadataProvider->GetMFMetadata(NULL, 0, 0, &_mfMetadata)))
      break;
  } while (0);

  if (FAILED(hr))
  {
    if (_mfMetadata) _mfMetadata->Release();
    if (_mfMetadataProvider) _mfMetadataProvider->Release();
    if (_mfAsfProfile) _mfAsfProfile->Release();
    if (_mfAsfContentInfo) _mfAsfContentInfo->Release();

    throw std::exception("Unable to create MediaSinkContentInfo object");
  }
}






MediaSinkContentInfo::~MediaSinkContentInfo()
{
  _mfMetadata->Release();
  _mfMetadataProvider->Release();
  _mfAsfProfile->Release();
  _mfAsfContentInfo->Release();
}



void MediaSinkContentInfo::AddStreamSink(WORD streamNumber, AudioEncoderParameters* encoderParameters)
{
  HRESULT hr;

  // First off, get the appropriate Media Foundation MediaType from the encoder, based on the compressor parameters
  // we've specified

  IMFMediaType* mfMediaType = AudioEncoder::GetEncoderMediaType(encoderParameters);

  IMFASFStreamConfig* mfAsfStreamConfig = nullptr;

  do
  {
    if (!SUCCEEDED(hr = _mfAsfProfile->CreateStream(mfMediaType, &mfAsfStreamConfig)))
      break;

    if (!SUCCEEDED(hr = mfAsfStreamConfig->SetStreamNumber(streamNumber)))
      break;

    if (!SUCCEEDED(hr = _mfAsfProfile->SetStream(mfAsfStreamConfig)))
      break;

    // Here's the critical piece needed to make Quality-based VBR encoding work:
    // The audio encoder is instantiated indirectly based on the stream sink,
    // but the stream sink's MediaType alone is not enough to get the audio encoder
    // into a state where it will perform quality-based VBR compression.  That's
    // because some specific properties need to be set on the Media Foundation's
    // transform before we make the call to enumerate the available compression
    // options.  So, the AsfStreamConfig object has an additional property store
    // tacked on to it where we can set these property values that need to be set
    // on the encoder.  Somewhat oddly they're not attached to the AsfStreamConfig,
    // nor are they attached to the AsfProfile object that's used to add stream 
    // sinks.  Rather, this property store is tied to the AsfContentInfo object and
    // the association between the stream sinks and the encoder parameter property
    // store is maintained by the stream number

    if (encoderParameters->IsQualityBasedVbr())
    {
      IPropertyStore *streamPropertyStore;  // TODO when should this be released?

      if (!SUCCEEDED(hr = _mfAsfContentInfo->GetEncodingConfigurationPropertyStore(streamNumber, &streamPropertyStore)))
        break;

      AudioEncoder::SetEncoderPropertyStoreValuesForQualityBasedVbr(streamPropertyStore, encoderParameters->GetQualityLevel());
    }
  } while (0);

  if (mfAsfStreamConfig) mfAsfStreamConfig->Release();

  if (FAILED(hr))
    throw std::exception("Unable to add stream to the MediaSink");
}

void MediaSinkContentInfo::SetMetadataAsString(LPWSTR field, LPWSTR value)
{
  PROPVARIANT pv;

  InitPropVariantFromString(value, &pv);
  HRESULT hr = _mfMetadata->SetProperty(field, &pv);
//  PropVariantClear(&pv);
}



IMFASFContentInfo* MediaSinkContentInfo::GetMfAsfContentInfoObject()
{
  _mfAsfContentInfo->SetProfile(_mfAsfProfile);
  return _mfAsfContentInfo;
}