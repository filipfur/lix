#include "unit_test.h"

#include <SDL2/SDL.h>
#include <math.h>

#define SAMPLE_RATE 44100
#define AMPLITUDE 28000
float AMP{0.0f};
float FREQUENCY = 440.0f;
bool PLAY{false};
float SHARPNESS = 0.2f;

const std::vector<float> OCTAVES[] = {{
                                          16.35f,
                                          17.32f,
                                          18.35f,
                                          19.45f,
                                          20.60f,
                                          21.83f,
                                          23.12f,
                                          24.50f,
                                          25.96f,
                                          27.50f,
                                          29.14f,
                                          30.87f,
                                      },
                                      {
                                          32.70f,
                                          34.65f,
                                          36.71f,
                                          38.89f,
                                          41.20f,
                                          43.65f,
                                          46.25f,
                                          49.00f,
                                          51.91f,
                                          55.00f,
                                          58.27f,
                                          61.74f,
                                      },
                                      {
                                          65.41f,
                                          69.30f,
                                          73.42f,
                                          77.78f,
                                          82.41f,
                                          87.31f,
                                          92.50f,
                                          98.00f,
                                          103.8f,
                                          110.0f,
                                          116.5f,
                                          123.5f,
                                      },
                                      {
                                          130.8f,
                                          138.6f,
                                          146.8f,
                                          155.6f,
                                          164.8f,
                                          174.6f,
                                          185.0f,
                                          196.0f,
                                          207.7f,
                                          220.0f,
                                          233.1f,
                                          246.9f,
                                      },
                                      {
                                          261.6f,
                                          277.2f,
                                          293.7f,
                                          311.1f,
                                          329.6f,
                                          349.2f,
                                          370.0f,
                                          392.0f,
                                          415.3f,
                                          440.0f,
                                          466.2f,
                                          493.9f,
                                      },
                                      {
                                          523.3f,
                                          554.4f,
                                          587.3f,
                                          622.3f,
                                          659.3f,
                                          698.5f,
                                          740.0f,
                                          784.0f,
                                          830.6f,
                                          880.0f,
                                          932.3f,
                                          987.8f,
                                      },
                                      {
                                          1047.0f,
                                          1109.0f,
                                          1175.0f,
                                          1245.0f,
                                          1319.0f,
                                          1397.0f,
                                          1480.0f,
                                          1568.0f,
                                          1661.0f,
                                          1760.0f,
                                          1865.0f,
                                          1976.0f,
                                      },
                                      {
                                          2093.0f,
                                          2217.0f,
                                          2349.0f,
                                          2489.0f,
                                          2637.0f,
                                          2794.0f,
                                          2960.0f,
                                          3136.0f,
                                          3322.0f,
                                          3520.0f,
                                          3729.0f,
                                          3951.0f,
                                      },
                                      {4186.0f, 4435.0f, 4699.0f, 4978.0f,
                                       5274.0f, 5588.0f, 5920.0f, 6272.0f,
                                       6645.0f, 7040.0f, 7459.0f, 7902.0f}};

enum Key { C, C_, D, Eb, E, F, F_, G, G_, A, Bb, B, NONE };

float lerp(float a, float b, float f) { return b * f + a * (1.0f - f); }

void audio_callback(void *userdata, Uint8 *stream, int len) {
    static double phase = 0;
    Sint16 *buffer = (Sint16 *)stream;
    int length = len / 2; // len is in bytes, we need samples

    if (PLAY) {
        if (AMP < AMPLITUDE) {
            AMP = lerp(AMP, AMPLITUDE, 0.2f);
            if (AMPLITUDE - AMP < 1.0f) {
                AMP = AMPLITUDE;
            }
        }
    } else {
        if (AMP > 0) {
            AMP = lerp(AMP, 0.0f, 0.3f);
            if (AMP < 1000.0f) {
                AMP = 0.0f;
            }
        }
    }

    for (int i = 0; i < length; i++) {
        // buffer[i] = (Sint16)(AMPLITUDE * sin(phase));
        buffer[i] = (Sint16)(AMP * sinf(lerp(phase, roundf(phase), SHARPNESS)));
        phase += 2.0 * M_PI * FREQUENCY / SAMPLE_RATE;
        if (phase > 2.0 * M_PI) {
            phase -= 2.0 * M_PI;
        }
    }
}

