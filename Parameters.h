#pragma once

#include <Windows.h>
#include <string>

struct Parameters
{
public:
  UINT32 Bitrate;
  PCWSTR Title;
  PCWSTR SubTitle;
  PCWSTR Artist;
  PCWSTR Composer;
  PCWSTR Comment;
  PCWSTR Album;
  PCWSTR AlbumArtist;
  PCWSTR Year;
  PCWSTR TrackNumber;
  PCWSTR Genre;
  UINT8 Quality;
  WCHAR InputFilename[MAX_PATH];
  WCHAR OutputFilename[MAX_PATH];
  WCHAR OutputFolder[MAX_PATH];

  Parameters()
  {
    Title = nullptr;
    SubTitle = nullptr;
    Artist = nullptr;
    Album = nullptr;
    Year = nullptr;
    TrackNumber = nullptr;
    Genre = nullptr;
    Comment = nullptr;
    Composer = nullptr;
    AlbumArtist = nullptr;

    Bitrate = 0;
    Quality = 75;

    *InputFilename = '\0';
    *OutputFilename = '\0';
    *OutputFolder = '\0';
  }
};
