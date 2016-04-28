// MfEncoder.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
/* old
#include <Windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <wmcontainer.h>
#include <Mferror.h>
#include <propvarutil.h>
#include <wmcodecdsp.h>
#include <mftransform.h>
*/

#include <new>
#include <mfidl.h>            // Media Foundation interfaces
#include <mfapi.h>            // Media Foundation platform APIs
#include <mferror.h>        // Media Foundation error codes
#include <wmcontainer.h>    // ASF-specific components
#include <wmcodecdsp.h>        // Windows Media DSP interfaces
#include <Dmo.h>            // DMO objects
#include <uuids.h>            // Definition for FORMAT_VideoInfo
#include <propvarutil.h>

const INT32 VIDEO_WINDOW_MSEC = 3000;


// Encoding mode
typedef enum ENCODING_MODE {
  NONE = 0x00000000,
  CBR = 0x00000001,
  VBR = 0x00000002,
};

static ENCODING_MODE EncodingMode = VBR;


template <class T> void SafeRelease(T **ppT)
{
  if (*ppT)
  {
    (*ppT)->Release();
    *ppT = NULL;
  }
}



HRESULT CreateMediaSource(PCWSTR sURL, IMFMediaSource **ppSource)
{
  MF_OBJECT_TYPE ObjectType = MF_OBJECT_INVALID;

  IMFSourceResolver* pSourceResolver = NULL;
  IUnknown* pSource = NULL;

  // Create the source resolver.
  HRESULT hr = MFCreateSourceResolver(&pSourceResolver);
  if (FAILED(hr))
  {
    goto done;
  }

  // Use the source resolver to create the media source.

  // Note: For simplicity this sample uses the synchronous method to create 
  // the media source. However, creating a media source can take a noticeable
  // amount of time, especially for a network source. For a more responsive 
  // UI, use the asynchronous BeginCreateObjectFromURL method.

  hr = pSourceResolver->CreateObjectFromURL(
    sURL,                       // URL of the source.
    MF_RESOLUTION_MEDIASOURCE,  // Create a source object.
    NULL,                       // Optional property store.
    &ObjectType,        // Receives the created object type. 
    &pSource            // Receives a pointer to the media source.
    );
  if (FAILED(hr))
  {
    goto done;
  }

  // Get the IMFMediaSource interface from the media source.
  hr = pSource->QueryInterface(IID_PPV_ARGS(ppSource));

done:
  SafeRelease(&pSourceResolver);
  SafeRelease(&pSource);
  return hr;
}


//-------------------------------------------------------------------
//  SetEncodingProperties
//  Create a media source from a URL.
//
//  guidMT:  Major type of the stream, audio or video
//  pProps:  A pointer to the property store in which 
//           to set the required encoding properties.
//-------------------------------------------------------------------

HRESULT SetEncodingProperties(const GUID guidMT, IPropertyStore* pProps)
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
      hr = InitPropVariantFromUInt32(98, &var);
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

