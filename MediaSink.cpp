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

