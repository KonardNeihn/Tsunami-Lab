#pragma once

#include "../constants.h"
#include <iostream>

struct Config {
  // number of cells in x- and y-direction
  tsunami_lab::t_idx nx = 100;
  tsunami_lab::t_idx ny = 1;

  // width of the observed space in meters
  tsunami_lab::t_real width = 10.0;

  // cell size in meters
  tsunami_lab::t_real dxy = width / nx;

  // time until abortion in seconds
  tsunami_lab::t_real endTime = 5;

  // selected solver
  std::string solver = "fwave";

  // selected setup
  std::string setup = "DamBreak1d";

  // possible insanity
  bool insanity = false;

  // if setup is 2d
  bool is_2d = false;
};

Config parseArgs(int argc, char** argv);