HRESULT GetOutputTypeFromWMAEncoder(IMFMediaType** ppAudioType)
{
  if (!ppAudioType)
  {
    return E_POINTER;
  }

  IMFTransform* pMFT = NULL;
  IMFMediaType* pType = NULL;
  IPropertyStore* pProps = NULL;

  //We need to find a suitable output media type
  //We need to create the encoder to get the available output types
  //and discard the instance.

  CLSID *pMFTCLSIDs = NULL;   // Pointer to an array of CLISDs. 
  UINT32 cCLSID = 0;            // Size of the array.

  MFT_REGISTER_TYPE_INFO tinfo;

  tinfo.guidMajorType = MFMediaType_Audio;
  tinfo.guidSubtype = MFAudioFormat_WMAudioV9;

  // Look for an encoder.
  HRESULT hr = MFTEnum(
    MFT_CATEGORY_AUDIO_ENCODER,
    0,                  // Reserved
    NULL,                // Input type to match. None.
    &tinfo,             // WMV encoded type.
    NULL,               // Attributes to match. (None.)
    &pMFTCLSIDs,        // Receives a pointer to an array of CLSIDs.
    &cCLSID             // Receives the size of the array.
    );
  if (FAILED(hr))
  {
    goto done;
  }

  // MFTEnum can return zero matches.
  if (cCLSID == 0)
  {
    hr = MF_E_TOPO_CODEC_NOT_FOUND;
    goto done;
  }
  else
  {
    // Create the MFT decoder
    hr = CoCreateInstance(pMFTCLSIDs[0], NULL,
      CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pMFT));

    if (FAILED(hr))
    {
      goto done;
    }

  }

  // Get the encoder's property store

  hr = pMFT->QueryInterface(IID_PPV_ARGS(&pProps));
  if (FAILED(hr))
  {
    goto done;
  }

  //Set encoding properties
  hr = SetEncodingProperties(MFMediaType_Audio, pProps);
  if (FAILED(hr))
  {
    goto done;
  }

  //Get the first output type
  //You can loop through the available output types to choose 
  //the one that meets your target requirements
  hr = pMFT->GetOutputAvailableType(0, 0, &pType);
  if (FAILED(hr))
  {
    goto done;
  }

  //Return to the caller
  *ppAudioType = pType;
  (*ppAudioType)->AddRef();

done:
  SafeRelease(&pProps);
  SafeRelease(&pType);
  SafeRelease(&pMFT);
  CoTaskMemFree(pMFTCLSIDs);
  return hr;
}


//-------------------------------------------------------------------
//  CreateAudioStream
//  Create an audio stream and add it to the profile.
//
//  pProfile: A pointer to the ASF profile.
//  wStreamNumber: Stream number to assign for the new stream.
//-------------------------------------------------------------------

HRESULT CreateAudioStream(IMFASFProfile* pProfile, WORD wStreamNumber)
{
  if (!pProfile)
  {
    return E_INVALIDARG;
  }
  if (wStreamNumber < 1 || wStreamNumber > 127)
  {
    return MF_E_INVALIDSTREAMNUMBER;
  }

  IMFMediaType* pAudioType = NULL;
  IMFASFStreamConfig* pAudioStream = NULL;

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
  SafeRelease(&pAudioStream);
  SafeRelease(&pAudioType);
  return hr;
}


HRESULT CreateMediaSink(PCWSTR filename, IMFMediaSource *pSource, IMFActivate **ppFileSinkActivate)
{
  HRESULT hr;
  IMFASFProfile *mfAsfProfile = NULL;
  IMFPresentationDescriptor *mfPresentationDescriptor = NULL;
  IMFStreamDescriptor *mfStreamDescriptor;
  IMFMediaTypeHandler *mfMediaTypeHandler;
  IMFMediaType *mfMediaType;
  DWORD streamDescriptorCount;
  BOOL selected;
  GUID majorType;


  hr = MFCreateASFProfile(&mfAsfProfile);
  hr = pSource->CreatePresentationDescriptor(&mfPresentationDescriptor);
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
  hr = MFCreateASFMediaSinkActivate(filename, mfAsfContentInfo, ppFileSinkActivate);

  return hr;
}


// Add a source node to a topology.
HRESULT AddSourceNode(
  IMFTopology *pTopology,           // Topology.
  IMFMediaSource *pSource,          // Media source.
  IMFPresentationDescriptor *pPD,   // Presentation descriptor.
  IMFStreamDescriptor *pSD,         // Stream descriptor.
  IMFTopologyNode **ppNode)         // Receives the node pointer.
{
  IMFTopologyNode *pNode = NULL;

  // Create the node.
  HRESULT hr = MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &pNode);
  if (FAILED(hr))
  {
    goto done;
  }

  // Set the attributes.
  hr = pNode->SetUnknown(MF_TOPONODE_SOURCE, pSource);
  if (FAILED(hr))
  {
    goto done;
  }

  hr = pNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, pPD);
  if (FAILED(hr))
  {
    goto done;
  }

  hr = pNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, pSD);
  if (FAILED(hr))
  {
    goto done;
  }

  // Add the node to the topology.
  hr = pTopology->AddNode(pNode);
  if (FAILED(hr))
  {
    goto done;
  }

  // Return the pointer to the caller.
  *ppNode = pNode;
  (*ppNode)->AddRef();

