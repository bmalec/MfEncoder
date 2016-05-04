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
    IMFPresentationDescriptor *pPD,   // Presentation descriptor.
    IMFStreamDescriptor *pSD,         // Stream descriptor.
    IMFTopologyNode **ppNode);         // Receives the node pointer.

  HRESULT Topology::AddTransformOutputNodes(
    IMFActivate* pSinkActivate,
    IMFMediaType* pSourceType,
    IMFTopologyNode **ppNode    // Receives the node pointer.
  );


  HRESULT AddOutputNode(
    IMFActivate *pActivate,     // Media sink activation object.
    DWORD dwId,                 // Identifier of the stream sink.
    IMFTopologyNode **ppNode);   // Receives the node pointer.

  void _buildPartialTopograpy(MediaSource* source, MediaSink* sink);



protected:
  Topology(IMFTopology* mfTopology);

public:
//  static Topology* Create();
  static Topology* BuildPartialTopograpy(MediaSource* source, MediaSink* sink);  // TODO this should not be separate from Create()
  HRESULT Encode();

  ~Topology();

};

