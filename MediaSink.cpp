#include "stdafx.h"
#include <exception>
#include "MediaSink.h"



MediaSink::MediaSink(IMFMediaSink *mfMediaSink, MediaSinkContentInfo* mediaSinkContentInfo)
{
  _mfMediaSink = mfMediaSink;
  _mediaSinkContentInfo = mediaSinkContentInfo;
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

MediaSink* MediaSink::Create(const wchar_t *url, MediaSinkContentInfo* contentInfo)
{
  IMFASFContentInfo* mfAsfContentInfo = contentInfo->GetMfAsfContentInfoObject();

  IMFActivate* mfActivate = nullptr;
  IMFMediaSink* mfMediaSink = nullptr;

  //Create the activation object for the  file sink
  HRESULT hr = MFCreateASFMediaSinkActivate(url, mfAsfContentInfo, &mfActivate);

  if (!SUCCEEDED(hr = mfActivate->ActivateObject(__uuidof(IMFMediaSink), (void**)&mfMediaSink)))
    throw std::exception("Could not activate MediaSink");

  mfActivate->Release();
  mfAsfContentInfo->Release();

  return new MediaSink(mfMediaSink, contentInfo);

}



IPropertyStore* MediaSink::GetEncoderConfigurationPropertyStore(WORD streamNumber)
{
  return _mediaSinkContentInfo->GetEncoderConfigurationPropertyStore(streamNumber);

}


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





IMFTopologyNode* MediaSink::CreateTopologyTransformNode(WORD streamNumber)
{
  return nullptr;

}


