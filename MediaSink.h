#pragma once

#include <mfidl.h>
#include "MediaSource.h"
#include "Parameters.h"

class MediaSink
{
private:
  IMFActivate *_mfActivate;

protected:
  MediaSink(IMFActivate* mfActivate);

public:
  static MediaSink *Create(const wchar_t *url, MediaSource* source, Parameters* params);
  ~MediaSink();

  IMFActivate* GetActivationObject();
};

