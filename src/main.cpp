/**
 * @author Alexander Breuer (alex.breuer AT uni-jena.de)
 *
 * @section DESCRIPTION
 * Entry-point for simulations.
 **/
#include "patches/WavePropagation1d.h"
#include "patches/WavePropagation2d.h"
<<<<<<< HEAD
#include "setups/DamBreak1d.h"
#include "setups/ShockShock1d.h"
#include "setups/RareRare1d.h"
#include "setups/Bathymetry1d.h"
#include "setups/SubcriticalFlow1d.h"
#include "setups/HydraulicJump1d.h"
#include "setups/TsunamiEvent1d.h"
#include "setups/CircularDamBreak2d.h"
#include "setups/DamBreak2d.h"
#include "setups/ArtificialTsunami2d.h"
#include "setups/TsunamiEvent2d.h"
#include "setups/ChileEvent2d.h"
#include "setups/TohokuEvent2d.h"
#include "setups/Checkpoint2d.h"
=======
>>>>>>> 6d14a054b16f82be185f631a312d7e05f874f2ee
#include "io/Csv.h"
#include "io/NetCdf.h" 
#include "io/NetCdfCheckpoint.h"
#include "io/Station.h"
#include "io/XmlReader.h"
#include <vector>
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <fstream>
#include <limits>
#include <filesystem>
#include "config/Config.h"
#include "factory/SetupFactory.h"

int is_number(char* input);