done:
  SafeRelease(&pNode);
  return hr;
}




// Add an output node to a topology.
HRESULT AddOutputNode(
  IMFTopology *pTopology,     // Topology.
  IMFActivate *pActivate,     // Media sink activation object.
  DWORD dwId,                 // Identifier of the stream sink.
  IMFTopologyNode **ppNode)   // Receives the node pointer.
{
  IMFTopologyNode *pNode = NULL;

  // Create the node.
  HRESULT hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &pNode);
  if (FAILED(hr))
  {
    goto done;
  }

  // Set the object pointer.
  hr = pNode->SetObject(pActivate);
  if (FAILED(hr))
  {
    goto done;
  }

  // Set the stream sink ID attribute.
  hr = pNode->SetUINT32(MF_TOPONODE_STREAMID, dwId);
  if (FAILED(hr))
  {
    goto done;
  }

  hr = pNode->SetUINT32(MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);
  if (FAILED(hr))
  {
    goto done;
  }

  // Add the node to the topology.
  hr = pTopology->AddNode(pNode);
  if (FAILED(hr))
  {
    goto done;
  }

  // Return the pointer to the caller.
  *ppNode = pNode;
  (*ppNode)->AddRef();

done:
  SafeRelease(&pNode);
  return hr;
}



//-------------------------------------------------------------------
//  AddTransformOutputNodes
//  Creates and adds the sink node to the encoding topology.
//  Creates and adds the required encoder activates.

//  pTopology:  A pointer to the topology.
//  pSinkActivate:  A pointer to the file sink's activation object.
//  pSourceType: A pointer to the source stream's media type.
//  ppNode:  Receives a pointer to the topology node.
//-------------------------------------------------------------------

