#pragma once

#include "MediaSource.h"
#include "MediaSink.h"
#include "Parameters.h"


class Topology
{
private:
  IMFTopology* _mfTopology;
  MediaSink* _mediaSink;

  void _buildPartialTopograpy(MediaSource* source, MediaSink* sink, WORD streamNumber);



protected:
  Topology(IMFTopology* mfTopology, MediaSink* mediaSink);

public:
  static Topology* CreatePartialTopograpy(MediaSource* source, MediaSink* sink, WORD streamNumber);  // TODO this should not be separate from Create()
  HRESULT Encode(Parameters* parameters);

  ~Topology();

};

