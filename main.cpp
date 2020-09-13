#include <stdlib.h>

#include "gaterecorder.h"
#include <cxxopts.hpp>

using namespace kfr;

int main(int argc, const char ** argv)
{
    cxxopts::Options options("gate_recorder", "Record loud audio frames");
    options.add_options()
        ("l,loudness", "Loudness threshold", cxxopts::value<float>()->default_value("18"))
        ("c,cutoff", "Highpass cutoff freq", cxxopts::value<float>()->default_value("0"))
        ("r,rolloff", "Highpass rolloff value", cxxopts::value<float>()->default_value("8"))
        ("o,odir", "Output directory", cxxopts::value<std::string>()->default_value("."))
        ("q,quiet", "No output", cxxopts::value<bool>()->default_value("false"))
        ;

    auto o = options.parse(argc, argv);

    chdir(o["o"].as<std::string>().c_str());
    GateRecorder gr(
                o["l"].as<float>(),
                o["c"].as<float>(),
                o["r"].as<float>());

    while(1)
        sleep(100500);

    return 0;
}