HRESULT AddTransformOutputNodes(
  IMFTopology* pTopology,
  IMFActivate* pSinkActivate,
  IMFMediaType* pSourceType,
  IMFTopologyNode **ppNode    // Receives the node pointer.
  )
{
  if (!pTopology || !pSinkActivate || !pSourceType)
  {
    return E_INVALIDARG;
  }

  IMFTopologyNode* pEncNode = NULL;
  IMFTopologyNode* pOutputNode = NULL;
  IMFASFContentInfo* pContentInfo = NULL;
  IMFASFProfile* pProfile = NULL;
  IMFASFStreamConfig* pStream = NULL;
  IMFMediaType* pMediaType = NULL;
  IPropertyStore* pProps = NULL;
  IMFActivate *pEncoderActivate = NULL;
  IMFMediaSink *pSink = NULL;

  GUID guidMT = GUID_NULL;
  GUID guidMajor = GUID_NULL;

  DWORD cStreams = 0;
  WORD wStreamNumber = 0;

  HRESULT hr = S_OK;

  hr = pSourceType->GetMajorType(&guidMajor);
  if (FAILED(hr))
  {
    goto done;
  }

  // Create the node.
  hr = MFCreateTopologyNode(MF_TOPOLOGY_TRANSFORM_NODE, &pEncNode);
  if (FAILED(hr))
  {
    goto done;
  }

  //Activate the sink
  hr = pSinkActivate->ActivateObject(__uuidof(IMFMediaSink), (void**)&pSink);
  if (FAILED(hr))
  {
    goto done;
  }
  //find the media type in the sink
  //Get content info from the sink
  hr = pSink->QueryInterface(__uuidof(IMFASFContentInfo), (void**)&pContentInfo);
  if (FAILED(hr))
  {
    goto done;
  }


  hr = pContentInfo->GetProfile(&pProfile);
  if (FAILED(hr))
  {
    goto done;
  }

  hr = pProfile->GetStreamCount(&cStreams);
  if (FAILED(hr))
  {
    goto done;
  }

  for (DWORD index = 0; index < cStreams; index++)
  {
    hr = pProfile->GetStream(index, &wStreamNumber, &pStream);
    if (FAILED(hr))
    {
      goto done;
    }
    hr = pStream->GetMediaType(&pMediaType);
    if (FAILED(hr))
    {
      goto done;
    }
    hr = pMediaType->GetMajorType(&guidMT);
    if (FAILED(hr))
    {
      goto done;
    }
    if (guidMT != guidMajor)
    {
      SafeRelease(&pStream);
      SafeRelease(&pMediaType);
      guidMT = GUID_NULL;

      continue;
    }
    //We need to activate the encoder
    hr = pContentInfo->GetEncodingConfigurationPropertyStore(wStreamNumber, &pProps);
    if (FAILED(hr))
    {
      goto done;
    }

    if (guidMT == MFMediaType_Audio)
    {
      hr = MFCreateWMAEncoderActivate(pMediaType, pProps, &pEncoderActivate);
      if (FAILED(hr))
      {
        goto done;
      }
      wprintf_s(L"Audio Encoder created. Stream Number: %d .\n", wStreamNumber);

      break;
    }
    if (guidMT == MFMediaType_Video)
    {
      hr = MFCreateWMVEncoderActivate(pMediaType, pProps, &pEncoderActivate);
      if (FAILED(hr))
      {
        goto done;
      }
      wprintf_s(L"Video Encoder created. Stream Number: %d .\n", wStreamNumber);

      break;
    }
  }

  // Set the object pointer.
  hr = pEncNode->SetObject(pEncoderActivate);
  if (FAILED(hr))
  {
    goto done;
  }

  // Add the node to the topology.
  hr = pTopology->AddNode(pEncNode);
  if (FAILED(hr))
  {
    goto done;
  }

  //Add the output node to this node.
  hr = AddOutputNode(pTopology, pSinkActivate, wStreamNumber, &pOutputNode);
  if (FAILED(hr))
  {
    goto done;
  }

  //now we have the output node, connect it to the transform node
  hr = pEncNode->ConnectOutput(0, pOutputNode, 0);
  if (FAILED(hr))
  {
    goto done;
  }

  // Return the pointer to the caller.
  *ppNode = pEncNode;
  (*ppNode)->AddRef();


done:
  SafeRelease(&pEncNode);
  SafeRelease(&pOutputNode);
  SafeRelease(&pEncoderActivate);
  SafeRelease(&pMediaType);
  SafeRelease(&pProps);
  SafeRelease(&pStream);
  SafeRelease(&pProfile);
  SafeRelease(&pContentInfo);
  SafeRelease(&pSink);
  return hr;
}

//-------------------------------------------------------------------
//  BuildPartialTopology
//  Create a partial encoding topology by adding the source and the sink.
//
//  pSource:  A pointer to the media source to enumerate the source streams.
//  pSinkActivate: A pointer to the activation object for ASF file sink.
//  ppTopology:  Receives a pointer to the topology.
//-------------------------------------------------------------------

