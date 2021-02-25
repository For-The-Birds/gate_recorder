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

GateRecorder::GateRecorder(float loudness, float cutoff_, float rolloff_)
    : JackCpp::AudioIO("gate_recorder", 2, 2)
    , loudness_threshold(loudness)
    , cutoff(cutoff_)
    , rolloff(rolloff_)
{
    printf("%s, loudness_threshold: %.2f\n", kfr::library_version(), loudness_threshold);

    //chunks = getBufferSize()/chunk_size;

    af.samplerate = getSampleRate();
    af.channels = 1;

    max_frames_wait = frames_in_seconds(10);
    frames_begin = frames_in_seconds(1);
    frames_end = frames_in_seconds(1);
    buffer_limit_soft = frames_in_seconds(60*1);
    buffer_limit_hard = frames_in_seconds(60*3);

    if (rolloff > 0 && cutoff > 0)
    {
        printf("Using highpass filter cutoff %2.f, rolloff %.2f\n", cutoff, rolloff);
        filt = kfr::iir_highpass(kfr::bessel<kfr::fbase>(rolloff), cutoff, getSampleRate());
        bqs = kfr::to_sos(filt);
    }

    start();
}

int GateRecorder::audioCallback(jack_nframes_t nframes, JackCpp::AudioIO::audioBufVector inBufs, JackCpp::AudioIO::audioBufVector outBufs)
{
    frames_buffer.emplace_back(kfr::make_univector(inBufs[0], nframes));

    if (rolloff > 0 && cutoff > 0)
    {
        kfr::univector<float> filtered = kfr::biquad<32>(bqs, frames_buffer.back());
        float * p = &outBufs[0][0];
        for (auto & v : filtered)
            *p++ = v;
        frames_buffer.back() = filtered;
    }

    if (passthrough)
        frames_passthrough.emplace_back(frames_buffer.back());

    std::map<float, int> loud_samples;
    int loud_samples_total = 0;
    for (auto & sample : frames_buffer.back())
    {
        auto l = kfr::energy_to_loudness(sample);
        if (l > loudness_threshold)
            ++loud_samples_total;
        for (int i=-8; i <= 12; i += 2)
        {
            auto t = loudness_threshold - i;
            loud_samples[t];
            if (l > t) {
                ++loud_samples[t];
                break;
            }
        }
    }

    for (auto & ls : loud_samples)
        if (ls.first < loudness_threshold)
            printf("%5.2f(%3d)  ", ls.first, ls.second);
        else
            printf("%5.2f{%3d}  ", ls.first, ls.second);

    printf("   ls:%3d", loud_samples[loudness_threshold]);
    if (loud_samples_total > 100)
    {
        frames_past_loud = 0;
        if (!recording)
        {
            while(frames_buffer.size() > frames_begin)
                frames_buffer.pop_front();
            passthrough = recording = true;
            if (frames_passthrough.size() == 0)
                frames_passthrough = frames_buffer;
        }
    }


    ++frames_past_loud;

    if (frames_past_loud > frames_end && passthrough)
    {
        passthrough = false;
    }

    if (frames_past_loud == max_frames_wait && recording)
    {
        // Too long silence. Normal end of recording.
        frames_buffer = bflush(max_frames_wait - frames_end);
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

    printf("   fo:%3lu", frames_passthrough.size());
    if (frames_passthrough.size() > 0)
    {
        std::copy_n(frames_passthrough.front().begin(), nframes, outBufs[0]);
        frames_passthrough.pop_front();
    }
    else
        std::fill_n(outBufs[0], nframes, 0);

    if (frames_past_loud > max_frames_wait)
    {
        // Too long after last loud frame. This means that we are not recording
        // and do not need to pile up frames_buffer
        frames_buffer.pop_front();

        frames_past_loud = max_frames_wait+1; // avoid overflow
    }

    if (recording)
        printf("\n");
    else
        printf("\r");
    fflush(stdout);
    return 0;
}

size_t GateRecorder::frames_in_seconds(size_t seconds) const
{
    return getSampleRate()*seconds/getBufferSize();
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
