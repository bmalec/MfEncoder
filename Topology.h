#pragma once

#include "MediaSource.h"
#include "MediaSink.h"


class Topology
{
  /*
  struct MetadataKeyValuePair
  {
  wchar_t Key[40];
  wchar_t *Value;
  };
  */


private:
  IMFTopology* _mfTopology;

  HRESULT AddSourceNode(
    MediaSource* source,          // Media source.
//    IMFPresentationDescriptor *pPD,   // Presentation descriptor.
    IMFStreamDescriptor *pSD,         // Stream descriptor.
    IMFTopologyNode **ppNode);         // Receives the node pointer.

  HRESULT Topology::AddTransformOutputNodes(
//    MediaSink* mediaSink,
    IMFActivate* pSinkActivate,
//    IMFMediaType* pSourceType,
    GUID guidMajor,
    IMFTopologyNode **ppNode    // Receives the node pointer.
  );


  IMFTopologyNode* AddOutputNode(IMFActivate *mediaSinkActivate, DWORD dwId);


  void _buildPartialTopograpy(MediaSource* source, MediaSink* sink);



protected:
  Topology(IMFTopology* mfTopology);

public:
//  static Topology* Create();
  static Topology* BuildPartialTopograpy(MediaSource* source, MediaSink* sink);  // TODO this should not be separate from Create()
  HRESULT Encode();

  ~Topology();

};

