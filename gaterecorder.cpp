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

void audio_writer(GateRecorder::buffer_type buf, kfr::audio_format af)
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

GateRecorder::GateRecorder(float loudness)
    : JackCpp::AudioIO("gate_recorder", 2, 0)
    , loudness_threshold(loudness)
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

    start();
}

int GateRecorder::audioCallback(jack_nframes_t nframes, JackCpp::AudioIO::audioBufVector inBufs, JackCpp::AudioIO::audioBufVector outBufs)
{
    frames_buffer.emplace_back(kfr::make_univector(inBufs[0], nframes));

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
        printf("---\n");
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

    std::thread t(audio_writer, std::move(frames_buffer), af);
    t.detach();
    frames_past_loud = 0;
    return tailb;
}
