// Callback 
// BouKiCHi 2019


#ifndef _CALLBACK_H_
#define _CALLBACK_H_

typedef void (*CALLBACK_PTR)(void *CallbackInstance, void *MethodInstance);

class Callback  {
public:
	// 繧ｳ繝ｼ繝ｫ繝舌ャ繧ｯ
    Callback();

    void Set(void *MethodInstance, CALLBACK_PTR MethodPointer);
    void Run();
protected:
	CALLBACK_PTR Method;
	void *Instance;
};

class AudioCallback : public Callback {
public:
    void *mix;
    int size;
};


class TimerCallback : public Callback {
public:
    int tick;
};


#endif
