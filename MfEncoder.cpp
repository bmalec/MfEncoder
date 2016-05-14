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


int wmain(int argc, wchar_t *argv[])
{
  Parameters parameters;

  if (argc < 1)
  {
    printf("Command line help goes here...");
    return 0;
  }

  CommandLineParser::Parse(argc, argv, &parameters);

  AudioEncoderParameters* encoderParameters = AudioEncoderParameters::CreateQualityBasedVbrParameters(75, 2, 44100, 16);

  // Verify that output folder exists, if specified
  // (and add a '\' to it if it doesn't exist)

  if (*parameters.OutputFolder)
  {
    WIN32_FILE_ATTRIBUTE_DATA fileData;

    BOOL success = GetFileAttributesEx(parameters.OutputFolder, GET_FILEEX_INFO_LEVELS::GetFileExInfoStandard, &fileData);

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

    size_t outputFolderLength = wcslen(parameters.OutputFolder);

    if (outputFolderLength < MAX_PATH - 1)
    {
      if (*(parameters.OutputFolder + outputFolderLength - 1) != '\\')
      {
        *(parameters.OutputFolder + outputFolderLength) = '\\';
        *(parameters.OutputFolder + outputFolderLength + 1) = '\0';
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

    wcscpy(srcFileFolder, parameters.InputFilename);

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

    HANDLE hFindFile = FindFirstFile(parameters.InputFilename, &findData);

    if (hFindFile != INVALID_HANDLE_VALUE)
    {
      do
      {
        wcscpy(srcFileName, srcFileFolder);
        wcscat(srcFileName, findData.cFileName);

        MediaSource* mediaSource = MediaSource::Open(srcFileName);

        // if an output folder is specified, use that

        wchar_t outputFilename[MAX_PATH];

        if (*parameters.OutputFolder)
        {
          wcscpy(outputFilename, parameters.OutputFolder);
          wcscat(outputFilename, findData.cFileName);
          PathRenameExtension(outputFilename, L".wma");
        }
        else
        {
          wcscpy(outputFilename, parameters.OutputFilename);
        }

// old        MediaSink* mediaSink = MediaSink::Create(outputFilename, mediaSource, &parameters);
// slightly newer, but still old        MediaSink* mediaSink = MediaSink::Create(outputFilename, mfMediaType, &parameters);

        MediaSinkContentInfo *contentInfo = new MediaSinkContentInfo();
        contentInfo->AddStreamSink(1, encoderParameters);

        MediaSink* mediaSink = MediaSink::Create(outputFilename, contentInfo);

        //Build the encoding topology.

        Topology* topology = Topology::CreatePartialTopograpy(mediaSource, mediaSink);

        wprintf_s(L"Encoding %s\n", findData.cFileName);
        topology->Encode(&parameters);

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

