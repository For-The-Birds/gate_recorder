#ifndef AUDIODRIVER_H
#define AUDIODRIVER_H

#include <memory>

#include "jackaudioio.hpp"

class GateRecorder;

class AudioDriver
{
protected:
    AudioDriver();
    ~AudioDriver();

    std::unique_ptr<GateRecorder> gate_recorder;
    void process_frame(float * buf, float * obuf, size_t nsamples);
};

class AudioDriverJack : public AudioDriver, JackCpp::AudioIO
{
public:
    AudioDriverJack(float loudness, float cutoff_, float rolloff_);
    virtual int audioCallback(jack_nframes_t nframes, audioBufVector inBufs,
        audioBufVector outBufs);
};

#endif // AUDIODRIVER_H
