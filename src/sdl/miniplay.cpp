#include <stdio.h>
#include <signal.h>
#include <SDL.h>
#include "audiosdl.h"
#include "mucom_module.h"

static volatile sig_atomic_t s_break_flag = 0;
static void s_signal_handler(int) {
    s_break_flag = 1;
}

class Player {
public:
    Player();
    ~Player();
    int Play(const char *filename);
    void Stop();

    void AudioCallback(void *mix, int size);

    AudioSdl *sdl;
    MucomModule *module;
private:
    bool EventCheck();
};

Player::Player() {
}

Player::~Player() {
}

bool Player::EventCheck() {
    if (s_break_flag) return true;

    SDL_Event evt;

    while(SDL_PollEvent(&evt)) {
        switch(evt.type) {
        case SDL_QUIT:
            return true;
        break;
        }
    }
    return false;
}

// オーディオコールバック
static void RunAudioCallback(void *CallbackInstance,void *MethodInstance);

static void RunAudioCallback(void *CallbackInstance,void *MethodInstance) {
    AudioCallback *acb = (AudioCallback *)CallbackInstance;
    ((Player *)MethodInstance)->AudioCallback(acb->mix, acb->size);
}

// オーディオ処理
void Player::AudioCallback(void *mix, int size) {
    module->Mix((short*)mix, size);
}

int Player::Play(const char *filename) {
    sdl = new AudioSdl();
    sdl->UserAudioCallback->Set(this,RunAudioCallback);
    module = new MucomModule();

    printf("File:%s\n", filename);
    bool r = module->Open(".", filename);
    puts(module->GetResult());
    if (!r) return -1;
    r = module->Play();
    if (!r) return -1;

    sdl->Open(44100);

    // SIGINT/SIGTERM を自前ハンドラで捕捉しブレークフラグを立てる
    signal(SIGINT, s_signal_handler);
    signal(SIGTERM, s_signal_handler);

    // イベントループ
    printf("Playing..\n");
    while(1) {
        if (EventCheck()) break;
        SDL_Delay(20);
    }
    return 0;
}

void Player::Stop() {
    sdl->Close();
    delete sdl;
    sdl = NULL;

    module->Close();
    delete module;
    module = NULL;
}


int main(int argc,char *argv[]) {
#if defined(USE_SDL) && defined(_WIN32)
    freopen("CON", "w", stdout);
    freopen("CON", "w", stderr );
#endif
    printf("MUCOM88 miniplay\n");
    if (argc < 2) {
        printf("usage miniplay <song.muc>\n");
        return 0;
    }
    Player *p = new Player();
    p->Play(argv[1]);
    p->Stop();
    return 0;
}