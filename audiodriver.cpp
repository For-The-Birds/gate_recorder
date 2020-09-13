#include "audiodriver.h"

#include "gaterecorder.h"

AudioDriver::AudioDriver()
{
}

AudioDriver::~AudioDriver()
{
}

void AudioDriver::process_frame(float *buf, float * obuf, size_t nsamples)
{
    gate_recorder->process_frame(buf, obuf, nsamples);
}

AudioDriverJack::AudioDriverJack(float loudness, float cutoff_, float rolloff_)
    : JackCpp::AudioIO("gate_recorder", 1, 1)
{
    gate_recorder.reset(
                new GateRecorder(loudness, cutoff_, rolloff_,
                                 getSampleRate(), getBufferSize())
                );

    start();
}

int AudioDriverJack::audioCallback(jack_nframes_t nframes, JackCpp::AudioIO::audioBufVector inBufs, JackCpp::AudioIO::audioBufVector outBufs)
{
    process_frame(inBufs[0], outBufs[0], nframes);
    return 0;
}