HRESULT BuildPartialTopology(
  IMFMediaSource *pSource,
  IMFActivate* pSinkActivate,
  IMFTopology** ppTopology)
{
  if (!pSource || !pSinkActivate)
  {
    return E_INVALIDARG;
  }
  if (!ppTopology)
  {
    return E_POINTER;
  }

  HRESULT hr = S_OK;

  IMFPresentationDescriptor* pPD = NULL;
  IMFStreamDescriptor *pStreamDesc = NULL;
  IMFMediaTypeHandler* pMediaTypeHandler = NULL;
  IMFMediaType* pSrcType = NULL;

  IMFTopology* pTopology = NULL;
  IMFTopologyNode* pSrcNode = NULL;
  IMFTopologyNode* pEncoderNode = NULL;
  IMFTopologyNode* pOutputNode = NULL;


  DWORD cElems = 0;
  DWORD dwSrcStream = 0;
  DWORD StreamID = 0;
  GUID guidMajor = GUID_NULL;
  BOOL fSelected = FALSE;


  //Create the topology that represents the encoding pipeline
  hr = MFCreateTopology(&pTopology);
  if (FAILED(hr))
  {
    goto done;
  }


  hr = pSource->CreatePresentationDescriptor(&pPD);
  if (FAILED(hr))
  {
    goto done;
  }

  hr = pPD->GetStreamDescriptorCount(&dwSrcStream);
  if (FAILED(hr))
  {
    goto done;
  }

  for (DWORD iStream = 0; iStream < dwSrcStream; iStream++)
  {
    hr = pPD->GetStreamDescriptorByIndex(
      iStream, &fSelected, &pStreamDesc);
    if (FAILED(hr))
    {
      goto done;
    }

    if (!fSelected)
    {
      continue;
    }

    hr = AddSourceNode(pTopology, pSource, pPD, pStreamDesc, &pSrcNode);
    if (FAILED(hr))
    {
      goto done;
    }

    hr = pStreamDesc->GetMediaTypeHandler(&pMediaTypeHandler);
    if (FAILED(hr))
    {
      goto done;
    }

    hr = pStreamDesc->GetStreamIdentifier(&StreamID);
    if (FAILED(hr))
    {
      goto done;
    }

    hr = pMediaTypeHandler->GetMediaTypeByIndex(0, &pSrcType);
    if (FAILED(hr))
    {
      goto done;
    }

    hr = pSrcType->GetMajorType(&guidMajor);
    if (FAILED(hr))
    {
      goto done;
    }

    hr = AddTransformOutputNodes(pTopology, pSinkActivate, pSrcType, &pEncoderNode);
    if (FAILED(hr))
    {
      goto done;
    }

    //now we have the transform node, connect it to the source node
    hr = pSrcNode->ConnectOutput(0, pEncoderNode, 0);
    if (FAILED(hr))
    {
      goto done;
    }


    SafeRelease(&pStreamDesc);
    SafeRelease(&pMediaTypeHandler);
    SafeRelease(&pSrcType);
    SafeRelease(&pEncoderNode);
    SafeRelease(&pOutputNode);
    guidMajor = GUID_NULL;
  }

  *ppTopology = pTopology;
  (*ppTopology)->AddRef();


  wprintf_s(L"Partial Topology Built.\n");

done:
  SafeRelease(&pStreamDesc);
  SafeRelease(&pMediaTypeHandler);
  SafeRelease(&pSrcType);
  SafeRelease(&pEncoderNode);
  SafeRelease(&pOutputNode);
  SafeRelease(&pTopology);

  return hr;
}



//-------------------------------------------------------------------
//  PostEncodingUpdate
//  Updates the file sink with encoding properties set on the encoder
//  during the encoding session.
//1. Get the output nodes
//2. For each node, get the downstream node
//3. For the downstream node, get the MFT
//4. Get the property store
//5. Get the required values
//6. Set them on the stream sink
//
//  pTopology: A pointer to the full topology retrieved from the media session.
//-------------------------------------------------------------------

