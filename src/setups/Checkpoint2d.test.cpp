/**
 * @section DESCRIPTION
 * Tests the Checkpoints for the 2d setup.
 **/
#include <catch2/catch.hpp>
#include <filesystem>
#include <vector>

#include "Checkpoint2d.h"
#include "../io/NetCdfCheckpoint.h"
#include "../config/Config.h"

TEST_CASE("Checkpoint2d: Read and write with Ghost Cells", "[Checkpoint2d]") {
    // path
    Config l_config;
    l_config.latestCheckpoint = 999;
    
    // Grid-Dimensions (without Ghost Cells)
    l_config.nx = 2;
    l_config.ny = 2;
    l_config.dxy = 1.0;
    l_config.domainStartX = 0.0;
    l_config.domainStartY = 0.0;

    std::string l_testFilePath = "solutions/checkpoint999.nc";
    if (std::filesystem::exists(l_testFilePath)) {
        std::filesystem::remove(l_testFilePath);
    }

    // NetCdfCheckpoint in Define-Mode
    tsunami_lab::io::NetCdfCheckpoint l_ncCheckpoint(l_config);
    l_ncCheckpoint.createCheckpoint(l_testFilePath, 5.0); // simTime = 5.0

    // Bathymetry
    tsunami_lab::t_real l_bathyGhost[16] = {
        0.0,   0.0,   0.0, 0.0,  // Ghost Row Top    (y=-1)
        0.0, -10.0,  -8.0, 0.0,  // Data Row 0       (y=0) ,so: x=0 (-10.0), x=1 (-8.0)
        0.0,  -6.0,  -4.0, 0.0,  // Data Row 1       (y=1) so: x=0 (-6.0),  x=1 (-4.0)
        0.0,   0.0,   0.0, 0.0   // Ghost Row Bottom (y=2)
    };

    // Height 
    tsunami_lab::t_real l_heightGhost[16] = {
        0.0, 0.0, 0.0, 0.0,
        0.0, 1.5, 2.0, 0.0,      // Data Row 0 
        0.0, 2.5, 3.0, 0.0,      // Data Row 1 
        0.0, 0.0, 0.0, 0.0
    };

    // MomentumX Daten mit Ghost-Rand
    tsunami_lab::t_real l_huGhost[16] = {
        0.0, 0.0, 0.0, 0.0,
        0.0, 0.1, 0.2, 0.0,      // Data Row 0
        0.0, 0.3, 0.4, 0.0,      // Data Row 1 
        0.0, 0.0, 0.0, 0.0
    };

    // MomentumY
    tsunami_lab::t_real l_hvGhost[16] = {
        0.0, 0.0, 0.0, 0.0,
        0.0, 0.5, 0.6, 0.0,      // Data Row 0 
        0.0, 0.7, 0.8, 0.0,      // Data Row 1 
        0.0, 0.0, 0.0, 0.0
    };

    // write variables 
    l_ncCheckpoint.write2DVariable(l_testFilePath, "bathymetry", l_bathyGhost, l_config.nx, l_config.ny);
    l_ncCheckpoint.write2DVariable(l_testFilePath, "height",     l_heightGhost, l_config.nx, l_config.ny);
    l_ncCheckpoint.write2DVariable(l_testFilePath, "momentumX",  l_huGhost,     l_config.nx, l_config.ny);
    l_ncCheckpoint.write2DVariable(l_testFilePath, "momentumY",  l_hvGhost,     l_config.nx, l_config.ny);

    // initialize setup
    tsunami_lab::setups::Checkpoint2d l_checkpointSetup(l_config);

    // Assertions
    SECTION("Bathymetry retrieval") {
        REQUIRE(l_checkpointSetup.getBathymetry(0, 0) == Approx(-10.0));
        REQUIRE(l_checkpointSetup.getBathymetry(1, 0) == Approx(-8.0));
        REQUIRE(l_checkpointSetup.getBathymetry(0, 1) == Approx(-6.0));
        REQUIRE(l_checkpointSetup.getBathymetry(1, 1) == Approx(-4.0));
    }

    SECTION("Height retrieval") {
        REQUIRE(l_checkpointSetup.getHeight(0, 0) == Approx(1.5));
        REQUIRE(l_checkpointSetup.getHeight(1, 0) == Approx(2.0));
        REQUIRE(l_checkpointSetup.getHeight(0, 1) == Approx(2.5));
        REQUIRE(l_checkpointSetup.getHeight(1, 1) == Approx(3.0));
    }

    SECTION("Momentum retrieval") {
        REQUIRE(l_checkpointSetup.getMomentumX(0, 0) == Approx(0.1));
        REQUIRE(l_checkpointSetup.getMomentumY(0, 0) == Approx(0.5));
    }

    SECTION("Truncation / Index Check with floating coordinates") {
        REQUIRE(l_checkpointSetup.getBathymetry(0.9, 0.1) == Approx(-10.0));
    }

    // Cleanup
    if (std::filesystem::exists(l_testFilePath)) {
        std::filesystem::remove(l_testFilePath);
    }
}