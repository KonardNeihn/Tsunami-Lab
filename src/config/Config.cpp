#include "Config.h"
#include "CLI11.hpp"
#include <vector>

Config parseArgs(int argc, char** argv) {
    Config c;

    CLI::App app{"Tsunami Lab"};

    std::vector<int> dims;

    app.add_option("-n,--ncells", dims, "Set number of cells. Defaults: x = 100, (y=1). 1d and 2d is possible.")
        ->expected(1, 2);
    app.add_option("-s,--solver", c.solver, "Select a solver. Possible is {fwave, roe}. Default is fwave.")
        ->check(CLI::IsMember({"fwave", "roe"}));
    app.add_flag("-i,--insanity", c.insanity, "Set flag for insanity. Default is sanity");
    app.add_option("-w,--width", c.width, "Set width of the observed space in meters. default is 10");
    app.add_option("-t,--time", c.endTime, "Set time until aborting in s. default is 1.25");
    app.add_option("-S,--setup", c.setup, "Select setup to simulate. Possible is \
        {DamBreak1d, RareRare1d, ShockShock1d, Bathymetry1d, SubcriticalFlow1d, \
        HydraulicJump1d, TsunamiEvent1d, CircularDamBreak2d, DamBreak2d, ArtificialTsunami2d, \
        TsunamiEvent2d, ChileEvent2d, TohokuEvent2d, Checkpoint2d}. Default is DamBreak1d");

    app.parse(argc, argv);

    if(!dims.empty())
        c.nx = dims[0];
    if (dims.size() == 2)
        c.ny = dims[1];

    c.dxy = c.width / c.nx;

    return c;
}