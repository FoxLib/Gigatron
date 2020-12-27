#include "gigatron.h"
#include <math.h>

// Аудиобуфер на 16 фреймов для предзагрузки
void play_audio_buffer(void* udata, unsigned char* stream, int len) {

    // Выдача данных
    for (int i = 0; i < 441; i++) {

        int v = au_data_buffer[441*au_sdl_frame + i];

        // left + right
        stream[i] = v;
    }

    // К следующему (если можно)
    if (au_sdl_frame != au_cpu_frame) {
        au_sdl_frame = ((au_sdl_frame + 1) % 16);
    } // Если догнал - то отстать на несколько кадров
    else {
        au_sdl_frame = ((au_sdl_frame + 16) - 8) % 16;
    }
}

void Gigatron::audio_init() {

    // Количество семплов 441 x 100 = 44100
    audio_device.freq     = 44100;
    audio_device.format   = AUDIO_S8;
    audio_device.channels = 1;
    audio_device.samples  = 2*441;
    audio_device.callback = play_audio_buffer;
    audio_device.userdata = NULL;

    if (SDL_OpenAudio(& audio_device, NULL) < 0) {
        fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
        exit(1);
    }

    // Очистить буфер
    for (int i = 0; i < 16*441; i++)
        au_data_buffer[i] = 0;

    // Первичная инициализация
    au_cycle = 0;
    au_sdl_frame = 0;
    au_cpu_frame = 0;
    au_cpu_shift = 0;
    au_sample_shift = 0;

    SDL_PauseAudio(0);
}

// Один аудиотик (10 мс)
void Gigatron::audio_tick() {

    int let_sample = (outx >> 4);

    // Отсчеты внутри кольцевого буфера
    au_sample_shift += 441;

    if (au_sample_shift >= 62500) {
        au_sample_shift -= 62500;

        au_cycle++;
        if (au_cycle == 441) {

            au_cycle     = 0;
            au_cpu_frame = (au_cpu_frame + 1) % 16;
            au_cpu_shift = 441*au_cpu_frame;
        }
    }

    if (started) au_data_buffer[au_cpu_shift + au_cycle] = 4*(let_sample - 8);
}
