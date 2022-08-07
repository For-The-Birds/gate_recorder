#include <stdlib.h>

#include "gaterecorder.h"

#include <cxxopts.hpp>

#include <plog/Log.h>
#include <plog/Init.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Appenders/ColorConsoleAppender.h>

using namespace kfr;

bool quiet;

int main(int argc, const char ** argv)
{
    static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
    plog::init(plog::verbose, &consoleAppender);

    cxxopts::Options options("gate_recorder",
                             "Records and/or passes through loud audio frames, "
                             "like gate, but keeps some pre-buffer ans post-buffer");
    options.add_options()
        ("l,loudness", "Loudness threshold for recording", cxxopts::value<float>()->default_value("-40"))
        ("p,passthrough", "Loudness threshold for passthrough", cxxopts::value<float>()->default_value("-50"))
        ("b,before", "Keep this amount of seconds before the event", cxxopts::value<float>()->default_value("0.2"))
        ("a,after", "Keep this amount of seconds after the event", cxxopts::value<float>()->default_value("1"))
        ("w,wait", "Time to wait for next loud event to be stored in same file. Release time in seconds.", cxxopts::value<float>()->default_value("10"))
        ("e,event", "Minimum loud event time in seconds to start recording", cxxopts::value<float>()->default_value("0.1"))
        ("c,cutoff", "Highpass cutoff freq", cxxopts::value<float>()->default_value("0"))
        ("r,rolloff", "Highpass rolloff value", cxxopts::value<float>()->default_value("8"))
        ("o,odir", "Output directory", cxxopts::value<std::string>()->default_value("."))
        ("d,debug", "Log level", cxxopts::value<int>()->default_value("4"))
        ("q,quiet", "Do not print lodness statistics", cxxopts::value<bool>()->default_value("false"))
        ;

    try {
        auto o = options.parse(argc, argv);

        quiet = o["q"].as<bool>();

        plog::get()->setMaxSeverity(static_cast<plog::Severity>(o["d"].as<int>()));

        auto ret = chdir(o["o"].as<std::string>().c_str());
        if (ret == -1) {
            LOGE << "cant chdir " << strerror(errno);
            return -3;
        }

        GateRecorder gr(
                    o["l"].as<float>(),
                    o["p"].as<float>(),
                    o["c"].as<float>(),
                    o["r"].as<float>(),
                    o["b"].as<float>(),
                    o["a"].as<float>(),
                    o["w"].as<float>(),
                    o["e"].as<float>());
        while(1)
            sleep(100500);

    }
    catch (cxxopts::OptionException & e)
    {
        LOGE << e.what() << "\n\n" << options.help();
        return 0;
    }
    catch (std::exception & e)
    {
        LOGE << e.what();
        return -1;
    }
    catch(...)
    {
        LOGE << "unknown exception";
        return -2;
    }

    return 0;
}
