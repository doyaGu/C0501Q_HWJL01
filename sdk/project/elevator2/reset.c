#include "ite/itu.h"

extern void MainReset(void);
extern void SettingReset(void);

void resetScene(void)
{
    MainReset();
    SettingReset();
}
