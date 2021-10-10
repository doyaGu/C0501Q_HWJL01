/*
by powei
*/

#ifndef hwengine_h
#define hwengine_h

typedef enum HWEngineId{
    HW_EC_ID = 0,
	HW_ENC_ID,
	HW_DEC_ID,
	HW_ENGINE_COUNT
} HWEngineId;

typedef int (*HWEngineFunc)(void *arg);

struct _HWEngineDesc{
	HWEngineId id;
	HWEngineFunc process;
};

typedef struct _HWEngineDesc HWEngineDesc;

void hw_engine_init(void);

void hw_engine_link_filter(HWEngineId id, void* filter);

void hw_engine_uninit(void);

void hw_engine_lock(void);

void hw_engine_unlock(void);

#endif
