/**
 * @author Alexander Breuer (alex.breuer AT uni-jena.de)
 *
 * @section DESCRIPTION
 * Entry-point for simulations.
 **/
#include "patches/WavePropagation1d.h"
#include "patches/WavePropagation2d.h"
#include "setups/DamBreak1d.h"
#include "setups/ShockShock1d.h"
#include "setups/RareRare1d.h"
#include "setups/Bathymetry1d.h"
#include "setups/SubcriticalFlow1d.h"
#include "setups/HydraulicJump1d.h"
#include "setups/TsunamiEvent1d.h"
#include "setups/CircularDamBreak2d.h"
#include "setups/DamBreak2d.h"
#include "io/Csv.h"
#include "io/NetCdf.h" 
#include "io/Station.h"
#include "io/XmlReader.h"
#include <vector>
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <fstream>
#include <limits>
#include <filesystem>

int is_number(char* input);

int main( int   i_argc,
          char *i_argv[] ) {
  // number of cells in x- and y-direction
  tsunami_lab::t_idx l_nx = 100;
  tsunami_lab::t_idx l_ny = 1;

  // width of the observed space
  tsunami_lab::t_real l_w = 10.0;

  // time until abortion
  tsunami_lab::t_real l_endTime = 1.25;

  // set cell size
  tsunami_lab::t_real l_dxy = l_w / l_nx;

  // string of selected solver
  std::string l_solver = "fwave";

  // possible insanity
  std::string l_sanity = "true";

  // string of selected setup
  std::string l_setup_selection = "DamBreak1d";

  std::cout << "####################################" << std::endl;
  std::cout << "### Tsunami Lab                  ###" << std::endl;
  std::cout << "###                              ###" << std::endl;
  std::cout << "### https://scalable.uni-jena.de ###" << std::endl;
  std::cout << "####################################" << std::endl;

  // Handling input options

  for (int i = 1; i < i_argc; i++) {
    std::string arg = i_argv[i];

    // check if help is needed
    if (arg == "-h" || arg == "--help") {
      std::cout << "__________Manual__________" << std::endl;
      std::cout << std::endl;
      std::cout << "Usage: ./build/tsunami_lab <N_CELLS_X> [OPTIONS]" << std::endl;
      std::cerr << "where N_CELLS_X is the number of cells in x-direction" << std::endl;
      std::cout << std::endl;
      std::cout << "  Option       Long option         Meaning" << std::endl;
      std::cout << "  -h,          --help              Display this help and exit." << std::endl;
      std::cout << "  -s <name>,   (--solver=<name>)   Select a solver. Possible is {fwave, roe}. Default is fwave." << std::endl;
      std::cout << "  -i,          --insanity          Set flag for insanity. Default is sanity" << std::endl; // oder --sanity=true  ?
      std::cout << "  -n <number>, (--ncells=<number>) Set number of cells. Default is 100." << std::endl;
      std::cout << "  -n <number> <number>             Set number of cells in x and y direction. Defaults: x = 100, y = 1" << std::endl;
      std::cout << "  -w <number>, (--width=<number>)  Set width of the observed space in meters. default is 10" << std::endl;
      std::cout << "  -t <number>, (--time=<number>)   Set time until aborting in s. default is 1.25" << std::endl;
      std::cout << "  -S <name>,   (--setup=<name>)    Select setup to simulate. Possible is {DamBreak1d, RareRare1d, ShockShock1d, Bathymetry1d, SubcriticalFlow1d, HydraulicJump1d, TsunamiEvent1d, CircularDamBreak2d}. Default is DamBreak1d" << std::endl;
      return EXIT_SUCCESS;
    }
    // check if N_CELLS_X is passed
    else if (is_number(i_argv[i])) {
      l_nx = atoi( i_argv[i] );
      if( l_nx < 1 ) {
        std::cerr << "invalid number of cells, must be > 0" << std::endl;
        return EXIT_FAILURE;
      }
      l_dxy = l_w / l_nx;
    }
    // ckeck if a solver is specified
    else if (arg == "-s" && i + 1 < i_argc) {  // || arg == "--solver=<>"
      l_solver = i_argv[++i]; // nächstes argument lesen

      if (l_solver != "fwave" && l_solver != "roe") {
        std::cout << "tsunami_lab: invalid solver '" << i_argv[i] << "'" << std::endl;
        std::cout << "Try 'tsunami_lab --help' for more information." << std::endl;
        return EXIT_FAILURE;
      }
    }
    // ckeck if number of cells is specified only in x direction
    else if (arg == "-n" && i + 1 < i_argc) {  // || arg == "--ncells=<>"
      if (is_number(i_argv[++i])) {
        l_nx = atoi( i_argv[i] );
        if( l_nx < 1 ) {
          std::cerr << "invalid number of cells, must be > 0" << std::endl;
          return EXIT_FAILURE;
        }
        l_dxy = l_w / l_nx;
      }
    }
    // check if number of cells is specified for x and y direction
    else if (arg == "-n" && i + 2 < i_argc) {  // || arg == "--ncells=<>"
      if (is_number(i_argv[++i]) && is_number(i_argv[++i])) {
        l_nx = atoi(i_argv[i - 1]);
        l_ny = atoi(i_argv[i]);
        if(l_nx < 1 || l_ny < 1) {
          std::cerr << "invalid number of cells, must be > 0" << std::endl;
          return EXIT_FAILURE;
        }
        l_dxy = l_w / l_nx;
      }
    }
    // ckeck if observed width is specified
    else if (arg == "-w" && i + 1 < i_argc) {  // || arg == "--width=<>"
      if (is_number(i_argv[++i])) {
        l_w = atoi( i_argv[i] );
        if( l_w <= 0 ) {
          std::cerr << "invalid width, must be > 0" << std::endl;
          return EXIT_FAILURE;
        }
        l_dxy = l_w / l_nx;
      }
    }
    // ckeck if max time is specified
    else if (arg == "-t" && i + 1 < i_argc) {  // || arg == "--time=<>"
      if (is_number(i_argv[++i])) {
        l_endTime = atoi( i_argv[i] );
        if( l_endTime <= 0 ) {
          std::cerr << "invalid time, must be > 0" << std::endl;
          return EXIT_FAILURE;
        }
      }
    }
    // ckeck if a setup is specified
    else if (arg == "-S" && i + 1 < i_argc) {  // || arg == "--solver=<>"
      l_setup_selection = i_argv[++i]; // nächstes argument lesen
      if (l_setup_selection != "DamBreak1d" && l_setup_selection != "RareRare1d" 
                                            && l_setup_selection != "ShockShock1d"  
                                            && l_setup_selection != "Bathymetry1d"
                                            && l_setup_selection != "SubcriticalFlow1d"
                                            && l_setup_selection != "HydraulicJump1d"
                                            && l_setup_selection != "TsunamiEvent1d"
                                            && l_setup_selection != "CircularDamBreak2d"
                                            && l_setup_selection != "DamBreak2d"
                                            && l_setup_selection != "ArtificialTsunami2d"
                                          ) {
        std::cout << "tsunami_lab: invalid setup '" << i_argv[i] << "'" << std::endl;
        std::cout << "Try 'tsunami_lab --help' for more information." << std::endl;
        return EXIT_FAILURE;
      }
    }
    // check if insanity is wanted
    else if (arg == "-i" || arg == "--insanity") {
      l_sanity = "false";
    }
    // handling unknown argument
    else {
      std::cout << "tsunami_lab: invalid option '" << i_argv[i] << "'" << std::endl;
      std::cout << "Try 'tsunami_lab --help' for more information." << std::endl;
      return EXIT_FAILURE;
    }
  }

  // notify user about selectet configuration
  std::cout << "runtime configuration" << std::endl;
  std::cout << "  number of cells in x-direction: " << l_nx << std::endl;
  std::cout << "  number of cells in y-direction: " << l_ny << std::endl;
  std::cout << "  cell size:                      " << l_dxy << std::endl;
  std::cout << "  sanity:                         " << l_sanity << std::endl;
  std::cout << "  selected solver:                " << l_solver << std::endl;
  std::cout << "  selected time:                  " << l_endTime << std::endl;

  // set output directory
  std::filesystem::path outDir = "solutions";


  // ensure output directory exists
  if (!std::filesystem::exists(outDir)) {
    std::filesystem::create_directory(outDir);
  } else {
    // delete old solution files
    for (const auto& entry : std::filesystem::directory_iterator(outDir)) {
      if (entry.is_regular_file()) {
        auto name = entry.path().filename().string();

        // only delete solution files
        if (name.rfind("solution_", 0) == 0 && entry.path().extension() == ".csv") {
          std::filesystem::remove(entry.path());
        }
      }
    }
  }

  // set output path for NetCdf
  std::string l_ncPath = "solutions/netcdf_output.nc";   

  // construct setup
  tsunami_lab::setups::Setup *l_setup;
  tsunami_lab::setups::TsunamiEvent1d *l_tsunami = nullptr; 
  // set if simulation is 2D as to keep 1D-simulations working
  bool l_is2D = false;

  if (l_sanity == "true"){
    if (l_setup_selection == "DamBreak1d") { // könnte man auch gleich oben in der argumentübergabe machen?
      l_setup = new tsunami_lab::setups::DamBreak1d( 10,    // height left
                                                      5,    // height right
                                                      5 );  // dam location
    } else if (l_setup_selection == "RareRare1d") {
      l_setup = new tsunami_lab::setups::RareRare1d( 10,    // height 
                                                     10,    // momentum
                                                     (l_w / 2) );   // middle location
    } else if (l_setup_selection == "ShockShock1d") {
      l_setup = new tsunami_lab::setups::ShockShock1d( 10,    // height
                                                       10,    // momentum
                                                        5 );  // impact location
    } else if (l_setup_selection == "Bathymetry1d") {
      l_setup = new tsunami_lab::setups::Bathymetry1d( 12,          // height left
                                                       7,          // height right
                                                       (l_w / 3),   // dam location
                                                       0,           // bathymetry left
                                                       5,           // bathymetry right
                                                       ((2 * l_w) / 3) );   // bathymetry change location
    } else if (l_setup_selection == "SubcriticalFlow1d") {
      l_setup = new tsunami_lab::setups::SubcriticalFlow1d( l_w / 2,   // bump location in the middle of the simulation
                                                            0.8,       // bump height: 0.8m 
                                                            0.5,       // bump width: 0.5m
                                                            2.0,       // water surface at 2m, well above the 0.8m bump
                                                            1.5        // constant momentum (positive means left to right)
                                                          );
    } else if (l_setup_selection == "HydraulicJump1d") {
      l_setup = new tsunami_lab::setups::HydraulicJump1d( -0.13, // obstacle height
                                                          -0.33, // base height of the bathymetry
                                                          0.18  // momentum of the water
                                                        );
    } else if (l_setup_selection == "TsunamiEvent1d") {
      l_tsunami = new tsunami_lab::setups::TsunamiEvent1d( "src/bathymetry/output/03_dem_03.csv"
                                                          );
      l_setup = l_tsunami;
      
      // Use actual domain width from the CSV x-coordinates
      l_w   = l_tsunami->getDomainWidth();
      l_dxy = l_w / l_nx;

    } else if (l_setup_selection == "CircularDamBreak2d") {
      l_w   = 100.0;
      l_ny  = l_nx;
      l_dxy = l_w / l_nx;
      l_setup = new tsunami_lab::setups::CircularDamBreak2d();

    } else if (l_setup_selection == "DamBreak2d") {
      l_ny  = l_nx;
      l_setup = new tsunami_lab::setups::DamBreak2d(15, // height water dam
                                                    10, // height water
                                                    2.5, // position of dam X
                                                    5,  // position of dam Y
                                                    2.5); // radius of dam
    } else if (l_setup_selection == "ArtificialTsunami2d") {
      l_ny = l_nx;
      l_setup = new tsunami_lab::setups::ArtificialTsunami2d(10.0, 
                                                            15.0);

    } else {
      std::cerr << "Somethings wrong. Did you add the setup_selection?" << std::endl;
      return EXIT_FAILURE;
    }
  }
  // very rudimentary sanity-check, to be expanded later
  else if (l_sanity == "false") {
  l_setup = new tsunami_lab::setups::ShockShock1d( 8899.326826472694,
                                                   122.0337839252433,
                                                   5 );
  } else {
        std::cerr << "Still sane?" << std::endl;
        return EXIT_FAILURE;
      }


  // check if setup is 2d ("2d" HAS TO BE INCLUDED IN SETUP NAME)
  l_is2D = false;
  int l_stringLength = l_setup_selection.length();
  for (int i = 1; i < l_stringLength; i++) {
    if (l_setup_selection[i-1] == '2' && l_setup_selection[i] == 'd') {
      l_is2D = true;
      break;
    }
  }

 // construct solver
 tsunami_lab::patches::WavePropagation *l_waveProp;
  if( l_is2D ) {
    l_waveProp = new tsunami_lab::patches::WavePropagation2d( l_nx, l_ny );
  } else {
    l_waveProp = new tsunami_lab::patches::WavePropagation1d( l_nx, l_solver );
  }

  // maximum observed height in the setup
  tsunami_lab::t_real l_hMax = std::numeric_limits< tsunami_lab::t_real >::lowest();

  // set up solver
  for( tsunami_lab::t_idx l_cy = 0; l_cy < l_ny; l_cy++ ) {

    // old implementation just in case
    //tsunami_lab::t_real l_y = l_cy * l_dxy; 
    //tsunami_lab::t_real l_domainStart = (l_tsunami != nullptr) ? l_tsunami->getDomainStart() : 0.0;
    
    // in case of 2d, we need to shift the domain by 50
    tsunami_lab::t_real l_domainStartX = l_is2D ? -50.0 : 0.0;
    tsunami_lab::t_real l_domainStartY = l_is2D ? -50.0 : 0.0;

    //tsunami_lab::t_real l_domainStartX = 0.0;
    //tsunami_lab::t_real l_domainStartY = 0.0;

    for( tsunami_lab::t_idx l_cx = 0; l_cx < l_nx; l_cx++ ) {
        tsunami_lab::t_real l_x = (l_cx + 0.5) * l_dxy + l_domainStartX;
        tsunami_lab::t_real l_y = l_is2D ? (l_cy + 0.5) * l_dxy + l_domainStartY : l_cy * l_dxy;
          

      // get initial values of the setup
      tsunami_lab::t_real l_h = l_setup->getHeight( l_x,
                                                    l_y );
      l_hMax = std::max( l_h, l_hMax );

      tsunami_lab::t_real l_hu = l_setup->getMomentumX( l_x,
                                                        l_y );

      tsunami_lab::t_real l_hv = l_setup->getMomentumY( l_x,
                                                        l_y );

      tsunami_lab::t_real l_b = l_setup->getBathymetry( l_x,
                                                        l_y );

      tsunami_lab::t_real l_depth = std::max( tsunami_lab::t_real(0), l_h - l_b );

      // set initial values in wave propagation solver
      l_waveProp->setHeight( l_cx,
                             l_cy,
                             l_depth );

      l_waveProp->setMomentumX( l_cx,
                                l_cy,
                                l_hu );

      l_waveProp->setMomentumY( l_cx,
                                l_cy,
                                l_hv );
                                
      l_waveProp->setBathymetry( l_cx,
                                 l_cy,
                                 l_b );

    }
  }

  // derive maximum wave speed in setup; the momentum is ignored
  tsunami_lab::t_real l_speedMax = std::sqrt( 9.81 * l_hMax );

  // derive constant time step; changes at simulation time are ignored
  tsunami_lab::t_real l_dt = 0.5 * l_dxy / l_speedMax;

  // derive scaling for a time step
  tsunami_lab::t_real l_scaling = l_dt / l_dxy;

  // set up time and print control
  tsunami_lab::t_idx  l_timeStep = 0;
  tsunami_lab::t_idx  l_nOut = 0;
  //tsunami_lab::t_real l_endTime = 1.25; is set above
  tsunami_lab::t_real l_simTime = 0;

  std::cout << "entering time loop" << std::endl;

  // Load stations from xml
  std::vector<tsunami_lab::io::StationConfig> stationConfigs;
  tsunami_lab::io::OutputConfig outputConfig;
  std::string stationsXmlPath = "src/io/stations.xml";
  tsunami_lab::io::loadXmlConfig(stationsXmlPath, stationConfigs, outputConfig);
  std::vector<tsunami_lab::io::Station> stations;
  for (const auto& stationConfig : stationConfigs) {
    // construct Object directly into vector
    stations.emplace_back(stationConfig.x,
                          stationConfig.y,
                          outputConfig.interval,
                          stationConfig.name,
                          l_nx,
                          outputConfig.path);
  }

  // construct writer for NetCdf
  tsunami_lab::io::NetCdf l_ncWriter( l_ncPath,                                         
                                      l_nx,                                             
                                      l_is2D ? l_ny : 1,          // pass 1 for 1D runs
                                      l_dxy,                                            
                                      l_waveProp->getStride(),                          
                                      l_waveProp->getBathymetry()                                     
);     

  // iterate over time
  while( l_simTime < l_endTime ) {
      if( l_timeStep % 25 == 0 ) {
          std::cout << "  simulation time / #time steps: "
                    << l_simTime << " / " << l_timeStep << std::endl;

          std::string l_path = (outDir / ("solution_" + std::to_string(l_nOut) + ".csv")).string();
          std::cout << "  writing wave field to " << l_path << std::endl;

          std::ofstream l_file;
          l_file.open( l_path );

        //*
          if( l_is2D ) {
              tsunami_lab::io::Csv::write( l_dxy, l_nx, l_ny,
                                          l_waveProp->getStride(),
                                          l_waveProp->getBathymetry(),
                                          l_waveProp->getHeight(),
                                          l_waveProp->getMomentumX(),
                                          l_waveProp->getMomentumY(),
                                          l_file );
          } else {
              tsunami_lab::io::Csv::write( l_dxy, l_nx, 1, 1,
                                          l_waveProp->getBathymetry(),
                                          l_waveProp->getHeight(),
                                          l_waveProp->getMomentumX(),
                                          nullptr,
                                          l_file );
          }
          l_file.close();
      //

          // netCDF write
          l_ncWriter.write( l_simTime,
                            l_waveProp->getHeight(),
                            l_waveProp->getMomentumX(),
                            l_is2D ? l_waveProp->getMomentumY() : nullptr );

          l_nOut++;
      }                                                                   



    /**  instead of: l_waveProp->setGhostOutflow(); we now check if boundaries are outflow or reflecting
    *    
    *   OLD: it only checked if the actual depth of the water is lower than 200m.
    
    // checks if the depth of the water on the left is smaller than 20 meters, by checking the middle of the left-outermost cell
    bool l_leftReflecting = l_setup->getHeight( 0.5 * l_dxy, 0 ) < 200;
    // checks if the depth of the water on the left is smaller than 20 meters, by checking the middle of the right-outermost cell
    bool l_rightReflecting = l_setup->getHeight( (l_nx - 0.5) * l_dxy, 0 ) < 200;

    l_waveProp->setGhostCells( l_leftReflecting, l_rightReflecting );
    */

    // New setup for ghost-cells with false defaults in the setup.h
    l_waveProp->setGhostCells(
      l_setup->isLeftBoundaryReflecting(),
      l_setup->isRightBoundaryReflecting(),
      l_setup->isBottomBoundaryReflecting(),
      l_setup->isTopBoundaryReflecting()
    );

    l_waveProp->timeStep(l_scaling);

    // Station Updates
    for (auto& station : stations) {
      // nullptr mit getMomentumY tasuchen sobald geht
      station.timeStep(l_dt, l_waveProp->getHeight(), l_waveProp->getMomentumX(), nullptr);
    }

    l_timeStep++;
    l_simTime += l_dt;
  }

  std::cout << "finished time loop" << std::endl;

  // free memory
  std::cout << "freeing memory" << std::endl;
  delete l_setup;
  delete l_waveProp;

  std::cout << "finished, exiting" << std::endl;
  return EXIT_SUCCESS;
}

int is_number(char* input) {
  char* end;
  errno = 0;

  //long val = std::strtol(input, &end, 10);
  std::strtol(input, &end, 10);

  // Validierung
  if (end == input || *end != '\0' || errno != 0) {
    return 0;
  }
  return 1;
}