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