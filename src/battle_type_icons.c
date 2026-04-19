#include "global.h"
#include "battle.h"
#include "battle_anim.h"
#include "battle_controllers.h"
#include "decompress.h"
#include "graphics.h"
#include "pokedex.h"
#include "sprite.h"
#include "palette.h"
#include "battle_type_icons.h"

// Asset Loading
static void LoadTypeIconsPerBattler(u8 battlerId, u8 slot);

// Logic Helpers
static bool32 ShouldHideTypeIcon(u16 species);
static bool32 UseDoubleBattleCoords(void);

// Sprite Management
static void SpriteCB_TypeIcon(struct Sprite* sprite);
static void DestroyTypeIcon(struct Sprite* sprite);
static void FreeAllTypeIconResources(void);

// Animation/Movement
static s32 GetTypeIconSlideMovement(bool32 useDoubleBattleCoords, u32 position, s32 xPos);
static s32 GetTypeIconHideMovement(bool32 useDoubleBattleCoords, u32 position);
static s32 GetTypeIconBounceMovement(u32 originalY, u32 position);

static const u16 sMoveTypeIcons_Pal[] = INCBIN_U16("graphics/interface/typeIconsPal_HPbar.gbapal");

const struct Coords16 sTypeIconPositions[MAX_BATTLERS_COUNT][2] =
{
    [B_POSITION_PLAYER_LEFT] =
    {
        [FALSE] = {221, 86}, //Singles
        [TRUE] = {142, 71}, //Doubles
    },
    [B_POSITION_OPPONENT_LEFT] =
    {
        [FALSE] = {20, 25}, //Singles
        [TRUE] = {99, 14}, //Doubles
    },
    [B_POSITION_PLAYER_RIGHT] =
    {
        [TRUE] = {154, 96},
    },
    [B_POSITION_OPPONENT_RIGHT] =
    {
        [TRUE] = {87, 39},
    },
};

//Sprite Stuff
const struct CompressedSpriteSheet sSpriteSheet_PkmnTypes =
{
    .data = gInterfaceTypeIconsBattleMini_Gfx,
    .size = NUMBER_OF_MON_TYPES * 0x100,
    .tag = TYPE_ICON_TAG
};

static const struct OamData sOamData_PkmnTypes =
{
    .y = 0,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(8x16),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(8x16),
    .tileNum = 0,
    .priority = 2,
    .paletteNum = 13,
    .affineParam = 0,
};

static const union AnimCmd sSpriteAnim_TypeNormal[] = {
    ANIMCMD_FRAME(TYPE_NORMAL * 2, 0, FALSE, FALSE),
    ANIMCMD_END
};
static const union AnimCmd sSpriteAnim_TypeFighting[] = {
    ANIMCMD_FRAME(TYPE_FIGHTING * 2, 0, FALSE, FALSE),
    ANIMCMD_END
};
static const union AnimCmd sSpriteAnim_TypeFlying[] = {
    ANIMCMD_FRAME(TYPE_FLYING * 2, 0, FALSE, FALSE),
    ANIMCMD_END
};
static const union AnimCmd sSpriteAnim_TypePoison[] = {
    ANIMCMD_FRAME(TYPE_POISON * 2, 0, FALSE, FALSE),
    ANIMCMD_END
};
static const union AnimCmd sSpriteAnim_TypeGround[] = {
    ANIMCMD_FRAME(TYPE_GROUND * 2, 0, FALSE, FALSE),
    ANIMCMD_END
};
static const union AnimCmd sSpriteAnim_TypeRock[] = {
    ANIMCMD_FRAME(TYPE_ROCK * 2, 0, FALSE, FALSE),
    ANIMCMD_END
};
static const union AnimCmd sSpriteAnim_TypeBug[] = {
    ANIMCMD_FRAME(TYPE_BUG * 2, 0, FALSE, FALSE),
    ANIMCMD_END
};
static const union AnimCmd sSpriteAnim_TypeGhost[] = {
    ANIMCMD_FRAME(TYPE_GHOST * 2, 0, FALSE, FALSE),
    ANIMCMD_END
};
static const union AnimCmd sSpriteAnim_TypeSteel[] = {
    ANIMCMD_FRAME(TYPE_STEEL * 2, 0, FALSE, FALSE),
    ANIMCMD_END
};
static const union AnimCmd sSpriteAnim_TypeMystery[] = {
    ANIMCMD_FRAME(TYPE_MYSTERY * 2, 0, FALSE, FALSE),
    ANIMCMD_END
};
static const union AnimCmd sSpriteAnim_TypeFire[] = {
    ANIMCMD_FRAME(TYPE_FIRE * 2, 0, FALSE, FALSE),
    ANIMCMD_END
};
static const union AnimCmd sSpriteAnim_TypeWater[] = {
    ANIMCMD_FRAME(TYPE_WATER * 2, 0, FALSE, FALSE),
    ANIMCMD_END
};
static const union AnimCmd sSpriteAnim_TypeGrass[] = {
    ANIMCMD_FRAME(TYPE_GRASS * 2, 0, FALSE, FALSE),
    ANIMCMD_END
};
static const union AnimCmd sSpriteAnim_TypeElectric[] = {
    ANIMCMD_FRAME(TYPE_ELECTRIC * 2, 0, FALSE, FALSE),
    ANIMCMD_END
};
static const union AnimCmd sSpriteAnim_TypePsychic[] = {
    ANIMCMD_FRAME(TYPE_PSYCHIC * 2, 0, FALSE, FALSE),
    ANIMCMD_END
};
static const union AnimCmd sSpriteAnim_TypeIce[] = {
    ANIMCMD_FRAME(TYPE_ICE * 2, 0, FALSE, FALSE),
    ANIMCMD_END
};
static const union AnimCmd sSpriteAnim_TypeDragon[] = {
    ANIMCMD_FRAME(TYPE_DRAGON * 2, 0, FALSE, FALSE),
    ANIMCMD_END
};
static const union AnimCmd sSpriteAnim_TypeDark[] = {
    ANIMCMD_FRAME(TYPE_DARK * 2, 0, FALSE, FALSE),
    ANIMCMD_END
};
static const union AnimCmd sSpriteAnim_TypeFairy[] = {
    ANIMCMD_FRAME(TYPE_FAIRY * 2, 0, FALSE, FALSE),
    ANIMCMD_END
};

