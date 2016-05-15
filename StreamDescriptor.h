#pragma once

#include <mfidl.h>

class StreamDescriptor
{
private:
  IMFStreamDescriptor* _mfStreamDescriptor;
  BOOL _isSelected;
    
public:
  StreamDescriptor(IMFStreamDescriptor *mfStreamDescriptor, BOOL isSelected);
  ~StreamDescriptor();

  BOOL GetIsSelected();
//  GUID GetMajorType();

  IMFStreamDescriptor* GetMfStreamDescriptor();
};


