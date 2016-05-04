#include "stdafx.h"
#include <objbase.h>
#include <propvarutil.h>
#include "MediaSource.h"

IMFPresentationDescriptor* MediaSource::GetPresentationDescriptor()
{
  return _mfPresentationDescriptor;
}


void MediaSource::LoadMetadataFromSource()
{
//  IMFPresentationDescriptor* mfPresentationDescriptor;
  IMFMetadataProvider* mfMetadataProvider;
  IMFMetadata* mfMetadata;

//  HRESULT hr = _mfMediaSource->CreatePresentationDescriptor(&mfPresentationDescriptor);

  HRESULT hr = MFGetService(_mfMediaSource, MF_METADATA_PROVIDER_SERVICE, IID_PPV_ARGS(&mfMetadataProvider));


  hr = mfMetadataProvider->GetMFMetadata(_mfPresentationDescriptor, 0, 0, &mfMetadata);

  PROPVARIANT metadataKeys;

  hr = mfMetadata->GetAllPropertyNames(&metadataKeys);

  PWSTR *metadataPropertyKeys;
  ULONG metadataPropertyCount;

  PropVariantToStringVectorAlloc(metadataKeys, &metadataPropertyKeys, &metadataPropertyCount);

  _metadataItemCount = metadataPropertyCount;

  _metadata = (MetadataKeyValuePair *)malloc(metadataPropertyCount * sizeof(MetadataKeyValuePair));

  for (int i = 0; i < metadataPropertyCount; i++)
  {
    PROPVARIANT metadataValue;
    PWSTR pvStringBuffer;

    wchar_t *metadataKey = *(metadataPropertyKeys + i);

    hr = mfMetadata->GetProperty(metadataKey, &metadataValue);

    MetadataKeyValuePair *kvp = (_metadata + i);

    wcscpy_s(kvp->Key, sizeof(kvp->Key) / sizeof(kvp->Key[0]), metadataKey);

    hr = PropVariantToStringAlloc(metadataValue, &pvStringBuffer);

    kvp->Value = (wchar_t *)malloc((wcslen(pvStringBuffer) + 1) * sizeof(wchar_t));

    wcscpy_s(kvp->Value, wcslen(pvStringBuffer) + 1, pvStringBuffer);

    CoTaskMemFree(pvStringBuffer);
    //			kvp->Value = metadataValue;

    hr = S_OK;
  }

  CoTaskMemFree(metadataPropertyKeys);

  PropVariantClear(&metadataKeys);


}

MediaSource::MediaSource(IMFMediaSource *mfMediaSource)
{
  _mfMediaSource = mfMediaSource;
  _mfMediaSource->CreatePresentationDescriptor(&_mfPresentationDescriptor);
  _mfPresentationDescriptor->GetStreamDescriptorCount(&_streamDescriptorCount);

  LoadMetadataFromSource();
}

MediaSource::~MediaSource()
{
  if (_mfMediaSource != nullptr)
    _mfMediaSource->Release();
}


DWORD MediaSource::GetStreamDescriptorCount()
{
  return _streamDescriptorCount;
}

GUID MediaSource::GetMajorType()
{
  IMFStreamDescriptor *mfStreamDescriptor = nullptr;
  IMFMediaTypeHandler *mfMediaTypeHandler = nullptr;
  IMFMediaType *mfMediaType = nullptr; 
  GUID majorType = GUID_NULL;
  HRESULT hr;
  BOOL selected;

//  streamDescriptorCount = source->GetStreamDescriptorCount();
  //hr = mfPresentationDescriptor->GetStreamDescriptorCount(&streamDescriptorCount);

  do
  {
    hr = _mfPresentationDescriptor->GetStreamDescriptorByIndex(0, &selected, &mfStreamDescriptor);
    hr = mfStreamDescriptor->GetMediaTypeHandler(&mfMediaTypeHandler);
    hr = mfMediaTypeHandler->GetMediaTypeByIndex(0, &mfMediaType);
    hr = mfMediaType->GetMajorType(&majorType);
  } while (0);

//  hr = _mfPresentationDescriptor->GetStreamDescriptorByIndex(0, &selected, &mfStreamDescriptor);

//  hr = mfStreamDescriptor->GetMediaTypeHandler(&mfMediaTypeHandler);
//  hr = mfMediaTypeHandler->GetMediaTypeByIndex(0, &mfMediaType);

//  hr = mfMediaType->GetMajorType(&majorType);

  if (mfMediaType) mfMediaType->Release();
  if (mfMediaTypeHandler) mfMediaTypeHandler->Release();
  if (mfStreamDescriptor) mfStreamDescriptor->Release();

  return majorType;
}


MediaSource* MediaSource::Open(const wchar_t *url)
{
  MF_OBJECT_TYPE ObjectType = MF_OBJECT_INVALID;

  IMFSourceResolver* pSourceResolver = nullptr;
  IMFMediaSource* mfMediaSource = nullptr;
  IUnknown* pSource = nullptr;

  // Create the source resolver.
  HRESULT hr = MFCreateSourceResolver(&pSourceResolver);

  // Use the source resolver to create the media source.

  hr = pSourceResolver->CreateObjectFromURL(
    url,                       // URL of the source.
    MF_RESOLUTION_MEDIASOURCE,  // Create a source object.
    nullptr,                       // Optional property store.
    &ObjectType,        // Receives the created object type. 
    &pSource            // Receives a pointer to the media source.
  );

  // Get the IMFMediaSource interface from the media source.
  hr = pSource->QueryInterface(IID_PPV_ARGS(&mfMediaSource));

  pSource->Release();
  pSourceResolver->Release();

  return new MediaSource(mfMediaSource);
}

IMFMediaSource* MediaSource::GetMFMediaSource()
{
  return _mfMediaSource;
}


StreamDescriptor* MediaSource::GetStreamDescriptorByIndex(DWORD index)
{
  IMFStreamDescriptor* mfStreamDescriptor = nullptr;
  BOOL isSelected = FALSE;

  HRESULT hr = _mfPresentationDescriptor->GetStreamDescriptorByIndex(index, &isSelected, &mfStreamDescriptor);

  return new StreamDescriptor(mfStreamDescriptor, isSelected);
}