static const union AnimCmd *const sSpriteAnimTable_PkmnTypes[NUMBER_OF_MON_TYPES] = {
    sSpriteAnim_TypeNormal,
    sSpriteAnim_TypeFighting,
    sSpriteAnim_TypeFlying,
    sSpriteAnim_TypePoison,
    sSpriteAnim_TypeGround,
    sSpriteAnim_TypeRock,
    sSpriteAnim_TypeBug,
    sSpriteAnim_TypeGhost,
    sSpriteAnim_TypeSteel,
    sSpriteAnim_TypeMystery,
    sSpriteAnim_TypeFire,
    sSpriteAnim_TypeWater,
    sSpriteAnim_TypeGrass,
    sSpriteAnim_TypeElectric,
    sSpriteAnim_TypePsychic,
    sSpriteAnim_TypeIce,
    sSpriteAnim_TypeDragon,
    sSpriteAnim_TypeDark,
	sSpriteAnim_TypeFairy,
};

const struct SpriteTemplate sSpriteTemplate_PkmnTypes =
{
    .tileTag = TYPE_ICON_TAG,
    .paletteTag = TYPE_ICON_TAG,
    .oam = &sOamData_PkmnTypes,
    .anims = sSpriteAnimTable_PkmnTypes,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_TypeIcon
};

static void DestroyTypeIcon(struct Sprite* sprite)
{
    u32 i;
    u8 battlerId = sprite->tBattlerId;

    for (i = 0; i < 2; i++)
    {
        u8 spriteId = gBattleStruct->typeIconSpriteIds[battlerId][i];
        if (spriteId != MAX_SPRITES && &gSprites[spriteId] == sprite)
        {
            gBattleStruct->typeIconSpriteIds[battlerId][i] = MAX_SPRITES;
        }
    }

    DestroySprite(sprite);
	
    for (i = 0; i < MAX_SPRITES; i++)
    {
        if (gSprites[i].inUse && gSprites[i].callback == SpriteCB_TypeIcon)
        {
            return;
        }
    }

    FreeAllTypeIconResources();
}

static void FreeAllTypeIconResources(void)
{
    FreeSpriteTilesByTag(TYPE_ICON_TAG);
    FreeSpritePaletteByTag(TYPE_ICON_TAG);
}

static s32 GetTypeIconSlideMovement(bool32 useDoubleBattleCoords, u32 position, s32 xPos)
{
	s16 targetX = sTypeIconPositions[position][useDoubleBattleCoords].x;
	
    if (useDoubleBattleCoords)
    {
        switch (position)
        {
        case B_POSITION_PLAYER_LEFT:
        case B_POSITION_PLAYER_RIGHT:
            if (xPos > targetX - 10)
                    return -1;
                break;
        default:
        case B_POSITION_OPPONENT_LEFT:
        case B_POSITION_OPPONENT_RIGHT:
            if (xPos < targetX + 10)
                    return 1;
                break;
        }
        return 0;
    }

    if (position == B_POSITION_PLAYER_LEFT)
    {
        if (xPos < targetX + 10)
                return 1;
    }
    else
    {
        if (xPos > targetX - 10)
                return -1;
    }
    return 0;
}

static bool32 ShouldHideTypeIcon(u16 species)
{
    if (!GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_GET_SEEN))
        return TRUE;

    return FALSE;
}

static s32 GetTypeIconHideMovement(bool32 useDoubleBattleCoords, u32 position)
{
    if (useDoubleBattleCoords)
    {
        if (position == B_POSITION_PLAYER_LEFT || position == B_POSITION_PLAYER_RIGHT)
            return 1;
        else
            return -1;
    }
    else
    {
        if (position == B_POSITION_PLAYER_LEFT)
            return -1;
        else
            return 1;
    }
}

