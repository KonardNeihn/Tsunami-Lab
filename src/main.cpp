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
#include "core/Initializer.h"

int is_number(char* input);

int main( int   i_argc,
          char *i_argv[] ) {

  // rate at which checkpoints are written (relative to end time)
  tsunami_lab::t_real l_checkpointRate = 0.5; 

  std::cout << "####################################" << std::endl;
  std::cout << "### Tsunami Lab                  ###" << std::endl;
  std::cout << "###                              ###" << std::endl;
  std::cout << "### https://scalable.uni-jena.de ###" << std::endl;
  std::cout << "####################################" << std::endl;

  // Handling input options
  Config g_config = parseArgs(i_argc, i_argv);

  // construct setup
  auto setup = createSetup(g_config);


  // current simulation time
  tsunami_lab::t_real l_simTime = g_config.simTime;

  // notify user about selectet configuration
  std::cout << "runtime configuration" << std::endl;
  std::cout << "  selected setup:                 " << g_config.setup << std::endl;
  std::cout << "  number of cells in x-direction: " << g_config.nx << std::endl;
  std::cout << "  number of cells in y-direction: " << g_config.ny << std::endl;
  std::cout << "  cell size:                      " << g_config.dxy << std::endl;
  std::cout << "  starts at time:                 " << l_simTime << std::endl;
  std::cout << "  ends at time:                   " << g_config.endTime << std::endl;
  std::cout << "  selected solver:                " << g_config.solver << std::endl;
  std::cout << "  insanity:                       " << g_config.insanity << std::endl;


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
  if( g_config.is_2d ) {
    l_waveProp = new tsunami_lab::patches::WavePropagation2d( g_config.nx, g_config.ny );
  } else {
    l_waveProp = new tsunami_lab::patches::WavePropagation1d( g_config.nx, g_config.solver );
  }

  // maximum observed height during setup
  tsunami_lab::t_real l_hMax;

  tsunami_lab::initialize(
    l_waveProp,
    setup,
    g_config,
    l_hMax
  );

  // derive maximum wave speed in setup; the momentum is ignored
  tsunami_lab::t_real l_speedMax = std::sqrt( 9.81 * l_hMax );

  // derive constant time step; changes at simulation time are ignored
  tsunami_lab::t_real l_dt = 0.5 * g_config.dxy / l_speedMax;

  // derive scaling for a time step
  tsunami_lab::t_real l_scaling = l_dt / g_config.dxy;

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
  if(g_config.is_2d) {
    for (const auto& stationConfig : stationConfigs) {
      // construct Object directly into vector
      stations.emplace_back(stationConfig.x,
                            stationConfig.y,
                            outputConfig.interval,
                            stationConfig.name,
                            g_config.nx,
                            outputConfig.path);
    }
  }
  

  // construct writer for NetCdf
  tsunami_lab::io::NetCdf l_ncWriter( l_ncPath,                                         
                                      g_config.nx,                                             
                                      g_config.is_2d ? g_config.ny : 1,          // pass 1 for 1D runs
                                      g_config.dxy,                                            
                                      l_waveProp->getStride(),                          
                                      l_waveProp->getBathymetry()                                     
);     

tsunami_lab::io::NetCdfCheckpoint l_checkpoint(g_config);
tsunami_lab::t_real l_checkpointTimer = 0.0;

  // iterate over time
  while( l_simTime < g_config.endTime ) {
      if( l_timeStep % 25 == 0 ) {
          std::cout << "  simulation time: " << l_simTime << " time steps: " << l_timeStep << std::endl;

          std::string l_path = (outDir / ("netcdf_output.nc")).string();
          std::cout << "  writing wave field to " << l_path << std::endl;

          // netCDF write
          l_ncWriter.write( l_simTime,
                            l_waveProp->getHeight(),
                            l_waveProp->getMomentumX(),
                            g_config.is_2d ? l_waveProp->getMomentumY() : nullptr );

          l_nOut++;
      }                                                                   



    /**  instead of: l_waveProp->setGhostOutflow(); we now check if boundaries are outflow or reflecting
    *    
    *   OLD: it only checked if the actual depth of the water is lower than 200m.
    
    // checks if the depth of the water on the left is smaller than 20 meters, by checking the middle of the left-outermost cell
    bool l_leftReflecting = setup->getHeight( 0.5 * g_config.dxy, 0 ) < 200;
    // checks if the depth of the water on the left is smaller than 20 meters, by checking the middle of the right-outermost cell
    bool l_rightReflecting = setup->getHeight( (g_config.nx - 0.5) * g_config.dxy, 0 ) < 200;

    l_waveProp->setGhostCells( l_leftReflecting, l_rightReflecting );
    */

    // New setup for ghost-cells with false defaults in the setup.h
    l_waveProp->setGhostCells(
      setup->isLeftBoundaryReflecting(),
      setup->isRightBoundaryReflecting(),
      setup->isBottomBoundaryReflecting(),
      setup->isTopBoundaryReflecting()
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

      l_checkpoint.createCheckpoint(checkpointPath, l_simTime);
      
      l_checkpoint.write2DVariable(
        checkpointPath,
        "height",
        l_waveProp->getHeight(),
        g_config.nx,
        g_config.ny
      );

      l_checkpoint.write2DVariable(
          checkpointPath,
          "bathymetry",
          l_waveProp->getBathymetry(),
          g_config.nx,
          g_config.ny
      );

      l_checkpoint.write2DVariable(
          checkpointPath,
          "momentumX",
          l_waveProp->getMomentumX(),
          g_config.nx,
          g_config.ny
      );

      if(g_config.is_2d) {
          l_checkpoint.write2DVariable(
              checkpointPath,
              "momentumY",
              l_waveProp->getMomentumY(),
              g_config.nx,
              g_config.ny
          );
      }

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
  delete setup;
  delete l_waveProp;

  std::cout << "finished, exiting" << std::endl;
  return EXIT_SUCCESS;
}