#ifndef GUARD_UI_MENU_H
#define GUARD_UI_MENU_H

#include "main.h"

void Task_OpenBirchCase(u8 taskId);
void Task_OpenBirchCaseGotoBattle(u8 taskId);
void BirchCase_Init(MainCallback callback);

//Needed for VAR_STARTER_MON
//passing to monchoicedata
//for birch case upgrade
enum StarterIds
{
    GRASS_STARTER,
    FIRE_STARTER,
    WATER_STARTER,
};


#endif // GUARD_UI_MENU_H
