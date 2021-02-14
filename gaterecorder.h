#ifndef GATERECORDER_H
#define GATERECORDER_H

#include <kfr/all.hpp>

#include <deque>

#include "jackaudioio.hpp"

class GateRecorder : public JackCpp::AudioIO
{
public:
    using ftype = jack_default_audio_sample_t;
    using buffer_type = std::deque<kfr::univector<ftype>>;
    static constexpr size_t fftsize = 1024;
    static constexpr size_t chunk_size = 128;

    GateRecorder(float loudness, float cutoff_, float rolloff_);

    virtual int audioCallback(jack_nframes_t nframes, audioBufVector inBufs,
        audioBufVector outBufs);

    size_t frames_in_seconds(size_t seconds) const;
private:
    float loudness_threshold, cutoff, rolloff;
    buffer_type frames_buffer;
    kfr::audio_format af;

    size_t chunks;
    size_t frames_past_loud = 0, max_frames_wait;
    size_t frames_begin, frames_end;
    size_t buffer_limit_soft, buffer_limit_hard;
    buffer_type frames_out;
    bool recording = false;

    buffer_type bflush(size_t tail_return = 0);

    kfr::zpk<kfr::fbase> filt;
    std::vector<kfr::biquad_params<kfr::fbase>> bqs;

};

#endif // GATERECORDER_H
