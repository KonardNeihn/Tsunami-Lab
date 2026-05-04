/**
 * @author 
 *
 * @section DESCRIPTION
 * Stations for recording data at specific locations inside the simulation.
 **/

#ifndef TSUNAMI_LAB_IO_STATION
#define TSUNAMI_LAB_IO_STATION

#include "Csv.h"
#include <string>

namespace tsunami_lab {
  namespace io {
    class Station;
  }
}

class tsunami_lab::io::Station {
    private:
        t_idx   m_xPosition;
        t_idx   m_yPosition;
        float   m_recordingInterval;

        t_real  m_recordedHeight;
        t_real  m_recordedMomentumX;
        t_real  m_recordedMomentumY;

        float m_timeSinceLastRecording;

        std::string m_filePath;

        t_idx m_linearPosition;
    public:

    /**  Constrctor
    * @param i_xPosition x-position of the station in the grid.
    * @param i_yPosition y-position of the station in the grid.
    * @param i_recordingInterval time interval at which the station records data.
    * @param i_stationName name of the station
    **/
    Station(t_idx i_xPosition, 
        t_idx i_yPosition, 
        float i_recordingInterval,
        std::string i_stationName,
        t_idx i_nx,
        std::string i_path);

    // Destructor
    ~Station() = default;

    /**
    * @param i_timeStep current time step of the simulation.
    * @param i_height recorded height at the current time step.
    * @param i_momentumX recorded momentum in the x-direction at the current time step.
    * @param i_momentumY recorded momentum in the y-direction at the current time step.
    **/
    void timeStep(t_real i_deltaTimeStep, 
        const t_real *i_height, 
        const t_real *i_momentumX, 
        const t_real *i_momentumY);
};

#endif

