#include "stdafx.h"
#include <wmcontainer.h>
#include <wmcodecdsp.h>
#include <mfapi.h>
#include "Globals.h"
#include "Topology.h"

Topology::Topology(IMFTopology* mfTopology)
{
  _mfTopology = mfTopology;
}


Topology::~Topology()
{
  _mfTopology->Release();
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

static HRESULT PostEncodingUpdate(IMFTopology *pTopology)
{
  /*
  if (!pTopology)
  {
    return E_INVALIDARG;
  }
  */

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
  pOutputColl->Release();
  pNodeUnk->Release();
  pType->Release();
  pNode->Release();
  pSinkUnk->Release();
  pStreamSink->Release();
  pEncoderNode->Release();
  pEncoderUnk->Release();
  pEncoder->Release();
  pStreamSinkProps->Release();
  pEncoderProps->Release();

  return hr;

}




//-------------------------------------------------------------------
//  Encode
//  Controls the encoding session and handles events from the media session.
//
//  pTopology:  A pointer to the encoding topology.
//-------------------------------------------------------------------

HRESULT Topology::Encode()
{
  /*
  if (!pTopology)
  {
    return E_INVALIDARG;
  }
  */

  IMFMediaSession *pSession = NULL;
  IMFMediaEvent* pEvent = NULL;
  IMFTopology* pFullTopology = NULL;


  MediaEventType meType = MEUnknown;  // Event type

  HRESULT hr = S_OK;
  HRESULT hrStatus = S_OK;            // Event status

  MF_TOPOSTATUS TopoStatus = MF_TOPOSTATUS_INVALID; // Used with MESessionTopologyStatus event.    


  hr = MFCreateMediaSession(NULL, &pSession);
  if (FAILED(hr))
  {
    goto done;
  }

  hr = pSession->SetTopology(MFSESSION_SETTOPOLOGY_IMMEDIATE, _mfTopology);
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

    pEvent->Release();

  }
done:
  pEvent->Release();
  pSession->Release();
  pFullTopology->Release();
  return hr;
}




// Add an output node to a topology.
HRESULT Topology::AddOutputNode(
//  IMFTopology *pTopology,     // Topology.
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
  hr = _mfTopology->AddNode(pNode);
  if (FAILED(hr))
  {
    goto done;
  }

  // Return the pointer to the caller.
  *ppNode = pNode;
  (*ppNode)->AddRef();

done:
  pNode->Release();
//  SafeRelease(&pNode);
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

HRESULT Topology::AddTransformOutputNodes(
//  IMFTopology* pTopology,
  IMFActivate* pSinkActivate,
  IMFMediaType* pSourceType,
  IMFTopologyNode **ppNode    // Receives the node pointer.
)
{
  /*
  if (!pTopology || !pSinkActivate || !pSourceType)
  {
    return E_INVALIDARG;
  }
  */

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
      pStream->Release();
      pMediaType->Release();
//      SafeRelease(&pStream);
//      SafeRelease(&pMediaType);
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
  hr = _mfTopology->AddNode(pEncNode);
  if (FAILED(hr))
  {
    goto done;
  }

  //Add the output node to this node.
  hr = AddOutputNode(pSinkActivate, wStreamNumber, &pOutputNode);
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
  pEncNode->Release();
  pOutputNode->Release();
  pEncoderActivate->Release();
  pMediaType->Release();
  pProps->Release();
  pStream->Release();
  pProfile->Release();
  pContentInfo->Release();
  pSink->Release();


  /*
  SafeRelease(&pEncNode);
  SafeRelease(&pOutputNode);
  SafeRelease(&pEncoderActivate);
  SafeRelease(&pMediaType);
  SafeRelease(&pProps);
  SafeRelease(&pStream);
  SafeRelease(&pProfile);
  SafeRelease(&pContentInfo);
  SafeRelease(&pSink);
  */
  return hr;
}



// Add a source node to a topology.
HRESULT Topology::AddSourceNode(
//  IMFTopology *pTopology,           // Topology.
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
  hr = _mfTopology->AddNode(pNode);
  if (FAILED(hr))
  {
    goto done;
  }

  // Return the pointer to the caller.
  *ppNode = pNode;
  (*ppNode)->AddRef();

done:
  pNode->Release();
//  SafeRelease(&pNode);
  return hr;
}

Topology* Topology::Create()
{
  IMFTopology* mfTopology = nullptr;
  //Create the topology that represents the encoding pipeline
  HRESULT hr = MFCreateTopology(&mfTopology);

  return new Topology(mfTopology);
}




void Topology::BuildPartialTopograpy(MediaSource* source, MediaSink* sink)
{
  HRESULT hr = S_OK;

  IMFPresentationDescriptor* pPD = NULL;
  IMFStreamDescriptor *pStreamDesc = NULL;
  IMFMediaTypeHandler* pMediaTypeHandler = NULL;
  IMFMediaType* pSrcType = NULL;

  IMFTopologyNode* pSrcNode = NULL;
  IMFTopologyNode* pEncoderNode = NULL;


  DWORD cElems = 0;
  DWORD dwSrcStream = 0;
  DWORD StreamID = 0;
  GUID guidMajor = GUID_NULL;
  BOOL fSelected = FALSE;


  //Create the topology that represents the encoding pipeline
  /*
  hr = MFCreateTopology(&pTopology);
  if (FAILED(hr))
  {
    goto done;
  }
  */

  pPD = source->CreatePresentationDescriptor();

/* old
  hr = pSource->CreatePresentationDescriptor(&pPD);
  if (FAILED(hr))
  {
    goto done;
  }
*/

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

    hr = AddSourceNode(source->GetMFMediaSource(), pPD, pStreamDesc, &pSrcNode);
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

    hr = AddTransformOutputNodes(sink->GetActivationObject(), pSrcType, &pEncoderNode);
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


    guidMajor = GUID_NULL;
  }

//  *ppTopology = pTopology;
//  (*ppTopology)->AddRef();


  wprintf_s(L"Partial Topology Built.\n");

done:
  pStreamDesc->Release();
  pMediaTypeHandler->Release();
  pSrcType->Release();
  pEncoderNode->Release();
}