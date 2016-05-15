#include "stdafx.h"
#include "StreamDescriptor.h"


_StreamDescriptor::_StreamDescriptor(IMFStreamDescriptor *mfStreamDescriptor, BOOL isSelected)
{
  _mfStreamDescriptor = mfStreamDescriptor;
  _isSelected = isSelected;
}


_StreamDescriptor::~_StreamDescriptor()
{
  if (_mfStreamDescriptor) _mfStreamDescriptor->Release();
}


IMFStreamDescriptor* _StreamDescriptor::GetMfStreamDescriptor()
{
  return _mfStreamDescriptor;
}