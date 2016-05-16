// MfEncoder.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <mfapi.h>
#include <Shlwapi.h>
#include "CommandLineParser.h"
#include "Parameters.h"
#include "MediaSource.h"
#include "MediaSink.h"
#include "Topology.h"
#include "AudioEncoder.h"
#include "AsfContentInfoBuilder.h"


static void SetMediaSinkContentInfoMetadata(AsfContentInfoBuilder* contentInfo, MediaSource* mediaSource, Parameters* commandLineParameters)
{
  // please work

  wchar_t *value;

  value = commandLineParameters->Album;

  if (!value)
  {
    value = mediaSource->GetMetadataValue(L"WM/AlbumTitle");
  }
  
  if (value)
  {
    contentInfo->SetMetadataAsString(L"WM/AlbumTitle", value);
  }

  value = commandLineParameters->Artist;

  if (!value)
  {
    value = mediaSource->GetMetadataValue(L"Author");
  }

  if (value)
  {
    contentInfo->SetMetadataAsString(L"Author", value);
  }

  value = commandLineParameters->Artist;

  if (!value)
  {
    value = mediaSource->GetMetadataValue(L"WM/AlbumArtist");
  }

  if (value)
  {
    contentInfo->SetMetadataAsString(L"WM/AlbumArtist", value);
  }

  value = commandLineParameters->Genre;

  if (!value)
  {
    value = mediaSource->GetMetadataValue(L"WM/Genre");
  }

  if (value)
  {
    contentInfo->SetMetadataAsString(L"WM/Genre", value);
  }

  value = commandLineParameters->Title;

  if (!value)
  {
    value = mediaSource->GetMetadataValue(L"Title");
  }

  if (value)
  {
    contentInfo->SetMetadataAsString(L"Title", value);
  }

  value = commandLineParameters->TrackNumber;

  if (!value)
  {
    value = mediaSource->GetMetadataValue(L"WM/TrackNumber");
  }

  if (value)
  {
    contentInfo->SetMetadataAsString(L"WM/TrackNumber", value);
  }

  value = commandLineParameters->Year;

  if (!value)
  {
    value = mediaSource->GetMetadataValue(L"WM/Year");
  }

  if (value)
  {
    contentInfo->SetMetadataAsString(L"WM/Year", value);
  }
}


int wmain(int argc, wchar_t *argv[])
{
  Parameters commandLineParameters;

  if (argc < 1)
  {
    printf("Command line help goes here...");
    return 0;
  }

  CommandLineParser::Parse(argc, argv, &commandLineParameters);

  AudioEncoderParameters* encoderParameters = nullptr;

  if (commandLineParameters.Quality == 100)
  {
    encoderParameters = AudioEncoderParameters::CreateLosslessEncoderParameters(2, 44100, 16);
  }
  else
  {
    encoderParameters = AudioEncoderParameters::CreateQualityBasedVbrParameters(commandLineParameters.Quality, 2, 44100, 16);
  }

  // Verify that output folder exists, if specified
  // (and add a '\' to it if it doesn't exist)

  if (*commandLineParameters.OutputFolder)
  {
    WIN32_FILE_ATTRIBUTE_DATA fileData;

    BOOL success = GetFileAttributesEx(commandLineParameters.OutputFolder, GET_FILEEX_INFO_LEVELS::GetFileExInfoStandard, &fileData);

    // check if the file system object exists, but it's not a directory...

    if (success && ((fileData.dwFileAttributes & 0x10) == 0))
    {
      printf("Specified output directory is not a directory");
      return 0;
    }

    if (!success)
    {
      printf("Specified output directory does not exist");
      return 0;
    }

    size_t outputFolderLength = wcslen(commandLineParameters.OutputFolder);

    if (outputFolderLength < MAX_PATH - 1)
    {
      if (*(commandLineParameters.OutputFolder + outputFolderLength - 1) != '\\')
      {
        *(commandLineParameters.OutputFolder + outputFolderLength) = '\\';
        *(commandLineParameters.OutputFolder + outputFolderLength + 1) = '\0';
      }
    }
  }

  try
  {
    // Initialize COM & Media Foundation

    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    HRESULT hr = MFStartup(MF_VERSION);

//    IMFMediaType* mfMediaType = AudioEncoder::GetQualityBasedMediaType(parameters.Quality);

    // Use the Windows shell API to extract the path component from the input filename

    wchar_t srcFileFolder[MAX_PATH];
    wchar_t srcFileName[MAX_PATH];

    wcscpy(srcFileFolder, commandLineParameters.InputFilename);

    BOOL ret = PathRemoveFileSpec(srcFileFolder);

    size_t srcFolderLength = wcslen(srcFileFolder);

    if (srcFolderLength < MAX_PATH - 1)
    {
      if (srcFolderLength > 0)
      {
        if (*(srcFileFolder + srcFolderLength - 1) != '\\')
        {
          *(srcFileFolder + srcFolderLength) = '\\';
          *(srcFileFolder + srcFolderLength + 1) = '\0';
        }
      }
    }

    // do some basic parsing of input filename, as FirstFirstFile / FindNext 
    // does not return the full path so we'll have to prepend
    // any directory info specified

    WIN32_FIND_DATA findData;

    HANDLE hFindFile = FindFirstFile(commandLineParameters.InputFilename, &findData);

    if (hFindFile != INVALID_HANDLE_VALUE)
    {
      do
      {
        wcscpy(srcFileName, srcFileFolder);
        wcscat(srcFileName, findData.cFileName);

        MediaSource* mediaSource = MediaSource::Open(srcFileName);

        // if an output folder is specified, use that

        wchar_t outputFilename[MAX_PATH];

        if (*commandLineParameters.OutputFolder)
        {
          wcscpy(outputFilename, commandLineParameters.OutputFolder);
          wcscat(outputFilename, findData.cFileName);
          PathRenameExtension(outputFilename, L".wma");
        }
        else
        {
          wcscpy(outputFilename, commandLineParameters.OutputFilename);
        }

        AsfContentInfoBuilder *contentInfo = new AsfContentInfoBuilder();
        contentInfo->AddStreamSink(1, encoderParameters);
        SetMediaSinkContentInfoMetadata(contentInfo, mediaSource, &commandLineParameters);

        MediaSink* mediaSink = MediaSink::Create(outputFilename, contentInfo->ConstructMfAsfContentInfo());

        //Build the encoding topology.

        Topology* topology = Topology::CreatePartialTopograpy(mediaSource, mediaSink, 1);

        wprintf_s(L"Encoding %s\n", findData.cFileName);
        topology->Encode(encoderParameters);

        delete topology;
        delete mediaSink;
        delete mediaSource;

      } while (FindNextFile(hFindFile, &findData));

      FindClose(hFindFile);
    }
    else
    {
      // input file does not exit

      throw std::invalid_argument("Input filename does not exist");
    }

    MFShutdown();
    CoUninitialize();
  }
  catch (std::exception &ex)
  {
    printf("ERROR: %s\n", ex.what());
  }


    return 0;
}

