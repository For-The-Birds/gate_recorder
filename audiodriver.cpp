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

AudioDriverAlsa::AudioDriverAlsa(float loudness, float cutoff_, float rolloff_)
    : acd("hw:0,0", 48000, 1, frames, SND_PCM_FORMAT_FLOAT)
{
    gate_recorder.reset(
                new GateRecorder(loudness, cutoff_, rolloff_,
                                 48000, frames)
                );

    acd.open();
    while(true)
    {
        auto captured = acd.capture_into_buffer(reinterpret_cast<char*>(buffer.data()), frames);
        process_frame(buffer.data(), nullptr, captured);
    }
}
