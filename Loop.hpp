#ifndef LOOP_HPP
#define LOOP_HPP

#include "Pcm.hpp"
#include <vector>
#include <string>

class Loop
{
private:
    unsigned int buffer_size;
    unsigned int sample_size;
    std::vector<float> buffer;
    bool recording;
    size_t phase;

public:
    Loop()
    {
        buffer_size = Pcm::getFrameRate() * 120;
        sample_size = buffer_size;
        phase = 0;
        buffer.resize(buffer_size);
        setRecordState(false);
    }

    bool getRecordState() const
    {
        return recording;
    }

    void setRecordState(bool Value)
    {
        if (recording == Value)
            return;
        if (recording)
            sample_size = phase;
        recording = Value;
        phase = 0;
    }

    void clear()
    {
        recording = false;
        std::fill(buffer.begin(), buffer.begin() + sample_size, 0.0f);
        sample_size = buffer_size;
    }

    void run(float *read_buffer, float *write_buffer, size_t len)
    {
        if (recording)
            read(read_buffer, len);
        else
            write(write_buffer, len);
    }

    void read(float *buf, size_t len)
    {
        for (size_t i = 0; i < len; i++)
        {
            buffer[phase] = buf[i];
            phase = phase + 1;
            if (phase == buffer_size)
            {
                setRecordState(false);
                break;
            }
        }
    }

    void write(float *buf, size_t len)
    {
        for (size_t i = 0; i < len; i++)
        {
            buf[i] = buffer[phase];
            phase = phase + 1;
            if (phase == sample_size)
                phase = 0;
        }
    }

    std::string print()
    {
        if (recording)
            return "RECORDING";
        return "PLAYING";
    }
};
#endif
