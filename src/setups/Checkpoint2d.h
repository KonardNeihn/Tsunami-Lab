/**
 * 
 * @section DESCRIPTION
 * 2D Checkpoint
 **/
#ifndef TSUNAMI_LAB_SETUPS_CHECKPOINT_2D_H
#define TSUNAMI_LAB_SETUPS_CHECKPOINT_2D_H

#include "Setup.h"
#include <string>
#include <vector>
#include "../io/NetCdfReader.h"
#include "../io/NetCdfCheckpoint.h"

namespace tsunami_lab {
  namespace setups {
    class Checkpoint2d;
  }
}

/**
 *  2d checkpoint setup.
 * Loads a checkpoint file to get latest calculated timestep and continue
 **/
class tsunami_lab::setups::Checkpoint2d: public Setup {
  private:
    Config& g_config;
    std::string m_checkpointPath;
    t_real m_lastTimeStep;

    std::vector<std::vector<t_real>> m_2dBathymetry;
    std::vector<std::vector<t_real>> m_2dDisplacement;
    std::vector<std::vector<t_real>> m_2dMomentumX;
    std::vector<std::vector<t_real>> m_2dMomentumY;
  public:
    /**
     * Constructor.
     **/
    Checkpoint2d(Config& config);

    /**
     * Gets the water height at a given point.
     *
     * @param i_x x-coordinate of the queried point.
     * @return height at the given point. 
     **/
    t_real getHeight( t_real i_x,
                      t_real i_y     ) const;

    /**
     * Gets the momentum in x-direction.
     *
     * @return momentum in x-direction.
     **/
    t_real getMomentumX( t_real i_x,
                         t_real i_y) const;

    /**
     * Gets the momentum in y-direction.
     *
     * @return momentum in y-direction.
     **/
    t_real getMomentumY( t_real i_x,
                         t_real i_y) const;

    
    /**
     * Gets the bathymetry at a given point.
      *
      * @return bathymetry at the given point.
    **/
    t_real getBathymetry( t_real i_x,
                          t_real i_y ) const; 

    t_idx getNX() const;
    t_idx getNY() const;
    t_real getLastTimeStep() const;
    t_real getEndTime() const;
    t_real getWidth() const;
    t_real getDomainStartX() const;
    t_real getDomainStartY() const;
};

#endif