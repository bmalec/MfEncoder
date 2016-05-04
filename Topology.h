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
  //  IMFMediaSource* _mfMediaSource;
  //  MetadataKeyValuePair *_metadata;
  //  int _metadataItemCount;
  //  void LoadMetadataFromSource();

  HRESULT AddSourceNode(
    //  IMFTopology *pTopology,           // Topology.
    IMFMediaSource *pSource,          // Media source.
    IMFPresentationDescriptor *pPD,   // Presentation descriptor.
    IMFStreamDescriptor *pSD,         // Stream descriptor.
    IMFTopologyNode **ppNode);         // Receives the node pointer.

  HRESULT Topology::AddTransformOutputNodes(
    //  IMFTopology* pTopology,
    IMFActivate* pSinkActivate,
    IMFMediaType* pSourceType,
    IMFTopologyNode **ppNode    // Receives the node pointer.
  );


  HRESULT AddOutputNode(
    //  IMFTopology *pTopology,     // Topology.
    IMFActivate *pActivate,     // Media sink activation object.
    DWORD dwId,                 // Identifier of the stream sink.
    IMFTopologyNode **ppNode);   // Receives the node pointer.

//  HRESULT PostEncodingUpdate();



protected:
  Topology(IMFTopology* mfTopology);

public:
  static Topology* Create();
  void BuildPartialTopograpy(MediaSource* source, MediaSink* sink);  // TODO this should not be separate from Create()
  HRESULT Encode();

  ~Topology();

//  IMFActivate* GetActivationObject();

  //  IMFMediaType *GetCurrentMediaType();
  //  IMFMediaSource* GetMFMediaSource();
  //  wchar_t *GetMetadataValue(wchar_t *metadataKey);
};

