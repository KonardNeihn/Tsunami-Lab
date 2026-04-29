/**
 * @section DESCRIPTION
 * Tsunami Simulation.
 **/
#ifndef TSUNAMI_LAB_SETUPS_TSUNAMI_EVENT_1D_H
#define TSUNAMI_LAB_SETUPS_TSUNAMI_EVENT_1D_H

#include "Setup.h"
#include <string>
#include <vector>

namespace tsunami_lab {
  namespace setups {
    class TsunamiEvent1d;
  }
}

/**
 * 1D Tsunami Event setup.
 *
 * Initializes water height h, momentum hu, and bathymetry b from a bathymetry.csv and artificial vertical displacement.
 */
class tsunami_lab::setups::TsunamiEvent1d: public Setup {
private:
  //! delta: small positive constant to avoid wet/dry issues (20 m)
  tsunami_lab::t_real m_delta = 20;

  //! x-coordinates loaded from the bathymetry.csv
  std::vector<tsunami_lab::t_real> m_bathyX;

  //! bathymetry values b_in(x) loaded from the .csv
  std::vector<tsunami_lab::t_real> m_bathyB;

  /**
   * Linearly interpolates the raw bathymetry b_in at position x.
   *
   * @param i_x query position
   * @return interpolated b_in value.
   */
  tsunami_lab::t_real getBathymetryRaw( tsunami_lab::t_real i_x ) const;

  /**
   * Computes the vertical displacement d(x), just like the task says:
   *
   *         { 10 * sin( (x - 175000) / 37500 * pi + pi ),  if 175000 < x < 250000
   * d(x) = {
   *         { 0,                                           else
   *
   * @param i_x query position
   * @return displacement
   */
  static tsunami_lab::t_real getDisplacement( tsunami_lab::t_real i_x );

public:
  /**
   * Constructor rhat loads the bathymetry data.
   *
   * The CSV has two columns?:
   *   column 0: x-coordinate
   *   column 1: elevation / bathymetry b_in (m, negative = below sea level)
   *
   * @param i_bathymetryFile path to the bathymetry CSV file.
   */
  TsunamiEvent1d( const std::string & i_bathymetryFile );

  /**
   * Gets the water height h at position (x, 0).
   *
   *       { max( -b_in, delta ),  if b_in < 0
   * h = {
   *       { 0,                    else
   *
   * @param i_x x-coordinate
   * @param i_y y-coordinate (unused, 1-D setup).
   * @return water height 
   */
  tsunami_lab::t_real getHeight( tsunami_lab::t_real i_x,
                                 tsunami_lab::t_real i_y ) const override;

  /**
   * Gets the momentum hu at position (x, 0).
   * Always returns 0 (water is at rest initially).
   *
   * @param i_x x-coordinate 
   * @param i_y y-coordinate (unused).
   * @return momentum hu 
   */
  tsunami_lab::t_real getMomentumX( tsunami_lab::t_real i_x,
                                    tsunami_lab::t_real i_y ) const override;

  /**
   * Gets the y-momentum hv at position (x, 0).
   * Always returns 0 (1-D setup).
   *
   * @param i_x x-coordinate 
   * @param i_y y-coordinate (unused).
   * @return momentum hv 
   */
  tsunami_lab::t_real getMomentumY( tsunami_lab::t_real i_x,
                                    tsunami_lab::t_real i_y ) const override;

  /**
   * Gets the bathymetry b at position (x, 0), including displacement.
   *
   *       { min( b_in, -delta ) + d,  if b_in < 0
   * b = {
   *       { max( b_in,  delta ) + d,  otherwise
   *
   * @param i_x x-coordinate 
   * @param i_y y-coordinate (unused).
   * @return bathymetry 
   */
  tsunami_lab::t_real getBathymetry( tsunami_lab::t_real i_x,
                                     tsunami_lab::t_real i_y ) const override;
                                     
  tsunami_lab::t_real getDomainWidth() const {
      return m_bathyX.back() - m_bathyX.front();
  }
  // and optionally the start offset:
  tsunami_lab::t_real getDomainStart() const {
      return m_bathyX.front();
}
};

#endif 
