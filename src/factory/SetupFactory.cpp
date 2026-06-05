#include "SetupFactory.h"
#include "../setups/DamBreak1d.h"
#include "../setups/ShockShock1d.h"
#include "../setups/RareRare1d.h"
#include "../setups/Bathymetry1d.h"
#include "../setups/SubcriticalFlow1d.h"
#include "../setups/HydraulicJump1d.h"
#include "../setups/TsunamiEvent1d.h"
#include "../setups/CircularDamBreak2d.h"
#include "../setups/DamBreak2d.h"
#include "../setups/ArtificialTsunami2d.h"
#include "../setups/TsunamiEvent2d.h"
#include "../setups/Checkpoint2d.h"

#include <iostream>


using namespace tsunami_lab::setups;

Setup* createSetup(Config& c) {

    // very rudimentary sanity-check, to be expanded later
    if (c.insanity) {
        return new tsunami_lab::setups::ShockShock1d(   8899.326826472694,
                                                        122.0337839252433,
                                                        5 );
        
    } else {
        if (c.setup == "DamBreak1d") {
            return new DamBreak1d(  10,    // height left
                                    5,    // height right
                                    5 );  // dam location
        }

        else if (c.setup == "ShockShock1d") {
            return new ShockShock1d(10,    // height 
                                    10,    // momentum
                                    (c.width / 2) );   // middle location
        }

        else if (c.setup == "RareRare1d") {
            return new RareRare1d(  10,    // height 
                                    10,    // momentum
                                    (c.width / 2) );   // middle location
        }

        else if (c.setup == "Bathymetry1d") {
            return new Bathymetry1d(12,             // height left
                                    7,              // height right
                                    (c.width / 3),  // dam location
                                    0,              // bathymetry left
                                    5,              // bathymetry right
                                    ((2 * c.width) / 3) );   // bathymetry change location
        }

        else if (c.setup == "SubcriticalFlow1d") {
            return new SubcriticalFlow1d(   c.width / 2,// bump location in the middle of the simulation
                                            0.8,        // bump height: 0.8m 
                                            0.5,        // bump width: 0.5m
                                            2.0,        // water surface at 2m, well above the 0.8m bump
                                            1.5 );      // constant momentum (positive means left to right)
        }

        else if (c.setup == "HydraulicJump1d") {
            return new HydraulicJump1d( -0.13,  // obstacle height
                                        -0.33,  // base height of the bathymetry
                                        0.18 ); // momentum of the water
        }

        else if (c.setup == "TsunamiEvent1d") {
            Setup* setup = new TsunamiEvent1d( "src/bathymetry/output/03_dem_03.csv" );

            // Use actual domain width from the CSV x-coordinates
            c.width = setup->getDomainWidth();
            return setup;
        }

        else if (c.setup == "CircularDamBreak2d") {
            return new CircularDamBreak2d();
        }

        else if (c.setup == "DamBreak2d") {
            c.is_2d = true;
            return new DamBreak2d(  15,     // height water dam
                                    10,     // height water
                                    2.5,    // position of dam X
                                    5,      // position of dam Y
                                    2.5) ;  // radius of dam);
        }

        else if (c.setup == "ArtificialTsunami2d") {
            c.is_2d = true;
            c.width = 1000;
            c.dxy = c.width / c.nx;
            return new ArtificialTsunami2d( -100.0, 
                                            0 );
        }

        else if (c.setup == "TsunamiEvent2d") {
            c.is_2d = true;
            return new TsunamiEvent2d(c,
                "src/NetCdf/artificialtsunami_bathymetry_1000.nc",
                "src/NetCdf/artificialtsunami_displ_1000.nc"
            );
        }

        else if (c.setup == "ChileEvent2d") {
            c.is_2d = true;
            return new TsunamiEvent2d(c,
                                    "src/bathymetry/output/chile_gebco20_usgs_250m_bath_fixed.nc",
                                    "src/bathymetry/output/chile_gebco20_usgs_250m_displ_fixed.nc" );
        }

        else if (c.setup == "TohokuEvent2d") {
            c.is_2d = true;
            return new TsunamiEvent2d(c,
                                    "src/bathymetry/output/tohoku_gebco20_usgs_250m_bath_fixed.nc",
                                    "src/bathymetry/output/tohoku_gebco20_usgs_250m_displ_fixed.nc" );
        }

        else if (c.setup == "Checkpoint2d") {
            c.is_2d = true;
            c.isCheckpoint = true;
            return new Checkpoint2d(c);
        }

        throw std::runtime_error("Unknown setup: " + c.setup);
    }
}