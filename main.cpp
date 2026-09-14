#include "main.hpp"

void print_header()
{
    std::ifstream logo("logo.txt");
    std::string line;
    while (getline(logo, line))
        std::cout << line << std::endl;
    std::cout << std::endl;
}

void init_env(t_env *env, Fdn *f, Mixer *m, Loop *loop)
{
    env->fdn = f;
    env->mixer = m;
    env->loop = loop;
    env->running = true;
}

char read_key()
{
    char c;
    termios old, current;

    tcgetattr(STDIN_FILENO, &old);
    current = old;
    current.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &current);
    read(STDIN_FILENO, &c, 1);
    tcsetattr(STDIN_FILENO, TCSANOW, &old);
    return c;
}

bool loop_cmd(Loop *loop, char key)
{
    if (key == '$')
    {
        loop->setRecordState(!loop->getRecordState());
        return true;
    }
    else if (key == 'c')
    {
        loop->clear();
        return true;
    }
    return false;
}

void *input_routine(void *addr)
{
    t_env *env = reinterpret_cast<t_env *>(addr);
    Fdn *fdn = env->fdn;
    Loop *loop = env->loop;
    std::vector<Command> commands = env->commands;
    std::string cmd;
    float value;

    for (auto it = commands.begin(); it != commands.end(); it++)
        std::cout << it->getName() << "\t";
    std::cout << std::endl;
    for (auto it = commands.begin(); it != commands.end(); it++)
        std::cout << it->getKeysName() << "\t\t";
    std::cout << std::endl;
    while (true)
    {
        std::cout << "\x1b[2K\r";
        for (auto it = commands.begin(); it != commands.end(); it++)
            std::cout << round(it->getValue() * 100) / 100 << "\t\t";
        std::cout << loop->print() << std::flush;
        char key = read_key();
        if (key == 27)
            break;
        if (loop_cmd(loop, key))
            continue;
        for (auto it = commands.begin(); it != commands.end(); it++)
            it->exec(key);
    }
    env->running = false;
    return NULL;
}

void multiplyFloatArray(float *array, float value, size_t len)
{
    for (size_t i = 0; i < len; i++)
        array[i] *= value;
}

void init_commands(t_env *env)
{
    env->commands.push_back(Command("ADC GAIN", GAIN_MIN, GAIN_MAX, GAIN_INC, 'a', 'q', [&]()
                                    { return env->mixer->getAdcGain(); }, [&](float x)
                                    { env->mixer->setAdcGain(x); }));
    env->commands.push_back(Command("DAC GAIN", GAIN_MIN, GAIN_MAX, GAIN_INC, 'z', 's', [&]()
                                    { return env->mixer->getDacGain(); }, [&](float x)
                                    { env->mixer->setDacGain(x); }));
    env->commands.push_back(Command("MIX ADC | FDN", GAIN_MIN, GAIN_MAX, GAIN_INC, 'e', 'd', [&]()
                                    { return env->mixer->getBalance(); }, [&](float x)
                                    { env->mixer->setBalance(x); }));
    env->commands.push_back(Command("FDN IN GAIN", GAIN_MIN, GAIN_MAX, GAIN_INC, 'r', 'f', [&]()
                                    { return env->fdn->getInputGain(); }, [&](float x)
                                    { env->fdn->setInputGain(x); }));
    env->commands.push_back(Command("FDN OUT GAIN", GAIN_MIN, GAIN_MAX, GAIN_INC, 't', 'g', [&]()
                                    { return env->fdn->getGain(); }, [&](float x)
                                    { env->fdn->setOutputGain(x); }));
    env->commands.push_back(Command("FDN CUT FREQ", 0, 10000, 20, 'y', 'h', [&]()
                                    { return env->fdn->getCutoffFrequency(); }, [&](float x)
                                    { env->fdn->setCutoffFrequency(x); }));
}

void loadConfig(float *settings, size_t fdn_buffer_size[FDN_NB_BUFFER], char *path)
{
    size_t i = 0;
    std::ifstream stream(path);
    std::string line;
    while (getline(stream, line) && i < NB_CMD)
    {
        const char *str = line.c_str();
        float v = strtof(str, nullptr);
        settings[i] = v;
        i++;
    }
    if (i != 6)
        std::cout << "Configuration loading failed. Number of settings loaded: " << i << std::endl;
    i = 0;
    while (getline(stream, line) && i < FDN_NB_BUFFER)
    {
        const char *str = line.c_str();
        float v = strtof(str, nullptr);
        fdn_buffer_size[i] = v;
        i++;
    }
    if (i != 4)
        std::cout << "Configuration loading failed. Number of buffer sizes loaded: " << i << std::endl;
}

int main(int argc, char **argv)
{
    float settings[NB_CMD] = {
        0.8,
        0.9,
        0.5,
        0.8,
        0.9,
        4000};
    size_t fdn_buffer_size[FDN_NB_BUFFER] = {
        5890,
        17170,
        1270,
        12230};
    if (argc == 2)
        loadConfig(settings, fdn_buffer_size, argv[1]);
    Pcm adc("default", 0, 1);
    Pcm dac("default", 1, 1);
    Mixer mixer(settings[0], settings[1], settings[2]);
    Fdn fdn(settings[3], settings[4], settings[5], fdn_buffer_size);
    Loop loop;
    t_env env;
    pthread_t cmd_thread;
    float *adc_buffer = new float[PCM_BUFFER_LEN];
    float *fdn_buffer = new float[PCM_BUFFER_LEN];
    float *loop_buffer = new float[PCM_BUFFER_LEN];
    float *dac_buffer = new float[PCM_BUFFER_LEN];

    print_header();
    init_env(&env, &fdn, &mixer, &loop);
    init_commands(&env);

    if (pthread_create(&cmd_thread, NULL, &input_routine, reinterpret_cast<void *>(&env)) != 0)
        exit(1);
    while (env.running)
    {
        adc.read(adc_buffer, PCM_BUFFER_LEN);
        multiplyFloatArray(adc_buffer, mixer.getAdcGain(), PCM_BUFFER_LEN);
        fdn.write(fdn_buffer, PCM_BUFFER_LEN);
        fdn.read(adc_buffer, PCM_BUFFER_LEN);
        fdn.incPhases(PCM_BUFFER_LEN);
        mixer.mix(dac_buffer, adc_buffer, fdn_buffer, PCM_BUFFER_LEN);
        multiplyFloatArray(dac_buffer, mixer.getDacGain(), PCM_BUFFER_LEN);
        loop.run(dac_buffer, loop_buffer, PCM_BUFFER_LEN);
        mixer.mix(dac_buffer, loop_buffer, dac_buffer, PCM_BUFFER_LEN);
        dac.write(dac_buffer, PCM_BUFFER_LEN);
    }
    return 0;
}