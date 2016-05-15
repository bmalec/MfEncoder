#include "stdafx.h"
#include <objbase.h>
#include <propvarutil.h>
#include <stdexcept>
#include "MediaSource.h"

IMFPresentationDescriptor* MediaSource::GetPresentationDescriptor()
{
  return _mfPresentationDescriptor;
}


void MediaSource::LoadMetadataFromSource()
{
  IMFMetadataProvider* mfMetadataProvider;
  IMFMetadata* mfMetadata;
  PROPVARIANT metadataKeys, metadataValue;
  PWSTR *metadataPropertyKeys;
  HRESULT hr;

  _metadataPropertyCount = 0;
  PropVariantInit(&metadataKeys);
  PropVariantInit(&metadataValue);
  
  do
  {
    if (!SUCCEEDED(hr = MFGetService(_mfMediaSource, MF_METADATA_PROVIDER_SERVICE, IID_PPV_ARGS(&mfMetadataProvider))))
      break;

    if (!SUCCEEDED(hr = mfMetadataProvider->GetMFMetadata(_mfPresentationDescriptor, 0, 0, &mfMetadata)))
      break;

    if (!SUCCEEDED(hr = mfMetadata->GetAllPropertyNames(&metadataKeys)))
      break;

    if (!SUCCEEDED(hr = PropVariantToStringVectorAlloc(metadataKeys, &metadataPropertyKeys, &_metadataPropertyCount)))
      break;

    _metadata = (MetadataKeyValuePair *)malloc(_metadataPropertyCount * sizeof(MetadataKeyValuePair));

    for (ULONG i = 0; i < _metadataPropertyCount; i++)
    {
      wchar_t *metadataKey = *(metadataPropertyKeys + i);

      if (!SUCCEEDED(hr = mfMetadata->GetProperty(metadataKey, &metadataValue)))
        break;

      (_metadata + i)->Key = metadataKey;

      if (!SUCCEEDED(hr = PropVariantToStringAlloc(metadataValue, &((_metadata + i)->Value))))
        break;

      PropVariantClear(&metadataValue);
    }

    if (FAILED(hr))
      break;

  } while (0);

  if (metadataPropertyKeys) CoTaskMemFree(metadataPropertyKeys);
  if (mfMetadata) mfMetadata->Release();
  if (mfMetadataProvider) mfMetadataProvider->Release();

  if (FAILED(hr))
  {
    throw std::exception("Error occurred reading metadata from media source");
  }
}

MediaSource::MediaSource(IMFMediaSource *mfMediaSource)
{
  _mfMediaSource = mfMediaSource;
  _mfMediaSource->CreatePresentationDescriptor(&_mfPresentationDescriptor);
//  _mfPresentationDescriptor->GetStreamDescriptorCount(&_streamDescriptorCount);

  LoadMetadataFromSource();
}

MediaSource::~MediaSource()
{
  if (_mfPresentationDescriptor) _mfPresentationDescriptor->Release();

  if (_metadata)
  {
    for (ULONG i = 0; i < _metadataPropertyCount; i++)
    {
      CoTaskMemFree((_metadata + i)->Key);
      CoTaskMemFree((_metadata + i)->Value);
    }
  }

  if (_mfMediaSource)
  {
    _mfMediaSource->Shutdown();
    _mfMediaSource->Release();
  }
}

/*
DWORD MediaSource::GetStreamDescriptorCount()
{
  return _streamDescriptorCount;
}
*/

/*
GUID MediaSource::GetMajorType()
{
  GUID majorType = GUID_NULL;
  StreamDescriptor* streamDescriptor = GetStreamDescriptorByIndex(0);
  majorType = streamDescriptor->GetMajorType();
  delete streamDescriptor;

  return majorType;
}
*/


MediaSource* MediaSource::Open(const wchar_t *url)
{
  MF_OBJECT_TYPE ObjectType = MF_OBJECT_INVALID;

  IMFSourceResolver* mfSourceResolver = nullptr;
  IMFMediaSource* mfMediaSource = nullptr;
  IUnknown* pSource = nullptr;

  // Create the source resolver.
  HRESULT hr = MFCreateSourceResolver(&mfSourceResolver);

  // Use the source resolver to create the media source.

  hr = mfSourceResolver->CreateObjectFromURL(
    url,                       // URL of the source.
    MF_RESOLUTION_MEDIASOURCE,  // Create a source object.
    nullptr,                       // Optional property store.
    &ObjectType,        // Receives the created object type. 
    &pSource            // Receives a pointer to the media source.
  );

  // Get the IMFMediaSource interface from the media source.
  hr = pSource->QueryInterface(IID_PPV_ARGS(&mfMediaSource));

  if (pSource) pSource->Release();
  if (mfSourceResolver) mfSourceResolver->Release();

  return new MediaSource(mfMediaSource);
}

IMFMediaSource* MediaSource::GetMFMediaSource()
{
  return _mfMediaSource;
}

/*
StreamDescriptor* MediaSource::GetStreamDescriptorByIndex(DWORD index)
{
  IMFStreamDescriptor* mfStreamDescriptor = nullptr;
  BOOL isSelected = FALSE;

  HRESULT hr = _mfPresentationDescriptor->GetStreamDescriptorByIndex(index, &isSelected, &mfStreamDescriptor);

  return new StreamDescriptor(mfStreamDescriptor, isSelected);
}
*/

wchar_t* MediaSource::GetMetadataValue(wchar_t *metadataKey)
{
  wchar_t *result = nullptr;

  for (int i = 0; i < _metadataPropertyCount; i++)
  {
    if (wcscmp((_metadata + i)->Key, metadataKey) == 0)
    {
      result = (_metadata + i)->Value;
      break;
    }
  }

  return result;
}



IMFTopologyNode* MediaSource::CreateTopologySourceNode()
{
  IMFTopologyNode* mfTopologyNode = nullptr;
  IMFStreamDescriptor* mfStreamDescriptor = nullptr;
  HRESULT hr;
  DWORD streamDescriptorCount;
  BOOL isSelected;

  do
  {
    if (!SUCCEEDED(hr = _mfPresentationDescriptor->GetStreamDescriptorCount(&streamDescriptorCount)))
      break;

    for (DWORD i = 0; i < streamDescriptorCount; i++)
    {
      if (!SUCCEEDED(hr = _mfPresentationDescriptor->GetStreamDescriptorByIndex(i, &isSelected, &mfStreamDescriptor)))
        break;

      if (isSelected)
      {
        break;
      }
      else
      {
        mfStreamDescriptor->Release();
        mfStreamDescriptor = nullptr;
      }
    }

    if (FAILED(hr) || !mfStreamDescriptor)
    {
      throw std::exception("Could not find selected stream in MediaSource");
    }

    // otherwise, we're in good shape :-)

    if (!SUCCEEDED(hr = MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &mfTopologyNode)))
      break;

    if (!SUCCEEDED(hr = mfTopologyNode->SetUnknown(MF_TOPONODE_SOURCE, _mfMediaSource)))
      break;

    if (!SUCCEEDED(hr = mfTopologyNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, _mfPresentationDescriptor)))
      break;

    if (!SUCCEEDED(hr = mfTopologyNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, mfStreamDescriptor)))
      break;
  } while (0);

  if (FAILED(hr))
  {
    throw std::exception("Could not create topology source node");
  }

  // todo: should mfStreamDescriptor be released here?

  return mfTopologyNode;
}
