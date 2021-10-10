#include <sys/stat.h>
#include <assert.h>
#include <malloc.h>
#include <stdio.h>
#include "scene.h"
#include "ctrlboard.h"

static ITUIcon* screensaverIcon;
static ITUBackground* screensaverBackground;

bool ScreensaverOnEnter(ITUWidget* widget, char* param)
{
    if (!screensaverBackground)
    {
        screensaverIcon = ituSceneFindWidget(&theScene, "screensaverIcon");
        assert(screensaverIcon);

        screensaverBackground = ituSceneFindWidget(&theScene, "screensaverBackground");
        assert(screensaverBackground);
    }
    ituWidgetSetVisible(screensaverBackground, false);
    ituWidgetSetVisible(screensaverIcon, false);

    switch (theConfig.screensaver_type)
    {
    case SCREENSAVER_CLOCK:
        ituWidgetSetVisible(screensaverBackground, true);
       break;

    case SCREENSAVER_PHOTO:
        {
            // try to load screensaver jpeg file if exists
            FILE* f = fopen(CFG_PUBLIC_DRIVE ":/screensaver.jpg", "rb");
            if (f)
            {
                uint8_t* data;
                int size;
                struct stat sb;

                if (fstat(fileno(f), &sb) != -1)
                {
                    size = sb.st_size;
                    data = malloc(size);
                    if (data)
                    {
                        size = fread(data, 1, size, f);
                        ituIconLoadJpegData(screensaverIcon, data, size);
                        free(data);
                    }
                }
                fclose(f);
            }
            ituWidgetSetVisible(screensaverIcon, true);
       }
       break;
    }
	return true;
}


void ScreensaverReset(void)
{
    screensaverBackground = NULL;
}
