#pragma once

#include "Parameters.h"

class CommandLineParser
{
public:
  static void Parse(int argc, PWSTR argv[], Parameters* parameters);
};
