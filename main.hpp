#ifndef MAIN_HPP
#define MAIN_HPP

#include "array"
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <pthread.h>
#include "Pcm.hpp"
#include "Fdn.hpp"
#include "Loop.hpp"
#include "Mixer.hpp"
#include "Command.hpp"
#include <fstream>

#define PCM_BUFFER_LEN 128
#define NB_CMD 6
#define GAIN_MIN 0.00
#define GAIN_INC 0.01
#define GAIN_MAX 4.0

typedef struct s_env
{
    Fdn *fdn;
    Mixer *mixer;
    Loop *loop;
    std::vector<Command> commands;
    bool running;
} t_env;

#endif