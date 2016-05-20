#pragma once

#include "MediaSource.h"
#include "MediaSink.h"
#include "AudioEncoder.h"


class _Topology
{
private:
  IMFTopology* _mfTopology;
  IMFTransform* _audioEncoder;
  MediaSink* _mediaSink;


  void _buildPartialTopograpy(MediaSource* source, MediaSink* sink, WORD streamNumber);

  HRESULT UpdateVbrStreamProperties(IMFMediaSession *mfMediaSession, MediaSink* mediaSink);



protected:
  _Topology(IMFTopology* mfTopology, MediaSink* mediaSink);

public:
  static _Topology* CreatePartialTopograpy(MediaSource* source, MediaSink* sink, WORD streamNumber);  // TODO this should not be separate from Create()
  HRESULT Encode(AudioEncoderParameters* encoderParameters);

  ~_Topology();

};

