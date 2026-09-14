#ifndef COMMAND
#define COMMAND

#include <string>
#include <string.h>
#include <functional>

class Command
{
private:
    std::string name;
    float value;
    float min;
    float max;
    float inc;
    char keys[2];
    std::function<float()> get;
    std::function<void(float)> set;

public:
    Command() {};

    Command(
        std::string Name,
        float Min,
        float Max,
        float Inc,
        char a,
        char b,
        std::function<float()> Get,
        std::function<void(float)> Set)
        : name(Name),
          min(Min),
          max(Max),
          inc(Inc),
          get(Get),
          set(Set)
    {
        keys[0] = a;
        keys[1] = b;
        value = get();
    }

    ~Command()
    {
    }

    std::string getName()
    {
        return name;
    }

    std::string getKeysName()
    {
        return {keys[0], ' ', '|', ' ', keys[1]};
    }

    float getValue()
    {
        return value;
    }

    void exec(char key)
    {
        float temp;
        float direction[] = {1.0f, -1.0f};
        for (size_t i = 0; i < 2; i++)
        {
            if (key == keys[i])
            {
                temp = value + direction[i] * inc;
                if (temp >= min && temp <= max)
                {
                    set(temp);
                    value = get();
                }
            }
        }
    }
};

#endif