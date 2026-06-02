#pragma once

#include "../constants.h"
#include <iostream>

struct Config {
  // number of cells in x- and y-direction
  tsunami_lab::t_idx nx = 100;
  tsunami_lab::t_idx ny = 1;

  // Domainstart
  tsunami_lab::t_real domainStartX = 0.0;
  tsunami_lab::t_real domainStartY = 0.0;

  // width of the observed space in meters
  tsunami_lab::t_real width = 10.0;

  // cell size in meters
  tsunami_lab::t_real dxy = width / nx;

  // time until abortion in seconds
  tsunami_lab::t_real endTime = 5;

  // current simulation time (important for checkpoint)
  tsunami_lab::t_real simTime = 0;

  // rate at which checkpoints are written (relative to end time) (important for checkpoint)
  tsunami_lab::t_real l_checkpointRate = 0.5; 

  // derive constant time step; changes at simulation time are ignored (important for checkpoint)
  tsunami_lab::t_real l_dt = 1;

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