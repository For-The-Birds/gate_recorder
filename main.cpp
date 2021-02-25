#include <stdlib.h>

#include "gaterecorder.h"
#include <cxxopts.hpp>

using namespace kfr;

bool quiet;

int main(int argc, const char ** argv)
{
    cxxopts::Options options("gate_recorder",
                             "Records loud audio frames, "
                             "like gate but keeps some pre-buffer ans post-buffer");
    options.add_options()
        ("l,loudness", "Loudness threshold for recording", cxxopts::value<float>()->default_value("18"))
        ("p,passthrough", "Loudness threshold for passthrough", cxxopts::value<float>()->default_value("13"))
        ("c,cutoff", "Highpass cutoff freq", cxxopts::value<float>()->default_value("0"))
        ("r,rolloff", "Highpass rolloff value", cxxopts::value<float>()->default_value("8"))
        ("o,odir", "Output directory", cxxopts::value<std::string>()->default_value("."))
        ("q,quiet", "No output", cxxopts::value<bool>()->default_value("false"))
        ;

    try {
        auto o = options.parse(argc, argv);

        quiet = o["q"].as<bool>();
        chdir(o["o"].as<std::string>().c_str());
        GateRecorder gr(
                    o["l"].as<float>(),
                    o["p"].as<float>(),
                    o["c"].as<float>(),
                    o["r"].as<float>());
        while(1)
            sleep(100500);

    }
    catch (cxxopts::OptionException & e)
    {
        std::cout << e.what() << "\n\n" << options.help();
        return 0;
    }
    catch (std::exception & e)
    {
        std::cout << e.what();
        return -1;
    }
    catch(...)
    {
        std::cout << "unknown exception";
        return -2;
    }

    return 0;
}
