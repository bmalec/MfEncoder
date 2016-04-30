#pragma once

#include <Windows.h>
#include <string>

struct Parameters
{
public:
  INT32 Bitrate = -1;
  wchar_t *Title;
  wchar_t *Artist;
  wchar_t *Album;
  wchar_t *Year;
  wchar_t *TrackNumber;
  wchar_t *Genre;
  UINT8 Quality = 0;
  wchar_t InputFilename[MAX_PATH];
  wchar_t OutputFilename[MAX_PATH];
  wchar_t OutputFolder[MAX_PATH];

  Parameters()
  {
    Title = nullptr;
    Artist = nullptr;
    Album = nullptr;
    Year = nullptr;
    TrackNumber = nullptr;
    Genre = nullptr;

    *InputFilename = '\0';
    *OutputFilename = '\0';
    *OutputFolder = '\0';
  }
};