int main( int   i_argc,
          char *i_argv[] ) {
  // number of cells in x- and y-direction
  tsunami_lab::t_idx l_nx = 100;
  tsunami_lab::t_idx l_ny = 100;

  // width of the observed space
  tsunami_lab::t_real l_w = 10.0;

  // time until abortion
  tsunami_lab::t_real l_endTime = 1.25;

  // current simulation time
  tsunami_lab::t_real l_simTime = 0;

  // set cell size
  tsunami_lab::t_real l_dxy = l_w / l_nx;

  // Domainstart
  tsunami_lab::t_real l_domainStartX = 0.0;
  tsunami_lab::t_real l_domainStartY = 0.0;

  // rate at which checkpoints are written (relative to end time)
  tsunami_lab::t_real l_checkpointRate = 0.5; 

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
  Config l_config = parseArgs(i_argc, i_argv);

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
      std::cout << "  -S <name>,   (--setup=<name>)    Select setup to simulate. Possible is {DamBreak1d, RareRare1d, ShockShock1d, Bathymetry1d, SubcriticalFlow1d, HydraulicJump1d, TsunamiEvent1d, CircularDamBreak2d, DamBreak2d, ArtificialTsunami2d, TsunamiEvent2d, ChileEvent2d, TohokuEvent2d}. Default is DamBreak1d" << std::endl;
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
                                            && l_setup_selection != "TsunamiEvent2d"
                                            && l_setup_selection != "ChileEvent2d"
                                            && l_setup_selection != "TohokuEvent2d"
                                            && l_setup_selection != "Checkpoint2d"
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
  // construct setup
  auto l_setup = createSetup(l_config);

  // notify user about selectet configuration
  std::cout << "runtime configuration" << std::endl;
  std::cout << "  number of cells in x-direction: " << l_config.nx << std::endl;
  std::cout << "  number of cells in y-direction: " << l_config.ny << std::endl;
  std::cout << "  cell size:                      " << l_config.dxy << std::endl;
  std::cout << "  sanity:                         " << l_config.insanity << std::endl;
  std::cout << "  selected solver:                " << l_config.solver << std::endl;
  std::cout << "  selected time:                  " << l_config.endTime << std::endl;


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
      l_w = 1000;
      l_dxy = l_w / l_nx;
      l_setup = new tsunami_lab::setups::ArtificialTsunami2d(-100.0, 
                                                            0);
    } else if (l_setup_selection == "TsunamiEvent2d") {
      l_ny = l_nx;
      l_w   = 10.0;
      l_dxy = l_w / l_nx;
      l_setup = new tsunami_lab::setups::TsunamiEvent2d("src/NetCdf/artificialtsunami_bathymetry_1000.nc",
                                                        "src/NetCdf/artificialtsunami_displ_1000.nc");
    } else if (l_setup_selection == "ChileEvent2d") {
      l_nx = 100;
      l_ny = l_nx;
      l_w   = l_nx;
      l_dxy = l_w / l_nx;
      l_setup = new tsunami_lab::setups::ChileEvent2d("src/bathymetry/output/chile_gebco20_usgs_250m_bath_fixed.nc",
                                                        "src/bathymetry/output/chile_gebco20_usgs_250m_displ_fixed.nc",
                                                      l_nx, l_ny);

    } else if (l_setup_selection == "TohokuEvent2d") {
      l_nx = 1000;
      l_ny = l_nx;
      l_w   = l_nx;
      l_dxy = l_w / l_nx;
      l_setup = new tsunami_lab::setups::TohokuEvent2d("src/bathymetry/output/tohoku_gebco20_usgs_250m_bath.nc",
                                                        "src/bathymetry/output/tohoku_gebco20_usgs_250m_displ.nc",
                                                      l_nx, l_ny);
     } else if (l_setup_selection == "Checkpoint2d") {
        auto l_checkpointSetup = new tsunami_lab::setups::Checkpoint2d("solutions/checkpoint.nc");
        l_nx = l_checkpointSetup->getNX();
        l_ny = l_checkpointSetup->getNY();
        l_w = l_checkpointSetup->getWidth();
        l_dxy = l_w / l_nx;
        l_domainStartX = l_checkpointSetup->getDomainStartX();
        l_domainStartY = l_checkpointSetup->getDomainStartY();
        l_simTime = l_checkpointSetup->getLastTimeStep();
        l_endTime = l_checkpointSetup->getEndTime();
        l_setup = l_checkpointSetup;
     }
    
    else {
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
  if( l_config.is_2d ) {
    l_waveProp = new tsunami_lab::patches::WavePropagation2d( l_config.nx, l_config.ny );
  } else {
    l_waveProp = new tsunami_lab::patches::WavePropagation1d( l_config.nx, l_config.solver );
  }

  // maximum observed height in the setup
  tsunami_lab::t_real l_hMax = std::numeric_limits< tsunami_lab::t_real >::lowest();

  // set up solver
  for( tsunami_lab::t_idx l_cy = 0; l_cy < l_config.ny; l_cy++ ) {

    for( tsunami_lab::t_idx l_cx = 0; l_cx < l_nx; l_cx++ ) {
        tsunami_lab::t_real l_x = (l_cx + 0.5) * l_dxy + l_domainStartX;
        tsunami_lab::t_real l_y = l_is2D ? (l_cy + 0.5) * l_dxy + l_domainStartY : l_cy * l_dxy;
    tsunami_lab::t_real l_domainStartX = 0.0;
    tsunami_lab::t_real l_domainStartY = 0.0;

    for( tsunami_lab::t_idx l_cx = 0; l_cx < l_config.nx; l_cx++ ) {

      // l_x and l_y is in meters
      tsunami_lab::t_real l_x = (l_cx + 0.5) * l_config.dxy + l_domainStartX;
      tsunami_lab::t_real l_y = l_config.is_2d ? (l_cy + 0.5) * l_config.dxy + l_domainStartY : l_cy * l_config.dxy;
          

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

      // tsunami_lab::t_real l_depth = std::max( tsunami_lab::t_real(0), l_h - l_b );

      // set initial values in wave propagation solver
      l_waveProp->setHeight( l_cx,
                             l_cy,
                             l_h );

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
  tsunami_lab::t_real l_dt = 0.5 * l_config.dxy / l_speedMax;

  // derive scaling for a time step
  tsunami_lab::t_real l_scaling = l_dt / l_config.dxy;

  // set up time and print control
  tsunami_lab::t_idx  l_timeStep = 0;
  tsunami_lab::t_idx  l_nOut = 0;
  //tsunami_lab::t_real l_endTime = 1.25; is set above

  std::cout << "entering time loop" << std::endl;

  // Load stations from xml
  std::vector<tsunami_lab::io::StationConfig> stationConfigs;
  tsunami_lab::io::OutputConfig outputConfig;
  std::string stationsXmlPath = "src/io/stations.xml";
  tsunami_lab::io::loadXmlConfig(stationsXmlPath, stationConfigs, outputConfig);
  std::vector<tsunami_lab::io::Station> stations;
  if(l_config.is_2d) {
    for (const auto& stationConfig : stationConfigs) {
      // construct Object directly into vector
      stations.emplace_back(stationConfig.x,
                            stationConfig.y,
                            outputConfig.interval,
                            stationConfig.name,
                            l_config.nx,
                            outputConfig.path);
    }
  }
  

  // construct writer for NetCdf
  tsunami_lab::io::NetCdf l_ncWriter( l_ncPath,                                         
                                      l_config.nx,                                             
                                      l_config.is_2d ? l_config.ny : 1,          // pass 1 for 1D runs
                                      l_config.dxy,                                            
                                      l_waveProp->getStride(),                          
                                      l_waveProp->getBathymetry()                                     
);     

tsunami_lab::io::NetCdfCheckpoint l_checkpoint;
tsunami_lab::t_real l_checkpointTimer = 0.0;

  // iterate over time
  while( l_simTime < l_config.endTime ) {
      if( l_timeStep % 25 == 0 ) {
          std::cout << "  simulation time: " << l_simTime << " time steps: " << l_timeStep << std::endl;

<<<<<<< HEAD
          //std::string l_path = (outDir / ("solution_" + std::to_string(l_nOut) + ".csv")).string();
          //std::cout << "  writing wave field to " << l_path << std::endl;
        /*
          std::ofstream l_file;
          l_file.open( l_path );

        
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
      */
=======
          std::string l_path = (outDir / ("solution_" + std::to_string(l_nOut) + ".csv")).string();
          std::cout << "  writing wave field to " << l_path << std::endl;
>>>>>>> 6d14a054b16f82be185f631a312d7e05f874f2ee

          // netCDF write
          l_ncWriter.write( l_simTime,
                            l_waveProp->getHeight(),
                            l_waveProp->getMomentumX(),
                            l_config.is_2d ? l_waveProp->getMomentumY() : nullptr );

          l_nOut++;
      }                                                                   



    /**  instead of: l_waveProp->setGhostOutflow(); we now check if boundaries are outflow or reflecting
    *    
    *   OLD: it only checked if the actual depth of the water is lower than 200m.
    
    // checks if the depth of the water on the left is smaller than 20 meters, by checking the middle of the left-outermost cell
    bool l_leftReflecting = l_setup->getHeight( 0.5 * l_config.dxy, 0 ) < 200;
    // checks if the depth of the water on the left is smaller than 20 meters, by checking the middle of the right-outermost cell
    bool l_rightReflecting = l_setup->getHeight( (l_config.nx - 0.5) * l_config.dxy, 0 ) < 200;

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
      station.timeStep(l_dt, l_waveProp->getHeight(), l_waveProp->getMomentumX(), l_waveProp->getMomentumY());
    }

    // Handle Checkpoint
    if (l_checkpointTimer >= l_checkpointRate) {

      std::string checkpointPath = "solutions/checkpoint.nc";
      std::cout << "Creating checkpoint to " << checkpointPath << std::endl;
      l_checkpoint.createCheckpoint(checkpointPath, l_nx, l_ny, l_simTime, l_endTime, l_w, l_domainStartX, l_domainStartY);
      
      l_checkpoint.write2DVariable(
        checkpointPath,
        "height",
        l_waveProp->getHeight(),
        l_nx,
        l_ny
      );

      l_checkpoint.write2DVariable(
          checkpointPath,
          "bathymetry",
          l_waveProp->getBathymetry(),
          l_nx,
          l_ny
      );

      l_checkpoint.write2DVariable(
          checkpointPath,
          "momentumX",
          l_waveProp->getMomentumX(),
          l_nx,
          l_ny
      );

      l_checkpoint.write2DVariable(
          checkpointPath,
          "momentumY",
          l_waveProp->getMomentumY(),
          l_nx,
          l_ny
      );

      std::cout << "Finished creating Checkpoint" << std::endl;
      l_checkpointTimer = 0.0;
    }

    l_timeStep++;
    l_checkpointTimer += l_dt;
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