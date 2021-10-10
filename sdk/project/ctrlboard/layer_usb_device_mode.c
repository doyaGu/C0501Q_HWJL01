#include <assert.h>
#include "scene.h"
#include "ctrlboard.h"

bool UsbDeviceModeOnEnter(ITUWidget* widget, char* param)
{
    //ituFtExit();

    return true;
}

bool UsbDeviceModeOnLeave(ITUWidget* widget, char* param)
{
    //ituFtInit();
    //ituFtLoadFont(0, CFG_PRIVATE_DRIVE ":/font/" CFG_FONT_FILENAME, ITU_GLYPH_8BPP);

    return true;
}

void UsbDeviceModeReset(void)
{
}
