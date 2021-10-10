#include "ite/itu.h"

extern void StandbyReset(void);
extern void MainReset(void);
extern void SettingReset(void);
extern void ProgramReset(void);

void resetScene(void)
{
    StandbyReset();
    MainReset();
    SettingReset();
    ProgramReset();
}