HRESULT PostEncodingUpdate(IMFTopology *pTopology)
{
  if (!pTopology)
  {
    return E_INVALIDARG;
  }

  HRESULT hr = S_OK;

  IMFCollection* pOutputColl = NULL;
  IUnknown* pNodeUnk = NULL;
  IMFMediaType* pType = NULL;
  IMFTopologyNode* pNode = NULL;
  IUnknown* pSinkUnk = NULL;
  IMFStreamSink* pStreamSink = NULL;
  IMFTopologyNode* pEncoderNode = NULL;
  IUnknown* pEncoderUnk = NULL;
  IMFTransform* pEncoder = NULL;
  IPropertyStore* pStreamSinkProps = NULL;
  IPropertyStore* pEncoderProps = NULL;

  GUID guidMajorType = GUID_NULL;

  PROPVARIANT var;
  PropVariantInit(&var);

  DWORD cElements = 0;

  hr = pTopology->GetOutputNodeCollection(&pOutputColl);
  if (FAILED(hr))
  {
    goto done;
  }


  hr = pOutputColl->GetElementCount(&cElements);
  if (FAILED(hr))
  {
    goto done;
  }


  for (DWORD index = 0; index < cElements; index++)
  {
    hr = pOutputColl->GetElement(index, &pNodeUnk);
    if (FAILED(hr))
    {
      goto done;
    }

    hr = pNodeUnk->QueryInterface(IID_IMFTopologyNode, (void**)&pNode);
    if (FAILED(hr))
    {
      goto done;
    }

    hr = pNode->GetInputPrefType(0, &pType);
    if (FAILED(hr))
    {
      goto done;
    }

    hr = pType->GetMajorType(&guidMajorType);
    if (FAILED(hr))
    {
      goto done;
    }

    hr = pNode->GetObject(&pSinkUnk);
    if (FAILED(hr))
    {
      goto done;
    }

    hr = pSinkUnk->QueryInterface(IID_IMFStreamSink, (void**)&pStreamSink);
    if (FAILED(hr))
    {
      goto done;
    }

    hr = pNode->GetInput(0, &pEncoderNode, NULL);
    if (FAILED(hr))
    {
      goto done;
    }

    hr = pEncoderNode->GetObject(&pEncoderUnk);
    if (FAILED(hr))
    {
      goto done;
    }

    hr = pEncoderUnk->QueryInterface(IID_IMFTransform, (void**)&pEncoder);
    if (FAILED(hr))
    {
      goto done;
    }

    hr = pStreamSink->QueryInterface(IID_IPropertyStore, (void**)&pStreamSinkProps);
    if (FAILED(hr))
    {
      goto done;
    }

    hr = pEncoder->QueryInterface(IID_IPropertyStore, (void**)&pEncoderProps);
    if (FAILED(hr))
    {
      goto done;
    }

    if (guidMajorType == MFMediaType_Video)
    {
      hr = pEncoderProps->GetValue(MFPKEY_BAVG, &var);
      if (FAILED(hr))
      {
        goto done;
      }
      hr = pStreamSinkProps->SetValue(MFPKEY_STAT_BAVG, var);
      if (FAILED(hr))
      {
        goto done;
      }

      PropVariantClear(&var);
      hr = pEncoderProps->GetValue(MFPKEY_RAVG, &var);
      if (FAILED(hr))
      {
        goto done;
      }
      hr = pStreamSinkProps->SetValue(MFPKEY_STAT_RAVG, var);
      if (FAILED(hr))
      {
        goto done;
      }

      PropVariantClear(&var);
      hr = pEncoderProps->GetValue(MFPKEY_BMAX, &var);
      if (FAILED(hr))
      {
        goto done;
      }
      hr = pStreamSinkProps->SetValue(MFPKEY_STAT_BMAX, var);
      if (FAILED(hr))
      {
        goto done;
      }

      PropVariantClear(&var);
      hr = pEncoderProps->GetValue(MFPKEY_RMAX, &var);
      if (FAILED(hr))
      {
        goto done;
      }
      hr = pStreamSinkProps->SetValue(MFPKEY_STAT_RMAX, var);
      if (FAILED(hr))
      {
        goto done;
      }
    }
    else if (guidMajorType == MFMediaType_Audio)
    {
      hr = pEncoderProps->GetValue(MFPKEY_STAT_BAVG, &var);
      if (FAILED(hr))
      {
        goto done;
      }
      hr = pStreamSinkProps->SetValue(MFPKEY_STAT_BAVG, var);
      if (FAILED(hr))
      {
        goto done;
      }

      PropVariantClear(&var);
      hr = pEncoderProps->GetValue(MFPKEY_STAT_RAVG, &var);
      if (FAILED(hr))
      {
        goto done;
      }
      hr = pStreamSinkProps->SetValue(MFPKEY_STAT_RAVG, var);
      if (FAILED(hr))
      {
        goto done;
      }

      PropVariantClear(&var);
      hr = pEncoderProps->GetValue(MFPKEY_STAT_BMAX, &var);
      if (FAILED(hr))
      {
        goto done;
      }
      hr = pStreamSinkProps->SetValue(MFPKEY_STAT_BMAX, var);
      if (FAILED(hr))
      {
        goto done;
      }

      PropVariantClear(&var);
      hr = pEncoderProps->GetValue(MFPKEY_STAT_RMAX, &var);
      if (FAILED(hr))
      {
        goto done;
      }
      hr = pStreamSinkProps->SetValue(MFPKEY_STAT_RMAX, var);
      if (FAILED(hr))
      {
        goto done;
      }

      PropVariantClear(&var);
      hr = pEncoderProps->GetValue(MFPKEY_WMAENC_AVGBYTESPERSEC, &var);
      if (FAILED(hr))
      {
        goto done;
      }
      hr = pStreamSinkProps->SetValue(MFPKEY_WMAENC_AVGBYTESPERSEC, var);
      if (FAILED(hr))
      {
        goto done;
      }
    }
    PropVariantClear(&var);
  }
done:
  SafeRelease(&pOutputColl);
  SafeRelease(&pNodeUnk);
  SafeRelease(&pType);
  SafeRelease(&pNode);
  SafeRelease(&pSinkUnk);
  SafeRelease(&pStreamSink);
  SafeRelease(&pEncoderNode);
  SafeRelease(&pEncoderUnk);
  SafeRelease(&pEncoder);
  SafeRelease(&pStreamSinkProps);
  SafeRelease(&pEncoderProps);

  return hr;

}



