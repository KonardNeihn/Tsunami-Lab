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
#include "setups/ArtificialTsunami2d.h"
#include "setups/TsunamiEvent2d.h"
#include "setups/ChileEvent2d.h"
#include "setups/TohokuEvent2d.h"
#include "setups/Checkpoint2d.h"
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

  // current simulation time
  tsunami_lab::t_real l_simTime = 0;

  // Domainstart
  tsunami_lab::t_real l_domainStartX = 0.0;
  tsunami_lab::t_real l_domainStartY = 0.0;

  // rate at which checkpoints are written (relative to end time)
  tsunami_lab::t_real l_checkpointRate = 0.5; 

  std::cout << "####################################" << std::endl;
  std::cout << "### Tsunami Lab                  ###" << std::endl;
  std::cout << "###                              ###" << std::endl;
  std::cout << "### https://scalable.uni-jena.de ###" << std::endl;
  std::cout << "####################################" << std::endl;

  // Handling input options
  Config l_config = parseArgs(i_argc, i_argv);

  // construct setup
  auto l_setup = createSetup(l_config);

  // notify user about selectet configuration
  std::cout << "runtime configuration" << std::endl;
  std::cout << "  number of cells in x-direction: " << l_config.nx << std::endl;
  std::cout << "  number of cells in y-direction: " << l_config.ny << std::endl;
  std::cout << "  cell size:                      " << l_config.dxy << std::endl;
  std::cout << "  insanity:                       " << l_config.insanity << std::endl;
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

          std::string l_path = (outDir / ("solution_" + std::to_string(l_nOut) + ".csv")).string();
          std::cout << "  writing wave field to " << l_path << std::endl;

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
      l_checkpoint.createCheckpoint(checkpointPath, l_config.nx, l_config.ny, l_simTime, l_config.endTime, l_config.width, l_domainStartX, l_domainStartY);
      
      l_checkpoint.write2DVariable(
        checkpointPath,
        "height",
        l_waveProp->getHeight(),
        l_config.nx,
        l_config.ny
      );

      l_checkpoint.write2DVariable(
          checkpointPath,
          "bathymetry",
          l_waveProp->getBathymetry(),
          l_config.nx,
          l_config.ny
      );

      l_checkpoint.write2DVariable(
          checkpointPath,
          "momentumX",
          l_waveProp->getMomentumX(),
          l_config.nx,
          l_config.ny
      );

      l_checkpoint.write2DVariable(
          checkpointPath,
          "momentumY",
          l_waveProp->getMomentumY(),
          l_config.nx,
          l_config.ny
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