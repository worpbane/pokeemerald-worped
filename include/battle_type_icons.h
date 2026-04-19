#ifndef GUARD_BATTLE_TYPE_ICONS_H
#define GUARD_BATTLE_TYPE_ICONS_H

void LoadTypeIcons(u8 battlerId);

#define B_SHOW_TYPES        SHOW_TYPES_SEEN

#define TYPE_ICON_TAG 0x2720
#define NUM_FRAMES_HIDE_TYPE_ICON 10

#define tMonPosition      data[0]
#define tBattlerId        data[1]
#define tHideIconTimer    data[2]
#define tVerticalPosition data[3]
#define tState         	  data[4]

#define STATE_SLIDE_IN  0
#define STATE_ACTIVE    1
#define STATE_SLIDE_OUT 2

#define TYPE_ICON_FRAME(monType) ((monType - 1) * 2)

void LoadTypeIcons(u8 battlerId);
void LoadTypeSpritesAndPalettes(void);
void ResetTypeIconPalette(void);

#endif // GUARD_BATTLE_TYPE_ICONS_H