//-------------------------------------------------------------------
//  Encode
//  Controls the encoding session and handles events from the media session.
//
//  pTopology:  A pointer to the encoding topology.
//-------------------------------------------------------------------

HRESULT Encode(IMFTopology *pTopology)
{
  if (!pTopology)
  {
    return E_INVALIDARG;
  }

  IMFMediaSession *pSession = NULL;
  IMFMediaEvent* pEvent = NULL;
  IMFTopology* pFullTopology = NULL;
  IUnknown* pTopoUnk = NULL;


  MediaEventType meType = MEUnknown;  // Event type

  HRESULT hr = S_OK;
  HRESULT hrStatus = S_OK;            // Event status

  MF_TOPOSTATUS TopoStatus = MF_TOPOSTATUS_INVALID; // Used with MESessionTopologyStatus event.    


  hr = MFCreateMediaSession(NULL, &pSession);
  if (FAILED(hr))
  {
    goto done;
  }

  hr = pSession->SetTopology(MFSESSION_SETTOPOLOGY_IMMEDIATE, pTopology);
  if (FAILED(hr))
  {
    goto done;
  }

  //Get media session events synchronously
  while (1)
  {
    hr = pSession->GetEvent(0, &pEvent);
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

        hr = pSession->Start(NULL, &var);
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
        hr = pSession->Close();
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
      hr = pSession->Close();
      if (FAILED(hr))
      {
        goto done;
      }
      break;

    case MEEndOfPresentation:
    {
      if (EncodingMode == VBR)
      {
        hr = pSession->GetFullTopology(MFSESSION_GETFULLTOPOLOGY_CURRENT, 0, &pFullTopology);
        if (FAILED(hr))
        {
          goto done;
        }
        hr = PostEncodingUpdate(pFullTopology);
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

      hr = pSession->Shutdown();
      goto done;
    }
    if (FAILED(hr))
    {
      goto done;
    }

    SafeRelease(&pEvent);

  }
done:
  SafeRelease(&pEvent);
  SafeRelease(&pSession);
  SafeRelease(&pFullTopology);
  SafeRelease(&pTopoUnk);
  return hr;
}




int main()
{
  CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
  HRESULT hr = MFStartup(MF_VERSION);

  IMFMediaSource *source = NULL;
  IMFActivate *pFileSinkActivate = NULL;
  IMFTopology* pTopology = NULL;

  hr = CreateMediaSource(L"c:\\users\\bmalec\\Ring05.wav", &source);

  hr = CreateMediaSink(L"c:\\users\\bmalec\\Ring05.wma", source, &pFileSinkActivate);


  //Build the encoding topology.
  hr = BuildPartialTopology(source, pFileSinkActivate, &pTopology);


  hr = Encode(pTopology);


  SafeRelease(&source);
  SafeRelease(&pTopology);
  SafeRelease(&pFileSinkActivate);





  MFShutdown();
  CoUninitialize();

    return 0;
}

