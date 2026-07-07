#include "OutputManager.h"
#include <iostream>
#include <filesystem>
#include <algorithm>
#include "../patches/WavePropagationAdaptiveGrid2d.h"

namespace tsunami_lab {

OutputManager::OutputManager(
    Config& config,
    patches::WavePropagation* solver,
    const std::vector<std::vector<t_idx>>& gridResolutionLevels
)
:
g_config(config),
g_solver(solver),
m_ncPath("solutions/netcdf_output.nc"),
m_checkpoint(config),
m_gridResolutionLevels(gridResolutionLevels),
m_maxResolution(1)
{
    // Determine maximum refinement level
    m_maxResolution = 1;

    // Only compute dynamic maxResolution if we are actively using an adaptive grid
    if (g_config.is_2d && g_config.use_adaptive_grid) {
        for (const auto& row : gridResolutionLevels)
            for (auto r : row)
                m_maxResolution = std::max(m_maxResolution, r);
    }

    m_nxOut  = g_config.nx * m_maxResolution;
    m_nyOut  = g_config.ny * m_maxResolution;
    m_dxyOut = g_config.dxy / static_cast<t_real>(m_maxResolution);

    // Export Bathymetry depending on Grid Type
    std::vector<t_real> b;
    std::vector<t_real> h;
    std::vector<t_real> hu;
    std::vector<t_real> hv;

    if (g_config.is_2d && g_config.use_adaptive_grid) {
        auto* adaptive = dynamic_cast<patches::WavePropagationAdaptiveGrid2d*>(g_solver);
        if (adaptive == nullptr) {
            throw std::runtime_error("OutputManager error: Expected WavePropagationAdaptiveGrid2d instance.");
        }
        adaptive->exportUniformGrid(m_maxResolution, b, h, hu, hv);
    } 
    else {
        // Static 2D or 1D Fallback: Copy arrays directly from the base solver interface
        // Assuming your solver size matches m_nxOut * m_nyOut when static (since maxResolution = 1)
        size_t totalCells = m_nxOut * m_nyOut; 
        
        const t_real* raw_b = g_solver->getBathymetry();
        if (raw_b != nullptr) {
            b.assign(raw_b, raw_b + totalCells);
        }
    }

    // Generate NetCDF Writer
    m_ncWriter = std::make_unique<io::NetCdfAdaptiveGrid>(
        m_ncPath,
        m_nxOut,
        m_nyOut,
        m_dxyOut,
        b,
        g_config.isCheckpoint
    );
}

void OutputManager::writeStep(t_real simTime)
{
    std::vector<t_real> b; // unused for steps but required if signature demands it
    std::vector<t_real> h;
    std::vector<t_real> hu;
    std::vector<t_real> hv;

    if (g_config.is_2d && g_config.use_adaptive_grid) {
        auto* adaptive = dynamic_cast<patches::WavePropagationAdaptiveGrid2d*>(g_solver);
        if (adaptive == nullptr) {
            throw std::runtime_error("OutputManager error: Expected WavePropagationAdaptiveGrid2d instance in writeStep.");
        }
        adaptive->exportUniformGrid(m_maxResolution, b, h, hu, hv);
    } 
    else {
        // Static 2D or 1D Fallback: Extract raw steady-state grid arrays directly
        size_t totalCells = m_nxOut * m_nyOut;

        const t_real* raw_h  = g_solver->getHeight();
        const t_real* raw_hu = g_solver->getMomentumX();
        
        if (raw_h != nullptr)  h.assign(raw_h, raw_h + totalCells);
        if (raw_hu != nullptr) hu.assign(raw_hu, raw_hu + totalCells);

        if (g_config.is_2d) {
            const t_real* raw_hv = g_solver->getMomentumY();
            if (raw_hv != nullptr) hv.assign(raw_hv, raw_hv + totalCells);
        } else {
            hv.resize(totalCells, 0.0); // Safe fallback padding for 1D setups
        }
    }

    m_ncWriter->write(
        simTime,
        h,
        hu,
        hv
    );
}

void OutputManager::writeCheckpoint(t_real simTime)
{   
    std::string tmpPath = "solutions/checkpoint" + std::to_string(g_config.latestCheckpoint + 1) + ".nc.tmp";
    std::string finalPath = "solutions/checkpoint" + std::to_string(g_config.latestCheckpoint + 1) + ".nc";

    std::cout << "Creating checkpoint at " << tmpPath << std::endl;

    m_checkpoint.createCheckpoint(tmpPath, simTime);

    m_checkpoint.write2DVariable(   tmpPath, 
                                    "height",
                                    g_solver->getHeight(), 
                                    g_config.nx, 
                                    g_config.ny);

    m_checkpoint.write2DVariable(   tmpPath, 
                                    "bathymetry",
                                    g_solver->getBathymetry(), 
                                    g_config.nx, 
                                    g_config.ny);

    m_checkpoint.write2DVariable(   tmpPath, 
                                    "momentumX",
                                    g_solver->getMomentumX(), 
                                    g_config.nx, 
                                    g_config.ny);

    if (g_config.is_2d) {
        m_checkpoint.write2DVariable(   tmpPath, 
                                        "momentumY",
                                        g_solver->getMomentumY(), 
                                        g_config.nx, 
                                        g_config.ny);
    }

    std::cout << "Finished creating Checkpoint" << std::endl;

    std::filesystem::rename(tmpPath, finalPath);

    g_config.latestCheckpoint++;
}

void OutputManager::deleteCheckpoints()
{
    if (std::filesystem::exists("solutions")) {
        for (const auto& entry : std::filesystem::directory_iterator("solutions")) {
            std::string name = entry.path().filename().string();
            if (name.substr(0, 10) == "checkpoint") {
                std::filesystem::remove(entry.path());
            }
        }
    }
}

void OutputManager::detectLatestCheckpoint()
{
    deleteUncompletedCheckpoints();
    std::vector<int> ids;

    if (std::filesystem::exists("solutions")) {
        for (const auto& entry : std::filesystem::directory_iterator("solutions"))
        {
            std::string name = entry.path().filename().string();

            if (name.substr(0, 10) == "checkpoint" && entry.path().extension() == ".nc")
            {
                std::string number =
                    name.substr(
                        std::string("checkpoint").size(),
                        name.size()
                        - std::string("checkpoint").size()
                        - 3); // ".nc"

                ids.push_back(std::stoi(number));
            }
        }
    }

    if (ids.empty()) {
        throw std::runtime_error("No checkpoint found");
    }
    std::sort(ids.begin(), ids.end());
    g_config.latestCheckpoint = ids.back();
}

void OutputManager::deleteUncompletedCheckpoints()
{
    if (std::filesystem::exists("solutions")) {
        for (const auto& entry : std::filesystem::directory_iterator("solutions"))
        {
            if (entry.path().extension() == ".tmp")
            {
                std::cout << "Removing incomplete checkpoint " << entry.path() << std::endl;
                std::filesystem::remove(entry.path());
            }
        }
    }
}

} // namespace tsunami_lab