#pragma once

#include <mfidl.h>

class _StreamDescriptor
{
private:
  IMFStreamDescriptor* _mfStreamDescriptor;
  BOOL _isSelected;
    
public:
  _StreamDescriptor(IMFStreamDescriptor *mfStreamDescriptor, BOOL isSelected);
  ~_StreamDescriptor();

//   BOOL GetIsSelected();

  IMFStreamDescriptor* GetMfStreamDescriptor();
};


