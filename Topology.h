#pragma once

#include "MediaSource.h"
#include "MediaSink.h"
#include "Parameters.h"


class Topology
{
private:
  IMFTopology* _mfTopology;

  IMFTopologyNode* AddSourceNode(MediaSource* source, IMFStreamDescriptor *pSD);

  HRESULT Topology::AddTransformOutputNodes(
    IMFActivate* pSinkActivate,
    GUID guidMajor,
    IMFTopologyNode **ppNode    // Receives the node pointer.
  );


  IMFTopologyNode* AddOutputNode(IMFActivate *mediaSinkActivate, DWORD dwId);


  void _buildPartialTopograpy(MediaSource* source, MediaSink* sink);



protected:
  Topology(IMFTopology* mfTopology);

public:
  static Topology* CreatePartialTopograpy(MediaSource* source, MediaSink* sink);  // TODO this should not be separate from Create()
  HRESULT Encode(Parameters* parameters);

  ~Topology();

};

