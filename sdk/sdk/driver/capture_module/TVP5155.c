#include "iic/mmp_iic.h"
#include "iic/iic.h"
#include "TVP5155.h"
//#include "capture_module.h"  //Benson



typedef struct CaptureModuleDriverTVP5155Struct
{
    CaptureModuleDriverStruct base;
} CaptureModuleDriverTVP5155Struct;

enum {MAX_CAPTURE_MODULE = 2};

static int states[MAX_CAPTURE_MODULE];
static int lastId;
static int lastState;


void CaptureModuleDriverSpy_Reset(void)
{
    int i;
    for (i = 0; i < MAX_CAPTURE_MODULE; i++)
    {
        states[i] = LIGHT_STATE_UNKNOWN;
    }
    lastId = LIGHT_ID_UNKNOWN;
    lastState = LIGHT_STATE_UNKNOWN;
}

void TVP5155CaptureModuleDriver_AddToController(void)
{
    int i;
    for (i = 0; i < MAX_CAPTURE_MODULE; i++)
    {
        CaptureModuleDriver TVP5155 = (CaptureModuleDriver)TVP5155CaptureModule_Create(i);
        CaptureController_Add(i, TVP5155);
    }
}


static void save(int id, int state)
{
    states[id] = state;
    lastId = id;
    lastState = state;
}

static void destroy(CaptureModuleDriver base)
{
    TVP5155CaptureModuleDriver self = (TVP5155CaptureModuleDriver)base;
    free(self);
}

static void turnOn(CaptureModuleDriver base)
{
    TVP5155CaptureModuleDriver self = (TVP5155CaptureModuleDriver)base;
    printf("TVP5155 Turn on\n");
    //update(self->base.id, LIGHT_ON);
}
static void turnOff(CaptureModuleDriver base)
{
    TVP5155CaptureModuleDriver self = (TVP5155CaptureModuleDriver)base;
    //update(self->base.id, LIGHT_OFF);
}

static CaptureModuleDriverInterfaceStruct interface =
{
    turnOn,
    turnOff,
    destroy
};

CaptureModuleDriver TVP5155CaptureModule_Create()
{
    TVP5155CaptureModuleDriver self = calloc(1, sizeof(CaptureModuleDriverTVP5155Struct));
    self->base.vtable = &interface;
    self->base.type = "TVP5150";
    //self->base.id = id;
    return (TVP5155CaptureModuleDriver)self;
}

#if 0

CaptureModuleDriver TVP5155CaptureModule_Create(int id)
{
    TVP5155CaptureModuleDriver self = calloc(1, sizeof(CaptureModuleDriverTVP5155Struct));
    self->base.type = TVP5150;
    self->base.id = id;
    return (TVP5155CaptureModuleDriver)self;
}


void TVP5155CaptureModuleDriver_InstallInterface(void)
{
    CaptureModuleDriver_SetInterface(&interface);
}


//T1
void TVP5155CaptureModule_Destroy(CaptureModuleDriver super)
{
    TVP5155CaptureModuleDriver self = (TVP5155CaptureModuleDriver)super;
    states[self->base.id] = LIGHT_STATE_UNKNOWN;
    free(self);
}

void TVP5155CaptureModule_TurnOn(CaptureModuleDriver super)
{
    printf("TVP5155CaptureModule_TurnOn \n");
    TVP5155CaptureModuleDriver self = (TVP5155CaptureModuleDriver)super;
    save(self->base.id, LIGHT_ON);
}

void TVP5155CaptureModule_TurnOff(CaptureModuleDriver super)
{
    TVP5155CaptureModuleDriver self = (TVP5155CaptureModuleDriver)super;
    save(self->base.id, LIGHT_OFF);
}
//end of T1

//T2
void TVP5155CaptureModule_TurnOn(CaptureModuleDriver base)
{
    printf("TVP5155CaptureModule_TurnOn T2\n");
    TVP5155CaptureModuleDriver self = (TVP5155CaptureModuleDriver)base;
    states[self->base.id] = LIGHT_ON;
    lastId = self->base.id;
    lastState = LIGHT_ON;
}
static CaptureModuleDriverInterfaceStruct interface =
{
    TVP5155CaptureModule_TurnOn,
    0,
    0
};
//end of T2
#endif 


