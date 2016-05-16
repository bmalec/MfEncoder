#include "stdafx.h"
#include <wmcodecdsp.h>
#include <exception>
#include "MediaSink.h"



MediaSink::MediaSink(IMFMediaSink *mfMediaSink, IMFASFContentInfo* asfContentInfo)
{
  _mfMediaSink = mfMediaSink;
  _mfAsfContentInfo = asfContentInfo;
}


MediaSink::~MediaSink()
{
  _mfMediaSink->Release();
}


IMFMediaSink* MediaSink::GetMFMediaSink()
{
  return _mfMediaSink;
}


IMFStreamSink* MediaSink::GetMFStreamSinkByIndex(DWORD index)
{
  IMFStreamSink* mfStreamSink;

  HRESULT hr;

  if (!SUCCEEDED(hr = _mfMediaSink->GetStreamSinkByIndex(index, &mfStreamSink)))
    throw std::exception("Unable to retrieve stream sink");

  return mfStreamSink;
}

MediaSink* MediaSink::Create(const wchar_t *url, IMFASFContentInfo* afsContentInfo)
{
  IMFActivate* mfActivate = nullptr;
  IMFMediaSink* mfMediaSink = nullptr;

  //Create the activation object for the  file sink
  HRESULT hr = MFCreateASFMediaSinkActivate(url, afsContentInfo, &mfActivate);

  // Immediately activate the media sink as there's no real reason not to

  if (!SUCCEEDED(hr = mfActivate->ActivateObject(__uuidof(IMFMediaSink), (void**)&mfMediaSink)))
    throw std::exception("Could not activate MediaSink");

  mfActivate->Release();
// think I should not be releasing this  mfAsfContentInfo->Release();

  return new MediaSink(mfMediaSink, afsContentInfo);

}

/*

IPropertyStore* MediaSink::GetEncoderConfigurationPropertyStore(WORD streamNumber)
{
  return _mediaSinkContentInfo->GetEncoderConfigurationPropertyStore(streamNumber);

}

*/


IMFTopologyNode* MediaSink::CreateTopologyOutputNode(WORD streamNumber)
{
  IMFTopologyNode* mfTopologyNode = nullptr;
  IMFStreamSink* mfStreamSink = nullptr;
  HRESULT hr;

  do
  {
    // Get the stream sink based on the supplied streamNumber

    if (!SUCCEEDED(hr = _mfMediaSink->GetStreamSinkById(streamNumber, &mfStreamSink)))
      break;

    // Create the node.
    if (!SUCCEEDED(hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &mfTopologyNode)))
      break;

    // Set the object pointer.
    if (!SUCCEEDED(hr = mfTopologyNode->SetObject(mfStreamSink)))
      break;

    /* Doesn't seem like the following is needed?  I set it to arandom stream number, didn't appear to change anything

    // Set the stream sink ID attribute.
    if (!SUCCEEDED(hr = mfTopologyNode->SetUINT32(MF_TOPONODE_STREAMID, streamNumber)))
      break;
    */

    if (!SUCCEEDED(hr = mfTopologyNode->SetUINT32(MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE)))
      break;
  } while (0);

  if (FAILED(hr))
    throw std::exception("Unable to create topology output node");
    
  return mfTopologyNode;

}


IMFMediaType* MediaSink::GetMediaTypeForStream(WORD streamNumber)
{
  IMFASFStreamConfig *asfStreamConfig = nullptr;
  IMFASFProfile* mfAsfProfile;
  IMFMediaType* mfMediaType = nullptr;
  HRESULT hr;

  do
  {
    if (!SUCCEEDED(hr = _mfAsfContentInfo->GetProfile(&mfAsfProfile)))
      break;

    if (!SUCCEEDED(hr = mfAsfProfile->GetStreamByNumber(streamNumber, &asfStreamConfig)))
      break;

    if (!SUCCEEDED(hr = asfStreamConfig->GetMediaType(&mfMediaType)))
      break;

  } while (0);

  return mfMediaType;
}







