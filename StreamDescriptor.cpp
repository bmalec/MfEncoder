#include "stdafx.h"
#include "StreamDescriptor.h"


StreamDescriptor::StreamDescriptor(IMFStreamDescriptor *mfStreamDescriptor, BOOL isSelected)
{
  _mfStreamDescriptor = mfStreamDescriptor;
  _isSelected = isSelected;
}


StreamDescriptor::~StreamDescriptor()
{
  if (_mfStreamDescriptor) _mfStreamDescriptor->Release();
}


BOOL StreamDescriptor::GetIsSelected()
{
  return _isSelected;
}


GUID StreamDescriptor::GetMajorType()
{
  IMFMediaTypeHandler* mfMediaTypeHandler = nullptr;
  IMFMediaType* mfMediaType = nullptr;
  GUID result = GUID_NULL;

  HRESULT hr = _mfStreamDescriptor->GetMediaTypeHandler(&mfMediaTypeHandler);
  hr = mfMediaTypeHandler->GetMediaTypeByIndex(0, &mfMediaType);
  hr = mfMediaType->GetMajorType(&result);

  if (mfMediaType) mfMediaType->Release();
  if (mfMediaTypeHandler) mfMediaTypeHandler->Release();

  return result;
}


IMFStreamDescriptor* StreamDescriptor::GetMfStreamDescriptor()
{
  return _mfStreamDescriptor;
}