#pragma once

#include "Parameters.h"

class CommandLineParser
{
public:
  static void Parse(int argc, wchar_t* argv[], Parameters* parameters);
};
