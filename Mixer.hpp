#ifndef MIXER_HPP
#define MIXER_HPP

#include <string>

class Mixer
{
private:
    float adc_gain;
    float dac_gain;
    float balance;

public:
    Mixer(float Adc, float Dac, float Balance)
    {
        adc_gain = Adc;
        dac_gain = Dac;
        balance = Balance;
    }

    ~Mixer() {};

    void mix(float *dest, float *a, float *b, size_t len)
    {
        for (size_t i = 0; i < len; i++)
            dest[i] = a[i] * balance + b[i] * (1 - balance);
    }

    float getAdcGain() const
    {
        return adc_gain;
    }

    void setAdcGain(float value)
    {
        adc_gain = value;
    }

    float getDacGain() const
    {
        return dac_gain;
    }

    void setDacGain(float value)
    {
        dac_gain = value;
    }

    float getBalance() const
    {
        return balance;
    }

    void setBalance(float value)
    {
        balance = value;
    }
};

#endif