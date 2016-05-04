#pragma once


#include <mfidl.h>
//#include "MediaSource.h"
// #include <mfreadwrite.h>

class StreamDescriptor
{
  /*
  struct MetadataKeyValuePair
  {
  wchar_t Key[40];
  wchar_t *Value;
  };
  */


private:
  IMFStreamDescriptor* _mfStreamDescriptor;
  BOOL _isSelected;
    
  //  IMFMediaSource* _mfMediaSource;
  //  MetadataKeyValuePair *_metadata;
  //  int _metadataItemCount;
  //  void LoadMetadataFromSource();

protected:
//  MediaSink(IMFActivate* mfActivate);

public:
  StreamDescriptor(IMFStreamDescriptor *mfStreamDescriptor, BOOL isSelected);
  ~StreamDescriptor();

  //  IMFMediaSink* Activate();

//  IMFActivate* GetActivationObject();

  //  IMFMediaType *GetCurrentMediaType();
  //  IMFMediaSource* GetMFMediaSource();
  //  wchar_t *GetMetadataValue(wchar_t *metadataKey);
};


