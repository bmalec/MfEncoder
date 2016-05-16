#include "stdafx.h"
#include <wmcontainer.h>
#include <wmcodecdsp.h>
#include <mfapi.h>
#include "Util.h"
#include <stdexcept>
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

 HRESULT Topology::UpdateVbrStreamProperties(IMFMediaSession *mfMediaSession, MediaSink* mediaSink)
{
  IPropertyStore* encoderPropertyStore;
  HRESULT hr;

    hr = _audioEncoder->QueryInterface(IID_PPV_ARGS(&encoderPropertyStore));

    _mediaSink->UpdatePostEncodeStreamSinkProperties(1, encoderPropertyStore);

    /*
  PROPVARIANT var = GetPropertyStoreValue(encoderPropertyStore, MFPKEY_STAT_BAVG);

//  SetPropertyStoreValue(pStreamSinkProps, MFPKEY_STAT_BAVG, var);
//  PropVariantClear(&var);

  var = GetPropertyStoreValue(encoderPropertyStore, MFPKEY_STAT_RAVG);
//  SetPropertyStoreValue(pStreamSinkProps, MFPKEY_STAT_RAVG, var);
 // PropVariantClear(&var);

  var = GetPropertyStoreValue(encoderPropertyStore, MFPKEY_STAT_BMAX);
//  SetPropertyStoreValue(pStreamSinkProps, MFPKEY_STAT_BMAX, var);
//  PropVariantClear(&var);

  var = GetPropertyStoreValue(encoderPropertyStore, MFPKEY_STAT_RMAX);
//  SetPropertyStoreValue(pStreamSinkProps, MFPKEY_STAT_RMAX, var);
//  PropVariantClear(&var);

  var = GetPropertyStoreValue(encoderPropertyStore, MFPKEY_WMAENC_AVGBYTESPERSEC);

  return hr;


  



  /* old code

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
  */

return hr;

}




//-------------------------------------------------------------------
//  Encode
//  Controls the encoding session and handles events from the media session.
//
//  pTopology:  A pointer to the encoding topology.
//-------------------------------------------------------------------

HRESULT Topology::Encode(AudioEncoderParameters* encoderParameters)
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
      if (encoderParameters->IsQualityBasedVbr())
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
  return hr;
}






Topology* Topology::CreatePartialTopograpy(MediaSource* source, MediaSink* mediaSink, WORD streamNumber)
{
  IMFTopology* mfTopology = nullptr;
  //Create the topology that represents the encoding pipeline
  HRESULT hr = MFCreateTopology(&mfTopology);

  Topology* topology = new Topology(mfTopology, mediaSink);
  topology->_buildPartialTopograpy(source, mediaSink, streamNumber);

  return topology;
}




void Topology::_buildPartialTopograpy(MediaSource* source, MediaSink* sink, WORD streamNumber)
{
  IMFTopologyNode* encoderNode = nullptr;
  IMFTopologyNode* sourceNode = nullptr;
  IMFTopologyNode* outputNode = nullptr;

  HRESULT hr = S_OK;

  do {

    // Add source node

    sourceNode = source->CreateTopologySourceNode();

    if (!SUCCEEDED(hr = _mfTopology->AddNode(sourceNode)))
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



    _audioEncoder = sink->GetAudioEncoderForStream(1);

    hr = MFCreateTopologyNode(MF_TOPOLOGY_TRANSFORM_NODE, &encoderNode);

    // Set the object pointer.
    hr = encoderNode->SetObject(_audioEncoder);

    if (!SUCCEEDED(hr = _mfTopology->AddNode(encoderNode)))
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
    if (!SUCCEEDED(hr = _mfTopology->AddNode(outputNode)))
      break;

    // connect the transform node to the output node

    if (!SUCCEEDED(hr = encoderNode->ConnectOutput(0, outputNode, 0)))
      break;
  } while (0);

  if (sourceNode) sourceNode->Release();
  if (outputNode) outputNode->Release();
  if (encoderNode) encoderNode->Release();


  if (FAILED(hr))
    throw std::exception("Unable to construct topology");
}