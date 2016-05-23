#pragma once

#include <Windows.h>
#include <string>

struct Parameters
{
public:
  INT32 Bitrate = -1;
  LPWSTR Title;
  LPWSTR SubTitle;
  LPWSTR Artist;
  LPWSTR Composer;
  LPWSTR Comment;
  LPWSTR Album;
  LPWSTR AlbumArtist;
  LPWSTR Year;
  LPWSTR TrackNumber;
  LPWSTR Genre;
  UINT8 Quality = 0;
  wchar_t InputFilename[MAX_PATH];
  wchar_t OutputFilename[MAX_PATH];
  wchar_t OutputFolder[MAX_PATH];

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

    Bitrate = -1;
    Quality = 75;

    *InputFilename = '\0';
    *OutputFilename = '\0';
    *OutputFolder = '\0';
  }
};
