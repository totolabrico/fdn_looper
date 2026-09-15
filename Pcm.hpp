#ifndef PCM_HPP
#define PCM_HPP

#include "/usr/include/alsa/asoundlib.h" // must be compiled with -lasound flag
#include <iostream>
#include <string>

class Pcm
{
private:
    static constexpr snd_pcm_stream_t STREAM_TYPE[] = {SND_PCM_STREAM_CAPTURE, SND_PCM_STREAM_PLAYBACK};
    static constexpr _snd_pcm_format FORMAT = SND_PCM_FORMAT_S32_LE; // SND_PCM_FORMAT_FLOAT;
    static constexpr _snd_pcm_access ACCESS = SND_PCM_ACCESS_RW_INTERLEAVED;
    static constexpr unsigned int SOFT_RESAMPLE = 0;
    static constexpr unsigned int FRAME_RATE = 44100;
    static constexpr snd_pcm_uframes_t FRAME_LEN = 128;
    static constexpr unsigned int LATENCY = 10000;
    snd_pcm_t *_handle;
    unsigned int _channels;

public:
    Pcm(std::string const &device, unsigned int type, unsigned int channels)
    {
        std::cout << "New Pcm: " << device << std::endl;
        if (device == "default")
            std::cout << "Warning : default setting may involve latency" << std::endl;
        if (type != 0 && type != 1)
            type = 0;
        open(device, type);
        _channels = channels;
        setParams();
    }

    void open(std::string const &device, unsigned int type)
    {
        int rc;

        rc = snd_pcm_open(&_handle, device.c_str(), STREAM_TYPE[type], 0);
        if (rc < 0)
        {
            std::cout << "Unable to open pcm device for capture: " << snd_strerror(rc) << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    void setParams()
    {
        int rc;

        rc = snd_pcm_set_params(_handle,
                                FORMAT,
                                ACCESS,
                                _channels,
                                FRAME_RATE,
                                SOFT_RESAMPLE,
                                LATENCY);
        if (rc < 0)
        {
            std::cout << "Pcm hardware settings error\n"
                      << std::endl;
            exit(EXIT_FAILURE);
        }

        snd_pcm_uframes_t buffer_size;
        snd_pcm_uframes_t period_size;
        snd_pcm_get_params(_handle, &buffer_size, &period_size);
        std::cout << "buffer_size = " << buffer_size << '\n';
        std::cout << "period_size = " << period_size << '\n';
    }

    ~Pcm()
    {
        snd_pcm_close(_handle);
    }

    void read(int32_t *buffer, size_t len)
    {
        int rc;

        rc = snd_pcm_readi(_handle, buffer, len / _channels);
        if (rc < 0)
            std::cout << "Read Error: " << snd_strerror(rc) << std::endl;
        if (rc == -EPIPE)
            snd_pcm_prepare(_handle);
    }

    void write(int32_t *buffer, size_t len)
    {
        int rc;
        rc = snd_pcm_writei(_handle, buffer, len / _channels);
        if (rc < 0)
            std::cout << "Write Error: " << snd_strerror(rc) << std::endl;
        if (rc == -EPIPE)
            snd_pcm_prepare(_handle);
    }

    static unsigned int getFrameRate()
    {
        return FRAME_RATE;
    }
};

#endif