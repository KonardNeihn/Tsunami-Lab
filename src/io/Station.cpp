#include "Station.h"
#include "Csv.h"
#include <string>
#include <filesystem>

tsunami_lab::io::Station::Station(t_idx i_xPosition, t_idx i_yPosition, float i_recordingInterval, std::string i_stationName, t_idx i_nx) {
    m_xPosition = i_xPosition;
    m_yPosition = i_yPosition;
    m_recordingInterval = i_recordingInterval;

    std::filesystem::create_directories("solutions/station_data");
    m_filePath = "solutions/station_data/" + i_stationName + ".csv";

    m_recordedHeight = 0.0;
    m_recordedMomentumX = 0.0;
    m_recordedMomentumY = 0.0;

    m_timeSinceLastRecording = 0.0;   

    m_linearPosition = /*(i_nx + 0) * (m_yPosition + 0) + */(m_xPosition + 1) + (0 * i_nx);
}

void tsunami_lab::io::Station::timeStep(t_real i_deltaTimeStep,const t_real *i_height, const t_real *i_momentumX, const t_real *i_momentumY) {
    
    if (m_timeSinceLastRecording >= m_recordingInterval) {
        // new recording should be made

        m_recordedHeight = i_height[m_linearPosition];
        m_recordedMomentumX = i_momentumX[m_linearPosition];

        if (i_momentumY != nullptr)
            m_recordedMomentumY = i_momentumY[m_linearPosition];
        else
            m_recordedMomentumY = 0.0;

        // write recorded data into file
        std::ofstream l_file;
        l_file.open(m_filePath, std::ios::app);

        if (l_file.tellp() == 0) {
            l_file << "height,momentum_x,momentum_y\n";
        }

        tsunami_lab::io::Csv::writeStationData(m_recordedHeight, m_recordedMomentumX, m_recordedMomentumY, l_file);
        
        l_file.close();

        m_timeSinceLastRecording = 0.0;
    }

    m_timeSinceLastRecording = m_timeSinceLastRecording + i_deltaTimeStep;
}

