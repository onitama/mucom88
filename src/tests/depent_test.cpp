// OsDependent Test
// BouKiCHi 2019

#include <stdio.h>

// mainをSDL_mainにするために必要
#ifdef USE_SDL
#include <SDL.h>
#endif

#include "mucomvm_os.h"

class TestClass  {
public:
    TestClass();
    ~TestClass();

    void Info();

    // サウンド
    void TestAudio();
    void AudioCallback(void *mix, int size);
    void AudioTimerCallback(int tick);

    // タイマー
    void TestTimer();
    void TimerCallback(int tick);

    // 時間
    void TestTiming();
    
    // コールバック

private:
    OsDependent *osd;
    bool CallbackFlag;
    int PassTick;
};

TestClass::TestClass() {
    osd = new OSDEP_CLASS();
    osd->CoInitialize();
}

TestClass::~TestClass() {
    delete osd;
}

void TestClass::Info() {
    printf("TestClass : Tests OS dependent code.\n");
}

#define RATE 55467				// Sampling Rate 55K
#define BUFSIZE 200				// Stream Buffer 200ms

static void RunAudioCallback(void *CallbackInstance,void *MethodInstance);
static void RunTimerCallback2(void *CallbackInstance,void *MethodInstance);

// サウンド
void TestClass::TestAudio() {
    printf("TestAudio\n");
    PassTick = 0;
    osd->UserAudioCallback->Set(this,&RunAudioCallback);
    osd->UserTimerCallback->Set(this,&RunTimerCallback2);
    if (!osd->InitTimer()) return;
    if (!osd->InitAudio(NULL,RATE,BUFSIZE)) return;
    printf("Playing...\n");

    while(1) {
        if (PassTick >= 2000) break;
    }

    printf("Finish\n");
    osd->FreeTimer();
    osd->FreeAudio();
}

// オーディオコールバック
static void RunAudioCallback(void *CallbackInstance,void *MethodInstance) {
    AudioCallback *acb = (AudioCallback *)CallbackInstance;
    ((TestClass *)MethodInstance)->AudioCallback(acb->mix,acb->size);
}

// タイマーコールバック
static void RunTimerCallback2(void *CallbackInstance,void *MethodInstance) {
    TimerCallback *tcb = (TimerCallback *)CallbackInstance;
    ((TestClass *)MethodInstance)->AudioTimerCallback(tcb->tick);
}

int StreamCount = 0;

void TestClass::AudioTimerCallback(int tick) {
    PassTick += tick;
    StreamCount += tick;
    if (StreamCount >= 20) {
        StreamCount = 0;
        osd->SendAudio(StreamCount);
    }  
}


int step = 0;

// オーディオ処理
void TestClass::AudioCallback(void *mix,int size) {
    int *buf = (int *)mix;
    for(int i = 0; i < size*2; i++) {
        int val = step * 50;
        buf[i] = val;
        step++;
        if (step >= 256) step = 0;
    }
}


// コールバックで呼ばれる
static void RunTimerCallback(void *CallbackInstance,void *MethodInstance) {
    TimerCallback *cb = (TimerCallback *)CallbackInstance;
    ((TestClass *)MethodInstance)->TimerCallback(cb->tick);
}

void TestClass::TimerCallback(int tick) {
    PassTick += (tick/1024);
}



// タイマー
void TestClass::TestTimer() {
    printf("TestTimer\n");
    PassTick = 0;
    osd->UserTimerCallback->Set(this,&RunTimerCallback);
    osd->InitTimer();
    while(1) {
        if (PassTick >= 100) break;
        osd->Delay(10);
    }
    printf("PassTick = %d\n", PassTick);
    printf("OK\n");
    osd->FreeTimer();
}



// タイミング
void TestClass::TestTiming() {
    printf("TestTiming\n");
    int start = osd->GetMilliseconds();
    osd->Delay(10);
    int end = osd->GetMilliseconds();
    printf("Result: %d Start:%d End:%d\n", end - start, start, end);
}


// メイン
int main(int argc, char *argv[])
{
#if defined(USE_SDL) && defined(_WIN32)
    freopen( "CON", "w", stdout );
    freopen( "CON", "w", stderr );
#endif


    TestClass *tc = new TestClass();
    tc->Info();
    tc->TestTiming();
    tc->TestTimer();
    tc->TestAudio();

    delete tc;
}