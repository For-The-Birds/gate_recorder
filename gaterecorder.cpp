#include "gaterecorder.h"

#include <thread>
#include <map>

std::string get_filename()
{
    std::time_t t = std::time(nullptr);
    char str[256] = {0};
    std::strftime(str, sizeof(str), "%F_%T.wav", std::localtime(&t));
    return str;
}

void audio_dumper(GateRecorder::buffer_type buf, kfr::audio_format af)
{
    auto filename = get_filename();
    size_t sec = buf.size() * buf.front().size() / af.samplerate;
    printf("\nAW %lu sec '%s'\n", sec, filename.c_str());
    fflush(stdout);
    kfr::audio_writer_wav<float> aw(
                 kfr::open_file_for_writing(filename),
                 af);
    while(buf.size() > 0) {
        aw.write(buf.front());
        buf.pop_front();
    }
}

GateRecorder::GateRecorder(float loudness, float cutoff_, float rolloff_,
                           size_t sample_rate_, size_t buffer_size_)
    : loudness_threshold(loudness)
    , cutoff(cutoff_)
    , rolloff(rolloff_)
    , sample_rate(sample_rate_)
    , buffer_size(buffer_size_)
{
    printf("%s, loudness_threshold: %.2f\n", kfr::library_version(), loudness_threshold);

    //chunks = getBufferSize()/chunk_size;

    af.samplerate = sample_rate;
    af.channels = 1;

    max_frames_wait = frames_in_seconds(10);
    frames_begin = frames_in_seconds(1);
    frames_end = frames_in_seconds(1);
    buffer_limit_soft = frames_in_seconds(60*1);
    buffer_limit_hard = frames_in_seconds(60*3);

    if (rolloff > 0 && cutoff > 0)
    {
        printf("Using highpass filter cutoff %2.f, rolloff %.2f\n", cutoff, rolloff);
        filt = kfr::iir_highpass(kfr::bessel<kfr::fbase>(rolloff), cutoff, sample_rate);
        bqs = kfr::to_sos(filt);
    }

}

void GateRecorder::process_frame(float *buf, float *obuf, size_t nsamples)
{
    frames_buffer.emplace_back(kfr::make_univector(buf, nsamples));

    if (rolloff > 0 && cutoff > 0)
    {
        kfr::univector<float> filtered = kfr::biquad<32>(bqs, frames_buffer.back());
        if (obuf)
        {
            float * p = obuf;
            for (auto & v : filtered)
                *p++ = v;
        }
        frames_buffer.back() = filtered;
    }

    std::map<float, int> loud_samples;
    for (auto & sample : frames_buffer.back())
    {
        auto l = kfr::energy_to_loudness(sample);
        for (float i=12; i >= -8; i -= 2)
        {
            auto t = loudness_threshold - i;
            loud_samples[t];
            if (l > t) {
                ++loud_samples[t];
            }
        }
    }

    for (auto & ls : loud_samples)
        printf("%5.2f(%3d)  ", ls.first, ls.second);

    if (loud_samples[loudness_threshold] > 100)
    {
        printf(" [%3d]\n", loud_samples[loudness_threshold]);
        frames_past_loud = 0;
        if (!recording)
        {
            while(frames_buffer.size() > frames_begin)
                frames_buffer.pop_front();
            recording = true;
        }
    }
    else
        printf("\r");


    ++frames_past_loud;

    if (frames_past_loud == max_frames_wait && recording)
    {
        // Too long silence. Normal end of recording.
        frames_buffer = bflush(frames_past_loud - frames_end);
        recording = false;
    }

    if (frames_past_loud < max_frames_wait && recording)
    {
        // Normal recording state. Check file length limits.
        if (frames_buffer.size() > buffer_limit_soft)
        {
            if (frames_buffer.size() > buffer_limit_hard)
            {
                printf("hard hit\n");
                bflush();
            }
            else if(frames_past_loud > max_frames_wait/3)
            {
                printf("soft hit\n");
                frames_buffer = bflush(max_frames_wait/3 - frames_end);
            }
        }
    }

    if (frames_past_loud > max_frames_wait)
    {
        // Too long after last loud frame. This means that we are not recording
        // and do not need to pile up frames_buffer
        frames_buffer.pop_front();

        frames_past_loud = max_frames_wait+1; // avoid overflow
    }

    fflush(stdout);
}

size_t GateRecorder::frames_in_seconds(size_t seconds) const
{
    return sample_rate*seconds/buffer_size;
}

GateRecorder::buffer_type GateRecorder::bflush(size_t tail_return)
{
    buffer_type tailb;
    while(tail_return--)
    {
        tailb.emplace_front(frames_buffer.back());
        frames_buffer.pop_back();
    }

    std::thread t(audio_dumper, std::move(frames_buffer), af);
    t.detach();
    frames_past_loud = 0;
    return tailb;
}
