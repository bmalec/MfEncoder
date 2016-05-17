#include "stdafx.h"
#include <mfapi.h>
#include <wmcodecdsp.h>
#include <Mferror.h>
#include <stdexcept>
#include "Util.h"
#include "Topology.h"
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


AudioEncoderParameters* AudioEncoderParameters::CreateLosslessEncoderParameters(int channelCount, int samplesPerSecond, int bitsPerSample)
{
  return new AudioEncoderParameters(CompressionFamily::Lossless, 100, channelCount, samplesPerSecond, bitsPerSample);
}


BOOL AudioEncoderParameters::IsLossless()
{
  return _compressionFamily == CompressionFamily::Lossless;
}


BOOL AudioEncoderParameters::IsQualityBasedVbr()
{
  return (_compressionFamily == CompressionFamily::QualityBasedVariableBitRate) || (_compressionFamily == CompressionFamily::Lossless); 
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



/*
AudioEncoder::AudioEncoder(IMFActivate* mfActivate)
{

  _mfActivate = mfActivate;

  if (!SUCCEEDED(_mfActivate->ActivateObject(IID_PPV_ARGS(&_mfTransform))))
    throw std::exception("Unable to activate audio encoder");

  if (!SUCCEEDED(_mfTransform->QueryInterface(IID_PPV_ARGS(&_propertyStore))))
    throw std::exception("Unable to access audio encoder property store");






}
*/
/*
AudioEncoder::~AudioEncoder()
{
  _propertyStore->Release();
  _mfTransform->Release();
  _mfActivate->ShutdownObject();
  _mfActivate->Release();
}
*/

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

static IMFTopology* BuildPartialTopography(MediaSource* source, IMFTransform* __audioEncoder, MediaSink* sink, WORD streamNumber)
{
  IMFTopology* mfTopology = nullptr;
//  IMFTransform* audioEncoder = nullptr;
  IMFTopologyNode* encoderNode = nullptr;
  IMFTopologyNode* sourceNode = nullptr;
  IMFTopologyNode* outputNode = nullptr;
  //Create the topology that represents the encoding pipeline
  HRESULT hr = MFCreateTopology(&mfTopology);

//  Topology* topology = new Topology(mfTopology, mediaSink);
//  topology->_buildPartialTopograpy(source, mediaSink, streamNumber);


  do {

    // Add source node

    sourceNode = source->CreateTopologySourceNode();

    if (!SUCCEEDED(hr = mfTopology->AddNode(sourceNode)))
      break;

    // Add encoder node

    // this is a little more convoluted than hoped for, but bear with me.
    // at first glance it seems odd that we'd create the compressor transform
    // through the MediaSink object.  But, it turns
    // out the the media sink holds the two main ingredients needed to
    // create the encoder: the output stream MediaType, and, crucially,
    // the EncodingConfigurationPropertyStore that's a requirement if
    // you want to do quality-based VBR.
    //
    // Originally I had a method in MediaSink that functioned similiar to the
    // MediaSource::CreateTopologySourceNode() and MediaSink::CreateTopologyOutputNode().
    // This worked ok, but another problem with quality-based VBR is (apparently)
    // you're supposed to set some properties on the output stream,
    // based on values computed by the encoder during compressions.
    // The original example code jumped thorugh hoops to get back a reference
    // to the encoder by traversing the full topology, but I felt
    // it was much more understandable to first save a reference to
    // the actual encoder object, and use that for the post-processing
    // property patch.
    //
    // Say that ten times quick!



//    audioEncoder = sink->GetAudioEncoderForStream(1);

    hr = MFCreateTopologyNode(MF_TOPOLOGY_TRANSFORM_NODE, &encoderNode);

    // Set the object pointer.
    hr = encoderNode->SetObject(__audioEncoder);

    if (!SUCCEEDED(hr = mfTopology->AddNode(encoderNode)))
      break;




    // temp code end

    /*    encoderNode = sink->CreateTopologyTransformNode(1);

    if (!SUCCEEDED(hr = _mfTopology->AddNode(encoderNode)))
    break;
    */
    // connect the source node to the encoder

    if (!SUCCEEDED(hr = sourceNode->ConnectOutput(0, encoderNode, 0)))
      break;

    // Add sink node

    outputNode = sink->CreateTopologyOutputNode(streamNumber);
    if (!SUCCEEDED(hr = mfTopology->AddNode(outputNode)))
      break;

    // connect the transform node to the output node

    if (!SUCCEEDED(hr = encoderNode->ConnectOutput(0, outputNode, 0)))
      break;
  } while (0);

//  if (sourceNode) sourceNode->Release();
//  if (outputNode) outputNode->Release();
//  if (encoderNode) encoderNode->Release();


  if (FAILED(hr))
    throw std::exception("Unable to construct topology");

  return mfTopology;
}


static HRESULT UpdateVbrStreamProperties(IMFTransform* audioEncoder, MediaSink* mediaSink)
{
  IPropertyStore* encoderPropertyStore;
  HRESULT hr;

  hr = audioEncoder->QueryInterface(IID_PPV_ARGS(&encoderPropertyStore));

  mediaSink->UpdatePostEncodeStreamSinkProperties(1, encoderPropertyStore);

  return hr;
}



static HRESULT RunEncode(IMFTopology* mfTopology, MediaSink* mediaSink, IMFTransform* audioEncoder, AudioEncoderParameters* encoderParameters)
{

  IMFMediaSession *mfMediaSession = nullptr;
  IMFMediaEvent* pEvent = nullptr;


  MediaEventType meType = MEUnknown;  // Event type

  HRESULT hr = S_OK;
  HRESULT hrStatus = S_OK;            // Event status

  MF_TOPOSTATUS TopoStatus = MF_TOPOSTATUS_INVALID; // Used with MESessionTopologyStatus event.    


  hr = MFCreateMediaSession(nullptr, &mfMediaSession);
  if (FAILED(hr))
  {
    goto done;
  }

  hr = mfMediaSession->SetTopology(MFSESSION_SETTOPOLOGY_IMMEDIATE, mfTopology);
  if (FAILED(hr))
  {
    goto done;
  }

  //Get media session events synchronously
  while (1)
  {
    hr = mfMediaSession->GetEvent(0, &pEvent);
    if (FAILED(hr))
    {
      goto done;
    }

    hr = pEvent->GetType(&meType);
    if (FAILED(hr))
    {
      goto done;
    }

    hr = pEvent->GetStatus(&hrStatus);
    if (FAILED(hr))
    {
      goto done;
    }
    if (FAILED(hrStatus))
    {
      hr = hrStatus;
      goto done;
    }

    switch (meType)
    {
    case MESessionTopologyStatus:
    {
      // Get the status code.
      MF_TOPOSTATUS status = (MF_TOPOSTATUS)MFGetAttributeUINT32(
        pEvent, MF_EVENT_TOPOLOGY_STATUS, MF_TOPOSTATUS_INVALID);

      if (status == MF_TOPOSTATUS_READY)
      {
        PROPVARIANT var;
        PropVariantInit(&var);
        wprintf_s(L"Topology resolved and set on the media session.\n");

        hr = mfMediaSession->Start(nullptr, &var);
        if (FAILED(hr))
        {
          goto done;
        }

      }
      if (status == MF_TOPOSTATUS_STARTED_SOURCE)
      {
        wprintf_s(L"Encoding started.\n");
        break;
      }
      if (status == MF_TOPOSTATUS_ENDED)
      {
        wprintf_s(L"Encoding complete.\n");
        hr = mfMediaSession->Close();
        if (FAILED(hr))
        {
          goto done;
        }

        break;
      }
    }
    break;


    case MESessionEnded:
      wprintf_s(L"Encoding complete.\n");
      hr = mfMediaSession->Close();
      if (FAILED(hr))
      {
        goto done;
      }
      break;

    case MEEndOfPresentation:
    {
      if (encoderParameters->IsQualityBasedVbr())
      {
        hr = UpdateVbrStreamProperties(audioEncoder, mediaSink);
        if (FAILED(hr))
        {
          goto done;
        }
        wprintf_s(L"Updated sinks for VBR. \n");
      }
    }
    break;

    case MESessionClosed:
      wprintf_s(L"Encoding session closed.\n");

      hr = mfMediaSession->Shutdown();
      goto done;
    }
    if (FAILED(hr))
    {
      goto done;
    }

    pEvent->Release();

  }
done:
  pEvent->Release();
  if (mfMediaSession) mfMediaSession->Release();
  return hr;
}



void AudioEncoder::Encode(MediaSource* mediaSource, MediaSink* mediaSink, AudioEncoderParameters* encoderParameters)
{
//  Topology* topology = Topology::CreatePartialTopograpy(mediaSource, mediaSink, 1);



//   topology->Encode(encoderParameters);

  IMFTransform* audioEncoder = mediaSink->GetAudioEncoderForStream(1);

  IMFTopology* topology = BuildPartialTopography(mediaSource, audioEncoder, mediaSink, 1);
 
  RunEncode(topology, mediaSink, audioEncoder, encoderParameters);

}