#include "capture_module.h"
#include "ite/ith.h"

#if 0
typedef (CaptureModuleDriver) (*pfCreateFunction) (void);

typedef struct CaptureModeDriverSet 
{
	unsigned char *moduleName,
	pfCreateFunction createFunction;
}CaptureModeDriverSet;
static CaptureModeDriverSet moduleDriverArray[] = 
{
}
#endif

CaptureModuleDriver CaptureModuleDriver_GetDevice(unsigned char* moduleName)
{
	if(moduleName == "hm1375.c")
		return (CaptureModuleDriver)HM1375CaptureModuleDriver_Create();
	else if(moduleName == "gc0308.c")
		return (CaptureModuleDriver)GC0308CaptureModuleDriver_Create();
	else if(moduleName == "gc0328.c")
		return (CaptureModuleDriver)GC0328CaptureModuleDriver_Create();
	else if(moduleName == "gt5110e1.c")
		return (CaptureModuleDriver)GT5110E1CaptureModuleDriver_Create();
	else if(moduleName == "adv7180.c")
		return (CaptureModuleDriver)ADV7180CaptureModuleDriver_Create();
	else
		return NULL;
}

void CaptureModuleDriver_Init(CaptureModuleDriver self)
{
    if (self)
        self->vtable->Init();
}

void CaptureModuleDriver_Destroy(CaptureModuleDriver self)
{
    if (self)
        self->vtable->Destroy(self);
}

void CaptureModuleDriver_DeInit(CaptureModuleDriver self)
{
    if (self)
        self->vtable->Terminate();
}

unsigned char CaptureModuleDriver_IsSignalStable(CaptureModuleDriver self)
{
    if (self)
        return self->vtable->IsSignalStable();
}

void CaptureModuleDriver_GetProperty(CaptureModuleDriver self, CAP_GET_PROPERTY *CapGetProperty)
{
    if (self)
        self->vtable->GetProperty(CapGetProperty);
}

void CaptureModuleDriver_PowerDown(CaptureModuleDriver self, unsigned char enable)
{
    if (self)
        self->vtable->PowerDown(enable);
}

void CaptureModuleDriver_OutputPinTriState(CaptureModuleDriver self, unsigned char flag)
{
    if (self)
        self->vtable->OutputPinTriState(flag);
}

void CaptureModuleDriver_ForCaptureDriverSetting(CaptureModuleDriver self, CAP_CONTEXT *Capctxt)
{
    if (self)
        self->vtable->ForCaptureDriverSetting(Capctxt);
}