#ifndef FDN_HPP
#define FDN_HPP

#include "main.hpp"
#include "Pcm.hpp"
#include <iostream>
#include <array>
#include <vector>
#include <math.h>

#define FDN_NB_BUFFER 4

int ADAMAR_MATRIX[][FDN_NB_BUFFER] = {
    {1, 1, 1, 1},
    {1, -1, 1, -1},
    {1, 1, -1, -1},
    {1, -1, -1, 1}};

class Fdn
{
private:
    std::array<float *, FDN_NB_BUFFER> delay;
    std::array<size_t, FDN_NB_BUFFER> phase;
    std::array<float, FDN_NB_BUFFER> filter;
    float input_gain;
    float gain;
    float cutoff_frequency;
    float cutoff_coeficient;
    size_t delay_buffer_size[FDN_NB_BUFFER];

    size_t getPhase(int id, int inc)
    {
        return (phase[id] + inc) % delay_buffer_size[id];
    }

public:
    Fdn(float InputGain, float OutputGain, float CutoffFrequency, size_t DelayBufferSize[FDN_NB_BUFFER])
    {
        setInputGain(InputGain);
        setOutputGain(OutputGain);
        setCutoffFrequency(CutoffFrequency);
        for (int i = 0; i < FDN_NB_BUFFER; i++)
        {
            delay_buffer_size[i] = DelayBufferSize[i];
            delay[i] = new float[delay_buffer_size[i]];
            memset(delay[i], 0, sizeof(float) * delay_buffer_size[i]);
        }

        phase.fill(0);
        filter.fill(0.0f);
    }
    ~Fdn()
    {
        for (std::array<float *, FDN_NB_BUFFER>::iterator it = delay.begin(); it != delay.end(); it++)
            delete[] *it;
    };

    float getInputGain()
    {
        return input_gain;
    }

    void setInputGain(float value)
    {
        input_gain = value;
    }

    float getGain()
    {
        return gain;
    }

    void setOutputGain(float value)
    {
        gain = value;
    }

    float getCutoffFrequency()
    {
        return cutoff_frequency;
    }

    void setCutoffFrequency(float value)
    {
        float v;

        cutoff_frequency = value;
        v = value * 2 * M_PI;
        cutoff_coeficient = v / (v + Pcm::getFrameRate());
    }

    void read(float *buffer, size_t len)
    {
        size_t temp_phase[FDN_NB_BUFFER];
        float temp_value[FDN_NB_BUFFER];
        float filtered[FDN_NB_BUFFER];

        for (int i = 0; i < len; i++)
        {
            for (int j = 0; j < FDN_NB_BUFFER; j++)
            {
                temp_phase[j] = getPhase(j, i);
                filter[j] = filter[j] + cutoff_coeficient * (delay[j][temp_phase[j]] - filter[j]);
                filtered[j] = filter[j] * gain;
            }
            for (int j = 0; j < FDN_NB_BUFFER; j++)
            {
                temp_value[j] = 0;
                for (int k = 0; k < FDN_NB_BUFFER; k++)
                    temp_value[j] += (filtered[k] * 0.5f) * ADAMAR_MATRIX[j][k];
            }
            for (int j = 0; j < FDN_NB_BUFFER; j++)
            {
                if (j == 0)
                    temp_value[j] += +buffer[i] * input_gain;
                delay[j][temp_phase[j]] = temp_value[j];
            }
        }
    }

    void write(float *buffer, size_t len)
    {
        for (int i = 0; i < len; i++)
        {
            buffer[i] = 0;
            for (int j = 0; j < FDN_NB_BUFFER; j++)
                buffer[i] += delay[j][getPhase(j, i)] / 4;
        }
    }

    void incPhases(size_t value)
    {
        for (int i = 0; i < FDN_NB_BUFFER; i++)
            phase[i] = getPhase(i, value);
    }

    void print()
    {
        std::cout << "\r" << input_gain << "\t\t\t"
                  << gain << "\t\t\t"
                  << cutoff_frequency << std::flush;
    }
};
#endif