void TEST() {
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_EVENTS) < 0) {
        fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_AudioSpec want, have;
    SDL_AudioDeviceID dev;

    SDL_zero(want);
    want.freq = SAMPLE_RATE;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 4096;
    want.callback = audio_callback;

    dev = SDL_OpenAudioDevice(NULL, 0, &want, &have,
                              SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    if (dev == 0) {
        fprintf(stderr, "Failed to open audio: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_PauseAudioDevice(dev, 0);

    /*for(size_t i{0}; i < 10; ++i)
    {
        SDL_Delay(100);
        FREQUENCY += 100;
        SDL_Delay(100);
        FREQUENCY += 100;
        SDL_Delay(100);
        SHARPNESS += 0.1f;
        FREQUENCY -= 200;
    }*/

    int octave = 5;

    for (const auto &tone : {C, D, F_, NONE, C, D, G_, NONE}) {
        if (tone == NONE) {
            PLAY = false;
        } else {
            FREQUENCY = OCTAVES[octave].at(tone);
            PLAY = true;
        }
        SDL_Delay(100);
    }

    auto window = SDL_CreateWindow("untitled", 0, 0, 400, 400, 0);

    SDL_Event event;
    bool running{true};
    bool pressed[12] = {};  // C, C_, D, Eb, E, F, F_, G, G_, A, Bb, B, NONE
    bool pressed2[12] = {}; // C, C_, D, Eb, E, F, F_, G, G_, A, Bb, B, NONE
    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                case SDLK_UP:
                    octave = std::min(octave + 1,
                                      static_cast<int>(sizeof(OCTAVES) - 2));
                    break;
                case SDLK_DOWN:
                    octave = std::max(octave - 1, 0);
                    break;
                case SDLK_q:
                    pressed[0] = true; // C
                    break;
                case SDLK_2:
                    pressed[1] = true; // C_
                    break;
                case SDLK_w:
                    pressed[2] = true; // D
                    break;
                case SDLK_3:
                    pressed[3] = true; // Eb
                    break;
                case SDLK_e:
                    pressed[4] = true; // E
                    break;
                case SDLK_r:
                    pressed[5] = true; // F
                    break;
                case SDLK_5:
                    pressed[6] = true; // F_
                    break;
                case SDLK_t:
                    pressed[7] = true; // G
                    break;
                case SDLK_6:
                    pressed[8] = true; // G_
                    break;
                case SDLK_y:
                    pressed[9] = true; // A
                    break;
                case SDLK_7:
                    pressed[10] = true; // Bb
                    break;
                case SDLK_u:
                    pressed[11] = true; // B
                    break;
                case SDLK_z:
                    pressed2[0] = true; // C
                    break;
                case SDLK_s:
                    pressed2[1] = true; // C_
                    break;
                case SDLK_x:
                    pressed2[2] = true; // D
                    break;
                case SDLK_d:
                    pressed2[3] = true; // Eb
                    break;
                case SDLK_c:
                    pressed2[4] = true; // E
                    break;
                case SDLK_v:
                    pressed2[5] = true; // F
                    break;
                case SDLK_g:
                    pressed2[6] = true; // F_
                    break;
                case SDLK_b:
                    pressed2[7] = true; // G
                    break;
                case SDLK_h:
                    pressed2[8] = true; // G_
                    break;
                case SDLK_n:
                    pressed2[9] = true; // A
                    break;
                case SDLK_j:
                    pressed2[10] = true; // Bb
                    break;
                case SDLK_m:
                    pressed2[11] = true; // B
                    break;
                default:
                    break;
                }
                break;
            case SDL_KEYUP:
                switch (event.key.keysym.sym) {
                case SDLK_q:
                    pressed[0] = false; // C
                    break;
                case SDLK_2:
                    pressed[1] = false; // C_
                    break;
                case SDLK_w:
                    pressed[2] = false; // D
                    break;
                case SDLK_3:
                    pressed[3] = false; // Eb
                    break;
                case SDLK_e:
                    pressed[4] = false; // E
                    break;
                case SDLK_r:
                    pressed[5] = false; // F
                    break;
                case SDLK_5:
                    pressed[6] = false; // F_
                    break;
                case SDLK_t:
                    pressed[7] = false; // G
                    break;
                case SDLK_6:
                    pressed[8] = false; // G_
                    break;
                case SDLK_y:
                    pressed[9] = false; // A
                    break;
                case SDLK_7:
                    pressed[10] = false; // Bb
                    break;
                case SDLK_u:
                    pressed[11] = false; // B
                    break;
                case SDLK_z:
                    pressed2[0] = false; // C
                    break;
                case SDLK_s:
                    pressed2[1] = false; // C_
                    break;
                case SDLK_x:
                    pressed2[2] = false; // D
                    break;
                case SDLK_d:
                    pressed2[3] = false; // Eb
                    break;
                case SDLK_c:
                    pressed2[4] = false; // E
                    break;
                case SDLK_v:
                    pressed2[5] = false; // F
                    break;
                case SDLK_g:
                    pressed2[6] = false; // F_
                    break;
                case SDLK_b:
                    pressed2[7] = false; // G
                    break;
                case SDLK_h:
                    pressed2[8] = false; // G_
                    break;
                case SDLK_n:
                    pressed2[9] = false; // A
                    break;
                case SDLK_j:
                    pressed2[10] = false; // Bb
                    break;
                case SDLK_m:
                    pressed2[11] = false; // B
                    break;
                default:
                    break;
                }
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                }
                break;
            default:
                break;
            }
        }
        bool play = false;
        for (unsigned int i{0}; i < 12; ++i) {
            if (pressed[i]) {
                FREQUENCY = OCTAVES[octave].at(i);
                AMP = AMPLITUDE;
                play = true;
                break;
            }
            if (pressed2[i]) {
                FREQUENCY = OCTAVES[octave + 1].at(i);
                AMP = AMPLITUDE;
                play = true;
                break;
            }
        }
        PLAY = play;
        SDL_Delay(10);
    }

    SDL_CloseAudioDevice(dev);
    SDL_Quit();
}