static s32 GetTypeIconBounceMovement(u32 originalY, u32 position)
{
    struct Sprite *healthbox = &gSprites[gHealthboxSpriteIds[GetBattlerAtPosition(position)]];
    return originalY + healthbox->y2;
}

//End Sprite Stuff

static bool32 UseDoubleBattleCoords(void)
{
    if (gBattleTypeFlags & BATTLE_TYPE_DOUBLE)
        return TRUE;
    
    return FALSE;
}

void LoadTypeIcons(u8 battlerId)
{
    u32 position;
    u16 species = gBattleMons[battlerId].species;
	
	gBattleStruct->typeIconSpriteIds[battlerId][0] = MAX_SPRITES;
    gBattleStruct->typeIconSpriteIds[battlerId][1] = MAX_SPRITES;
	
    LoadTypeSpritesAndPalettes();
    LoadTypeIconsPerBattler(battlerId, 0);

    if (gBattleMons[battlerId].type1 != gBattleMons[battlerId].type2)
    {
        LoadTypeIconsPerBattler(battlerId, 1);
    }
}

void LoadTypeSpritesAndPalettes(void)
{
	struct SpritePalette pal = {sMoveTypeIcons_Pal, TYPE_ICON_TAG};
	
    if (IndexOfSpritePaletteTag(TYPE_ICON_TAG) != 0xFF)
        return;

    LoadCompressedSpriteSheet(&sSpriteSheet_PkmnTypes);
	LoadSpritePalette(&pal);
}

static bool32 ShouldFlipTypeIcon(u8 battlerId)
{
    bool32 isDouble = UseDoubleBattleCoords();

    if (isDouble)
    {
        return (GetBattlerSide(battlerId) == B_SIDE_OPPONENT);
    }
    else
    {
        return (GetBattlerSide(battlerId) == B_SIDE_PLAYER);
    }
}

void LoadTypeIconsPerBattler(u8 battlerId, u8 slot)
{
    u8 type;
    u8 spriteId;
    s16 x, y;
    bool32 isDouble = UseDoubleBattleCoords();
    u8 position = GetBattlerPosition(battlerId);

    if (!IsBattlerSpriteVisible(battlerId))
        return;
	
	if (slot == 1 && gBattleMons[battlerId].type1 == gBattleMons[battlerId].type2)
        return;
	
	x = sTypeIconPositions[position][isDouble].x;
	y = sTypeIconPositions[position][isDouble].y + (slot * 12);
	
    spriteId = CreateSprite(&sSpriteTemplate_PkmnTypes, x, y, 0);
	gSprites[spriteId].invisible = TRUE;
    
    gSprites[spriteId].tBattlerId = battlerId;
    gSprites[spriteId].tMonPosition = slot;
	
	gBattleStruct->typeIconSpriteIds[battlerId][slot] = spriteId;
    gSprites[spriteId].tBattlerId = battlerId;
    gSprites[spriteId].tMonPosition = slot;
    gSprites[spriteId].subpriority = 0; // Ensures it's in front of healthbox

    StartSpriteAnim(&gSprites[spriteId], type);
	
	if (ShouldFlipTypeIcon(battlerId))
        gSprites[spriteId].hFlip = TRUE;
    else
        gSprites[spriteId].hFlip = FALSE;
}

static void SpriteCB_TypeIcon(struct Sprite* sprite)
{
    u8 battlerId = sprite->tBattlerId;
    u8 slot = sprite->tMonPosition;
    bool32 isDouble = UseDoubleBattleCoords();
    u8 position = GetBattlerPosition(battlerId);
	
	if (!IsBattlerSpriteVisible(battlerId)) //Delete if enemy mon has fainted
    {
        DestroyTypeIcon(sprite);
        return;
    }
	
	if (sprite->tHideIconTimer > 0)
    {
        sprite->x += GetTypeIconHideMovement(isDouble, position);
        sprite->tHideIconTimer++;

        if (sprite->tHideIconTimer >= NUM_FRAMES_HIDE_TYPE_ICON)
        {
            sprite->invisible = TRUE;
            sprite->tHideIconTimer = 0;
            sprite->x = sTypeIconPositions[position][isDouble].x;
        }
    }
    else if (!sprite->invisible)
    {
        sprite->x += GetTypeIconSlideMovement(isDouble, position, sprite->x);
    }
	
	sprite->y = sTypeIconPositions[position][isDouble].y + (slot * 11) + GetTypeIconBounceMovement(0, position);
}

void ResetTypeIconPalette(void)
{
    if (IndexOfSpritePaletteTag(TYPE_ICON_TAG) == 0xFF)
    {
        struct SpritePalette pal = {sMoveTypeIcons_Pal, TYPE_ICON_TAG};
        LoadSpritePalette(&pal);
    }
}