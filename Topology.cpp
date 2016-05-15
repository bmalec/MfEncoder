#include "stdafx.h"
#include <wmcontainer.h>
#include <wmcodecdsp.h>
#include <mfapi.h>
#include "Util.h"
#include "Topology.h"

Topology::Topology(IMFTopology* mfTopology, MediaSink* mediaSink)
{
  _mfTopology = mfTopology;
  _mediaSink = mediaSink;
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

static HRESULT UpdateVbrStreamProperties(IMFMediaSession *mfMediaSession, MediaSink* mediaSink)
{

  HRESULT hr = S_OK;

  IMFTopology* mfFullTopology = nullptr;
  IMFCollection* mfOutputCollection = nullptr;
  IUnknown* pNodeUnk = nullptr;
  IMFTopologyNode* mfOutputNode = nullptr;
  IMFStreamSink* pStreamSink = nullptr;
  IMFTopologyNode* mfEncoderNode = nullptr;
  IUnknown* pEncoderUnk = nullptr;
  IMFTransform* pEncoder = nullptr;
  IPropertyStore* pStreamSinkProps = nullptr;
  IPropertyStore* pEncoderProps = nullptr;


  DWORD cElements = 0;

  if (!mfMediaSession) throw std::exception("Null parameter:  mfMediaSession");

  hr = mfMediaSession->GetFullTopology(MFSESSION_GETFULLTOPOLOGY_CURRENT, 0, &mfFullTopology);
  if (FAILED(hr))
  {
    goto done;
  }


  hr = mfFullTopology->GetOutputNodeCollection(&mfOutputCollection);
  if (FAILED(hr))
  {
    goto done;
  }


  hr = mfOutputCollection->GetElementCount(&cElements);
  if (FAILED(hr))
  {
    goto done;
  }


  for (DWORD index = 0; index < cElements; index++)
  {
    
    hr = mfOutputCollection->GetElement(index, &pNodeUnk);
    if (FAILED(hr))
    {
      goto done;
    }

    hr = pNodeUnk->QueryInterface(IID_IMFTopologyNode, (void**)&mfOutputNode);
    if (FAILED(hr))
    {
      goto done;
    }


    pStreamSink = mediaSink->GetMFStreamSinkByIndex(index);

    hr = pStreamSink->QueryInterface(IID_IPropertyStore, (void**)&pStreamSinkProps);
    if (FAILED(hr))
    {
      goto done;
    }


    hr = mfOutputNode->GetInput(0, &mfEncoderNode, nullptr);
    if (FAILED(hr))
    {
      goto done;
    }

    hr = mfEncoderNode->GetObject(&pEncoderUnk);
    if (FAILED(hr))
    {
      goto done;
    }

    hr = pEncoderUnk->QueryInterface(IID_IMFTransform, (void**)&pEncoder);
    if (FAILED(hr))
    {
      goto done;
    }


    hr = pEncoder->QueryInterface(IID_IPropertyStore, (void**)&pEncoderProps);
    if (FAILED(hr))
    {
      goto done;
    }

    PROPVARIANT var = GetPropertyStoreValue(pEncoderProps, MFPKEY_STAT_BAVG);
    SetPropertyStoreValue(pStreamSinkProps, MFPKEY_STAT_BAVG, var);
    PropVariantClear(&var);

    var = GetPropertyStoreValue(pEncoderProps, MFPKEY_STAT_RAVG);
    SetPropertyStoreValue(pStreamSinkProps, MFPKEY_STAT_RAVG, var);
    PropVariantClear(&var);

    var = GetPropertyStoreValue(pEncoderProps, MFPKEY_STAT_BMAX);
    SetPropertyStoreValue(pStreamSinkProps, MFPKEY_STAT_BMAX, var);
    PropVariantClear(&var);

    var = GetPropertyStoreValue(pEncoderProps, MFPKEY_STAT_RMAX);
    SetPropertyStoreValue(pStreamSinkProps, MFPKEY_STAT_RMAX, var);
    PropVariantClear(&var);

    var = GetPropertyStoreValue(pEncoderProps, MFPKEY_WMAENC_AVGBYTESPERSEC);
    SetPropertyStoreValue(pStreamSinkProps, MFPKEY_WMAENC_AVGBYTESPERSEC, var);
    PropVariantClear(&var);


  }
  
done:
  mfOutputCollection->Release();
  pNodeUnk->Release();
//  mfMediaType->Release();
  mfOutputNode->Release();
//  pSinkUnk->Release();
  pStreamSink->Release();
  mfEncoderNode->Release();
  pEncoderUnk->Release();
  pEncoder->Release();
  pStreamSinkProps->Release();
  pEncoderProps->Release();
  if (mfFullTopology) mfFullTopology->Release();

  return hr;

}




//-------------------------------------------------------------------
//  Encode
//  Controls the encoding session and handles events from the media session.
//
//  pTopology:  A pointer to the encoding topology.
//-------------------------------------------------------------------

HRESULT Topology::Encode(Parameters* parameters)
{

  IMFMediaSession *mfMediaSession = nullptr;
  IMFMediaEvent* pEvent = nullptr;
//  IMFTopology* pFullTopology = nullptr;


  MediaEventType meType = MEUnknown;  // Event type

  HRESULT hr = S_OK;
  HRESULT hrStatus = S_OK;            // Event status

  MF_TOPOSTATUS TopoStatus = MF_TOPOSTATUS_INVALID; // Used with MESessionTopologyStatus event.    


  hr = MFCreateMediaSession(nullptr, &mfMediaSession);
  if (FAILED(hr))
  {
    goto done;
  }

  hr = mfMediaSession->SetTopology(MFSESSION_SETTOPOLOGY_IMMEDIATE, _mfTopology);
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
      if (parameters->Quality > 0)
      {
        hr = UpdateVbrStreamProperties(mfMediaSession, _mediaSink);
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
//  pFullTopology->Release();
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
  MediaSink* mediaSink,
//  IMFActivate* pSinkActivate,
//  GUID guidMajor,
  IMFTopologyNode **ppNode    // Receives the node pointer.
)
{
  IMFTopologyNode* pEncNode = nullptr;
//  IMFTopologyNode* pOutputNode = nullptr;
  IMFASFContentInfo* pContentInfo = nullptr;
  IMFASFProfile* pProfile = nullptr;
  IMFASFStreamConfig* pStream = nullptr;
  IMFMediaType* pMediaType = nullptr;
  IPropertyStore* encoderConfigurationProperties = nullptr;
  IMFActivate *pEncoderActivate = nullptr;
  IMFMediaSink* mfMediaSink = nullptr;

  GUID guidMT = GUID_NULL;

  DWORD cStreams = 0;
  WORD wStreamNumber = 0;

  HRESULT hr = S_OK;

  // Create the node.
  hr = MFCreateTopologyNode(MF_TOPOLOGY_TRANSFORM_NODE, &pEncNode);
  if (FAILED(hr))
  {
    goto done;
  }

  
  //find the media type in the sink
  //Get content info from the sink

  mfMediaSink = mediaSink->GetMFMediaSink();

  hr = mfMediaSink->QueryInterface(__uuidof(IMFASFContentInfo), (void**)&pContentInfo);
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
    
    //We need to activate the encoder

    encoderConfigurationProperties = mediaSink->GetEncoderConfigurationPropertyStore(wStreamNumber);



    if (guidMT == MFMediaType_Audio)
    {
      hr = MFCreateWMAEncoderActivate(pMediaType, encoderConfigurationProperties, &pEncoderActivate);
      if (FAILED(hr))
      {
        goto done;
      }
      wprintf_s(L"Audio Encoder created. Stream Number: %d .\n", wStreamNumber);

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

  IMFTopologyNode* mfOutputNode = nullptr;

  mfOutputNode = mediaSink->CreateTopologyOutputNode(2);

  hr = _mfTopology->AddNode(mfOutputNode);

  //Add the output node to this node.
// bam  pOutputNode = AddOutputNode(mediaSink, wStreamNumber);

  //now we have the output node, connect it to the transform node
  hr = pEncNode->ConnectOutput(0, mfOutputNode, 0);
  if (FAILED(hr))
  {
    goto done;
  }

  // Return the pointer to the caller.
  *ppNode = pEncNode;
  (*ppNode)->AddRef();


done:
  pEncNode->Release();
//  pOutputNode->Release();
  pEncoderActivate->Release();
  pMediaType->Release();
  encoderConfigurationProperties->Release();
  pStream->Release();
  pProfile->Release();
  pContentInfo->Release();

  return hr;
}



Topology* Topology::CreatePartialTopograpy(MediaSource* source, MediaSink* mediaSink)
{
  IMFTopology* mfTopology = nullptr;
  //Create the topology that represents the encoding pipeline
  HRESULT hr = MFCreateTopology(&mfTopology);

  Topology* topology = new Topology(mfTopology, mediaSink);
  topology->_buildPartialTopograpy(source, mediaSink);

  return topology;
}




void Topology::_buildPartialTopograpy(MediaSource* source, MediaSink* sink)
{
  IMFTopologyNode* pEncoderNode = nullptr;
  HRESULT hr = S_OK;

  // Add source node

  IMFTopologyNode* mfTopologySourceNode = nullptr;

  mfTopologySourceNode = source->CreateTopologySourceNode();

  _mfTopology->AddNode(mfTopologySourceNode);


    hr = AddTransformOutputNodes(sink, &pEncoderNode);
    if (FAILED(hr))
    {
      goto done;
    }

    //now we have the transform node, connect it to the source node
    hr = mfTopologySourceNode->ConnectOutput(0, pEncoderNode, 0);
    if (FAILED(hr))
    {
      goto done;
    }


  wprintf_s(L"Partial Topology Built.\n");

done:
  pEncoderNode->Release();
}