IMFTopologyNode* MediaSink::CreateTopologyTransformNode(WORD streamNumber)
{
  IMFTopologyNode* topologyTransformNode = nullptr;
//  IMFASFStreamConfig *asfStreamConfig = nullptr;
  IMFMediaType* streamMediaType = nullptr;
  IPropertyStore* encodingConfigurationProperties = nullptr;
  IMFActivate* encoderActivationObj = nullptr;
  HRESULT hr;
    
  do
  {
    // Need to get the mediatype for the output stream in order to instatiated the 
    // encoder.  My first inclination was to get the stream sink like we did in 
    // CreateTopologyOutputNode, and that could work, but... we also need to
    // get the encoder configuration parameters, which are tacked on to the 
    // IMFMediaSinkContentInfo object.  Uggh.  The Microsoft example does a 
    // little bit of both in order to maximize confusion...

    streamMediaType = GetMediaTypeForStream(streamNumber);

//    IMFASFContentInfo* mfAsfContentInfo = _mediaSinkContentInfo->ConstructMfAsfContentInfo();

    hr = _mfAsfContentInfo->GetEncodingConfigurationPropertyStore(streamNumber, &encodingConfigurationProperties);

    hr = MFCreateWMAEncoderActivate(streamMediaType, encodingConfigurationProperties, &encoderActivationObj);

    hr = MFCreateTopologyNode(MF_TOPOLOGY_TRANSFORM_NODE, &topologyTransformNode);

    // Set the object pointer.
    hr = topologyTransformNode->SetObject(encoderActivationObj);


  } while (0);

  return topologyTransformNode;

}



IMFTransform* MediaSink::GetAudioEncoderForStream(WORD streamNumber)
{
//  IMFASFStreamConfig *asfStreamConfig = nullptr;
//  IMFStreamSink* mfStreamSink = nullptr;
  IMFMediaType* streamMediaType = nullptr;
  IPropertyStore* encodingConfigurationProperties = nullptr;
  IMFActivate* encoderActivationObj = nullptr;
  IMFTransform* mfTransform;
  HRESULT hr;

  do
  {
    // Need to get the mediatype for the output stream in order to instatiated the 
    // encoder.  My first inclination was to get the stream sink like we did in 
    // CreateTopologyOutputNode, and that could work, but... we also need to
    // get the encoder configuration parameters, which are tacked on to the 
    // IMFMediaSinkContentInfo object.  Uggh.  The Microsoft example does a 
    // little bit of both in order to maximize confusion...

    streamMediaType = GetMediaTypeForStream(streamNumber);

    //    IMFASFContentInfo* mfAsfContentInfo = _mediaSinkContentInfo->ConstructMfAsfContentInfo();

    hr = _mfAsfContentInfo->GetEncodingConfigurationPropertyStore(streamNumber, &encodingConfigurationProperties);

    hr = MFCreateWMAEncoderActivate(streamMediaType, encodingConfigurationProperties, &encoderActivationObj);
    hr = encoderActivationObj->ActivateObject(IID_PPV_ARGS(&mfTransform));




  } while (0);

  return mfTransform;
}


static void CopyProperty(IPropertyStore* src, IPropertyStore* dest, const PROPERTYKEY& key)
{
  PROPVARIANT pv;
  HRESULT hr;

  do
  {
    if (!SUCCEEDED(hr = src->GetValue(key, &pv)))
      break;

    if (!SUCCEEDED(hr = dest->SetValue(key, pv)))
      break;

    if (!SUCCEEDED(hr = PropVariantClear(&pv)))
      break;
  } while (0);

  if (FAILED(hr))
    throw std::exception("Unable to copy property");
}



void MediaSink::UpdatePostEncodeStreamSinkProperties(WORD streamNumber, IPropertyStore* encoderProperties)
{
  IMFStreamSink* mfStreamSink = nullptr;
  IPropertyStore* streamSinkProperties;
  HRESULT hr;

  do
  {
    if (!SUCCEEDED(hr = _mfMediaSink->GetStreamSinkById(streamNumber, &mfStreamSink)))
      break;

    if (!SUCCEEDED(hr = mfStreamSink->QueryInterface(IID_PPV_ARGS(&streamSinkProperties))))
      break;

    CopyProperty(encoderProperties, streamSinkProperties, MFPKEY_STAT_BAVG);
    CopyProperty(encoderProperties, streamSinkProperties, MFPKEY_STAT_RAVG);
    CopyProperty(encoderProperties, streamSinkProperties, MFPKEY_STAT_BMAX);
    CopyProperty(encoderProperties, streamSinkProperties, MFPKEY_STAT_RMAX);
    CopyProperty(encoderProperties, streamSinkProperties, MFPKEY_WMAENC_AVGBYTESPERSEC);
  } while (0);

  if (FAILED(hr))
    throw std::exception("Unabled to updated post-encoding values on the stream sink");
}

  