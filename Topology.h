#pragma once

#include "MediaSource.h"
#include "MediaSink.h"
#include "Parameters.h"


class Topology
{
private:
  IMFTopology* _mfTopology;
  MediaSink* _mediaSink;

  IMFTopologyNode* AddSourceNode(MediaSource* source, IMFStreamDescriptor *pSD);

  HRESULT Topology::AddTransformOutputNodes(
    MediaSink* mediaSink,
//    IMFActivate* pSinkActivate,
    GUID guidMajor,
    IMFTopologyNode **ppNode    // Receives the node pointer.
  );


  IMFTopologyNode* AddOutputNode(MediaSink* mediaSink, DWORD dwId);


  void _buildPartialTopograpy(MediaSource* source, MediaSink* sink);



protected:
  Topology(IMFTopology* mfTopology, MediaSink* mediaSink);

public:
  static Topology* CreatePartialTopograpy(MediaSource* source, MediaSink* sink);  // TODO this should not be separate from Create()
  HRESULT Encode(Parameters* parameters);

  ~Topology();

};

