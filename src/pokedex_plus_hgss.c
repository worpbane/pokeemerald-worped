#include "global.h"
#include "battle_main.h"
#ifdef BATTLE_ENGINE
#include "battle_util.h"
#endif
#include "bg.h"
#include "contest_effect.h"
#include "data.h"
#include "daycare.h"
#include "decompress.h"
#include "event_data.h"
#include "gpu_regs.h"
#include "graphics.h"
#include "international_string_util.h"
#include "item.h"
#include "item_icon.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "m4a.h"
#include "overworld.h"
#include "palette.h"
#include "party_menu.h"
#include "pokedex.h"
#include "pokedex_plus_hgss.h"
#include "pokedex_area_screen.h"
#include "pokedex_cry_screen.h"
#include "pokemon_icon.h"
#include "pokemon_summary_screen.h"
#ifdef POKEMON_EXPANSION
#include "region_map.h"
#include "pokemon.h"
#endif
#include "reset_rtc_screen.h"
#include "scanline_effect.h"
#include "shop.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text_window.h"
#include "trainer_pokemon_sprites.h"
#include "trig.h"
#include "window.h"
#include "constants/abilities.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/party_menu.h"
#include "constants/rgb.h"
#include "constants/songs.h"
#include "region_map.h"  // for GetMapName
#ifdef TX_RANDOMIZER_AND_CHALLENGES
    #include "tx_randomizer_and_challenges.h"
#endif


enum
{
    PAGE_MAIN,
    PAGE_INFO,
    PAGE_SEARCH,
    PAGE_SEARCH_RESULTS,
    PAGE_UNK,
    PAGE_AREA,
    PAGE_CRY,
    PAGE_SIZE
};

enum
{
    INFO_SCREEN,
    STATS_SCREEN,
    EVO_SCREEN,
    FORMS_SCREEN, //Pokemonexpansion only (rhh)
    AREA_SCREEN,
    CRY_SCREEN,
    SIZE_SCREEN,
    CANCEL_SCREEN,
    SCREEN_COUNT
};

enum
{
    SEARCH_NAME,
    SEARCH_COLOR,
    SEARCH_TYPE_LEFT,
    SEARCH_TYPE_RIGHT,
    SEARCH_ORDER,
    SEARCH_MODE,
    SEARCH_OK,
    SEARCH_COUNT
};

enum
{
    SEARCH_TOPBAR_SEARCH,
    SEARCH_TOPBAR_SHIFT,
    SEARCH_TOPBAR_CANCEL,
    SEARCH_TOPBAR_COUNT
};

enum
{
   ORDER_NUMERICAL,
   ORDER_ALPHABETICAL,
   ORDER_HEAVIEST,
   ORDER_LIGHTEST,
   ORDER_TALLEST,
   ORDER_SMALLEST
};

enum
{
    NAME_ABC = 1,
    NAME_DEF,
    NAME_GHI,
    NAME_JKL,
    NAME_MNO,
    NAME_PQR,
    NAME_STU,
    NAME_VWX,
    NAME_YZ,
};


extern const u8 *const gMonFootprintTable[];
extern const u16 gPokedexOrder_Alphabetical[];
extern const u16 gPokedexOrder_Height[];
extern const u16 gPokedexOrder_Weight[];
extern const struct Evolution gEvolutionTable[][EVOS_PER_MON];
extern const struct PokedexEntry gPokedexEntries[];


static const u8 sText_No000[] = _("{NO}000");
static const u8 sCaughtBall_Gfx[] = INCBIN_U8("graphics/pokedex/caught_ball.4bpp");
static const u32 sMonType_Gfx[] = INCBIN_U32("graphics/interface/typeIconsMenu.4bpp");
static const u32 sMonType_Pal[] = INCBIN_U32("graphics/interface/typeIconsPal.gbapal");
static const u8 sText_TenDashes[] = _("----------");
ALIGNED(4) static const u8 sExpandedPlaceholder_PokedexDescription[] = _("");
static const u16 sSizeScreenSilhouette_Pal[] = INCBIN_U16("graphics/pokedex/size_silhouette.gbapal");
static const u8 sText_TenDashes2[] = _("----------");


#define SCROLLING_MON_X 136
#define HGSS_HIDE_UNSEEN_EVOLUTION_NAMES 1 //0 false, 1 true WorpTODO: Make this default



// For scrolling search parameter
#define MAX_SEARCH_PARAM_ON_SCREEN   6
#define MAX_SEARCH_PARAM_CURSOR_POS  (MAX_SEARCH_PARAM_ON_SCREEN - 1)

#define MAX_MONS_ON_SCREEN 4

#define LIST_SCROLL_STEP         16

#define POKEBALL_ROTATION_TOP    64
#define POKEBALL_ROTATION_BOTTOM (POKEBALL_ROTATION_TOP - 16)

// Coordinates of the Pokémon sprite on its page (info/cry screens)
#define MON_PAGE_X 48
#define MON_PAGE_Y 56

static EWRAM_DATA struct PokedexView *sPokedexView = NULL;
static EWRAM_DATA u16 sLastSelectedPokemon = 0;
static EWRAM_DATA u8 sPokeBallRotation = 0;
static EWRAM_DATA struct PokedexListItem *sPokedexListItem = NULL;
static EWRAM_DATA void (*sExternalReturnCallback)(void) = NULL;
//Pokedex Plus HGSS_Ui
#ifndef BATTLE_ENGINE
#define MOVES_COUNT_TOTAL (EGG_MOVES_ARRAY_COUNT + MAX_LEVEL_UP_MOVES + NUM_TECHNICAL_MACHINES + NUM_HIDDEN_MACHINES + TUTOR_MOVE_COUNT)
#else
#define MOVES_COUNT_TOTAL (EGG_MOVES_ARRAY_COUNT + MAX_LEVEL_UP_MOVES + NUM_TECHNICAL_MACHINES + NUM_HIDDEN_MACHINES + 20)
#endif
EWRAM_DATA static u16 sStatsMoves[MOVES_COUNT_TOTAL] = {0};
EWRAM_DATA static u16 sStatsMovesTMHM_ID[NUM_TECHNICAL_MACHINES + NUM_HIDDEN_MACHINES] = {0};

struct SearchOptionText
{
    const u8 *description;
    const u8 *title;
};

struct SearchOption
{
    const struct SearchOptionText *texts;
    u8 taskDataCursorPos;
    u8 taskDataScrollOffset;
    u16 numOptions;
};

struct SearchMenuTopBarItem
{
    const u8 *description;
    u8 highlightX;
    u8 highlightY;
    u8 highlightWidth;
};

struct SearchMenuItem
{
    const u8 *description;
    u8 titleBgX;
    u8 titleBgY;
    u8 titleBgWidth;
    u8 selectionBgX;
    u8 selectionBgY;
    u8 selectionBgWidth;
};

struct PokedexListItem
{
    u16 dexNum;
    u16 seen:1;
    u16 owned:1;
};

//HGSS_Ui
struct PokemonStats
{
    u16 species;
    u8  genderRatio;
    u8  baseHP;
    u8  baseSpeed;
    u8  baseAttack;
    u8  baseSpAttack;
    u8  baseDefense;
    u8  baseSpDefense;
    u8  differentEVs;
    u8  evYield_HP;
    u8  evYield_Speed;
    u8  evYield_Attack;
    u8  evYield_SpAttack;
    u8  evYield_Defense;
    u8  evYield_SpDefense;
    u8  catchRate;
    u8  growthRate;
    u8  eggGroup1;
    u8  eggGroup2;
    u8  eggCycles;
    u8  expYield;
    u8  friendship;
    u16 ability0;
    u16 ability1;
    u16 abilityHidden;
    u8  baseHP_old;
    u8  baseSpeed_old;
    u8  baseAttack_old;
    u8  baseSpAttack_old;
    u8  baseDefense_old;
    u8  baseSpDefense_old;
};

struct EvoScreenData
{
    bool8 fromEvoPage;
    u8 numAllEvolutions;
    u16 targetSpecies[10];
    u8 numSeen;
    bool8 seen[10];
    u8 menuPos;
    u8 arrowSpriteId;
};

#ifdef POKEMON_EXPANSION
struct FromScreenData
{
    u8 numForms;
    u16 formIds[30];
    bool8 inSubmenu;
    u8 menuPos;
    u8 arrowSpriteId;
};
#endif

struct PokedexView
{
    struct PokedexListItem pokedexList[NATIONAL_DEX_COUNT + 1];
    u16 pokemonListCount;
    u16 selectedPokemon;
    u16 selectedPokemonBackup;
    u16 dexMode;
    u16 dexModeBackup;
    u16 dexOrder;
    u16 dexOrderBackup;
    u16 seenCount;
    u16 ownCount;
    u16 monSpriteIds[MAX_MONS_ON_SCREEN];
    u8 typeIconSpriteIds[2]; //HGSS_Ui
    u16 moveSelected; //HGSS_Ui
    u8 movesTotal; //HGSS_Ui
    bool8 justScrolled; //HGSS_Ui
    u8 splitIconSpriteId;  //HGSS_Ui Physical/Special Split from BE
    u8 numEggMoves; //HGSS_Ui
    u8 numLevelUpMoves; //HGSS_Ui
    u8 numTMHMMoves; //HGSS_Ui
    u8 numTutorMoves; //HGSS_Ui
    u8 numPreEvolutions; //HGSS_Ui
    struct PokemonStats sPokemonStats; //HGSS_Ui
    struct EvoScreenData sEvoScreenData; //HGSS_Ui
    #ifdef POKEMON_EXPANSION
    struct FromScreenData sFormScreenData; //HGSS_Ui
    #endif
    u16 formSpecies;
    u16 selectedMonSpriteId;
    u16 pokeBallRotationStep;
    u16 pokeBallRotationBackup;
    u8 pokeBallRotation;
    u8 initialVOffset;
    u8 scrollTimer;
    u8 scrollDirection;
    s16 listVOffset;
    s16 listMovingVOffset;
    u16 scrollMonIncrement;
    u16 maxScrollTimer;
    u16 scrollSpeed;
    u16 unkArr1[4]; // Cleared, never read
    u16 originalSearchSelectionNum;
    u8 filler[6];
    u8 currentPage;
    u8 currentPageBackup;
    bool8 isSearchResults:1;
    u8 selectedScreen;
    u8 screenSwitchState;
    u8 menuIsOpen;
    u16 menuCursorPos;
    s16 menuY;     //Menu Y position (inverted because we use REG_BG0VOFS for this)
    u8 unkArr2[8]; // Cleared, never read
    u8 unkArr3[8]; // Cleared, never read
};
static void ResetPokedexView(struct PokedexView *pokedexView);
static void VBlankCB_Pokedex(void);
static void CB2_Pokedex(void);
static void Task_OpenPokedexMainPage(u8);
static void Task_HandlePokedexInput(u8);
static void Task_WaitForScroll(u8);
static void Task_OpenInfoScreenAfterMonMovement(u8);
static void Task_WaitForExitInfoScreen(u8);
static void Task_WaitForExitSearch(u8);
static void Task_ClosePokedex(u8);
static void Task_OpenSearchResults(u8);
static void Task_HandleSearchResultsInput(u8);
static void Task_WaitForSearchResultsScroll(u8);
static void Task_OpenSearchResultsInfoScreenAfterMonMovement(u8);
static void Task_WaitForExitSearchResultsInfoScreen(u8);
static void Task_ReturnToPokedexFromSearchResults(u8);
static bool8 LoadPokedexListPage(u8);
static void LoadPokedexBgPalette(bool8 isSearchResults, bool8 isMainPage);
static void FreeWindowAndBgBuffers(void);
static void CreatePokedexList(u8, u8);
static void CreateMonDexNum(u16, u8, u8, u16);
static void CreateCaughtBall(u16, u8, u8, u16);
static u8 CreateMonName(u16, u8, u8);
static u8 CreateMonTypeIcons(bool16 owned, u16 num, u8 left, u8 top);
static void ClearMonListEntry(u8 x, u8 y, u16 unused);
static void CreateMonSpritesAtPos(u16, u16);
static bool8 UpdateDexListScroll(u8, u8, u8);
static u16 TryDoPokedexScroll(u16, u16);
static void UpdateSelectedMonSpriteId(void);
static bool8 TryDoInfoScreenScroll(void);
static u8 ClearMonSprites(void);
static u16 GetPokemonSpriteToDisplay(u16);
static u32 CreatePokedexMonSprite(u16, s16, s16);
static void CreateInterfaceSprites(u8);
static void PrintPokedexListNavText(u8 windowId, u8 windowId2, u8 page);
static void PrintPokedexCounts(u8 windowId);
static void SpriteCB_MoveMonForInfoScreen(struct Sprite *sprite);
static void SpriteCB_Scrollbar(struct Sprite *sprite);
static void SpriteCB_ScrollArrow(struct Sprite *sprite);
static void SpriteCB_DexListInterfaceText(struct Sprite *sprite);
static void SpriteCB_RotatingPokeBall(struct Sprite *sprite);
static void SpriteCB_PokedexListMonSprite(struct Sprite *sprite);
static u8 LoadInfoScreen(struct PokedexListItem *, u8 monSpriteId);
static bool8 IsInfoScreenScrolling(u8);
static u8 StartInfoScreenScroll(struct PokedexListItem *, u8);
static void Task_LoadInfoScreen(u8);
static void Task_HandleInfoScreenInput(u8);
static void Task_SwitchScreensFromInfoScreen(u8);
static void Task_LoadInfoScreenWaitForFade(u8);
static void Task_ExitInfoScreen(u8);
static void Task_ExitInfoScreenToExternal(u8);
static void Task_LoadAreaScreen(u8);
static void Task_WaitForAreaScreenInput(u8 taskId);
static void Task_SwitchScreensFromAreaScreen(u8);
static void Task_ExitAreaScreen(u8);
static void Task_ExitAreaScreenToExternal(u8);
static void Task_LoadCryScreen(u8);
static void Task_HandleCryScreenInput(u8);
static void Task_SwitchScreensFromCryScreen(u8);
static void LoadPlayArrowPalette(bool8);
static void Task_ExitCryScreen(u8);
static void Task_ExitCryScreenToExternal(u8);
static void Task_LoadSizeScreen(u8);
static void Task_HandleSizeScreenInput(u8);
static void Task_SwitchScreensFromSizeScreen(u8);
static void Task_ExitSizeScreen(u8);
static void Task_ExitSizeScreenToExternal(u8);
static void LoadScreenSelectBarMain(u16);
static void HighlightScreenSelectBarItem(u8, u16);
static void HighlightSubmenuScreenSelectBarItem(u8, u16);
static void Task_HandleCaughtMonPageInput(u8);
static void Task_ExitCaughtMonPage(u8);
static void SpriteCB_SlideCaughtMonToCenter(struct Sprite *sprite);
static void PrintMonInfo(u32 num, u32, u32 owned, u32 newEntry);
static void PrintMonHeight(u16 height, u8 left, u8 top);
static void PrintMonWeight(u16 weight, u8 left, u8 top);
static void ResetOtherVideoRegisters(u16);
static u8 PrintCryScreenSpeciesName(u8, u16, u8, u8);
static void PrintDecimalNum(u8 windowId, u16 num, u8 left, u8 top);
static void DrawFootprint(u8 windowId, u16 dexNum);
static u16 CreateSizeScreenTrainerPic(u16, s16, s16, s8);
static u16 GetNextPosition(u8, u16, u16, u16);
static u8 LoadSearchMenu(void);
static void Task_LoadSearchMenu(u8);
static void Task_SwitchToSearchMenuTopBar(u8);
static void Task_HandleSearchTopBarInput(u8);
static void Task_SwitchToSearchMenu(u8);
static void Task_HandleSearchMenuInput(u8);
static void Task_StartPokedexSearch(u8);
static void Task_WaitAndCompleteSearch(u8);
static void Task_SearchCompleteWaitForInput(u8);
static void Task_SelectSearchMenuItem(u8);
static void Task_HandleSearchParameterInput(u8);
static void Task_ExitSearch(u8);
static void Task_ExitSearchWaitForFade(u8);
static void HighlightSelectedSearchTopBarItem(u8);
static void HighlightSelectedSearchMenuItem(u8, u8);
static void PrintSelectedSearchParameters(u8);
static void DrawOrEraseSearchParameterBox(bool8);
static void PrintSearchParameterText(u8);
static u8 GetSearchModeSelection(u8 taskId, u8 option);
static void SetDefaultSearchModeAndOrder(u8);
static void CreateSearchParameterScrollArrows(u8);
static void EraseAndPrintSearchTextBox(const u8 *);
static void EraseSelectorArrow(u32);
static void PrintSelectorArrow(u32);
static void PrintSearchParameterTitle(u32, const u8 *);
static void ClearSearchParameterBoxText(void);
static void SetSpriteInvisibility(u8 spriteArrayId, bool8 invisible);
static void CreateTypeIconSprites(void);
static void SetSearchRectHighlight(u8 flags, u8 x, u8 y, u8 width);
static void PrintInfoSubMenuText(u8 windowId, const u8 *str, u8 left, u8 top);
//Stats screen HGSS_Ui
static void LoadTilesetTilemapHGSS(u8 page);
static void Task_HandleStatsScreenInput(u8 taskId);
static void Task_LoadStatsScreen(u8 taskId);
static void Task_SwitchScreensFromStatsScreen(u8 taskId);
static void Task_ExitStatsScreen(u8 taskId);
static void Task_ExitStatsScreenToExternal(u8 taskId);
static bool8 CalculateMoves(void);
static void PrintStatsScreen_NameGender(u8 taskId, u32 num, u32 value);
static void PrintStatsScreen_DestroyMoveItemIcon(u8 taskId);
static void PrintStatsScreen_Moves(u8 taskId);
static void PrintStatsScreen_BaseStats(u8 taskId);
static void PrintStatsScreen_Abilities(u8 taskId);
static void PrintInfoScreenTextWhite(u8 windowId, const u8* str, u8 left, u8 top, u8 colorId);
static void PrintInfoScreenTextSmall(const u8* str, u8 left, u8 top);
static void Task_LoadEvolutionScreen(u8 taskId);
static void Task_HandleEvolutionScreenInput(u8 taskId);
static void Task_SwitchScreensFromEvolutionScreen(u8 taskId);
static void Task_ExitEvolutionScreen(u8 taskId);
static void Task_ExitEvolutionScreenToExternal(u8 taskId);
static u8 PrintEvolutionTargetSpeciesAndMethod(u8 taskId, u16 species, u8 depth, u8 depth_i);
static u8 PrintPreEvolutions(u8 taskId, u16 species);

//HGSS_UI Physical Special Split icon for BattleEngine (rhh)

static u8 ShowSplitIcon(u32 split); //Physical/Special Split from BE
static void DestroySplitIcon(void); //Physical/Special Split from BE

//Physical/Special Split from BE
#define TAG_SPLIT_ICONS 30004

static const u16 sSplitIcons_Pal[] = INCBIN_U16("graphics/interface/split_icons.gbapal");
static const u32 sSplitIcons_Gfx[] = INCBIN_U32("graphics/interface/split_icons.4bpp.lz");

static const u8 sTextColors[][3] =
{
    {0, 4, 3},  //1 > 4, 2 > 3, 3 > 1, 4, 2
    {0, 1, 2},
    {0, 5, 6},
    {0, 7, 8},
    {0, 9, 10},
    {0, 11, 12},
    {0, 13, 14},
    {0, 7, 8},
    {13, 15, 14},
    {0, 4, 3},
    {0, 1, 2},
    {0, 5, 6},
    {0, 7, 8},
};

static const struct OamData sOamData_SplitIcons =
{
    .size = SPRITE_SIZE(16x16),
    .shape = SPRITE_SHAPE(16x16),
    .priority = 0,
};
static const struct CompressedSpriteSheet sSpriteSheet_SplitIcons =
{
    .data = sSplitIcons_Gfx,
    .size = 16*16*3/2,
    .tag = TAG_SPLIT_ICONS,
};
static const struct SpritePalette sSpritePal_SplitIcons =
{
    .data = sSplitIcons_Pal,
    .tag = TAG_SPLIT_ICONS
};
static const union AnimCmd sSpriteAnim_SplitIcon0[] =
{
    ANIMCMD_FRAME(0, 0),
    ANIMCMD_END
};
static const union AnimCmd sSpriteAnim_SplitIcon1[] =
{
    ANIMCMD_FRAME(4, 0),
    ANIMCMD_END
};
static const union AnimCmd sSpriteAnim_SplitIcon2[] =
{
    ANIMCMD_FRAME(8, 0),
    ANIMCMD_END
};
static const union AnimCmd *const sSpriteAnimTable_SplitIcons[] =
{
    sSpriteAnim_SplitIcon0,
    sSpriteAnim_SplitIcon1,
    sSpriteAnim_SplitIcon2,
};
static const struct SpriteTemplate sSpriteTemplate_SplitIcons =
{
    .tileTag = TAG_SPLIT_ICONS,
    .paletteTag = TAG_SPLIT_ICONS,
    .oam = &sOamData_SplitIcons,
    .anims = sSpriteAnimTable_SplitIcons,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy
};

static const struct OamData sOamData_ScrollBar =
{
    .y = DISPLAY_HEIGHT,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(8x8),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(8x8),
    .tileNum = 0,
    .priority = 1,
    .paletteNum = 0,
    .affineParam = 0
};

static const struct OamData sOamData_ScrollArrow =
{
    .y = DISPLAY_HEIGHT,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(16x8),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(16x8),
    .tileNum = 0,
    .priority = 0,
    .paletteNum = 0,
    .affineParam = 0
};

static const struct OamData sOamData_InterfaceText =
{
    .y = DISPLAY_HEIGHT,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(32x16),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(32x16),
    .tileNum = 0,
    .priority = 0,
    .paletteNum = 0,
    .affineParam = 0
};

static const struct OamData sOamData_RotatingPokeBall =
{
    .y = DISPLAY_HEIGHT,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_WINDOW,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(32x32),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(32x32),
    .tileNum = 0,
    .priority = 1,
    .paletteNum = 0,
    .affineParam = 0
};

static const struct OamData sOamData_SeenOwnText =
{
    .y = DISPLAY_HEIGHT,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(64x32),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(64x32),
    .tileNum = 0,
    .priority = 0,
    .paletteNum = 0,
    .affineParam = 0
};

static const struct OamData sOamData_Dex8x16 =
{
    .y = DISPLAY_HEIGHT,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(8x16),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(8x16),
    .tileNum = 0,
    .priority = 0,
    .paletteNum = 0,
    .affineParam = 0
};

static const union AnimCmd sSpriteAnim_ScrollBar[] =
{
    ANIMCMD_FRAME(3, 30),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_ScrollArrow[] =
{
    ANIMCMD_FRAME(1, 30),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_RotatingPokeBall[] =
{
    ANIMCMD_FRAME(16, 30),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_StartButton[] =
{
    ANIMCMD_FRAME(48, 30),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_SearchText[] =
{
    ANIMCMD_FRAME(40, 30),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_SelectButton[] =
{
    ANIMCMD_FRAME(32, 30),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_MenuText[] =
{
    ANIMCMD_FRAME(56, 30),
    ANIMCMD_END
};

static const union AnimCmd *const sSpriteAnimTable_ScrollBar[] =
{
    sSpriteAnim_ScrollBar
};

static const union AnimCmd *const sSpriteAnimTable_ScrollArrow[] =
{
    sSpriteAnim_ScrollArrow
};

static const union AnimCmd *const sSpriteAnimTable_RotatingPokeBall[] =
{
    sSpriteAnim_RotatingPokeBall
};

static const union AnimCmd *const sSpriteAnimTable_InterfaceText[] =
{
    sSpriteAnim_StartButton,
    sSpriteAnim_SearchText,
    sSpriteAnim_SelectButton,
    sSpriteAnim_MenuText
};

#define TAG_DEX_INTERFACE 4096 // Tile and pal tag used for all interface sprites.

static const struct SpriteTemplate sScrollBarSpriteTemplate =
{
    .tileTag = TAG_DEX_INTERFACE,
    .paletteTag = TAG_DEX_INTERFACE,
    .oam = &sOamData_ScrollBar,
    .anims = sSpriteAnimTable_ScrollBar,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_Scrollbar,
};

static const struct SpriteTemplate sScrollArrowSpriteTemplate =
{
    .tileTag = TAG_DEX_INTERFACE,
    .paletteTag = TAG_DEX_INTERFACE,
    .oam = &sOamData_ScrollArrow,
    .anims = sSpriteAnimTable_ScrollArrow,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_ScrollArrow,
};

static const struct SpriteTemplate sInterfaceTextSpriteTemplate =
{
    .tileTag = TAG_DEX_INTERFACE,
    .paletteTag = TAG_DEX_INTERFACE,
    .oam = &sOamData_InterfaceText,
    .anims = sSpriteAnimTable_InterfaceText,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_DexListInterfaceText,
};

static const struct SpriteTemplate sRotatingPokeBallSpriteTemplate =
{
    .tileTag = TAG_DEX_INTERFACE,
    .paletteTag = TAG_DEX_INTERFACE,
    .oam = &sOamData_RotatingPokeBall,
    .anims = sSpriteAnimTable_RotatingPokeBall,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_RotatingPokeBall,
};

static const struct CompressedSpriteSheet sInterfaceSpriteSheet =
{
	.data = gPokedexPlusHGSS_Interface_Gfx,
    .size = 0x2000,
    .tag = TAG_DEX_INTERFACE
};

static const struct SpritePalette sInterfaceSpritePalette[] = //WorpTODO: See if I can't get this to change based on the dex mode (hoenn, nat, search)
{
    {gPokedexPlusHGSS_Default_Pal, TAG_DEX_INTERFACE},
};

// By scroll speed. Last element of each unused
static const u8 sScrollMonIncrements[] = {4, 8, 16, 32, 32};
static const u8 sScrollTimers[] = {8, 4, 2, 1, 1};

static const struct BgTemplate sPokedex_BgTemplate[] =
{
    {
        .bg = 0,
        .charBaseIndex = 1,
        .mapBaseIndex = 12,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    },
    {
        .bg = 1,
        .charBaseIndex = 0,
        .mapBaseIndex = 13,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 1,
        .baseTile = 0
    },
    {
        .bg = 2,
        .charBaseIndex = 2,
        .mapBaseIndex = 14,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 2,
        .baseTile = 0
    },
    {
        .bg = 3,
        .charBaseIndex = 0,
        .mapBaseIndex = 15,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 3,
        .baseTile = 0
    }
};

static const struct WindowTemplate sPokemonList_WindowTemplate[] =
{
    {
        .bg = 2,
        .tilemapLeft = 0,
        .tilemapTop = 0,
        .width = 13,
        .height = 32,
        .paletteNum = 6,
        .baseBlock = 1,
    },
	{	// 1 Type Icons
        .bg = 2,
        .tilemapLeft = 20,
        .tilemapTop = 0,
        .width = 12,
        .height = 32,
        .paletteNum = 5,
        .baseBlock = 417,
    },
	{   // 2 Text (Nav Top)
		.bg = 0,
		.tilemapLeft = 16,
		.tilemapTop = 0,
		.width = 14,
		.height = 2,
		.paletteNum = 6,
		.baseBlock = 4,
	},
	{	// 3 Text (Dex Counts)
        .bg = 0,
        .tilemapLeft = 22,
        .tilemapTop = 2,
        .width = 8,
        .height = 15,
        .paletteNum = 6,
        .baseBlock = 32,
    },
	{   // 4 Text (Nav Bottom)
		.bg = 0,
		.tilemapLeft = 25,
		.tilemapTop = 18,
		.width = 5,
		.height = 2,
		.paletteNum = 6,
		.baseBlock = 152,
	},
    DUMMY_WIN_TEMPLATE
};

static const struct BgTemplate sInfoScreen_BgTemplate[] =
{
    {
        .bg = 0,
        .charBaseIndex = 2,
        .mapBaseIndex = 12,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 3,
        .baseTile = 0
    },
    {
        .bg = 1,
        .charBaseIndex = 0,
        .mapBaseIndex = 13,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    },
    {
        .bg = 2,
        .charBaseIndex = 2,
        .mapBaseIndex = 14,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 1,
        .baseTile = 0
    },
    {
        .bg = 3,
        .charBaseIndex = 0,
        .mapBaseIndex = 15,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 2,
        .baseTile = 0
    }
};

#define WIN_INFO_HEADER      0
#define WIN_INFO_FOOTPRINT   1
#define WIN_INFO_HT_WT       2
#define WIN_INFO_DESCRIPTION 3
#define WIN_EVO_DISPLAY      4
#define WIN_NAV_BUTTONS      5
#define WIN_VU_METER  		 6
#define WIN_CRY_WAVE         7

static const struct WindowTemplate sInfoScreen_WindowTemplates[] =
{
    [WIN_INFO_HEADER] =
    {
        .bg = 2,
        .tilemapLeft = 13,
        .tilemapTop = 2,
        .width = 16,
        .height = 4,
        .paletteNum = 6,
        .baseBlock = 1,
    },
    [WIN_INFO_FOOTPRINT] =
    {
        .bg = 2,
        .tilemapLeft = 17,
        .tilemapTop = 7,
        .width = 3,
        .height = 3,
        .paletteNum = 15,
        .baseBlock = 65,
    },
    [WIN_INFO_HT_WT] =
    {
        .bg = 2,
        .tilemapLeft = 20,
        .tilemapTop = 7,
        .width = 9,
        .height = 4,
        .paletteNum = 6,
        .baseBlock = 74,
    },
    [WIN_INFO_DESCRIPTION] =
    {
        .bg = 2,
        .tilemapLeft = 1,
        .tilemapTop = 12,
        .width = 28,
        .height = 7,
        .paletteNum = 6,
        .baseBlock = 110,
    },
    [WIN_EVO_DISPLAY] =
    {
        .bg = 2,
        .tilemapLeft = 0,
        .tilemapTop = 3,
        .width = 30,
        .height = 15,
        .paletteNum = 6,
        .baseBlock = 306,
    },
    [WIN_NAV_BUTTONS] =
    {
        .bg = 2,
        .tilemapLeft = 18,
        .tilemapTop = 18,
        .width = 12,
        .height = 2,
        .paletteNum = 6,
        .baseBlock = 756,
    },
    [WIN_VU_METER] =
    {
        .bg = 2,
        .tilemapLeft = 18,
        .tilemapTop = 3,
        .width = 10,
        .height = 8,
        .paletteNum = 9,
        .baseBlock = 780,
    },
    [WIN_CRY_WAVE] =
    {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 12,
        .width = 32,
        .height = 7,
        .paletteNum = 8,
        .baseBlock = 860,
    },
    DUMMY_WIN_TEMPLATE
};

#define WIN_STATS_NAME_GENDER         0
#define WIN_STATS_BASE_INFO           1
#define WIN_STATS_MOVES				  2
#define WIN_STATS_ABILITIES           3
#define WIN_STATS_NAVIGATION          4
#define WIN_STATS_END WIN_STATS_NAVIGATION
static const struct WindowTemplate sStatsScreen_WindowTemplates[] =
{
    [WIN_STATS_NAME_GENDER] =
    {
        .bg = 2,
        .tilemapLeft = 5,
        .tilemapTop = 2,
        .width = 8,
        .height = 6,
        .paletteNum = 6,
        .baseBlock = 1,
    },
    [WIN_STATS_BASE_INFO] =
    {
        .bg = 2,
        .tilemapLeft = 13,
        .tilemapTop = 2,
        .width = 15,
        .height = 5,
        .paletteNum = 6,
        .baseBlock = 49,
    },
    [WIN_STATS_MOVES] =
    {
        .bg = 2,
        .tilemapLeft = 1,
        .tilemapTop = 8,
        .width = 10,
        .height = 12,
        .paletteNum = 6,
        .baseBlock = 124,
    },
    [WIN_STATS_ABILITIES] =
    {
        .bg = 2,
        .tilemapLeft = 11,
        .tilemapTop = 10,
        .width = 19,
        .height = 8,
        .paletteNum = 6,
        .baseBlock = 244,
    },
    [WIN_STATS_NAVIGATION] =
    {
        .bg = 2,
        .tilemapLeft = 18,
        .tilemapTop = 18,
        .width = 12,
        .height = 2,
        .paletteNum = 6,
        .baseBlock = 400,
    },
    DUMMY_WIN_TEMPLATE
};

static const struct BgTemplate sNewEntryInfoScreen_BgTemplate[] =
{
    {
        .bg = 2,
        .charBaseIndex = 2,
        .mapBaseIndex = 14,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 2,
        .baseTile = 0
    },
    {
        .bg = 3,
        .charBaseIndex = 1,
        .mapBaseIndex = 15,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 3,
        .baseTile = 0
    },
};

static const struct WindowTemplate sNewEntryInfoScreen_WindowTemplates[] =
{
    [WIN_INFO_HEADER] =
    {
        .bg = 2,
        .tilemapLeft = 0,
        .tilemapTop = 0,
        .width = 32,
        .height = 20,
        .paletteNum = 0,
        .baseBlock = 1,
    },
    [WIN_INFO_FOOTPRINT] =
    {
        .bg = 2,
        .tilemapLeft = 15, //HGSSS_Ui
        .tilemapTop = 7, //HGSSS_Ui
        .width = 2,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 641,
    },
    DUMMY_WIN_TEMPLATE
};

// First character in range followed by number of characters in range for upper and lowercase
static const u8 sLetterSearchRanges[][4] =
{
    {}, // Name not specified, shouldn't be reached
    [NAME_ABC] = {CHAR_A, 3, CHAR_a, 3},
    [NAME_DEF] = {CHAR_D, 3, CHAR_d, 3},
    [NAME_GHI] = {CHAR_G, 3, CHAR_g, 3},
    [NAME_JKL] = {CHAR_J, 3, CHAR_j, 3},
    [NAME_MNO] = {CHAR_M, 3, CHAR_m, 3},
    [NAME_PQR] = {CHAR_P, 3, CHAR_p, 3},
    [NAME_STU] = {CHAR_S, 3, CHAR_s, 3},
    [NAME_VWX] = {CHAR_V, 3, CHAR_v, 3},
    [NAME_YZ]  = {CHAR_Y, 2, CHAR_y, 2},
};

#define LETTER_IN_RANGE_UPPER(letter, range) \
    ((letter) >= sLetterSearchRanges[range][0]                                  \
  && (letter) < sLetterSearchRanges[range][0] + sLetterSearchRanges[range][1])  \

#define LETTER_IN_RANGE_LOWER(letter, range) \
    ((letter) >= sLetterSearchRanges[range][2]                                  \
  && (letter) < sLetterSearchRanges[range][2] + sLetterSearchRanges[range][3])  \

static const struct SearchMenuTopBarItem sSearchMenuTopBarItems[SEARCH_TOPBAR_COUNT] =
{
    [SEARCH_TOPBAR_SEARCH] =
    {
        .description = gText_SearchForPkmnBasedOnParameters,
        .highlightX = 0,
        .highlightY = 0,
        .highlightWidth = 5,
    },
    [SEARCH_TOPBAR_SHIFT] =
    {
        .description = gText_SwitchPokedexListings,
        .highlightX = 6,
        .highlightY = 0,
        .highlightWidth = 5,
    },
    [SEARCH_TOPBAR_CANCEL] =
    {
        .description = gText_ReturnToPokedex,
        .highlightX = 12,
        .highlightY = 0,
        .highlightWidth = 5,
    },
};

static const struct SearchMenuItem sSearchMenuItems[SEARCH_COUNT] =
{
    [SEARCH_NAME] =
    {
        .description = gText_ListByFirstLetter,
        .titleBgX = 0,
        .titleBgY = 2,
        .titleBgWidth = 5,
        .selectionBgX = 5,
        .selectionBgY = 2,
        .selectionBgWidth = 12,
    },
    [SEARCH_COLOR] =
    {
        .description = gText_ListByBodyColor,
        .titleBgX = 0,
        .titleBgY = 4,
        .titleBgWidth = 5,
        .selectionBgX = 5,
        .selectionBgY = 4,
        .selectionBgWidth = 12,
    },
    [SEARCH_TYPE_LEFT] =
    {
        .description = gText_ListByType,
        .titleBgX = 0,
        .titleBgY = 6,
        .titleBgWidth = 5,
        .selectionBgX = 5,
        .selectionBgY = 6,
        .selectionBgWidth = 6,
    },
    [SEARCH_TYPE_RIGHT] =
    {
        .description = gText_ListByType,
        .titleBgX = 0,
        .titleBgY = 6,
        .titleBgWidth = 5,
        .selectionBgX = 11,
        .selectionBgY = 6,
        .selectionBgWidth = 6,
    },
    [SEARCH_ORDER] =
    {
        .description = gText_SelectPokedexListingMode,
        .titleBgX = 0,
        .titleBgY = 8,
        .titleBgWidth = 5,
        .selectionBgX = 5,
        .selectionBgY = 8,
        .selectionBgWidth = 12,
    },
    [SEARCH_MODE] =
    {
        .description = gText_SelectPokedexMode,
        .titleBgX = 0,
        .titleBgY = 10,
        .titleBgWidth = 5,
        .selectionBgX = 5,
        .selectionBgY = 10,
        .selectionBgWidth = 12,
    },
    [SEARCH_OK] =
    {
        .description = gText_ExecuteSearchSwitch,
        .titleBgX = 0,
        .titleBgY = 12,
        .titleBgWidth = 5,
        .selectionBgX = 0,
        .selectionBgY = 0,
        .selectionBgWidth = 0,
    },
};

// Left, Right, Up, Down
static const u8 sSearchMovementMap_SearchNatDex[SEARCH_COUNT][4] =
{
    [SEARCH_NAME] =
    {
        0xFF,
        0xFF,
        0xFF,
        SEARCH_COLOR
    },
    [SEARCH_COLOR] =
    {
        0xFF,
        0xFF,
        SEARCH_NAME,
        SEARCH_TYPE_LEFT
    },
    [SEARCH_TYPE_LEFT] =
    {
        0xFF,
        SEARCH_TYPE_RIGHT,
        SEARCH_COLOR,
        SEARCH_ORDER
    },
    [SEARCH_TYPE_RIGHT] =
    {   SEARCH_TYPE_LEFT,
        0xFF,
        SEARCH_COLOR,
        SEARCH_ORDER
    },
    [SEARCH_ORDER] =
    {
        0xFF,
        0xFF,
        SEARCH_TYPE_LEFT,
        SEARCH_MODE
    },
    [SEARCH_MODE] =
    {
        0xFF,
        0xFF,
        SEARCH_ORDER,
        SEARCH_OK
    },
    [SEARCH_OK] =
    {
        0xFF,
        0xFF,
        SEARCH_MODE,
        0xFF
    },
};

// Left, Right, Up, Down
static const u8 sSearchMovementMap_ShiftNatDex[SEARCH_COUNT][4] =
{
    [SEARCH_NAME] =
    {
        0xFF,
        0xFF,
        0xFF,
        0xFF
    },
    [SEARCH_COLOR] =
    {
        0xFF,
        0xFF,
        0xFF,
        0xFF
    },
    [SEARCH_TYPE_LEFT] =
    {
        0xFF,
        0xFF,
        0xFF,
        0xFF
    },
    [SEARCH_TYPE_RIGHT] =
    {
        0xFF,
        0xFF,
        0xFF,
        0xFF
    },
    [SEARCH_ORDER] =
    {
        0xFF,
        0xFF,
        0xFF,
        SEARCH_MODE
    },
    [SEARCH_MODE] =
    {
        0xFF,
        0xFF,
        SEARCH_ORDER,
        SEARCH_OK
    },
    [SEARCH_OK] =
    {
        0xFF,
        0xFF,
        SEARCH_MODE,
        0xFF
    },
};

// Left, Right, Up, Down
static const u8 sSearchMovementMap_SearchHoennDex[SEARCH_COUNT][4] =
{
    [SEARCH_NAME] =
    {
        0xFF,
        0xFF,
        0xFF,
        SEARCH_COLOR
    },
    [SEARCH_COLOR] =
    {
        0xFF,
        0xFF,
        SEARCH_NAME,
        SEARCH_TYPE_LEFT
    },
    [SEARCH_TYPE_LEFT] =
    {
        0xFF,
        SEARCH_TYPE_RIGHT,
        SEARCH_COLOR,
        SEARCH_ORDER
    },
    [SEARCH_TYPE_RIGHT] =
    {   SEARCH_TYPE_LEFT,
        0xFF,
        SEARCH_COLOR,
        SEARCH_ORDER
    },
    [SEARCH_ORDER] =
    {
        0xFF,
        0xFF,
        SEARCH_TYPE_LEFT,
        SEARCH_OK
    },
    [SEARCH_MODE] =
    {
        0xFF,
        0xFF,
        0xFF,
        0xFF
    },
    [SEARCH_OK] =
    {
        0xFF,
        0xFF,
        SEARCH_ORDER,
        0xFF
    },
};

// Left, Right, Up, Down
static const u8 sSearchMovementMap_ShiftHoennDex[SEARCH_COUNT][4] =
{
    [SEARCH_NAME] =
    {
        0xFF,
        0xFF,
        0xFF,
        0xFF
    },
    [SEARCH_COLOR] =
    {
        0xFF,
        0xFF,
        0xFF,
        0xFF
    },
    [SEARCH_TYPE_LEFT] =
    {
        0xFF,
        0xFF,
        0xFF,
        0xFF
    },
    [SEARCH_TYPE_RIGHT] =
    {
        0xFF,
        0xFF,
        0xFF,
        0xFF
    },
    [SEARCH_ORDER] =
    {
        0xFF,
        0xFF,
        0xFF,
        SEARCH_OK
    },
    [SEARCH_MODE] =
    {
        0xFF,
        0xFF,
        0xFF,
        0xFF
    },
    [SEARCH_OK] =
    {
        0xFF,
        0xFF,
        SEARCH_ORDER,
        0xFF
    },
};

static const struct SearchOptionText sDexModeOptions[] =
{
    [DEX_MODE_HOENN]    = {gText_DexHoennDescription, gText_DexHoennTitle},
    [DEX_MODE_NATIONAL] = {gText_DexNatDescription,   gText_DexNatTitle},
    {},
};

static const struct SearchOptionText sDexOrderOptions[] =
{
    [ORDER_NUMERICAL]    = {gText_DexSortNumericalDescription, gText_DexSortNumericalTitle},
    [ORDER_ALPHABETICAL] = {gText_DexSortAtoZDescription,      gText_DexSortAtoZTitle},
    [ORDER_HEAVIEST]     = {gText_DexSortHeaviestDescription,  gText_DexSortHeaviestTitle},
    [ORDER_LIGHTEST]     = {gText_DexSortLightestDescription,  gText_DexSortLightestTitle},
    [ORDER_TALLEST]      = {gText_DexSortTallestDescription,   gText_DexSortTallestTitle},
    [ORDER_SMALLEST]     = {gText_DexSortSmallestDescription,  gText_DexSortSmallestTitle},
    {},
};

static const struct SearchOptionText sDexSearchNameOptions[] =
{
    {gText_DexEmptyString, gText_DexSearchDontSpecify},
    [NAME_ABC] = {gText_DexEmptyString, gText_DexSearchAlphaABC},
    [NAME_DEF] = {gText_DexEmptyString, gText_DexSearchAlphaDEF},
    [NAME_GHI] = {gText_DexEmptyString, gText_DexSearchAlphaGHI},
    [NAME_JKL] = {gText_DexEmptyString, gText_DexSearchAlphaJKL},
    [NAME_MNO] = {gText_DexEmptyString, gText_DexSearchAlphaMNO},
    [NAME_PQR] = {gText_DexEmptyString, gText_DexSearchAlphaPQR},
    [NAME_STU] = {gText_DexEmptyString, gText_DexSearchAlphaSTU},
    [NAME_VWX] = {gText_DexEmptyString, gText_DexSearchAlphaVWX},
    [NAME_YZ]  = {gText_DexEmptyString, gText_DexSearchAlphaYZ},
    {},
};

static const struct SearchOptionText sDexSearchColorOptions[] =
{
    {gText_DexEmptyString, gText_DexSearchDontSpecify},
    [BODY_COLOR_RED + 1]    = {gText_DexEmptyString, gText_DexSearchColorRed},
    [BODY_COLOR_BLUE + 1]   = {gText_DexEmptyString, gText_DexSearchColorBlue},
    [BODY_COLOR_YELLOW + 1] = {gText_DexEmptyString, gText_DexSearchColorYellow},
    [BODY_COLOR_GREEN + 1]  = {gText_DexEmptyString, gText_DexSearchColorGreen},
    [BODY_COLOR_BLACK + 1]  = {gText_DexEmptyString, gText_DexSearchColorBlack},
    [BODY_COLOR_BROWN + 1]  = {gText_DexEmptyString, gText_DexSearchColorBrown},
    [BODY_COLOR_PURPLE + 1] = {gText_DexEmptyString, gText_DexSearchColorPurple},
    [BODY_COLOR_GRAY + 1]   = {gText_DexEmptyString, gText_DexSearchColorGray},
    [BODY_COLOR_WHITE + 1]  = {gText_DexEmptyString, gText_DexSearchColorWhite},
    [BODY_COLOR_PINK + 1]   = {gText_DexEmptyString, gText_DexSearchColorPink},
    {},
};

static const struct SearchOptionText sDexSearchTypeOptions[NUMBER_OF_MON_TYPES + 1] = // + 2 for "None" and terminator, - 1 for Mystery
{
    {gText_DexEmptyString, gText_DexSearchTypeNone},
    {gText_DexEmptyString, gTypeNames[TYPE_NORMAL]},
    {gText_DexEmptyString, gTypeNames[TYPE_FIGHTING]},
    {gText_DexEmptyString, gTypeNames[TYPE_FLYING]},
    {gText_DexEmptyString, gTypeNames[TYPE_POISON]},
    {gText_DexEmptyString, gTypeNames[TYPE_GROUND]},
    {gText_DexEmptyString, gTypeNames[TYPE_ROCK]},
    {gText_DexEmptyString, gTypeNames[TYPE_BUG]},
    {gText_DexEmptyString, gTypeNames[TYPE_GHOST]},
    {gText_DexEmptyString, gTypeNames[TYPE_STEEL]},
    {gText_DexEmptyString, gTypeNames[TYPE_FIRE]},
    {gText_DexEmptyString, gTypeNames[TYPE_WATER]},
    {gText_DexEmptyString, gTypeNames[TYPE_GRASS]},
    {gText_DexEmptyString, gTypeNames[TYPE_ELECTRIC]},
    {gText_DexEmptyString, gTypeNames[TYPE_PSYCHIC]},
    {gText_DexEmptyString, gTypeNames[TYPE_ICE]},
    {gText_DexEmptyString, gTypeNames[TYPE_DRAGON]},
    {gText_DexEmptyString, gTypeNames[TYPE_DARK]},
    {gText_DexEmptyString, gTypeNames[TYPE_FAIRY]},
    {},
};

static const u8 sPokedexModes[] = {DEX_MODE_HOENN, DEX_MODE_NATIONAL};
static const u8 sOrderOptions[] =
{
    ORDER_NUMERICAL,
    ORDER_ALPHABETICAL,
    ORDER_HEAVIEST,
    ORDER_LIGHTEST,
    ORDER_TALLEST,
    ORDER_SMALLEST,
};

static const u8 sDexSearchTypeIds[NUMBER_OF_MON_TYPES] =
{
    TYPE_NONE,
    TYPE_NORMAL,
    TYPE_FIGHTING,
    TYPE_FLYING,
    TYPE_POISON,
    TYPE_GROUND,
    TYPE_ROCK,
    TYPE_BUG,
    TYPE_GHOST,
    TYPE_STEEL,
    TYPE_FIRE,
    TYPE_WATER,
    TYPE_GRASS,
    TYPE_ELECTRIC,
    TYPE_PSYCHIC,
    TYPE_ICE,
    TYPE_DRAGON,
    TYPE_DARK,
    TYPE_FAIRY,
};

// Number pairs are the task data for tracking the cursor pos and scroll offset of each option list
// See task data defines above Task_LoadSearchMenu
static const struct SearchOption sSearchOptions[] =
{
    [SEARCH_NAME]       = {sDexSearchNameOptions,  6,  7, ARRAY_COUNT(sDexSearchNameOptions) - 1},
    [SEARCH_COLOR]      = {sDexSearchColorOptions, 8,  9, ARRAY_COUNT(sDexSearchColorOptions) - 1},
    [SEARCH_TYPE_LEFT]  = {sDexSearchTypeOptions, 10, 11, ARRAY_COUNT(sDexSearchTypeOptions) - 1},
    [SEARCH_TYPE_RIGHT] = {sDexSearchTypeOptions, 12, 13, ARRAY_COUNT(sDexSearchTypeOptions) - 1},
    [SEARCH_ORDER]      = {sDexOrderOptions,       4,  5, ARRAY_COUNT(sDexOrderOptions) - 1},
    [SEARCH_MODE]       = {sDexModeOptions,        2,  3, ARRAY_COUNT(sDexModeOptions) - 1},
};

static const struct BgTemplate sSearchMenu_BgTemplate[] =
{
    {
        .bg = 0,
        .charBaseIndex = 2,
        .mapBaseIndex = 12,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    },
    {
        .bg = 1,
        .charBaseIndex = 0,
        .mapBaseIndex = 13,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 1,
        .baseTile = 0
    },
    {
        .bg = 2,
        .charBaseIndex = 2,
        .mapBaseIndex = 14,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 2,
        .baseTile = 0
    },
    {
        .bg = 3,
        .charBaseIndex = 0,
        .mapBaseIndex = 15,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 3,
        .baseTile = 0
    }
};

static const struct WindowTemplate sSearchMenu_WindowTemplate[] =
{
    {
        .bg = 2,
        .tilemapLeft = 0,
        .tilemapTop = 0,
        .width = 32,
        .height = 20,
        .paletteNum = 0,
        .baseBlock = 0x0001,
    },
    DUMMY_WIN_TEMPLATE
};

//************************************
//*                                  *
//*        MAIN                      *
//*                                  *
//************************************
void CB2_OpenPokedexPlusHGSS(void)
{
    switch (gMain.state)
    {
    case 0:
    default:
        SetVBlankCallback(NULL);
        ResetOtherVideoRegisters(0);
        DmaFillLarge16(3, 0, (u8 *)VRAM, VRAM_SIZE, 0x1000);
        DmaClear32(3, OAM, OAM_SIZE);
        DmaClear16(3, PLTT, PLTT_SIZE);
        gMain.state = 1;
        break;
    case 1:
        ScanlineEffect_Stop();
        ResetTasks();
        ResetSpriteData();
        ResetPaletteFade();
        FreeAllSpritePalettes();
        gReservedSpritePaletteCount = 8;
        ResetAllPicSprites();
        gMain.state++;
        break;
    case 2:
        sPokedexView = AllocZeroed(sizeof(struct PokedexView));
        ResetPokedexView(sPokedexView);
        CreateTask(Task_OpenPokedexMainPage, 0);
        sPokedexView->dexMode = gSaveBlock2Ptr->pokedex.mode;
        if (!IsNationalPokedexEnabled())
            sPokedexView->dexMode = DEX_MODE_HOENN;
        sPokedexView->dexOrder = gSaveBlock2Ptr->pokedex.order;
        sPokedexView->selectedPokemon = sLastSelectedPokemon;
        sPokedexView->pokeBallRotation = sPokeBallRotation;
        sPokedexView->selectedScreen = AREA_SCREEN;
        if (!IsNationalPokedexEnabled())
        {
            sPokedexView->seenCount = GetHoennPokedexCount(FLAG_GET_SEEN);
            sPokedexView->ownCount = GetHoennPokedexCount(FLAG_GET_CAUGHT);
        }
        else
        {
            sPokedexView->seenCount = GetNationalPokedexCount(FLAG_GET_SEEN);
            sPokedexView->ownCount = GetNationalPokedexCount(FLAG_GET_CAUGHT);
        }
        sPokedexView->initialVOffset = 8;
        gMain.state++;
        break;
    case 3:
        EnableInterrupts(1);
        SetVBlankCallback(VBlankCB_Pokedex);
        SetMainCallback2(CB2_Pokedex);
        CreatePokedexList(sPokedexView->dexMode, sPokedexView->dexOrder);
        m4aMPlayVolumeControl(&gMPlayInfo_BGM, TRACKS_ALL, 0x80);
        break;
    }
}

static void ResetPokedexView(struct PokedexView *pokedexView)
{
    u16 i;

    for (i = 0; i < NATIONAL_DEX_COUNT; i++)
    {
        pokedexView->pokedexList[i].dexNum = 0xFFFF;
        pokedexView->pokedexList[i].seen = FALSE;
        pokedexView->pokedexList[i].owned = FALSE;
    }
    pokedexView->pokedexList[NATIONAL_DEX_COUNT].dexNum = 0;
    pokedexView->pokedexList[NATIONAL_DEX_COUNT].seen = FALSE;
    pokedexView->pokedexList[NATIONAL_DEX_COUNT].owned = FALSE;
    pokedexView->pokemonListCount = 0;
    pokedexView->selectedPokemon = 0;
    pokedexView->selectedPokemonBackup = 0;
    pokedexView->dexMode = DEX_MODE_HOENN;
    pokedexView->dexModeBackup = DEX_MODE_HOENN;
    pokedexView->dexOrder = ORDER_NUMERICAL;
    pokedexView->dexOrderBackup = ORDER_NUMERICAL;
    pokedexView->seenCount = 0;
    pokedexView->ownCount = 0;
    for (i = 0; i < MAX_MONS_ON_SCREEN; i++)
        pokedexView->monSpriteIds[i] = 0xFFFF;
    pokedexView->pokeBallRotationStep = 0;
    pokedexView->pokeBallRotationBackup = 0;
    pokedexView->pokeBallRotation = 0;
    pokedexView->initialVOffset = 0;
    pokedexView->scrollTimer = 0;
    pokedexView->scrollDirection = 0;
    pokedexView->listVOffset = 0;
    pokedexView->listMovingVOffset = 0;
    pokedexView->scrollMonIncrement = 0;
    pokedexView->maxScrollTimer = 0;
    pokedexView->scrollSpeed = 0;
    for (i = 0; i < ARRAY_COUNT(pokedexView->unkArr1); i++)
        pokedexView->unkArr1[i] = 0;
    pokedexView->currentPage = PAGE_MAIN;
    pokedexView->currentPageBackup = PAGE_MAIN;
    pokedexView->isSearchResults = FALSE;
    pokedexView->selectedScreen = AREA_SCREEN;
    pokedexView->screenSwitchState = 0;
    for (i = 0; i < ARRAY_COUNT(pokedexView->unkArr2); i++)
        pokedexView->unkArr2[i] = 0;
    for (i = 0; i < ARRAY_COUNT(pokedexView->unkArr3); i++)
        pokedexView->unkArr3[i] = 0;
    pokedexView->originalSearchSelectionNum = 0;
}

static void VBlankCB_Pokedex(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void CB2_Pokedex(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

void Task_OpenPokedexMainPage(u8 taskId)
{
    sPokedexView->isSearchResults = FALSE;
    if (LoadPokedexListPage(PAGE_MAIN))
        gTasks[taskId].func = Task_HandlePokedexInput;
}

#define tLoadScreenTaskId data[0]

static void Task_HandlePokedexInput(u8 taskId)
{
	if (JOY_NEW(A_BUTTON) && sPokedexView->pokedexList[sPokedexView->selectedPokemon].seen)
	{
		UpdateSelectedMonSpriteId();
		BeginNormalPaletteFade(~(1 << (gSprites[sPokedexView->selectedMonSpriteId].oam.paletteNum + 16)), 0, 0, 0x10, RGB_BLACK);
		gSprites[sPokedexView->selectedMonSpriteId].callback = SpriteCB_MoveMonForInfoScreen;
		gTasks[taskId].func = Task_OpenInfoScreenAfterMonMovement;
		PlaySE(SE_PIN);
		FreeWindowAndBgBuffers();
	}
	else if (JOY_NEW(START_BUTTON))
	{
		sPokedexView->selectedPokemon = 0;
		sPokedexView->pokeBallRotation = POKEBALL_ROTATION_TOP;
		ClearMonSprites();
		CreateMonSpritesAtPos(sPokedexView->selectedPokemon, 0xE);
		PlaySE(SE_SELECT);
	}
	else if (JOY_NEW(SELECT_BUTTON))
	{
		PlaySE(SE_SELECT);
		BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 0x10, RGB_BLACK);
		gTasks[taskId].tLoadScreenTaskId = LoadSearchMenu();
		sPokedexView->screenSwitchState = 0;
		sPokedexView->pokeBallRotationBackup = sPokedexView->pokeBallRotation;
		sPokedexView->selectedPokemonBackup = sPokedexView->selectedPokemon;
		sPokedexView->dexModeBackup = sPokedexView->dexMode;
		sPokedexView->dexOrderBackup = sPokedexView->dexOrder;
		gTasks[taskId].func = Task_WaitForExitSearch;
		PlaySE(SE_PC_LOGIN);
		FreeWindowAndBgBuffers();
	}
	else if (JOY_NEW(B_BUTTON))
	{
		BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 0x10, RGB_BLACK);
		gTasks[taskId].func = Task_ClosePokedex;
		PlaySE(SE_PC_OFF);
	}
	else
	{
		//Handle D-pad
		sPokedexView->selectedPokemon = TryDoPokedexScroll(sPokedexView->selectedPokemon, 0xE);
		gTasks[taskId].func = Task_WaitForScroll;
	}
}

static void Task_WaitForScroll(u8 taskId)
{
    if (UpdateDexListScroll(sPokedexView->scrollDirection, sPokedexView->scrollMonIncrement, sPokedexView->maxScrollTimer))
        gTasks[taskId].func = Task_HandlePokedexInput;
}

// Opening the info screen from list view. Pokémon sprite is moving to its new position, wait for it to arrive
static void Task_OpenInfoScreenAfterMonMovement(u8 taskId)
{
    if (gSprites[sPokedexView->selectedMonSpriteId].x == MON_PAGE_X && gSprites[sPokedexView->selectedMonSpriteId].y == MON_PAGE_Y)
    {
        sPokedexView->currentPageBackup = sPokedexView->currentPage;
        gTasks[taskId].tLoadScreenTaskId = LoadInfoScreen(&sPokedexView->pokedexList[sPokedexView->selectedPokemon], sPokedexView->selectedMonSpriteId);
        gTasks[taskId].func = Task_WaitForExitInfoScreen;
    }
}

static void Task_WaitForExitInfoScreen(u8 taskId)
{
    if (gTasks[gTasks[taskId].tLoadScreenTaskId].isActive)
    {
        // While active, handle scroll input
        if (sPokedexView->currentPage == PAGE_INFO && !IsInfoScreenScrolling(gTasks[taskId].tLoadScreenTaskId) && TryDoInfoScreenScroll())
            StartInfoScreenScroll(&sPokedexView->pokedexList[sPokedexView->selectedPokemon], gTasks[taskId].tLoadScreenTaskId);
    }
    else
    {
        // Exiting, back to list view
        sLastSelectedPokemon = sPokedexView->selectedPokemon;
        sPokeBallRotation = sPokedexView->pokeBallRotation;
        gTasks[taskId].func = Task_OpenPokedexMainPage;
    }
}


static void Task_ClosePokedex(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        gSaveBlock2Ptr->pokedex.mode = sPokedexView->dexMode;
        if (!IsNationalPokedexEnabled())
            gSaveBlock2Ptr->pokedex.mode = DEX_MODE_HOENN;
        gSaveBlock2Ptr->pokedex.order = sPokedexView->dexOrder;
        ClearMonSprites();
        FreeWindowAndBgBuffers();
        DestroyTask(taskId);
        SetMainCallback2(CB2_ReturnToFieldWithOpenMenu);
        m4aMPlayVolumeControl(&gMPlayInfo_BGM, TRACKS_ALL, 0x100);
        Free(sPokedexView);
    }
}

static const u16 sHeaderTiles_Hoenn[20] = {
	2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
	53, 54, 55, 56, 57, 58, 59, 60, 61, 62
};
static const u16 sHeaderTiles_National[24] = {
	2, 3, 1, 4, 393, 394, 395, 412, 396, 397,
	53, 54, 52, 55, 398, 399, 400, 401, 402, 403
};
static const u16 sHeaderTiles_Search[24] = {
	2, 3, 1, 1, 1, 404, 405, 406, 407, 8,
	53, 54, 52, 52, 52, 408, 409, 61, 410, 411
};

static void UpdateHeaderTilemap(bool8 isSearchResults, bool8 isMainPage)
{
    u16 x, y;
    const u16 *tiles;
	
	if (isSearchResults || !isMainPage)
        return;
	
    if (isSearchResults)
        tiles = sHeaderTiles_Search;
    else if (sPokedexView->dexMode == DEX_MODE_HOENN)
        tiles = sHeaderTiles_Hoenn;
    else
        tiles = sHeaderTiles_National;

    for (y = 0; y < 2; y++)
    {
        for (x = 0; x < 10; x++)
        {
            u16 tileEntry = tiles[y * 10 + x];
            
            FillBgTilemapBufferRect(1, tileEntry, x, y, 1, 1, 0);
        }
    }

    ScheduleBgCopyTilemapToVram(1);
}

static void LoadPokedexBgPalette(bool8 isSearchResults, bool8 isMainPage)
{
	if (isSearchResults)
		LoadPalette(gPokedexPlusHGSS_SearchResults_Pal + 1, BG_PLTT_ID(0) + 1, PLTT_SIZEOF(6 * 16 - 1));
	else if (sPokedexView->dexMode == DEX_MODE_HOENN)
		LoadPalette(gPokedexPlusHGSS_Default_Pal + 1, BG_PLTT_ID(0) + 1, PLTT_SIZEOF(6 * 16 - 1));
	else
		LoadPalette(gPokedexPlusHGSS_National_Pal + 1, BG_PLTT_ID(0) + 1, PLTT_SIZEOF(6 * 16 - 1));
	LoadPalette(GetOverworldTextboxPalettePtr(), 0xF0, 32);
	LoadPalette(gPokedexPlusWorped_Text_Pal, BG_PLTT_ID(6), 32);
	LoadPalette(sMonType_Pal, BG_PLTT_ID(5), 32);
	
	UpdateHeaderTilemap(isSearchResults, isMainPage);
}

//************************************
//*                                  *
//*    Main scrolling list screen    *
//*                                  *
//************************************

// For loading main pokedex page or pokedex search results
static bool8 LoadPokedexListPage(u8 page)
{
    switch (gMain.state)
    {
    case 0:
    default:
        if (gPaletteFade.active)
            return 0;
        SetVBlankCallback(NULL);
        sPokedexView->currentPage = page;
        ResetOtherVideoRegisters(0);
        SetGpuReg(REG_OFFSET_BG2VOFS, sPokedexView->initialVOffset);
        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(0, sPokedex_BgTemplate, ARRAY_COUNT(sPokedex_BgTemplate));
		
        SetBgTilemapBuffer(3, AllocZeroed(BG_SCREEN_SIZE));
        SetBgTilemapBuffer(2, AllocZeroed(BG_SCREEN_SIZE));
        SetBgTilemapBuffer(1, AllocZeroed(BG_SCREEN_SIZE));
        SetBgTilemapBuffer(0, AllocZeroed(BG_SCREEN_SIZE));
		
		DecompressAndLoadBgGfxUsingHeap(3, gPokedexPlusHGSS_Menu_Gfx, 0x4000, 0, 0);
		
        CopyToBgTilemapBuffer(1, gPokedexPlusHGSS_ScreenList_Tilemap, 0, 0);
        CopyToBgTilemapBuffer(3, gPokedexPlusHGSS_ScreenListUnderlay_Tilemap, 0, 0);
		
        ResetPaletteFade();
		
        if (page == PAGE_MAIN)
            sPokedexView->isSearchResults = FALSE;
        else
            sPokedexView->isSearchResults = TRUE;
        LoadPokedexBgPalette(sPokedexView->isSearchResults, true);
		
        InitWindows(sPokemonList_WindowTemplate);
        DeactivateAllTextPrinters();

        FillWindowPixelBuffer(2, PIXEL_FILL(0));
		FillWindowPixelBuffer(3, PIXEL_FILL(0));
		FillWindowPixelBuffer(4, PIXEL_FILL(0));
		
        gMain.state = 1;
        break;
    case 1:
        ResetSpriteData();
        FreeAllSpritePalettes();
        gReservedSpritePaletteCount = 8;
        LoadCompressedSpriteSheet(&sInterfaceSpriteSheet);
        LoadSpritePalettes(sInterfaceSpritePalette);
        CreateInterfaceSprites(page);
		
        PrintPokedexListNavText(2, 4, page);
		PrintPokedexCounts(3);
		
		PutWindowTilemap(0);
        PutWindowTilemap(1);
        PutWindowTilemap(2);
        PutWindowTilemap(3);
        PutWindowTilemap(4);

        CopyWindowToVram(0, COPYWIN_FULL);
        CopyWindowToVram(1, COPYWIN_FULL);
        CopyWindowToVram(2, COPYWIN_FULL);
        CopyWindowToVram(3, COPYWIN_FULL);
        CopyWindowToVram(4, COPYWIN_FULL);
		
        gMain.state++;
        break;
    case 2:
        gMain.state++;
        break;
    case 3:
        if (page == PAGE_MAIN)
            CreatePokedexList(sPokedexView->dexMode, sPokedexView->dexOrder);
        if (sPokedexView->originalSearchSelectionNum != 0)
        {
            sPokedexListItem->dexNum = sPokedexView->originalSearchSelectionNum;
            sPokedexView->originalSearchSelectionNum = 0;
        }
        CreateMonSpritesAtPos(sPokedexView->selectedPokemon, 0xE);
        CopyBgTilemapBufferToVram(0);
        CopyBgTilemapBufferToVram(1);
        CopyBgTilemapBufferToVram(2);
        CopyBgTilemapBufferToVram(3);
        gMain.state++;
        break;
    case 4:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0x10, 0, RGB_BLACK);
        SetVBlankCallback(VBlankCB_Pokedex);
        gMain.state++;
        break;
    case 5:
        SetGpuReg(REG_OFFSET_WININ, WININ_WIN0_ALL | WININ_WIN1_ALL);
        SetGpuReg(REG_OFFSET_WINOUT, WINOUT_WIN01_ALL | WINOUT_WINOBJ_BG0 | WINOUT_WINOBJ_BG2 | WINOUT_WINOBJ_BG3 | WINOUT_WINOBJ_OBJ);
        SetGpuReg(REG_OFFSET_WIN0H, 0);
        SetGpuReg(REG_OFFSET_WIN0V, 0);
        SetGpuReg(REG_OFFSET_WIN1H, 0);
        SetGpuReg(REG_OFFSET_WIN1V, 0);
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        SetGpuReg(REG_OFFSET_BLDY, 0);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_MODE_0 | DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON | DISPCNT_OBJWIN_ON);
        ShowBg(0);
        ShowBg(1);
        ShowBg(2);
        ShowBg(3);
        gMain.state++;
        break;
    case 6:
        if (!gPaletteFade.active)
        {
            gMain.state = 0;
            return TRUE;
        }
        break;
    }
    return FALSE;
}

static void CreatePokedexList(u8 dexMode, u8 order)
{
    u16 vars[3]; //I have no idea why three regular variables are stored in an array, but whatever.
#define temp_dexCount   vars[0]
#define temp_isHoennDex vars[1]
#define temp_dexNum     vars[2]
    s16 i;

    sPokedexView->pokemonListCount = 0;

    switch (dexMode)
    {
    default:
    case DEX_MODE_HOENN:
        temp_dexCount = HOENN_DEX_COUNT;
        temp_isHoennDex = TRUE;
        break;
    case DEX_MODE_NATIONAL:
        if (IsNationalPokedexEnabled())
        {
            temp_dexCount = NATIONAL_DEX_COUNT;
            temp_isHoennDex = FALSE;
        }
        else
        {
            temp_dexCount = HOENN_DEX_COUNT;
            temp_isHoennDex = TRUE;
        }
        break;
    }

    switch (order)
    {
    case ORDER_NUMERICAL:
        if (temp_isHoennDex)
        {
            for (i = 0; i < temp_dexCount; i++)
            {
                temp_dexNum = HoennToNationalOrder(i + 1);
                sPokedexView->pokedexList[i].dexNum = temp_dexNum;
                sPokedexView->pokedexList[i].seen = GetSetPokedexFlag(temp_dexNum, FLAG_GET_SEEN);
                sPokedexView->pokedexList[i].owned = GetSetPokedexFlag(temp_dexNum, FLAG_GET_CAUGHT);
                if (sPokedexView->pokedexList[i].seen)
                    sPokedexView->pokemonListCount = i + 1;
            }
        }
        else
        {
            s16 r5, r10;
            for (i = 0, r5 = 0, r10 = 0; i < temp_dexCount; i++)
            {
                temp_dexNum = i + 1;
                if (GetSetPokedexFlag(temp_dexNum, FLAG_GET_SEEN))
                    r10 = 1;
                if (r10)
                {
                    sPokedexView->pokedexList[r5].dexNum = temp_dexNum;
                    sPokedexView->pokedexList[r5].seen = GetSetPokedexFlag(temp_dexNum, FLAG_GET_SEEN);
                    sPokedexView->pokedexList[r5].owned = GetSetPokedexFlag(temp_dexNum, FLAG_GET_CAUGHT);
                    if (sPokedexView->pokedexList[r5].seen)
                        sPokedexView->pokemonListCount = r5 + 1;
                    r5++;
                }
            }
        }
        break;
    case ORDER_ALPHABETICAL:
        for (i = 0; i < NATIONAL_DEX_COUNT; i++)
        {
            temp_dexNum = gPokedexOrder_Alphabetical[i];

            if ((!temp_isHoennDex || NationalToHoennOrder(temp_dexNum) != 0) && GetSetPokedexFlag(temp_dexNum, FLAG_GET_SEEN))
            {
                sPokedexView->pokedexList[sPokedexView->pokemonListCount].dexNum = temp_dexNum;
                sPokedexView->pokedexList[sPokedexView->pokemonListCount].seen = TRUE;
                sPokedexView->pokedexList[sPokedexView->pokemonListCount].owned = GetSetPokedexFlag(temp_dexNum, FLAG_GET_CAUGHT);
                sPokedexView->pokemonListCount++;
            }
        }
        break;
    case ORDER_HEAVIEST:
        for (i = NATIONAL_DEX_COUNT - 1; i >= 0; i--)
        {
            temp_dexNum = gPokedexOrder_Weight[i];

            if ((!temp_isHoennDex || NationalToHoennOrder(temp_dexNum) != 0) && GetSetPokedexFlag(temp_dexNum, FLAG_GET_CAUGHT))
            {
                sPokedexView->pokedexList[sPokedexView->pokemonListCount].dexNum = temp_dexNum;
                sPokedexView->pokedexList[sPokedexView->pokemonListCount].seen = TRUE;
                sPokedexView->pokedexList[sPokedexView->pokemonListCount].owned = TRUE;
                sPokedexView->pokemonListCount++;
            }
        }
        break;
    case ORDER_LIGHTEST:
        for (i = 0; i < NATIONAL_DEX_COUNT; i++)
        {
            temp_dexNum = gPokedexOrder_Weight[i];

            if ((!temp_isHoennDex || NationalToHoennOrder(temp_dexNum) != 0) && GetSetPokedexFlag(temp_dexNum, FLAG_GET_CAUGHT))
            {
                sPokedexView->pokedexList[sPokedexView->pokemonListCount].dexNum = temp_dexNum;
                sPokedexView->pokedexList[sPokedexView->pokemonListCount].seen = TRUE;
                sPokedexView->pokedexList[sPokedexView->pokemonListCount].owned = TRUE;
                sPokedexView->pokemonListCount++;
            }
        }
        break;
    case ORDER_TALLEST:
        for (i = NATIONAL_DEX_COUNT - 1; i >= 0; i--)
        {
            temp_dexNum = gPokedexOrder_Height[i];

            if ((!temp_isHoennDex || NationalToHoennOrder(temp_dexNum) != 0) && GetSetPokedexFlag(temp_dexNum, FLAG_GET_CAUGHT))
            {
                sPokedexView->pokedexList[sPokedexView->pokemonListCount].dexNum = temp_dexNum;
                sPokedexView->pokedexList[sPokedexView->pokemonListCount].seen = TRUE;
                sPokedexView->pokedexList[sPokedexView->pokemonListCount].owned = TRUE;
                sPokedexView->pokemonListCount++;
            }
        }
        break;
    case ORDER_SMALLEST:
        for (i = 0; i < NATIONAL_DEX_COUNT; i++)
        {
            temp_dexNum = gPokedexOrder_Height[i];

            if ((!temp_isHoennDex || NationalToHoennOrder(temp_dexNum) != 0) && GetSetPokedexFlag(temp_dexNum, FLAG_GET_CAUGHT))
            {
                sPokedexView->pokedexList[sPokedexView->pokemonListCount].dexNum = temp_dexNum;
                sPokedexView->pokedexList[sPokedexView->pokemonListCount].seen = TRUE;
                sPokedexView->pokedexList[sPokedexView->pokemonListCount].owned = TRUE;
                sPokedexView->pokemonListCount++;
            }
        }
        break;
    }

    for (i = sPokedexView->pokemonListCount; i < NATIONAL_DEX_COUNT; i++)
    {
        sPokedexView->pokedexList[i].dexNum = 0xFFFF;
        sPokedexView->pokedexList[i].seen = FALSE;
        sPokedexView->pokedexList[i].owned = FALSE;
    }
}

static void PrintMonDexNumAndName(u8 windowId, u8 fontId, const u8 *str, u8 left, u8 top) //WorpTODO: Fold this into whatever calls it. Pointless to have it in a function when it's a single line.
{
    AddTextPrinterParameterized4(windowId, fontId, (left * 8) + 5, (top * 8) + 1, 0, 0, sTextColors[0], TEXT_SKIP_DRAW, str);
}

static void PrintMonDexNumAndName_2(u8 windowId, u8 fontId, const u8* str, u8 left, u8 top) //HGSS_Ui offset for closer numer + text
{
    AddTextPrinterParameterized4(windowId, fontId, left * 8 - 9, (top * 8) + 1, 0, 0, sTextColors[0], -1, str);
}

// u16 ignored is passed but never used
#define MON_LIST_X 1
#define OFFSET_BALL (MON_LIST_X + 4)
#define OFFSET_NAME (MON_LIST_X + 6)
#define OFFSET_TYPE (MON_LIST_X + 20)
static void CreateMonListEntry(u8 position, u16 b, u16 ignored)
{
    s16 entryNum;
    u16 i, vOffset;

    switch (position)
    {
    case 0: // Initial
    default:
        entryNum = b - 5;
        for (i = 0; i <= 10; i++)
        {
            if (entryNum < 0 || entryNum >= NATIONAL_DEX_COUNT || sPokedexView->pokedexList[entryNum].dexNum == 0xFFFF)
            {
                ClearMonListEntry(MON_LIST_X, i * 2, ignored);
            }
            else
            {
                ClearMonListEntry(MON_LIST_X, i * 2, ignored);
                if (sPokedexView->pokedexList[entryNum].seen)
                {
                    CreateMonDexNum(entryNum, MON_LIST_X, i * 2, ignored);
                    CreateCaughtBall(sPokedexView->pokedexList[entryNum].owned, OFFSET_BALL, i * 2, ignored);
                    CreateMonName(sPokedexView->pokedexList[entryNum].dexNum, OFFSET_NAME, i * 2);
					CreateMonTypeIcons(sPokedexView->pokedexList[entryNum].owned, sPokedexView->pokedexList[entryNum].dexNum, OFFSET_TYPE, i * 2);
                }
                else
                {
                    CreateMonDexNum(entryNum, MON_LIST_X, i * 2, ignored);
                    CreateCaughtBall(FALSE, OFFSET_NAME, i * 2, ignored);
                    CreateMonName(0, OFFSET_NAME, i * 2);
					CreateMonTypeIcons(FALSE, sPokedexView->pokedexList[entryNum].dexNum, OFFSET_TYPE, i * 2);
                }
            }
            entryNum++;
        }
        break;
    case 1: // Up
        entryNum = b - 5;
        if (entryNum < 0 || entryNum >= NATIONAL_DEX_COUNT || sPokedexView->pokedexList[entryNum].dexNum == 0xFFFF)
        {
            ClearMonListEntry(MON_LIST_X, sPokedexView->listVOffset * 2, ignored);
        }
        else
        {
            ClearMonListEntry(MON_LIST_X, sPokedexView->listVOffset * 2, ignored);
            if (sPokedexView->pokedexList[entryNum].seen)
            {
                CreateMonDexNum(entryNum, MON_LIST_X, sPokedexView->listVOffset * 2, ignored);
                CreateCaughtBall(sPokedexView->pokedexList[entryNum].owned, OFFSET_BALL, sPokedexView->listVOffset * 2, ignored);
                CreateMonName(sPokedexView->pokedexList[entryNum].dexNum, OFFSET_NAME, sPokedexView->listVOffset * 2);
				CreateMonTypeIcons(sPokedexView->pokedexList[entryNum].owned, sPokedexView->pokedexList[entryNum].dexNum, OFFSET_TYPE, sPokedexView->listVOffset * 2);
            }
            else
            {
                CreateMonDexNum(entryNum, MON_LIST_X, sPokedexView->listVOffset * 2, ignored);
                CreateCaughtBall(FALSE, OFFSET_BALL, sPokedexView->listVOffset * 2, ignored);
                CreateMonName(0, OFFSET_NAME, sPokedexView->listVOffset * 2);
				CreateMonTypeIcons(FALSE, sPokedexView->pokedexList[entryNum].dexNum, OFFSET_TYPE, sPokedexView->listVOffset * 2);
            }
        }
        break;
    case 2: // Down
        entryNum = b + 5;
        vOffset = sPokedexView->listVOffset + 10;
        if (vOffset >= LIST_SCROLL_STEP)
            vOffset -= LIST_SCROLL_STEP;
        if (entryNum < 0 || entryNum >= NATIONAL_DEX_COUNT || sPokedexView->pokedexList[entryNum].dexNum == 0xFFFF)
            ClearMonListEntry(MON_LIST_X, vOffset * 2, ignored);
        else
        {
            ClearMonListEntry(MON_LIST_X, vOffset * 2, ignored);
            if (sPokedexView->pokedexList[entryNum].seen)
            {
                CreateMonDexNum(entryNum, MON_LIST_X, vOffset * 2, ignored);
                CreateCaughtBall(sPokedexView->pokedexList[entryNum].owned, OFFSET_BALL, vOffset * 2, ignored);
                CreateMonName(sPokedexView->pokedexList[entryNum].dexNum, OFFSET_NAME, vOffset * 2);
				CreateMonTypeIcons(sPokedexView->pokedexList[entryNum].owned, sPokedexView->pokedexList[entryNum].dexNum, OFFSET_TYPE, vOffset * 2);
            }
            else
            {
                CreateMonDexNum(entryNum, MON_LIST_X, vOffset * 2, ignored);
                CreateCaughtBall(FALSE, OFFSET_BALL, vOffset * 2, ignored);
                CreateMonName(0, OFFSET_NAME, vOffset * 2);
				CreateMonTypeIcons(FALSE, sPokedexView->pokedexList[entryNum].dexNum, OFFSET_TYPE, vOffset * 2);
            }
        }
        break;
    }
    CopyWindowToVram(0, COPYWIN_GFX);
    CopyWindowToVram(1, COPYWIN_GFX);
}

static void CreateMonDexNum(u16 entryNum, u8 left, u8 top, u16 unused)
{
    u8 text[6];
    u16 dexNum;

    memcpy(text, sText_No000, ARRAY_COUNT(text));
    dexNum = sPokedexView->pokedexList[entryNum].dexNum;
    if (sPokedexView->dexMode == DEX_MODE_HOENN)
        dexNum = NationalToHoennOrder(dexNum);
    text[2] = CHAR_0 + dexNum / 100;
    text[3] = CHAR_0 + (dexNum % 100) / 10;
    text[4] = CHAR_0 + (dexNum % 100) % 10;
    PrintMonDexNumAndName(0, FONT_NARROW, text, left, top);
}

static void CreateCaughtBall(bool16 owned, u8 x, u8 y, u16 unused)
{
    if (owned)
        BlitBitmapToWindow(0, sCaughtBall_Gfx, (x * 8) - 1, y * 8, 8, 16);
    else
        FillWindowPixelRect(0, PIXEL_FILL(0), (x * 8) - 1, y * 8, 8, 16);
}

static u8 CreateMonName(u16 num, u8 left, u8 top)
{
    const u8 *str;

    num = NationalPokedexNumToSpecies(num);
    if (num)
        str = gSpeciesNames[num];
    else
        str = sText_TenDashes;
    PrintMonDexNumAndName_2(0, FONT_NARROW, str, left, top);
    return StringLength(str);
}

static u8 CreateMonTypeIcons(bool16 owned, u16 num, u8 left, u8 top)
{
	u8 x = 1;
	
	if (owned)
		{
		u16 species = NationalPokedexNumToSpecies(num);
		u8 type1 = gSpeciesInfo[species].types[0];
		u8 type2 = gSpeciesInfo[species].types[1];

		BlitBitmapToWindow(1, (const u8 *)&sMonType_Gfx[type1 * 64], (x * 8) + 4, top * 8, 32, 16);

		if (type1 != type2)
		{
			BlitBitmapToWindow(1, (const u8 *)&sMonType_Gfx[type2 * 64], (x * 8) + 36, top * 8, 32, 16);
		}
	}
	else
    {
        FillWindowPixelRect(1, PIXEL_FILL(0), (x * 8) + 4, top * 8, 96, 16);
    }
}

static void ClearMonListEntry(u8 x, u8 y, u16 unused)
{
    FillWindowPixelRect(0, PIXEL_FILL(0), x * 8, y * 8, 0x60, 16);
}

// u16 ignored is passed but never used
static void CreateMonSpritesAtPos(u16 selectedMon, u16 ignored)
{
    u8 i;
    u16 dexNum;
    u8 spriteId;

    gPaletteFade.bufferTransferDisabled = TRUE;

    for (i = 0; i < MAX_MONS_ON_SCREEN; i++)
        sPokedexView->monSpriteIds[i] = 0xFFFF;
    sPokedexView->selectedMonSpriteId = 0xFFFF;

    // Create top mon sprite
    dexNum = GetPokemonSpriteToDisplay(selectedMon - 1);
    if (dexNum != 0xFFFF)
    {
        spriteId = CreatePokedexMonSprite(dexNum, SCROLLING_MON_X, 0x50);
        gSprites[spriteId].callback = SpriteCB_PokedexListMonSprite;
        gSprites[spriteId].data[5] = -32;
    }

    // Create mid mon sprite
    dexNum = GetPokemonSpriteToDisplay(selectedMon);
    if (dexNum != 0xFFFF)
    {
        spriteId = CreatePokedexMonSprite(dexNum, SCROLLING_MON_X, 0x50);
        gSprites[spriteId].callback = SpriteCB_PokedexListMonSprite;
        gSprites[spriteId].data[5] = 0;
    }

    // Create bottom mon sprite
    dexNum = GetPokemonSpriteToDisplay(selectedMon + 1);
    if (dexNum != 0xFFFF)
    {
        spriteId = CreatePokedexMonSprite(dexNum, SCROLLING_MON_X, 0x50);
        gSprites[spriteId].callback = SpriteCB_PokedexListMonSprite;
        gSprites[spriteId].data[5] = 32;
    }

    CreateMonListEntry(0, selectedMon, ignored);
    SetGpuReg(REG_OFFSET_BG2VOFS, sPokedexView->initialVOffset);

    sPokedexView->listVOffset = 0;
    sPokedexView->listMovingVOffset = 0;

    gPaletteFade.bufferTransferDisabled = FALSE;
}

static bool8 UpdateDexListScroll(u8 direction, u8 monMoveIncrement, u8 scrollTimerMax)
{
    u16 i;
    u8 step;

    if (sPokedexView->scrollTimer)
    {
        sPokedexView->scrollTimer--;
        switch (direction)
        {
        case 1: // Up
            for (i = 0; i < MAX_MONS_ON_SCREEN; i++)
            {
                if (sPokedexView->monSpriteIds[i] != 0xFFFF)
                    gSprites[sPokedexView->monSpriteIds[i]].data[5] += monMoveIncrement;
            }
            step = LIST_SCROLL_STEP * (scrollTimerMax - sPokedexView->scrollTimer) / scrollTimerMax;
            SetGpuReg(REG_OFFSET_BG2VOFS, sPokedexView->initialVOffset + sPokedexView->listMovingVOffset * LIST_SCROLL_STEP - step);
            sPokedexView->pokeBallRotation -= sPokedexView->pokeBallRotationStep;
            break;
        case 2: // Down
            for (i = 0; i < MAX_MONS_ON_SCREEN; i++)
            {
                if (sPokedexView->monSpriteIds[i] != 0xFFFF)
                    gSprites[sPokedexView->monSpriteIds[i]].data[5] -= monMoveIncrement;
            }
            step = LIST_SCROLL_STEP * (scrollTimerMax - sPokedexView->scrollTimer) / scrollTimerMax;
            SetGpuReg(REG_OFFSET_BG2VOFS, sPokedexView->initialVOffset + sPokedexView->listMovingVOffset * LIST_SCROLL_STEP + step);
            sPokedexView->pokeBallRotation += sPokedexView->pokeBallRotationStep;
            break;
        }
        return FALSE;
    }
    else
    {
        SetGpuReg(REG_OFFSET_BG2VOFS, sPokedexView->initialVOffset + sPokedexView->listVOffset * LIST_SCROLL_STEP);
        return TRUE;
    }
}

static void CreateScrollingPokemonSprite(u8 direction, u16 selectedMon)
{
    u16 dexNum;
    u8 spriteId;

    sPokedexView->listMovingVOffset = sPokedexView->listVOffset;
    switch (direction)
    {
    case 1: // up
        dexNum = GetPokemonSpriteToDisplay(selectedMon - 1);
        if (dexNum != 0xFFFF)
        {
            spriteId = CreatePokedexMonSprite(dexNum, SCROLLING_MON_X, 0x50);
            gSprites[spriteId].callback = SpriteCB_PokedexListMonSprite;
            gSprites[spriteId].data[5] = -64;
        }
        if (sPokedexView->listVOffset > 0)
            sPokedexView->listVOffset--;
        else
            sPokedexView->listVOffset = LIST_SCROLL_STEP - 1;
        break;
    case 2: // down
        dexNum = GetPokemonSpriteToDisplay(selectedMon + 1);
        if (dexNum != 0xFFFF)
        {
            spriteId = CreatePokedexMonSprite(dexNum, SCROLLING_MON_X, 0x50);
            gSprites[spriteId].callback = SpriteCB_PokedexListMonSprite;
            gSprites[spriteId].data[5] = 64;
        }
        if (sPokedexView->listVOffset < LIST_SCROLL_STEP - 1)
            sPokedexView->listVOffset++;
        else
            sPokedexView->listVOffset = 0;
        break;
    }
}

// u16 ignored is passed but never used
static u16 TryDoPokedexScroll(u16 selectedMon, u16 ignored)
{
    u8 scrollTimer;
    u8 scrollMonIncrement;
    u8 i;
    u16 startingPos;
    u8 scrollDir = 0;

    if(JOY_NEW(DPAD_UP) && (selectedMon == 0))
    {
        selectedMon = sPokedexView->pokemonListCount - 1;
        ClearMonSprites();
        CreateMonSpritesAtPos(selectedMon, 0xE);
        sPokedexView->justScrolled = TRUE; //HGSS_Ui
        PlaySE(SE_DEX_SCROLL);
    }
    else if (JOY_NEW(DPAD_DOWN) && (selectedMon == sPokedexView->pokemonListCount - 1))
    {
        selectedMon = 0;
        ClearMonSprites();
        CreateMonSpritesAtPos(selectedMon, 0xE);
        sPokedexView->justScrolled = TRUE; //HGSS_Ui
        PlaySE(SE_DEX_SCROLL);
    }
    else if (JOY_HELD(DPAD_UP) && (selectedMon > 0))
    {
        scrollDir = 1;
        selectedMon = GetNextPosition(1, selectedMon, 0, sPokedexView->pokemonListCount - 1);
        CreateScrollingPokemonSprite(1, selectedMon);
        CreateMonListEntry(1, selectedMon, ignored);
        sPokedexView->justScrolled = TRUE; //HGSS_Ui
        PlaySE(SE_DEX_SCROLL);
    }
    else if (JOY_HELD(DPAD_DOWN) && (selectedMon < sPokedexView->pokemonListCount - 1))
    {
        scrollDir = 2;
        selectedMon = GetNextPosition(0, selectedMon, 0, sPokedexView->pokemonListCount - 1);
        CreateScrollingPokemonSprite(2, selectedMon);
        CreateMonListEntry(2, selectedMon, ignored);
        sPokedexView->justScrolled = TRUE; //HGSS_Ui
        PlaySE(SE_DEX_SCROLL);
    }
    else if (JOY_HELD(DPAD_LEFT) && JOY_HELD(R_BUTTON) && (selectedMon > 0))
    {
        startingPos = selectedMon;

        for (i = 0; i < 16; i++)
            selectedMon = GetNextPosition(1, selectedMon, 0, sPokedexView->pokemonListCount - 1);
        sPokedexView->pokeBallRotation += 16 * (selectedMon - startingPos);
        ClearMonSprites();
        CreateMonSpritesAtPos(selectedMon, 0xE);
        sPokedexView->justScrolled = TRUE; //HGSS_Ui
        PlaySE(SE_DEX_PAGE);
    }
    else if (JOY_HELD(DPAD_RIGHT) && JOY_HELD(R_BUTTON) && (selectedMon < sPokedexView->pokemonListCount - 1))
    {
        startingPos = selectedMon;
        for (i = 0; i < 16; i++)
            selectedMon = GetNextPosition(0, selectedMon, 0, sPokedexView->pokemonListCount - 1);
        sPokedexView->pokeBallRotation += 16 * (selectedMon - startingPos);
        ClearMonSprites();
        CreateMonSpritesAtPos(selectedMon, 0xE);
        sPokedexView->justScrolled = TRUE; //HGSS_Ui
        PlaySE(SE_DEX_PAGE);
    }
    else if (JOY_HELD(DPAD_LEFT) && (selectedMon > 0))
    {
        startingPos = selectedMon;

        for (i = 0; i < 7; i++)
            selectedMon = GetNextPosition(1, selectedMon, 0, sPokedexView->pokemonListCount - 1);
        sPokedexView->pokeBallRotation += 16 * (selectedMon - startingPos);
        ClearMonSprites();
        CreateMonSpritesAtPos(selectedMon, 0xE);
        sPokedexView->justScrolled = TRUE; //HGSS_Ui
        PlaySE(SE_DEX_SCROLL);
    }
    else if (JOY_HELD(DPAD_RIGHT) && (selectedMon < sPokedexView->pokemonListCount - 1))
    {
        startingPos = selectedMon;
        for (i = 0; i < 7; i++)
            selectedMon = GetNextPosition(0, selectedMon, 0, sPokedexView->pokemonListCount - 1);
        sPokedexView->pokeBallRotation += 16 * (selectedMon - startingPos);
        ClearMonSprites();
        CreateMonSpritesAtPos(selectedMon, 0xE);
        sPokedexView->justScrolled = TRUE; //HGSS_Ui
        PlaySE(SE_DEX_SCROLL);
    }

    if (scrollDir == 0)
    {
        // Left/right input just snaps up/down, no scrolling
        sPokedexView->scrollSpeed = 0;
        return selectedMon;
    }

    scrollMonIncrement = sScrollMonIncrements[sPokedexView->scrollSpeed / 4];
    scrollTimer = sScrollTimers[sPokedexView->scrollSpeed / 4];
    sPokedexView->scrollTimer = scrollTimer;
    sPokedexView->maxScrollTimer = scrollTimer;
    sPokedexView->scrollMonIncrement = scrollMonIncrement;
    sPokedexView->scrollDirection = scrollDir;
    sPokedexView->pokeBallRotationStep = scrollMonIncrement / 2;
    UpdateDexListScroll(sPokedexView->scrollDirection, sPokedexView->scrollMonIncrement, sPokedexView->maxScrollTimer);
    if (sPokedexView->scrollSpeed < 12)
        sPokedexView->scrollSpeed++;
    return selectedMon;
}

static void UpdateSelectedMonSpriteId(void)
{
    u16 i;

    for (i = 0; i < MAX_MONS_ON_SCREEN; i++)
    {
        u16 spriteId = sPokedexView->monSpriteIds[i];

        if (gSprites[spriteId].x2 == 0 && gSprites[spriteId].y2 == 0 && spriteId != 0xFFFF)
            sPokedexView->selectedMonSpriteId = spriteId;
    }
}

static bool8 TryDoInfoScreenScroll(void)
{
    u16 nextPokemon;
    u16 selectedPokemon = sPokedexView->selectedPokemon;

    if (sPokedexView->sEvoScreenData.fromEvoPage)
        return FALSE;

    if (JOY_NEW(DPAD_UP) && selectedPokemon)
    {
        nextPokemon = selectedPokemon;
        while (nextPokemon != 0)
        {
            nextPokemon = GetNextPosition(1, nextPokemon, 0, sPokedexView->pokemonListCount - 1);

            if (sPokedexView->pokedexList[nextPokemon].seen)
            {
                selectedPokemon = nextPokemon;
                break;
            }
        }

        if (sPokedexView->selectedPokemon == selectedPokemon)
            return FALSE;
        else
        {
            sPokedexView->selectedPokemon = selectedPokemon;
            sPokedexView->pokeBallRotation -= 16;
            return TRUE;
        }
    }
    else if (JOY_NEW(DPAD_DOWN) && selectedPokemon < sPokedexView->pokemonListCount - 1)
    {
        nextPokemon = selectedPokemon;
        while (nextPokemon < sPokedexView->pokemonListCount - 1)
        {
            nextPokemon = GetNextPosition(0, nextPokemon, 0, sPokedexView->pokemonListCount - 1);

            if (sPokedexView->pokedexList[nextPokemon].seen)
            {
                selectedPokemon = nextPokemon;
                break;
            }
        }

        if (sPokedexView->selectedPokemon == selectedPokemon)
            return FALSE;
        else
        {
            sPokedexView->selectedPokemon = selectedPokemon;
            sPokedexView->pokeBallRotation += 16;
            return TRUE;
        }
    }
    return FALSE;
}

static u8 ClearMonSprites(void)
{
    u16 i;

    for (i = 0; i < MAX_MONS_ON_SCREEN; i++)
    {
        if (sPokedexView->monSpriteIds[i] != 0xFFFF)
        {
            FreeAndDestroyMonPicSprite(sPokedexView->monSpriteIds[i]);
            sPokedexView->monSpriteIds[i] = 0xFFFF;
        }
    }
    return FALSE;
}

static u16 GetPokemonSpriteToDisplay(u16 species)
{
    if (species >= NATIONAL_DEX_COUNT || sPokedexView->pokedexList[species].dexNum == 0xFFFF)
        return 0xFFFF;
    else if (sPokedexView->pokedexList[species].seen)
        return sPokedexView->pokedexList[species].dexNum;
    else
        return 0;
}

static u32 CreatePokedexMonSprite(u16 num, s16 x, s16 y)
{
    u8 i;

    for (i = 0; i < MAX_MONS_ON_SCREEN; i++)
    {
        if (sPokedexView->monSpriteIds[i] == 0xFFFF)
        {
            u8 spriteId = CreateMonSpriteFromNationalDexNumber(num, x, y, i);

            gSprites[spriteId].oam.affineMode = ST_OAM_AFFINE_NORMAL;
            gSprites[spriteId].oam.priority = 3;
            gSprites[spriteId].data[0] = 0;
            gSprites[spriteId].data[1] = i;
            gSprites[spriteId].data[2] = NationalPokedexNumToSpecies(num);
            sPokedexView->monSpriteIds[i] = spriteId;
            return spriteId;
        }
    }
    return 0xFFFF;
}

#define sIsDownArrow data[1]
#define LIST_RIGHT_SIDE_TEXT_X 204
#define LIST_RIGHT_SIDE_TEXT_X_OFFSET 12
#define LIST_RIGHT_SIDE_TEXT_Y_OFFSET 13
static void CreateInterfaceSprites(u8 page)
{
    u8 spriteId;

    CreateSprite(&sScrollBarSpriteTemplate, 8, 30, 0);
}

static void PrintPokedexListNavText(u8 windowId, u8 windowId2, u8 page)
{
	u8 sText_Nav[] = _("{START_BUTTON} Top {SELECT_BUTTON} Search");
	u8 sText_Bottom_Exit[] = _("{B_BUTTON} Exit");
	u8 sText_Bottom_Back[] = _("{B_BUTTON} Back");
	
    FillWindowPixelBuffer(windowId, PIXEL_FILL(0)); 
    FillWindowPixelBuffer(windowId2, PIXEL_FILL(0)); 

    if (page == PAGE_MAIN)
	{
		AddTextPrinterParameterized4(windowId, FONT_SHORT, 0, 1, 0, 0, sTextColors[1], 0, sText_Nav);
		AddTextPrinterParameterized4(windowId2, 0, 3, 0, 0, 0, sTextColors[1], 0, sText_Bottom_Exit);
	}
	else if (page == PAGE_SEARCH_RESULTS)
	{
		AddTextPrinterParameterized4(windowId, FONT_SHORT, 0, 1, 0, 0, sTextColors[1], 0, sText_Nav);
		AddTextPrinterParameterized4(windowId2, 0, 3, 0, 0, 0, sTextColors[1], 0, sText_Bottom_Back);
	}

    CopyWindowToVram(windowId, COPYWIN_FULL);
    CopyWindowToVram(windowId2, COPYWIN_FULL);
}

static void PrintPokedexCounts(u8 windowId)
{
    u8 str[20];
	u8 sText_Hoenn[] = _("Hoenn Dex");
	u8 sText_National[] = _("Nat'l Dex");
	u8 sText_Seen[] = _("Seen");
	u8 sText_Caught[] = _("Caught");
    u16 hoennSeen = GetHoennPokedexCount(FLAG_GET_SEEN);
    u16 hoennCaught = GetHoennPokedexCount(FLAG_GET_CAUGHT);
    u16 nationalSeen = sPokedexView->seenCount;
    u16 nationalCaught = sPokedexView->ownCount;
	
	FillWindowPixelBuffer(windowId, PIXEL_FILL(0)); 
	
    u8 xNumbers = 47; 
    u8 xText = 2; 
    u8 yHoenn = 37 - 16;
    u8 yNational = 111 - 16;
	
	AddTextPrinterParameterized4(windowId, FONT_BW_SUMMARY_SCREEN, xText, yHoenn - 15, 0, 0, sTextColors[0], 0, sText_Hoenn); //windowID, Fontid, x, y,  letter spacing, line spacing, color, speed, string
	FillWindowPixelRect(windowId, 2, xText, yHoenn - 3, 50, 1);
	AddTextPrinterParameterized4(windowId, FONT_BW_SUMMARY_SCREEN, xText, yHoenn, 0, 0, sTextColors[0], 0, sText_Seen);
	AddTextPrinterParameterized4(windowId, FONT_BW_SUMMARY_SCREEN, xText, yHoenn + 12, 0, 0, sTextColors[0], 0, sText_Caught);
	
    ConvertIntToDecimalStringN(str, hoennSeen, STR_CONV_MODE_LEFT_ALIGN, 3);
	AddTextPrinterParameterized4(windowId, FONT_BW_SUMMARY_SCREEN, xNumbers, yHoenn, 0, 0, sTextColors[0], 0, str);
    ConvertIntToDecimalStringN(str, hoennCaught, STR_CONV_MODE_LEFT_ALIGN, 3);
	AddTextPrinterParameterized4(windowId, FONT_BW_SUMMARY_SCREEN, xNumbers, yHoenn + 12, 0, 0, sTextColors[0], 0, str);

    if (!sPokedexView->dexMode == DEX_MODE_HOENN || sPokedexView->isSearchResults)
    {
		AddTextPrinterParameterized4(windowId, FONT_BW_SUMMARY_SCREEN, xText, yNational - 15, 0, 0, sTextColors[0], 0, sText_National); //windowID, Fontid, x, y,  letter spacing, line spacing, color, speed, string
		FillWindowPixelRect(windowId, 2, xText, yNational - 3, 50, 1);
		AddTextPrinterParameterized4(windowId, FONT_BW_SUMMARY_SCREEN, xText, yNational, 0, 0, sTextColors[0], 0, sText_Seen);
		AddTextPrinterParameterized4(windowId, FONT_BW_SUMMARY_SCREEN, xText, yNational + 12, 0, 0, sTextColors[0], 0, sText_Caught);
		
        ConvertIntToDecimalStringN(str, nationalSeen, STR_CONV_MODE_LEFT_ALIGN, 3);
		AddTextPrinterParameterized4(windowId, FONT_BW_SUMMARY_SCREEN, xNumbers, yNational, 0, 0, sTextColors[0], 0, str);
        ConvertIntToDecimalStringN(str, nationalCaught, STR_CONV_MODE_LEFT_ALIGN, 3);
		AddTextPrinterParameterized4(windowId, FONT_BW_SUMMARY_SCREEN, xNumbers, yNational + 12, 0, 0, sTextColors[0], 0, str);
    }
	
	CopyWindowToVram(windowId, COPYWIN_FULL);
}

static void SpriteCB_EndMoveMonForInfoScreen(struct Sprite *sprite)
{
    // Once mon is done moving there's nothing left to do
}

static void SpriteCB_SeenOwnInfo(struct Sprite *sprite)
{
    if (sPokedexView->currentPage != PAGE_MAIN && sPokedexView->currentPage != PAGE_SEARCH_RESULTS)
        DestroySprite(sprite);
}

void SpriteCB_MoveMonForInfoScreen(struct Sprite *sprite)
{
    sprite->oam.priority = 0;
    sprite->oam.affineMode = ST_OAM_AFFINE_OFF;
    sprite->x2 = 0;
    sprite->y2 = 0;
    if (sprite->x != MON_PAGE_X || sprite->y != MON_PAGE_Y)
    {
        if (sprite->x > 48)
            sprite->x -= 4;
        if (sprite->x < 48)
            sprite->x = 48;

        if (sprite->y > 56)
            sprite->y -= 4;
        if (sprite->y < 56)
            sprite->y = 56;
    }
    else
    {
        sprite->callback = SpriteCB_EndMoveMonForInfoScreen;
    }
}

static void SpriteCB_PokedexListMonSprite(struct Sprite *sprite)
{
    u8 monId = sprite->data[1];

    if (sPokedexView->currentPage != PAGE_MAIN && sPokedexView->currentPage != PAGE_SEARCH_RESULTS)
    {
        FreeAndDestroyMonPicSprite(sPokedexView->monSpriteIds[monId]);
        sPokedexView->monSpriteIds[monId] = 0xFFFF;
    }
    else
    {
        u32 var;
        sprite->y2 = gSineTable[(u8)sprite->data[5]] * 76 / 256;
        var = SAFE_DIV(0x10000, gSineTable[sprite->data[5] + 64]);
        if (var > 0xFFFF)
            var = 0xFFFF;
        SetOamMatrix(sprite->data[1] + 1, 0x100, 0, 0, var);
        sprite->oam.matrixNum = monId + 1;

        if (sprite->data[5] > -64 && sprite->data[5] < 64)
        {
            sprite->invisible = FALSE;
            sprite->data[0] = 1;
        }
        else
        {
            sprite->invisible = TRUE;
        }

        if ((sprite->data[5] <= -64 || sprite->data[5] >= 64) && sprite->data[0] != 0)
        {
            FreeAndDestroyMonPicSprite(sPokedexView->monSpriteIds[monId]);
            sPokedexView->monSpriteIds[monId] = 0xFFFF;
        }
    }
}

static void SpriteCB_Scrollbar(struct Sprite *sprite)
{
    if (sPokedexView->currentPage != PAGE_MAIN && sPokedexView->currentPage != PAGE_SEARCH_RESULTS)
        DestroySprite(sprite);
    else
        sprite->y2 = sPokedexView->selectedPokemon * 110 / (sPokedexView->pokemonListCount - 1);
}

static void SpriteCB_ScrollArrow(struct Sprite *sprite)
{
    if (sPokedexView->currentPage != PAGE_MAIN && sPokedexView->currentPage != PAGE_SEARCH_RESULTS)
    {
        DestroySprite(sprite);
    }
    else
    {
        u8 r0;

        if (sprite->sIsDownArrow)
        {
            if (sPokedexView->selectedPokemon == sPokedexView->pokemonListCount - 1)
                sprite->invisible = TRUE;
            else
                sprite->invisible = FALSE;
            r0 = sprite->data[2];
        }
        else
        {
            if (sPokedexView->selectedPokemon == 0)
                sprite->invisible = TRUE;
            else
                sprite->invisible = FALSE;
            r0 = sprite->data[2] - 128;
        }
        sprite->y2 = gSineTable[r0] / 64;
        sprite->data[2] = sprite->data[2] + 8;
    }
}

static void SpriteCB_DexListInterfaceText(struct Sprite *sprite)
{
    if (sPokedexView->currentPage != PAGE_MAIN && sPokedexView->currentPage != PAGE_SEARCH_RESULTS)
        DestroySprite(sprite);
}

static void SpriteCB_RotatingPokeBall(struct Sprite *sprite)
{
    if (sPokedexView->currentPage != PAGE_MAIN && sPokedexView->currentPage != PAGE_SEARCH_RESULTS)
    {
        DestroySprite(sprite);
    }
    else
    {
        u8 val;
        s16 r3;
        s16 r0;

        val = sPokedexView->pokeBallRotation + sprite->data[1];
        r3 = gSineTable[val];
        r0 = gSineTable[val + 64];
        SetOamMatrix(sprite->data[0], r0, r3, -r3, r0);

        val = sPokedexView->pokeBallRotation + (sprite->data[1] + 64);
        r3 = gSineTable[val];
        r0 = gSineTable[val + 64];
        sprite->x2 = r0 * 40 / 256;
        sprite->y2 = r3 * 40 / 256;
    }
}

//************************************
//*                                  *
//*        Info screen               *
//*                                  *
//************************************
#define tScrolling       data[0]
#define tMonSpriteDone   data[1]
#define tBgLoaded        data[2]
#define tSkipCry         data[3]
#define tMonSpriteId     data[4]
#define tTrainerSpriteId data[5]

static u8 LoadInfoScreen(struct PokedexListItem *item, u8 monSpriteId)
{
    u8 taskId;

    // Clear party menu callback when opening from normal Pokédex
    sExternalReturnCallback = NULL;
    
    sPokedexListItem = item;
    taskId = CreateTask(Task_LoadInfoScreen, 0);
    gTasks[taskId].tScrolling = FALSE;
    gTasks[taskId].tMonSpriteDone = TRUE; // Already has sprite from list view
    gTasks[taskId].tBgLoaded = FALSE;
    gTasks[taskId].tSkipCry = FALSE;
    gTasks[taskId].tMonSpriteId = monSpriteId;
    gTasks[taskId].tTrainerSpriteId = SPRITE_NONE;
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sInfoScreen_BgTemplate, ARRAY_COUNT(sInfoScreen_BgTemplate));
    SetBgTilemapBuffer(3, AllocZeroed(BG_SCREEN_SIZE));
    SetBgTilemapBuffer(2, AllocZeroed(BG_SCREEN_SIZE));
    SetBgTilemapBuffer(1, AllocZeroed(BG_SCREEN_SIZE));
    SetBgTilemapBuffer(0, AllocZeroed(BG_SCREEN_SIZE));
    InitWindows(sInfoScreen_WindowTemplates);
    DeactivateAllTextPrinters();

    return taskId;
}

static bool8 IsInfoScreenScrolling(u8 taskId)
{
    if (!gTasks[taskId].tScrolling && gTasks[taskId].func == Task_HandleInfoScreenInput)
        return FALSE;
    else
        return TRUE;
}

static u8 StartInfoScreenScroll(struct PokedexListItem *item, u8 taskId)
{
    // Clear party menu callback when scrolling in normal Pokédex
    sExternalReturnCallback = NULL;
    
    sPokedexListItem = item;
    gTasks[taskId].tScrolling = TRUE;
    gTasks[taskId].tMonSpriteDone = FALSE;
    gTasks[taskId].tBgLoaded = FALSE;
    gTasks[taskId].tSkipCry = FALSE;
    return taskId;
}

static void Task_LoadInfoScreen(u8 taskId)
{
    switch (gMain.state)
    {
    case 0:
    default:
        if (!gPaletteFade.active)
        {
            u16 r2;

            sPokedexView->currentPage = PAGE_INFO;
            gPokedexVBlankCB = gMain.vblankCallback;
            SetVBlankCallback(NULL);
            r2 = 0;
            if (gTasks[taskId].tMonSpriteDone)
                r2 += DISPCNT_OBJ_ON;
            if (gTasks[taskId].tBgLoaded)
                r2 |= DISPCNT_BG1_ON;
            ResetOtherVideoRegisters(r2);
            gMain.state = 1;
        }
        break;
    case 1:
        LoadTilesetTilemapHGSS(INFO_SCREEN);
        
		FillWindowPixelBuffer(WIN_INFO_HEADER, PIXEL_FILL(0));
        FillWindowPixelBuffer(WIN_INFO_HT_WT, PIXEL_FILL(0));
        FillWindowPixelBuffer(WIN_INFO_DESCRIPTION, PIXEL_FILL(0));
		
        PutWindowTilemap(WIN_INFO_HEADER);
        PutWindowTilemap(WIN_INFO_FOOTPRINT);
        PutWindowTilemap(WIN_INFO_HT_WT);
        PutWindowTilemap(WIN_INFO_DESCRIPTION);
		
        DrawFootprint(WIN_INFO_FOOTPRINT, sPokedexListItem->dexNum);
        CopyWindowToVram(WIN_INFO_FOOTPRINT, COPYWIN_GFX);
        gMain.state++;
        break;
    case 2:
        LoadScreenSelectBarMain(0xD);
        LoadPokedexBgPalette(sPokedexView->isSearchResults, FALSE);
        gMain.state++;
        break;
    case 3:
        sPokedexView->typeIconSpriteIds[0] = 0xFF;
        sPokedexView->typeIconSpriteIds[1] = 0xFF;
        CreateTypeIconSprites();
        gMain.state++;
        break;
    case 4:
        PrintMonInfo(sPokedexListItem->dexNum, sPokedexView->dexMode == DEX_MODE_HOENN ? FALSE : TRUE, sPokedexListItem->owned, 0);
		
        if (!sPokedexListItem->owned)
            LoadPalette(gPlttBufferUnfaded + 1, BG_PLTT_ID(3) + 1, PLTT_SIZEOF(16 - 1));
		
        CopyWindowToVram(WIN_INFO_HEADER, COPYWIN_FULL);
        CopyWindowToVram(WIN_INFO_HT_WT, COPYWIN_FULL);
        CopyWindowToVram(WIN_INFO_DESCRIPTION, COPYWIN_FULL);
		
        CopyBgTilemapBufferToVram(1);
        CopyBgTilemapBufferToVram(2);
        CopyBgTilemapBufferToVram(3);
        gMain.state++;
        break;
    case 5:
        if (!gTasks[taskId].tMonSpriteDone)
        {
            gTasks[taskId].tMonSpriteId = (u16)CreateMonSpriteFromNationalDexNumber(sPokedexListItem->dexNum, MON_PAGE_X, MON_PAGE_Y, 0);
            gSprites[gTasks[taskId].tMonSpriteId].oam.priority = 0;
        }
        gMain.state++;
        break;
    case 6:
        {
            u32 preservedPalettes = 0;

            if (gTasks[taskId].tBgLoaded)
                preservedPalettes = 0x14; // each bit represents a palette index
            if (gTasks[taskId].tMonSpriteDone)
                preservedPalettes |= (1 << (gSprites[gTasks[taskId].tMonSpriteId].oam.paletteNum + 16));
			
            BeginNormalPaletteFade(~preservedPalettes, 0, 16, 0, RGB_BLACK);
            SetVBlankCallback(gPokedexVBlankCB);
            gMain.state++;
        }
        break;
    case 7:
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        SetGpuReg(REG_OFFSET_BLDY, 0);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON);
        HideBg(0);
        ShowBg(1);
        ShowBg(2);
        ShowBg(3);
        gMain.state++;
        break;
    case 8:
        if (!gPaletteFade.active)
        {
            gMain.state++;
            if (!gTasks[taskId].tSkipCry)
            {
                StopCryAndClearCrySongs();
                PlayCry_NormalNoDucking(NationalPokedexNumToSpecies(sPokedexListItem->dexNum), 0, CRY_VOLUME_RS, CRY_PRIORITY_NORMAL);
            }
            else
            {
                gMain.state++;
            }
        }
        break;
    case 9:
        // if (!IsCryPlayingOrClearCrySongs())
        gMain.state++;
        break;
    case 10:
        gTasks[taskId].tScrolling = FALSE;
        gTasks[taskId].tMonSpriteDone = FALSE; // Reload next time screen comes up
        gTasks[taskId].tBgLoaded = TRUE;
        gTasks[taskId].tSkipCry = TRUE;
        gTasks[taskId].func = Task_HandleInfoScreenInput;
        gMain.state = 0;
        break;
    }
}

static void FreeInfoScreenWindowAndBgBuffers(void)
{
    void *tilemapBuffer;

    FreeAllWindowBuffers();
    tilemapBuffer = GetBgTilemapBuffer(0);
    if (tilemapBuffer)
        Free(tilemapBuffer);
    tilemapBuffer = GetBgTilemapBuffer(1);
    if (tilemapBuffer)
        Free(tilemapBuffer);
    tilemapBuffer = GetBgTilemapBuffer(2);
    if (tilemapBuffer)
        Free(tilemapBuffer);
    tilemapBuffer = GetBgTilemapBuffer(3);
    if (tilemapBuffer)
        Free(tilemapBuffer);
}

static void Task_HandleInfoScreenInput(u8 taskId)
{
    if (gTasks[taskId].tScrolling)
    {
        // Scroll up/down
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].func = Task_LoadInfoScreenWaitForFade;
        PlaySE(SE_DEX_SCROLL);
        return;
    }
    if (JOY_NEW(B_BUTTON))
    {
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        // Use special exit handler if called from party menu
        if (sExternalReturnCallback != NULL)
            gTasks[taskId].func = Task_ExitInfoScreenToExternal;
        else
            gTasks[taskId].func = Task_ExitInfoScreen;
        PlaySE(SE_PC_OFF);
        return;
    }

    if ((JOY_NEW(DPAD_RIGHT) || (JOY_NEW(R_BUTTON) && gSaveBlock2Ptr->optionsButtonMode == OPTIONS_BUTTON_MODE_LR)))
    {
        sPokedexView->selectedScreen = AREA_SCREEN;
        BeginNormalPaletteFade(0xFFFFFFEB, 0, 0, 0x10, RGB_BLACK);
        sPokedexView->screenSwitchState = 1;
        gTasks[taskId].func = Task_SwitchScreensFromInfoScreen;
        PlaySE(SE_PIN);
    }

}

static void Task_SwitchScreensFromInfoScreen(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        FreeAndDestroyMonPicSprite(gTasks[taskId].tMonSpriteId);
        switch (sPokedexView->screenSwitchState)
        {
        case 1:
        default:
            gTasks[taskId].func = Task_LoadAreaScreen;
            break;
        case 2:
            gTasks[taskId].func = Task_LoadCryScreen;
            break;
        case 3:
            gTasks[taskId].func = Task_LoadSizeScreen;
            break;
        }
    }
}

static void Task_LoadInfoScreenWaitForFade(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        FreeAndDestroyMonPicSprite(gTasks[taskId].tMonSpriteId);
        gTasks[taskId].func = Task_LoadInfoScreen;
    }
}

static void Task_ExitInfoScreen(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        FreeAndDestroyMonPicSprite(gTasks[taskId].tMonSpriteId);
        FreeInfoScreenWindowAndBgBuffers();
        DestroyTask(taskId);
    }
}

static void Task_ExitInfoScreenToExternal(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        FreeAndDestroyMonPicSprite(gTasks[taskId].tMonSpriteId);
        FreeInfoScreenWindowAndBgBuffers();
        DestroyTask(taskId);
        
        if (sExternalReturnCallback != NULL)
        {
            void (*callback)(void) = sExternalReturnCallback;
            sExternalReturnCallback = NULL;
            SetMainCallback2(callback);
        }
    }
}

#undef tMonSpriteId

static void CB2_PartyPokedexInfoScreen(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    DoScheduledBgTilemapCopiesToVram();
    UpdatePaletteFade();
}

// Called from party menu to open Pokedex detail view for a specific Pokemon
void OpenPokedexInfoScreen(u16 species, void (*returnCallback)(void))
{
    u8 taskId;
    u16 dexNum;
    static struct PokedexListItem partyMonItem;
    
    // Temporarily define task data accessors for this function
    #define tScrolling       data[0]
    #define tMonSpriteDone   data[1]
    #define tBgLoaded        data[2]
    #define tSkipCry         data[3]
    #define tMonSpriteId     data[4]
    #define tTrainerSpriteId data[5]
    
    // Store the return callback
    sExternalReturnCallback = returnCallback;
    
    // Convert species to dex number
    dexNum = SpeciesToNationalPokedexNum(species);
    
    // Set up the list item for this Pokemon
    partyMonItem.dexNum = dexNum;
    partyMonItem.seen = TRUE;
    partyMonItem.owned = GetSetPokedexFlag(dexNum, FLAG_GET_CAUGHT);
    
    // Point to our party mon item
    sPokedexListItem = &partyMonItem;
    
    // Create the task and load the info screen
    /* Note from Skeletonkey36: 
    - You can change the Task_LoadInfoScreen to a different Task_LoadXXXScreen if you want to 
        start on a different screen
    - Reminder if you change Task_LoadInfoScreen, be sure to update name of OpenPokedexInfoScreen() 
        above as well just for code clarity (and the call in party_menu.c)
    */
    taskId = CreateTask(Task_LoadInfoScreen, 0);
    gTasks[taskId].tScrolling = FALSE;
    gTasks[taskId].tMonSpriteDone = FALSE;
    gTasks[taskId].tBgLoaded = FALSE;
    gTasks[taskId].tSkipCry = FALSE;
    gTasks[taskId].tMonSpriteId = SPRITE_NONE;
    gTasks[taskId].tTrainerSpriteId = SPRITE_NONE;
    
    // Clean up local macros
    #undef tScrolling
    #undef tMonSpriteDone
    #undef tBgLoaded
    #undef tSkipCry
    #undef tMonSpriteId
    #undef tTrainerSpriteId
    
    // Initialize backgrounds
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sInfoScreen_BgTemplate, ARRAY_COUNT(sInfoScreen_BgTemplate));
    SetBgTilemapBuffer(3, AllocZeroed(BG_SCREEN_SIZE));
    SetBgTilemapBuffer(2, AllocZeroed(BG_SCREEN_SIZE));
    SetBgTilemapBuffer(1, AllocZeroed(BG_SCREEN_SIZE));
    SetBgTilemapBuffer(0, AllocZeroed(BG_SCREEN_SIZE));
    InitWindows(sInfoScreen_WindowTemplates);
    DeactivateAllTextPrinters();
    
    // Set main state to 0 to start the loading process
    gMain.state = 0;
    
    // Allocate PokedexView if needed (for screen switching)
    if (sPokedexView == NULL)
        sPokedexView = AllocZeroed(sizeof(struct PokedexView));
    
    sPokedexView->currentPage = PAGE_INFO;
    sPokedexView->selectedScreen = INFO_SCREEN;
    
    SetMainCallback2(CB2_PartyPokedexInfoScreen);
}

//************************************
//*                                  *
//*        Area screen               *
//*                                  *
//************************************
static void Task_LoadAreaScreen(u8 taskId)
{
    switch (gMain.state)
    {
    case 0:
    default:
        if (!gPaletteFade.active)
        {
            sPokedexView->currentPage = PAGE_AREA;
            gPokedexVBlankCB = gMain.vblankCallback;
            SetVBlankCallback(NULL);
            ResetOtherVideoRegisters(DISPCNT_BG1_ON);
            sPokedexView->selectedScreen = AREA_SCREEN;
            gMain.state = 1;
        }
        break;
    case 1:
        LoadPokedexBgPalette(sPokedexView->isSearchResults, false);
        SetGpuReg(REG_OFFSET_BG1CNT, BGCNT_PRIORITY(0) | BGCNT_CHARBASE(0) | BGCNT_SCREENBASE(13) | BGCNT_16COLOR | BGCNT_TXT256x256);
        gMain.state++;
        break;
    case 2:
        ShowPokedexAreaScreen(NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum), &sPokedexView->screenSwitchState);
        SetVBlankCallback(gPokedexVBlankCB);
        sPokedexView->screenSwitchState = 0;
        gMain.state = 0;
        gTasks[taskId].func = Task_WaitForAreaScreenInput;
        break;
    }
}

static void Task_WaitForAreaScreenInput(u8 taskId)
{
// See Task_HandlePokedexAreaScreenInput() in pokedex_area_screen.c
    if (sPokedexView->screenSwitchState != 0)
        gTasks[taskId].func = Task_SwitchScreensFromAreaScreen;
}

static void Task_SwitchScreensFromAreaScreen(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        switch (sPokedexView->screenSwitchState)
        {
        case 1:
        default:
            gTasks[taskId].func = Task_LoadInfoScreen;
            break;
        case 2:
            if (!sPokedexListItem->owned)
                PlaySE(SE_FAILURE);
            else
                gTasks[taskId].func = Task_LoadStatsScreen;
            break;
        case 3:
            // Use special exit handler if called from party menu
            if (sExternalReturnCallback != NULL)
                gTasks[taskId].func = Task_ExitAreaScreenToExternal;
            else
                gTasks[taskId].func = Task_ExitAreaScreen;
            break;
        }
    }
}

static void Task_ExitAreaScreen(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        FreeInfoScreenWindowAndBgBuffers();
        DestroyTask(taskId);
    }
}

static void Task_ExitAreaScreenToExternal(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        FreeInfoScreenWindowAndBgBuffers();
        DestroyTask(taskId);
        if (sExternalReturnCallback != NULL)
            SetMainCallback2(sExternalReturnCallback);
    }
}



//************************************
//*                                  *
//*        Select bar                *
//*                                  *
//************************************
static void LoadScreenSelectBarMain(u16 unused)
{
    CopyToBgTilemapBuffer(1, gPokedexPlusHGSS_ScreenSelectBarSubmenu_Tilemap_Clear, 0, 0);
    CopyBgTilemapBufferToVram(1);
}

static void HighlightScreenSelectBarItem(u8 selectedScreen, u16 unused) //HGSS_Ui
{
    u8 i;
    u8 j;
    u16 *ptr = GetBgTilemapBuffer(1);

    for (i = 0; i < SCREEN_COUNT; i++)
    {
        u8 row = (i * 7) + 1;
        u16 newPalette;

        do
        {
            newPalette = 0x4000;
            if (i == selectedScreen)
                newPalette = 0x2000;
        } while (0);

        for (j = 0; j < 7; j++)
        {
            ptr[row + j] = (ptr[row + j] % 0x1000) | newPalette;
            ptr[row + j + 0x20] = (ptr[row + j + 0x20] % 0x1000) | newPalette;
        }
    }
    CopyBgTilemapBufferToVram(1);
}

static void HighlightSubmenuScreenSelectBarItem(u8 a, u16 b)
{
    u8 i;
    u8 j;
    u16 *ptr = GetBgTilemapBuffer(1);

    for (i = 0; i < 4; i++)
    {
        u8 row = i * 7 + 1;
        u32 newPalette;

        do
        {
            if (i == a || i == 3)
                newPalette = 0x2000;
            else
                newPalette = 0x4000;
        } while (0);

        for (j = 0; j < 7; j++)
        {
            ptr[row + j] = (ptr[row + j] % 0x1000) | newPalette;
            ptr[row + j + 0x20] = (ptr[row + j + 0x20] % 0x1000) | newPalette;
        }
    }
    CopyBgTilemapBufferToVram(1);
}



//************************************
//*                                  *
//*        CAUGHT MON                *
//*                                  *
//************************************
#define tState         data[0]
#define tSpecies       data[1]
#define tPalTimer      data[2]
#define tMonSpriteId   data[3]
#define tOtIdLo        data[12]
#define tOtIdHi        data[13]
#define tPersonalityLo data[14]
#define tPersonalityHi data[15]

void Task_DisplayCaughtMonDexPageHGSS(u8 taskId)
{
    u8 spriteId;
    u16 species = gTasks[taskId].tSpecies;
    u16 dexNum = SpeciesToNationalPokedexNum(species);

    switch (gTasks[taskId].tState)
    {
    case 0:
    default:
        if (!gPaletteFade.active)
        {
            gPokedexVBlankCB = gMain.vblankCallback;
            SetVBlankCallback(NULL);
            ResetOtherVideoRegisters(DISPCNT_BG0_ON);
            ResetBgsAndClearDma3BusyFlags(0);
            InitBgsFromTemplates(0, sNewEntryInfoScreen_BgTemplate, ARRAY_COUNT(sNewEntryInfoScreen_BgTemplate));
            SetBgTilemapBuffer(3, AllocZeroed(BG_SCREEN_SIZE));
            SetBgTilemapBuffer(2, AllocZeroed(BG_SCREEN_SIZE));
            InitWindows(sNewEntryInfoScreen_WindowTemplates);
            DeactivateAllTextPrinters();
            gTasks[taskId].tState = 1;
        }
        break;
    case 1:
        sPokedexView = AllocZeroed(sizeof(struct PokedexView)); //for type icons
        ResetPokedexView(sPokedexView);

        LoadTilesetTilemapHGSS(INFO_SCREEN);
        FillWindowPixelBuffer(WIN_INFO_HEADER, PIXEL_FILL(0));
        PutWindowTilemap(WIN_INFO_HEADER);
        PutWindowTilemap(WIN_INFO_FOOTPRINT);
        DrawFootprint(WIN_INFO_FOOTPRINT, dexNum);
        CopyWindowToVram(WIN_INFO_FOOTPRINT, COPYWIN_GFX);
        ResetPaletteFade();
        LoadPokedexBgPalette(FALSE, false);
        gTasks[taskId].tState++;
        break;
    case 2:
        sPokedexView->typeIconSpriteIds[0] = 0xFF;
        sPokedexView->typeIconSpriteIds[1] = 0xFF;
        CreateTypeIconSprites();
        gTasks[taskId].tState++;
        break;
    case 3:
        PrintMonInfo(dexNum, IsNationalPokedexEnabled(), 1, 1);
        CopyWindowToVram(WIN_INFO_HEADER, COPYWIN_FULL);
        CopyBgTilemapBufferToVram(2);
        CopyBgTilemapBufferToVram(3);
        gTasks[taskId].tState++;
        break;
    case 4:
        spriteId = CreateMonSpriteFromNationalDexNumber(dexNum, MON_PAGE_X, MON_PAGE_Y, 0);
        gSprites[spriteId].oam.priority = 0;
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0x10, 0, RGB_BLACK);
        SetVBlankCallback(gPokedexVBlankCB);
        gTasks[taskId].tMonSpriteId = spriteId;
        gTasks[taskId].tState++;
        break;
    case 5:
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        SetGpuReg(REG_OFFSET_BLDY, 0);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON);
        ShowBg(2);
        ShowBg(3);
        gTasks[taskId].tState++;
        break;
    case 6:
        if (!gPaletteFade.active)
        {
            PlayCry_Normal(NationalPokedexNumToSpeciesHGSS(dexNum), 0);
            gTasks[taskId].tPalTimer = 0;
            gTasks[taskId].func = Task_HandleCaughtMonPageInput;
        }
        break;
    }
}

static void Task_HandleCaughtMonPageInput(u8 taskId)
{
    if (JOY_NEW(A_BUTTON | B_BUTTON))
    {
        BeginNormalPaletteFade(PALETTES_BG, 0, 0, 16, RGB_BLACK);
        SetSpriteInvisibility(0, TRUE);
        SetSpriteInvisibility(1, TRUE);
        gSprites[gTasks[taskId].tMonSpriteId].callback = SpriteCB_SlideCaughtMonToCenter;
        gTasks[taskId].func = Task_ExitCaughtMonPage;
    }
    // Flicker caught screen color
    else if (++gTasks[taskId].tPalTimer & 16)
    {
		LoadPalette(gPokedexPlusHGSS_Default_Pal + 1, BG_PLTT_ID(3) + 1, PLTT_SIZEOF(7));
    }
    else
    {
		LoadPalette(gPokedexPlusHGSS_Default_Pal + 1, BG_PLTT_ID(3) + 1, PLTT_SIZEOF(7));
    }
}

static void Task_ExitCaughtMonPage(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        u16 species;
        u32 otId;
        u32 personality;
        u8 paletteNum;
        const u32 *lzPaletteData;
        void *buffer;

        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON);
        FreeAllWindowBuffers();
        buffer = GetBgTilemapBuffer(2);
        if (buffer)
            Free(buffer);
        buffer = GetBgTilemapBuffer(3);
        if (buffer)
            Free(buffer);

        species = gTasks[taskId].tSpecies;
        otId = ((u16)gTasks[taskId].tOtIdHi << 16) | (u16)gTasks[taskId].tOtIdLo;
        personality = ((u16)gTasks[taskId].tPersonalityHi << 16) | (u16)gTasks[taskId].tPersonalityLo;
        paletteNum = gSprites[gTasks[taskId].tMonSpriteId].oam.paletteNum;
        lzPaletteData = GetMonSpritePalFromSpeciesAndPersonality(species, otId, personality);
        LoadCompressedPalette(lzPaletteData, OBJ_PLTT_ID(paletteNum), PLTT_SIZE_4BPP);
        DestroyTask(taskId);
    }
}

static void SpriteCB_SlideCaughtMonToCenter(struct Sprite *sprite)
{
    if (sprite->x < 0x78)
        sprite->x += 2;
    if (sprite->x > 0x78)
        sprite->x -= 2;

    if (sprite->y < 0x50)
        sprite->y += 1;
    if (sprite->y > 0x50)
        sprite->y -= 1;
}

#undef tState
#undef tDexNum
#undef tPalTimer
#undef tMonSpriteId
#undef tOtIdLo
#undef tOtIdHi
#undef tPersonalityLo
#undef tPersonalityHi







//************************************
//*                                  *
//*        Print data                *
//*                                  *
//************************************
static void PrintInfoScreenText(u8 windowId, const u8 *str, u8 left, u8 top, u8 colorId)
{
	AddTextPrinterParameterized4(windowId, FONT_BW_SUMMARY_SCREEN, left, top, 0, 0, sTextColors[colorId], -1, str); //window id?, font, x, y, unk, line spacing, color, unk, string
}
static void PrintInfoScreenTextWhite(u8 windowId, const u8* str, u8 left, u8 top, u8 colorId) //WorpToDo: Possibly delete lol
{
    AddTextPrinterParameterized4(windowId, FONT_BW_SUMMARY_SCREEN, left, top, 0, 0, sTextColors[colorId], TEXT_SKIP_DRAW, str);
}
static void PrintInfoScreenTextSmall(const u8* str, u8 left, u8 top)
{
    u8 color[3];
    color[0] = TEXT_COLOR_TRANSPARENT;
    color[1] = TEXT_DYNAMIC_COLOR_6;
    color[2] = TEXT_COLOR_LIGHT_GRAY;

    AddTextPrinterParameterized4(0, FONT_BW_SUMMARY_SCREEN, left, top, 0, 0, color, 0, str);
}
//Stats screen
static void nPrintStatsScreenText(u8 windowId, const u8 *str, u8 left, u8 top, u8 colorId)
{
	AddTextPrinterParameterized4(windowId, FONT_BW_SUMMARY_SCREEN, left, top, 0, 0, sTextColors[colorId], -1, str);
}
static void nPrintStatsScreenTextNarrow(u8 windowId, const u8 *str, u8 left, u8 top, u8 colorId)
{
	AddTextPrinterParameterized4(windowId, FONT_NARROW, left, top, 0, 0, sTextColors[colorId], -1, str);
}
static void PrintStatsScreenTextSmall(u8 windowId, const u8* str, u8 left, u8 top)
{
    u8 color[3];
    color[0] = TEXT_COLOR_TRANSPARENT;
    color[1] = TEXT_DYNAMIC_COLOR_6;
    color[2] = TEXT_COLOR_LIGHT_GRAY;

    AddTextPrinterParameterized4(windowId, FONT_BW_SUMMARY_SCREEN, left, top, 0, 0, color, 0, str);
}
static void PrintStatsScreenTextSmallWhite(u8 windowId, const u8* str, u8 left, u8 top)
{
    u8 color[3];
    color[0] = TEXT_COLOR_TRANSPARENT;
    color[1] = TEXT_COLOR_WHITE;
    color[2] = TEXT_DYNAMIC_COLOR_6;

    AddTextPrinterParameterized4(windowId, FONT_BW_SUMMARY_SCREEN, left, top, 0, 0, color, 0, str);
}

//Type Icon
static void SetSpriteInvisibility(u8 spriteArrayId, bool8 invisible)
{
    gSprites[sPokedexView->typeIconSpriteIds[spriteArrayId]].invisible = invisible;
}
// different from pokemon_summary_screen
static void SetTypeIconPosAndPal(u8 typeId, u8 x, u8 y, u8 spriteArrayId)
{
    u8 spriteId = sPokedexView->typeIconSpriteIds[spriteArrayId];
    
    if (spriteId == 0xFF)
        return;

    gSprites[spriteId].oam.paletteNum = 13;
    gSprites[spriteId].x = x + 16;
    gSprites[spriteId].y = y + 8;
    StartSpriteAnim(&gSprites[spriteId], typeId);
    SetSpriteInvisibility(spriteArrayId, FALSE);
}
static void PrintCurrentSpeciesTypeInfo(u8 newEntry, u16 species)
{
    u32 i;
    u16 dexNum = SpeciesToNationalPokedexNum(species);
    u8 type1, type2;

    if (!newEntry)
    {
        species = NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum);
        dexNum = SpeciesToNationalPokedexNum(species);
    }
    //type icon(s)
    #ifdef TX_RANDOMIZER_AND_CHALLENGES
        type1 = GetTypeBySpecies(species, 1);
        type2 = GetTypeBySpecies(species, 2);
    #else
        type1 = gSpeciesInfo[species].types[0];
        type2 = gSpeciesInfo[species].types[1];
    #endif
    if (species == SPECIES_NONE)
        type1 = type2 = TYPE_MYSTERY;

    if (type1 == type2)
    {
        SetTypeIconPosAndPal(type1, 103, 58, 0);
        SetSpriteInvisibility(1, TRUE);
    }
    else
    {
        SetTypeIconPosAndPal(type1, 103, 58, 0);
        SetTypeIconPosAndPal(type2, 103, 70, 1);
    }

}
static void CreateTypeIconSprites(void)
{
    u8 i;

    LoadCompressedSpriteSheet(&sSpriteSheet_MoveTypes);
    LoadCompressedPalette(gInterfaceTypeIcons_Pal, 0x1D0, 0x20);
    for (i = 0; i < 2; i++)
    {
        if (sPokedexView->typeIconSpriteIds[i] == 0xFF)
            sPokedexView->typeIconSpriteIds[i] = CreateSprite(&sSpriteTemplate_MoveTypes, 10, 10, 2);

        SetSpriteInvisibility(i, TRUE);
    }
}

// u32 value is re-used, but passed as a bool that's TRUE if national dex is enabled
static void PrintMonInfo(u32 num, u32 value, u32 owned, u32 newEntry)
{
    u8 str[16];
    u8 str2[32];
    u16 species;
    const u8 *name;
    const u8 *category;
    const u8 *description;

    if (value == 0)
        value = NationalToHoennOrder(num);
    else
        value = num;
    ConvertIntToDecimalStringN(StringCopy(str, gText_NumberClear01), value, STR_CONV_MODE_LEADING_ZEROS, 3);
    PrintInfoScreenTextWhite(WIN_INFO_HEADER, str, 3, 4, 1);
    species = NationalPokedexNumToSpeciesHGSS(num);
    if (species)
        name = gSpeciesNames[species];
    else
        name = sText_TenDashes2;
    PrintInfoScreenTextWhite(WIN_INFO_HEADER, name, 3 + 34, 4, 1);
    if (owned)
    {
        CopyMonCategoryText(num, str2);
        category = str2;
    }
    else
    {
        category = gText_5MarksPokemon;
    }
    PrintInfoScreenText(WIN_INFO_HEADER, category, 3, 18, 0);
    if (owned)
    {
        PrintMonHeight(gPokedexEntries[num].height, 23, 2); //HGSS_Ui
        PrintMonWeight(gPokedexEntries[num].weight, 12, 15); //HGSS_Ui
    }
    else
    {
        PrintInfoScreenText(WIN_INFO_HT_WT, gText_UnkHeight, 23, 2, 0); //HGSS_Ui
        PrintInfoScreenText(WIN_INFO_HT_WT, gText_UnkWeight, 12, 15, 0); //HGSS_Ui
    }
    if (owned)
    {
        description = gPokedexEntries[num].description;
    }
    else
        description = sExpandedPlaceholder_PokedexDescription;
    PrintInfoScreenText(WIN_INFO_DESCRIPTION, description, GetStringCenterAlignXOffset(FONT_NORMAL, description, 0) + 2, 6, 0);

    //Type Icon(s) //HGSS_Ui
    if (owned)
        PrintCurrentSpeciesTypeInfo(newEntry, species); //HGSS_Ui
}

static void PrintMonHeight(u16 height, u8 left, u8 top)
{
    u8 buffer[16];
    u32 inches, feet;
    u8 i = 0;
    int offset;
    u8 result;
    offset = 0;

    if (gSaveBlock2Ptr->optionsUnitSystem == 1) //Imperial
    {
        inches = (height * 10000) / 254;
        if (inches % 10 >= 5)
            inches += 10;
        feet = inches / 120;
        inches = (inches - (feet * 120)) / 10;

        buffer[i++] = EXT_CTRL_CODE_BEGIN;
        buffer[i++] = EXT_CTRL_CODE_CLEAR_TO;
        if (feet / 10 == 0)
        {
            buffer[i++] = 18;
            buffer[i++] = feet + CHAR_0;
        }
        else
        {
            buffer[i++] = 12;
            buffer[i++] = feet / 10 + CHAR_0;
            buffer[i++] = (feet % 10) + CHAR_0;
        }
        buffer[i++] = CHAR_SGL_QUOTE_RIGHT;
        buffer[i++] = (inches / 10) + CHAR_0;
        buffer[i++] = (inches % 10) + CHAR_0;
        buffer[i++] = CHAR_DBL_QUOTE_RIGHT;
        buffer[i++] = EOS;
        PrintInfoScreenText(WIN_INFO_HT_WT, buffer, left, top, 0);
    }
    else //Metric
    {
        buffer[i++] = EXT_CTRL_CODE_BEGIN;
        buffer[i++] = EXT_CTRL_CODE_CLEAR_TO;
        i++;
        buffer[i++] = CHAR_SPACE;
        buffer[i++] = CHAR_SPACE;
        buffer[i++] = CHAR_SPACE;
        buffer[i++] = CHAR_SPACE;
        buffer[i++] = CHAR_SPACE;

        result = (height / 1000);
        if (result == 0)
        {
            offset = 6;
        }
        else
        {
            buffer[i++] = result + CHAR_0;
        }

        result = (height % 1000) / 100;
        if (result == 0 && offset != 0)
        {
            offset += 6;
        }
        else
        {
            buffer[i++] = result + CHAR_0;
        }

        buffer[i++] = (((height % 1000) % 100) / 10) + CHAR_0;
        buffer[i++] = CHAR_COMMA;
        buffer[i++] = (((height % 1000) % 100) % 10) + CHAR_0;
        buffer[i++] = CHAR_SPACE;
        buffer[i++] = CHAR_m;

        buffer[i++] = EOS;
        buffer[2] = offset;
        PrintInfoScreenText(WIN_INFO_HT_WT, buffer, left, top, 0);   
    }
}

static void PrintMonWeight(u16 weight, u8 left, u8 top)
{
    u8 buffer[16];
    u8 buffer_metric[18];
    bool8 output;
    u8 i = 0;
    u32 lbs = (weight * 100000) / 4536;
    int offset = 0;
    u8 result;

    if (gSaveBlock2Ptr->optionsUnitSystem == 1) //Imperial
    {
        if (lbs % 10u >= 5)
            lbs += 10;
        i = 0;
        output = FALSE;

        if ((buffer[i] = (lbs / 100000) + CHAR_0) == CHAR_0 && !output)
        {
            buffer[i++] = CHAR_SPACER;
        }
        else
        {
            output = TRUE;
            i++;
        }

        lbs %= 100000;
        if ((buffer[i] = (lbs / 10000) + CHAR_0) == CHAR_0 && !output)
        {
            buffer[i++] = CHAR_SPACER;
        }
        else
        {
            output = TRUE;
            i++;
        }

        lbs %= 10000;
        if ((buffer[i] = (lbs / 1000) + CHAR_0) == CHAR_0 && !output)
        {
            buffer[i++] = CHAR_SPACER;
        }
        else
        {
            output = TRUE;
            i++;
        }

        lbs %= 1000;
        buffer[i++] = (lbs / 100) + CHAR_0;
        lbs %= 100;
        buffer[i++] = CHAR_PERIOD;
        buffer[i++] = (lbs / 10) + CHAR_0;
        buffer[i++] = CHAR_SPACE;
        buffer[i++] = CHAR_l;
        buffer[i++] = CHAR_b;
        buffer[i++] = CHAR_s;
        buffer[i++] = CHAR_PERIOD;
        buffer[i++] = EOS;
        PrintInfoScreenText(WIN_INFO_HT_WT, buffer, left, top, 0);
    }
    else //Metric
    {
        buffer_metric[i++] = EXT_CTRL_CODE_BEGIN;
        buffer_metric[i++] = EXT_CTRL_CODE_CLEAR_TO;
        i++;
        buffer_metric[i++] = CHAR_SPACE;
        buffer_metric[i++] = CHAR_SPACE;
        buffer_metric[i++] = CHAR_SPACE;
        buffer_metric[i++] = CHAR_SPACE;
        buffer_metric[i++] = CHAR_SPACE;

        result = (weight / 1000);
        if (result == 0)
            offset = 6;
        else
            buffer_metric[i++] = result + CHAR_0;

        result = (weight % 1000) / 100;
        if (result == 0 && offset != 0)
            offset += 6;
        else
            buffer_metric[i++] = result + CHAR_0;

        buffer_metric[i++] = (((weight % 1000) % 100) / 10) + CHAR_0;
        buffer_metric[i++] = CHAR_COMMA;
        buffer_metric[i++] = (((weight % 1000) % 100) % 10) + CHAR_0;
        buffer_metric[i++] = CHAR_SPACE;
        buffer_metric[i++] = CHAR_k;
        buffer_metric[i++] = CHAR_g;

        buffer_metric[i++] = EOS;
        buffer_metric[2] = offset;
        PrintInfoScreenText(WIN_INFO_HT_WT, buffer_metric, left, top, 0);
    }
}

// Unused in the English version, used to print height/weight in versions which use metric system.
static void PrintDecimalNum(u8 windowId, u16 num, u8 left, u8 top)
{
    u8 str[6];
    bool8 outputted = FALSE;
    u8 result;

    result = num / 1000;
    if (result == 0)
    {
        str[0] = CHAR_SPACER;
        outputted = FALSE;
    }
    else
    {
        str[0] = CHAR_0 + result;
        outputted = TRUE;
    }

    result = (num % 1000) / 100;
    if (result == 0 && !outputted)
    {
        str[1] = CHAR_SPACER;
        outputted = FALSE;
    }
    else
    {
        str[1] = CHAR_0 + result;
        outputted = TRUE;
    }

    str[2] = CHAR_0 + ((num % 1000) % 100) / 10;
    str[3] = CHAR_DEC_SEPARATOR;
    str[4] = CHAR_0 + ((num % 1000) % 100) % 10;
    str[5] = EOS;
    PrintInfoSubMenuText(windowId, str, left, top);
}

static void DrawFootprint(u8 windowId, u16 dexNum)
{
    const u8 *footprintGfx = gMonFootprintTable[NationalPokedexNumToSpecies(dexNum)];
    u8 footprint[128];
    u16 i, j;
    u16 tileIdx = 0;

    for (i = 0; i < 32; i++)
    {
        u8 tile = footprintGfx[i];
        for (j = 0; j < 4; j++)
        {
            u8 value = ((tile >> (2 * j)) & 1 ? 2 : 0);
            if (tile & (2 << (2 * j)))
                value |= 0x20;
            
            footprint[tileIdx++] = value;
        }
    }

    struct WindowTemplate canvasTemp = {
        .bg = 0, .tilemapLeft = 0, .tilemapTop = 0,
        .width = 2, .height = 2, .paletteNum = 0, .baseBlock = 0
    };
    u8 canvasId = AddWindow(&canvasTemp);

    FillWindowPixelBuffer(canvasId, PIXEL_FILL(0));
    CopyToWindowPixelBuffer(canvasId, &footprint[0],  32, 0); 
    CopyToWindowPixelBuffer(canvasId, &footprint[32], 32, 1); 
    CopyToWindowPixelBuffer(canvasId, &footprint[64], 32, 2); 
    CopyToWindowPixelBuffer(canvasId, &footprint[96], 32, 3); 

    FillWindowPixelBuffer(windowId, PIXEL_FILL(0));
    
    BlitBitmapToWindow(windowId, (const u8 *)GetWindowAttribute(canvasId, WINDOW_TILE_DATA), 4, 10, 16, 16);
	//Actual black fucking magic this took me so long to figure out and I STILL DON'T UNDERSTAND. But this assembles the canvas all into a single image since using blits on their own was causing the footpritns to look like QR codes.

    RemoveWindow(canvasId);
}

static void PrintInfoSubMenuText(u8 windowId, const u8 *str, u8 left, u8 top)
{
    u8 color[3];
    color[0] = TEXT_COLOR_TRANSPARENT;
    color[1] = TEXT_DYNAMIC_COLOR_6;
    color[2] = TEXT_COLOR_LIGHT_GRAY;

    AddTextPrinterParameterized4(windowId, FONT_NORMAL, left, top, 0, 0, color, TEXT_SKIP_DRAW, str);
}

static u8 PrintCryScreenSpeciesName(u8 windowId, u16 num, u8 left, u8 top)
{
    u8 str[POKEMON_NAME_LENGTH + 1];
    u8 i;

    for (i = 0; i < ARRAY_COUNT(str); i++)
        str[i] = EOS;
    num = NationalPokedexNumToSpeciesHGSS(num);
    switch (num)
    {
    default:
        for (i = 0; gSpeciesNames[num][i] != EOS && i < POKEMON_NAME_LENGTH; i++)
            str[i] = gSpeciesNames[num][i];
        break;
    case 0:
        for (i = 0; i < 5; i++)
            str[i] = CHAR_HYPHEN;
        break;
    }
    PrintInfoSubMenuText(windowId, str, left, top);
    return i;
}

static u16 CreateSizeScreenTrainerPic(u16 species, s16 x, s16 y, s8 paletteSlot)
{
    return CreateTrainerPicSprite(species, TRUE, x, y, paletteSlot, TAG_NONE);
}


//************************************
//*                                  *
//*        Helper functions          *
//*                                  *
//************************************

static void ResetOtherVideoRegisters(u16 regBits)
{
    if (!(regBits & DISPCNT_BG0_ON))
    {
        ClearGpuRegBits(0, DISPCNT_BG0_ON);
        SetGpuReg(REG_OFFSET_BG0CNT, 0);
        SetGpuReg(REG_OFFSET_BG0HOFS, 0);
        SetGpuReg(REG_OFFSET_BG0VOFS, 0);
    }
    if (!(regBits & DISPCNT_BG1_ON))
    {
        ClearGpuRegBits(0, DISPCNT_BG1_ON);
        SetGpuReg(REG_OFFSET_BG1CNT, 0);
        SetGpuReg(REG_OFFSET_BG1HOFS, 0);
        SetGpuReg(REG_OFFSET_BG1VOFS, 0);
    }
    if (!(regBits & DISPCNT_BG2_ON))
    {
        ClearGpuRegBits(0, DISPCNT_BG2_ON);
        SetGpuReg(REG_OFFSET_BG2CNT, 0);
        SetGpuReg(REG_OFFSET_BG2HOFS, 0);
        SetGpuReg(REG_OFFSET_BG2VOFS, 0);
    }
    if (!(regBits & DISPCNT_BG3_ON))
    {
        ClearGpuRegBits(0, DISPCNT_BG3_ON);
        SetGpuReg(REG_OFFSET_BG3CNT, 0);
        SetGpuReg(REG_OFFSET_BG3HOFS, 0);
        SetGpuReg(REG_OFFSET_BG3VOFS, 0);
    }
    if (!(regBits & DISPCNT_OBJ_ON))
    {
        ClearGpuRegBits(0, DISPCNT_OBJ_ON);
        ResetSpriteData();
        FreeAllSpritePalettes();
        gReservedSpritePaletteCount = 8;
    }
}

static void FreeWindowAndBgBuffers(void)
{
    void *tilemapBuffer;

    FreeAllWindowBuffers();
    tilemapBuffer = GetBgTilemapBuffer(0);
    if (tilemapBuffer)
        Free(tilemapBuffer);
    tilemapBuffer = GetBgTilemapBuffer(1);
    if (tilemapBuffer)
        Free(tilemapBuffer);
    tilemapBuffer = GetBgTilemapBuffer(2);
    if (tilemapBuffer)
        Free(tilemapBuffer);
    tilemapBuffer = GetBgTilemapBuffer(3);
    if (tilemapBuffer)
        Free(tilemapBuffer);
}

static u16 GetNextPosition(u8 direction, u16 position, u16 min, u16 max)
{
    switch (direction)
    {
    case 1: // Up/Left
        if (position > min)
            position--;
        break;
    case 0: // Down/Right
        if (position < max)
            position++;
        break;
    case 3: // Up/Left with loop (unused)
        if (position > min)
            position--;
        else
            position = max;
        break;
    case 2: // Down/Right with loop (unused)
        if (position < max)
            position++;
        else
            position = min;
        break;
    }
    return position;
}

// Unown and Spinda use the personality of the first seen individual of that species
// All others use personality 0
static u32 GetPokedexMonPersonality(u16 species)
{
    if (species == SPECIES_UNOWN || species == SPECIES_SPINDA)
    {
        if (species == SPECIES_UNOWN)
            return gSaveBlock2Ptr->pokedex.unownPersonality;
        else
            return gSaveBlock2Ptr->pokedex.spindaPersonality;
    }
    else
    {
        return 0;
    }
}



//************************************
//*                                  *
//*        HGSS                      *
//*                                  *
//************************************
u16 NationalPokedexNumToSpeciesHGSS(u16 nationalNum)
{
    u16 species;

    if (!nationalNum)
        return 0;

    #ifndef POKEMON_EXPANSION
        return NationalPokedexNumToSpecies(nationalNum);
    #else
        if (sPokedexView->formSpecies != 0)
            return sPokedexView->formSpecies;
        else
            return NationalPokedexNumToSpecies(nationalNum);
    #endif
}

static void LoadTilesetTilemapHGSS(u8 page)
{
    switch (page)
    {
    case INFO_SCREEN:
	DecompressAndLoadBgGfxUsingHeap(3, gPokedexPlusHGSS_Menu_Gfx, 0x2000, 0, 0);
        CopyToBgTilemapBuffer(3, gPokedexPlusHGSS_ScreenInfo_Tilemap, 0, 0);
        break;
    case STATS_SCREEN:
		DecompressAndLoadBgGfxUsingHeap(3, gPokedexPlusHGSS_Menu_Gfx, 0x2000, 0, 0);
        CopyToBgTilemapBuffer(3, gPokedexPlusHGSS_ScreenStats_Tilemap, 0, 0);
        break;
    case EVO_SCREEN:
		DecompressAndLoadBgGfxUsingHeap(3, gPokedexPlusHGSS_Menu_Gfx, 0x2000, 0, 0);
		CopyToBgTilemapBuffer(3, gPokedexPlusHGSS_ScreenEvolution_Tilemap, 0, 0);
        break;
    case CRY_SCREEN:
		DecompressAndLoadBgGfxUsingHeap(3, gPokedexPlusHGSS_Menu_Gfx, 0x2000, 0, 0);
        CopyToBgTilemapBuffer(3, gPokedexPlusHGSS_ScreenCry_Tilemap, 0, 0);
        break;
    case SIZE_SCREEN:
		DecompressAndLoadBgGfxUsingHeap(3, gPokedexPlusHGSS_Menu_Gfx, 0x2000, 0, 0);
        CopyToBgTilemapBuffer(3, gPokedexPlusHGSS_ScreenSize_Tilemap, 0, 0);
        break;
    }
}

//Physical/Special Split from BE

static u8 ShowSplitIcon(u32 split)
{
    if (sPokedexView->splitIconSpriteId == 0xFF)
        sPokedexView->splitIconSpriteId = CreateSprite(&sSpriteTemplate_SplitIcons, 46, 107, 0);

    gSprites[sPokedexView->splitIconSpriteId].invisible = FALSE;
    StartSpriteAnim(&gSprites[sPokedexView->splitIconSpriteId], split);
    return sPokedexView->splitIconSpriteId;
}
static void DestroySplitIcon(void)
{
    if (sPokedexView->splitIconSpriteId != 0xFF)
        DestroySprite(&gSprites[sPokedexView->splitIconSpriteId]);
    sPokedexView->splitIconSpriteId = 0xFF;
}

//************************************
//*                                  *
//*        STATS                     *
//*                                  *
//************************************
static const u8 sStatsPageNavigationTextColor[] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY};

static void StatsPage_PrintNavigationButtons(void)
{
    u8 x = 9;
    u8 y = 0;
	AddTextPrinterParameterized3(WIN_STATS_NAVIGATION, 0, x, y, sStatsPageNavigationTextColor, 0, gText_Stats_Buttons);

    PutWindowTilemap(WIN_STATS_NAVIGATION);
    CopyWindowToVram(WIN_STATS_NAVIGATION, 3);
}

static void ResetStatsWindows(void)
{
    u8 i;

    FreeAllWindowBuffers();
    InitWindows(sStatsScreen_WindowTemplates);

    for (i = 0; i < WIN_STATS_END + 1; i++)
    {
        FillWindowPixelBuffer(i, PIXEL_FILL(0));
        PutWindowTilemap(i);
        CopyWindowToVram(i, COPYWIN_FULL);
    }
}

static void SaveMonDataInStruct(void)
{
    u16 species = NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum);
    u8 EVs[6] = {gSpeciesInfo[species].evYield_HP, gSpeciesInfo[species].evYield_Speed, gSpeciesInfo[species].evYield_Attack, gSpeciesInfo[species].evYield_SpAttack, gSpeciesInfo[species].evYield_Defense, gSpeciesInfo[species].evYield_SpDefense};
    u8 differentEVs;
    u8 i;

    //Count how many different EVs
    for (i = 0; i<6; i++)
    {
        if (EVs[i] > 0) //HP//Speed//Attack//Special Attack//Defense//Special Defense
            differentEVs++;
    }

    sPokedexView->sPokemonStats.species             = species;
    sPokedexView->sPokemonStats.genderRatio         = gSpeciesInfo[species].genderRatio;
    if ((gSpeciesInfo[species].baseHP_old != 0) && (gSaveBlock1Ptr->tx_Mode_New_Stats == 0))
        sPokedexView->sPokemonStats.baseHP_old              = gSpeciesInfo[species].baseHP_old;
    else
        sPokedexView->sPokemonStats.baseHP              = gSpeciesInfo[species].baseHP;
    if ((gSpeciesInfo[species].baseSpeed_old != 0) && (gSaveBlock1Ptr->tx_Mode_New_Stats == 0))
        sPokedexView->sPokemonStats.baseSpeed_old           = gSpeciesInfo[species].baseSpeed_old;
    else
        sPokedexView->sPokemonStats.baseSpeed           = gSpeciesInfo[species].baseSpeed;
    if ((gSpeciesInfo[species].baseAttack_old != 0) && (gSaveBlock1Ptr->tx_Mode_New_Stats == 0))
        sPokedexView->sPokemonStats.baseAttack_old          = gSpeciesInfo[species].baseAttack_old;
    else
        sPokedexView->sPokemonStats.baseAttack          = gSpeciesInfo[species].baseAttack;
    if ((gSpeciesInfo[species].baseSpAttack_old != 0) && (gSaveBlock1Ptr->tx_Mode_New_Stats == 0))
        sPokedexView->sPokemonStats.baseSpAttack_old        = gSpeciesInfo[species].baseSpAttack_old;
    else
        sPokedexView->sPokemonStats.baseSpAttack        = gSpeciesInfo[species].baseSpAttack;
    if ((gSpeciesInfo[species].baseDefense_old != 0) && (gSaveBlock1Ptr->tx_Mode_New_Stats == 0))
        sPokedexView->sPokemonStats.baseDefense_old         = gSpeciesInfo[species].baseDefense_old;
    else
        sPokedexView->sPokemonStats.baseDefense         = gSpeciesInfo[species].baseDefense;
    if ((gSpeciesInfo[species].baseSpDefense_old != 0) && (gSaveBlock1Ptr->tx_Mode_New_Stats == 0))
        sPokedexView->sPokemonStats.baseSpDefense_old       = gSpeciesInfo[species].baseSpDefense_old;
    else
        sPokedexView->sPokemonStats.baseSpDefense       = gSpeciesInfo[species].baseSpDefense;
    sPokedexView->sPokemonStats.differentEVs        = differentEVs;
    sPokedexView->sPokemonStats.evYield_HP          = EVs[0];
    sPokedexView->sPokemonStats.evYield_Speed       = EVs[1];
    sPokedexView->sPokemonStats.evYield_Attack      = EVs[2];
    sPokedexView->sPokemonStats.evYield_SpAttack    = EVs[3];
    sPokedexView->sPokemonStats.evYield_Defense     = EVs[4];
    sPokedexView->sPokemonStats.evYield_SpDefense   = EVs[5];
    sPokedexView->sPokemonStats.catchRate           = gSpeciesInfo[species].catchRate;
    sPokedexView->sPokemonStats.growthRate          = gSpeciesInfo[species].growthRate;
    sPokedexView->sPokemonStats.eggGroup1           = gSpeciesInfo[species].eggGroups[0];
    sPokedexView->sPokemonStats.eggGroup2           = gSpeciesInfo[species].eggGroups[1];
    sPokedexView->sPokemonStats.eggCycles           = gSpeciesInfo[species].eggCycles;
    sPokedexView->sPokemonStats.expYield            = gSpeciesInfo[species].expYield;
    sPokedexView->sPokemonStats.friendship          = gSpeciesInfo[species].friendship;
    sPokedexView->sPokemonStats.ability0            = GetAbilityBySpecies(species, 0);
    sPokedexView->sPokemonStats.ability1            = GetAbilityBySpecies(species, 1);
}

#define tMonSpriteId data[4]

static void Task_LoadStatsScreen(u8 taskId)
{
    switch (gMain.state)
    {
    case 0:
    default:
        if (!gPaletteFade.active)
        {
            u16 r2 = 0;
            sPokedexView->currentPage = STATS_SCREEN;
            gPokedexVBlankCB = gMain.vblankCallback;
            SetVBlankCallback(NULL);

            if (gTasks[taskId].data[1] != 0) r2 |= DISPCNT_OBJ_ON;
            if (gTasks[taskId].data[2] != 0) r2 |= DISPCNT_BG1_ON;
            
            ResetOtherVideoRegisters(r2);
            gMain.state = 1;
        }
        break;
    case 1:
        LoadTilesetTilemapHGSS(STATS_SCREEN);
        ResetStatsWindows();

        CopyBgTilemapBufferToVram(1);
        CopyBgTilemapBufferToVram(2);
        CopyBgTilemapBufferToVram(3);
        gMain.state++;
        break;
    case 2:
        LoadScreenSelectBarMain(0xD);
        LoadPokedexBgPalette(sPokedexView->isSearchResults, false);
        gMain.state++;
        break;
    case 3:
        sPokedexView->typeIconSpriteIds[0] = 0xFF;
        sPokedexView->typeIconSpriteIds[1] = 0xFF;
        sPokedexView->splitIconSpriteId = 0xFF;

        LoadCompressedPalette(gInterfaceTypeIcons_Pal, 0x1D0, 0x20);
        LoadCompressedSpriteSheet(&sSpriteSheet_SplitIcons);
        LoadSpritePalette(&sSpritePal_SplitIcons);
		
		CreateTypeIconSprites();
		
        gMain.state++;
        break;
    case 4:
        SaveMonDataInStruct();
        sPokedexView->moveSelected = 0;
        if (CalculateMoves())
            gMain.state++;
        break;
    case 5:
        if (gTasks[taskId].data[1] == 0)
        {
            u16 species = NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum);
            FreeMonIconPalettes();
            LoadMonIconPalette(species);
            gTasks[taskId].tMonSpriteId = CreateMonIcon(species, SpriteCB_MonIcon, 20, 40, 4, 0, TRUE);
            gSprites[gTasks[taskId].tMonSpriteId].oam.priority = 0;
        }
        gMain.state++;
        break;
        break;
    case 6:
        gTasks[taskId].data[5] = 0;
        
        PrintStatsScreen_NameGender(taskId, sPokedexListItem->dexNum, sPokedexView->dexMode != DEX_MODE_HOENN);
        PrintStatsScreen_Abilities(taskId);
        PrintStatsScreen_Moves(taskId);
        
        // FUTURE: PrintStatsScreen_BaseStats(taskId);
        // FUTURE: CreateMonTypeIcons(taskId); 

        if (!sPokedexListItem->owned) LoadPalette(gPlttBufferUnfaded + 1, 0x31, 0x1E);

        StatsPage_PrintNavigationButtons();
        gMain.state++;
        break;
    case 7:
        {
            u32 preserved = 0;
            if (gTasks[taskId].data[2] != 0) preserved |= 0x14;
            if (gTasks[taskId].data[1] != 0) preserved |= (1 << (gSprites[gTasks[taskId].tMonSpriteId].oam.paletteNum + 16));
			
            BeginNormalPaletteFade(~preserved, 0, 16, 0, RGB_BLACK);
            SetVBlankCallback(gPokedexVBlankCB);
            gMain.state++;
        }
        break;
    case 8:
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON | DISPCNT_BG1_ON | DISPCNT_BG2_ON | DISPCNT_BG3_ON);
        
        HideBg(0);
        ShowBg(1);
        ShowBg(2);
        ShowBg(3);
        gMain.state++;
        break;
    case 9:
        if (!gPaletteFade.active)
            gMain.state++;
        break;
    case 10:
        gMain.state++;
        break;
    case 11:
        gTasks[taskId].data[0] = 0;
        gTasks[taskId].data[1] = 0;
        gTasks[taskId].data[2] = 1;
        gTasks[taskId].func = Task_HandleStatsScreenInput;
        gMain.state = 0;
        break;
    }
}

static void FreeStatsScreenWindowAndBgBuffers(void)
{
    void *tilemapBuffer;

    FreeAllWindowBuffers();
    tilemapBuffer = GetBgTilemapBuffer(0);
    if (tilemapBuffer)
        Free(tilemapBuffer);
    tilemapBuffer = GetBgTilemapBuffer(1);
    if (tilemapBuffer)
        Free(tilemapBuffer);
    tilemapBuffer = GetBgTilemapBuffer(2);
    if (tilemapBuffer)
        Free(tilemapBuffer);
    tilemapBuffer = GetBgTilemapBuffer(3);
    if (tilemapBuffer)
        Free(tilemapBuffer);
}

static void Task_HandleStatsScreenInput(u8 taskId)
{
    u8 selected = sPokedexView->moveSelected;
    u16 move = sStatsMoves[selected];

    if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_DEX_PAGE);
        
        gTasks[taskId].data[5] ^= 1; 

        PrintStatsScreen_DestroyMoveItemIcon(taskId);

        FillWindowPixelBuffer(WIN_STATS_BASE_INFO, PIXEL_FILL(0));
        // PrintStatsScreen_BaseStats(taskId); 
        CopyWindowToVram(WIN_STATS_BASE_INFO, COPYWIN_FULL);

        FillWindowPixelBuffer(WIN_STATS_MOVES, PIXEL_FILL(0));
        PrintStatsScreen_Moves(taskId);
		
        CopyWindowToVram(WIN_STATS_MOVES, COPYWIN_FULL); 
    }
    if (JOY_NEW(B_BUTTON))
    {
        BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
        // Use special exit handler if called from party menu
        if (sExternalReturnCallback != NULL)
            gTasks[taskId].func = Task_ExitStatsScreenToExternal;
        else
            gTasks[taskId].func = Task_ExitStatsScreen;
        PlaySE(SE_PC_OFF);
        return;
    }

    //Change moves
    if (JOY_REPEAT(DPAD_UP) && sPokedexView->moveSelected > 0)
    {
        sPokedexView->moveSelected -= 1;
        PlaySE(SE_SELECT);
        
        PrintStatsScreen_DestroyMoveItemIcon(taskId);
        FillWindowPixelBuffer(WIN_STATS_MOVES, PIXEL_FILL(0));
        PrintStatsScreen_Moves(taskId);
        CopyWindowToVram(WIN_STATS_MOVES, COPYWIN_FULL);
    }
    if (JOY_REPEAT(DPAD_DOWN) && sPokedexView->moveSelected < sPokedexView->movesTotal - 1)
    {
        sPokedexView->moveSelected += 1;
        PlaySE(SE_SELECT);
        
        PrintStatsScreen_DestroyMoveItemIcon(taskId);
        FillWindowPixelBuffer(WIN_STATS_MOVES, PIXEL_FILL(0));
        PrintStatsScreen_Moves(taskId);
        CopyWindowToVram(WIN_STATS_MOVES, COPYWIN_FULL);
    }

    //Switch screens
    if ((JOY_NEW(DPAD_LEFT) || (JOY_NEW(L_BUTTON) && gSaveBlock2Ptr->optionsButtonMode == OPTIONS_BUTTON_MODE_LR)))
    {
        sPokedexView->selectedScreen = INFO_SCREEN;
        BeginNormalPaletteFade(0xFFFFFFEB, 0, 0, 0x10, RGB_BLACK);
        sPokedexView->screenSwitchState = 1;
        gTasks[taskId].func = Task_SwitchScreensFromStatsScreen;
        PlaySE(SE_PIN);
    }
    if ((JOY_NEW(DPAD_RIGHT) || (JOY_NEW(R_BUTTON) && gSaveBlock2Ptr->optionsButtonMode == OPTIONS_BUTTON_MODE_LR)))
    {
        if (!sPokedexListItem->owned)
            PlaySE(SE_FAILURE);
        else
        {
            sPokedexView->selectedScreen = EVO_SCREEN;
            BeginNormalPaletteFade(0xFFFFFFEB, 0, 0, 0x10, RGB_BLACK);
            sPokedexView->screenSwitchState = 3;
            gTasks[taskId].func = Task_SwitchScreensFromStatsScreen;
            PlaySE(SE_PIN);
        }
    }
}

#define ITEM_TAG 0xFDF3

static void PrintStatsScreen_DestroyMoveItemIcon(u8 taskId)
{
    FreeSpriteTilesByTag(ITEM_TAG);                         //Destroy item icon
    FreeSpritePaletteByTag(ITEM_TAG);                       //Destroy item icon
    FreeSpriteOamMatrix(&gSprites[gTasks[taskId].data[3]]); //Destroy item icon
    DestroySprite(&gSprites[gTasks[taskId].data[3]]);       //Destroy item icon
}

static bool8 CalculateMoves(void)
{
    u16 species = NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum);

    u16 statsMovesEgg[EGG_MOVES_ARRAY_COUNT] = {0};
    u16 statsMovesLevelUp[MAX_LEVEL_UP_MOVES] = {0};
    u16 statsMovesTMHM[NUM_TECHNICAL_MACHINES + NUM_HIDDEN_MACHINES] = {0};
    u16 move;

    u8 numEggMoves = 0;
    u8 numLevelUpMoves = 0;
    u8 numTMHMMoves = 0;
    u8 numTutorMoves = 0;
    u16 movesTotal = 0;
    u8 i,j;

    //Calculate amount of Egg and LevelUp moves
    numEggMoves = GetEggMovesSpecies(species, statsMovesEgg);
    numLevelUpMoves = GetLevelUpMovesBySpecies(species, statsMovesLevelUp);

    //Egg moves
    for (i=0; i < numEggMoves; i++)
    {
        sStatsMoves[movesTotal] = statsMovesEgg[i];
        movesTotal++;
    }

    //Level up moves
    for (i=0; i < numLevelUpMoves; i++)
    {
        sStatsMoves[movesTotal] = statsMovesLevelUp[i];
        movesTotal++;
    }

    //TMHM moves
    for (j = 0; j < NUM_TECHNICAL_MACHINES + NUM_HIDDEN_MACHINES; j++)
    {
        if (CanSpeciesLearnTMHM(species, j))
        {
            sStatsMoves[movesTotal] = ItemIdToBattleMoveId(ITEM_TM01 + j);
            movesTotal++;
            sStatsMovesTMHM_ID[numTMHMMoves] = (ITEM_TM01 + j);
            numTMHMMoves++;
        }
    }

    //Tutor moves
    for (i=0; i < TUTOR_MOVE_COUNT; i++)
    {
        if (CanLearnTutorMove(species, i)) //if (sTutorLearnsets[species] & (1 << i))
        {
            sStatsMoves[movesTotal] = gTutorMoves[i];
            numTutorMoves++;
            movesTotal++;
        }
    }

    sPokedexView->numEggMoves = numEggMoves;
    sPokedexView->numLevelUpMoves = numLevelUpMoves;
    sPokedexView->numTMHMMoves = numTMHMMoves;
    sPokedexView->numTutorMoves = numTutorMoves;
    sPokedexView->movesTotal = movesTotal;

    return TRUE;
}

static void PrintStatsScreen_Moves(u8 taskId)
{
    u8 numEggMoves     = sPokedexView->numEggMoves;
    u8 numLevelUpMoves = sPokedexView->numLevelUpMoves;
    u8 numTMHMMoves    = sPokedexView->numTMHMMoves;
    u8 numTutorMoves   = sPokedexView->numTutorMoves;
    u8 movesTotal      = sPokedexView->movesTotal;
    u8 selected        = sPokedexView->moveSelected;
    u8 level           = 0; 
    u16 move           = sStatsMoves[selected];
    u16 item           = ITEM_NONE;
    u16 species        = NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum);
    u8 contest_effectValue, contest_appeal = 0, contest_jam = 0;

    const u8 colLeftX  = 1;
    const u8 colRightX = 58;
    const u8 headerY   = 6;
    const u8 nameY     = 22;
    const u8 statsY    = 36;
    const u8 learnY    = 64;
	
	//Print Move Counter
    ConvertIntToDecimalStringN(gStringVar1, (selected + 1), STR_CONV_MODE_RIGHT_ALIGN, 3);
    ConvertIntToDecimalStringN(gStringVar2, movesTotal, STR_CONV_MODE_RIGHT_ALIGN, 3);
    StringExpandPlaceholders(gStringVar1, gText_Stats_MoveSelectedMax);
    nPrintStatsScreenTextNarrow(WIN_STATS_MOVES, gStringVar1, 3, headerY, 1);

    //Print Move Name
    StringCopy(gStringVar3, gMoveNames[move]);
    StringCopyPadded(gStringVar3, gStringVar3, CHAR_SPACE, 20);
    nPrintStatsScreenText(WIN_STATS_MOVES, gStringVar3, colLeftX, nameY, 0);
	
    // Print Move Stats
    if (gTasks[taskId].data[5] == 0) // Battle Stats
    {
        SetTypeIconPosAndPal(gBattleMoves[move].type, 56, 69, 0);
        SetSpriteInvisibility(1, TRUE); 

        nPrintStatsScreenText(WIN_STATS_MOVES, gText_Power, colLeftX, statsY, 0);
        nPrintStatsScreenText(WIN_STATS_MOVES, gText_Accuracy2, colLeftX, statsY + 12, 0);

        // Power Value
        if (gBattleMoves[move].power < 2)
            StringCopy(gStringVar1, gText_ThreeDashes);
        else
            ConvertIntToDecimalStringN(gStringVar1, gBattleMoves[move].power, STR_CONV_MODE_RIGHT_ALIGN, 3);
        nPrintStatsScreenText(WIN_STATS_MOVES, gStringVar1, colRightX, statsY, 0);

        // Accuracy Value
        if (gBattleMoves[move].accuracy == 0)
            StringCopy(gStringVar1, gText_ThreeDashes);
        else
            ConvertIntToDecimalStringN(gStringVar1, gBattleMoves[move].accuracy, STR_CONV_MODE_RIGHT_ALIGN, 3);
        nPrintStatsScreenText(WIN_STATS_MOVES, gStringVar1, colRightX, statsY + 12, 0);

        // Physical/Special/Status Split Icon
        DestroySplitIcon();
        if (gSaveBlock2Ptr->optionStyle == 0)
            ShowSplitIcon(gBattleMoves[move].category);
    }
    else // Contest Stats
    {
        SetTypeIconPosAndPal(NUMBER_OF_MON_TYPES + gContestMoves[move].contestCategory, 56, 69, 1);
        SetSpriteInvisibility(0, TRUE);
        DestroySplitIcon();

        nPrintStatsScreenText(WIN_STATS_MOVES, gText_Appeal, colLeftX, statsY, 0);
        nPrintStatsScreenText(WIN_STATS_MOVES, gText_Jam, colLeftX, statsY + 12, 0);

        // Appeal
        contest_effectValue = gContestEffects[gContestMoves[move].effect].appeal;
        contest_appeal = (contest_effectValue != 0xFF) ? (contest_effectValue / 10) : 0;
        ConvertIntToDecimalStringN(gStringVar1, contest_appeal, STR_CONV_MODE_RIGHT_ALIGN, 1);
        StringCopy(gStringVar2, gText_PlusSymbol);
        StringAppend(gStringVar2, gStringVar1);
        nPrintStatsScreenText(WIN_STATS_MOVES, gStringVar2, colRightX, statsY, 0);

        // Jam
        contest_effectValue = gContestEffects[gContestMoves[move].effect].jam;
        contest_jam = (contest_effectValue != 0xFF) ? (contest_effectValue / 10) : 0;
        ConvertIntToDecimalStringN(gStringVar1, contest_jam, STR_CONV_MODE_RIGHT_ALIGN, 1);
        StringCopy(gStringVar2, gText_Stats_Minus);
        StringAppend(gStringVar2, gStringVar1);
        nPrintStatsScreenText(WIN_STATS_MOVES, gStringVar2, colRightX + 1, statsY + 12, 0);
    }
	
	//Print Move Learn Stuff
	if (selected < numEggMoves)
    {
        nPrintStatsScreenText(WIN_STATS_MOVES, gText_ThreeDashes, 40, learnY, 0);
        item = ITEM_LUCKY_EGG;
    }
    else if (selected < (numEggMoves + numLevelUpMoves))
    {
        u16 levelData = (gSaveBlock1Ptr->tx_Mode_Modern_Moves == 0)
            ? gLevelUpLearnsets_Original[species][selected - numEggMoves]
            : gLevelUpLearnsets[species][selected - numEggMoves];
        
        level = (levelData & LEVEL_UP_MOVE_LV) >> 9;

        ConvertIntToDecimalStringN(gStringVar1, level, STR_CONV_MODE_LEFT_ALIGN, 3);
        PrintStatsScreenTextSmall(WIN_STATS_MOVES, gText_Stats_MoveLevel, 40, learnY);
        PrintStatsScreenTextSmall(WIN_STATS_MOVES, gStringVar1, 40, learnY + 12);
        item = ITEM_EXP_SHARE;
    }
    else if (selected < (numEggMoves + numLevelUpMoves + numTMHMMoves))
    {
        CopyItemName(sStatsMovesTMHM_ID[(selected - numEggMoves - numLevelUpMoves)], gStringVar1);
        PrintStatsScreenTextSmall(WIN_STATS_MOVES, gStringVar1, 40, learnY + 6);
        item = sStatsMovesTMHM_ID[(selected - numEggMoves - numLevelUpMoves)];
    }
    else // Tutor or Error
    {
        nPrintStatsScreenText(WIN_STATS_MOVES, gText_ThreeDashes, 40, learnY + 6, 0);
        item = ITEM_TEACHY_TV;
    }

    // Create Item Sprite
    gTasks[taskId].data[3] = AddItemIconSprite(ITEM_TAG, ITEM_TAG, item);
    gSprites[gTasks[taskId].data[3]].x2 = 38;
    gSprites[gTasks[taskId].data[3]].y2 = 146;
    gSprites[gTasks[taskId].data[3]].oam.priority = 0;
}

static void PrintStatsScreen_NameGender(u8 taskId, u32 num, u32 value)
{
	u8 str[16];
    u16 species = NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum);

    u8 base_x = 2;
    u8 base_y = 5;
    u8 gender_x = base_x;
    u8 gender_y = base_y + 26;

    // Print Number
    if (value == 0)
        value = NationalToHoennOrder(num);
    else
        value = num;
    ConvertIntToDecimalStringN(StringCopy(str, gText_NumberClear01), value, STR_CONV_MODE_LEADING_ZEROS, 3);
    nPrintStatsScreenText(WIN_STATS_NAME_GENDER, str, base_x, base_y, 1);

    // Print Name
    nPrintStatsScreenText(WIN_STATS_NAME_GENDER, gSpeciesNames[species], base_x, base_y + 13, 1);

    // Print Gender Ratio
    switch (sPokedexView->sPokemonStats.genderRatio)
    {
		case 0:
			nPrintStatsScreenText(WIN_STATS_NAME_GENDER, gText_Stats_Gender_0, gender_x, gender_y, 1);
			break;
		case 31:
			nPrintStatsScreenText(WIN_STATS_NAME_GENDER, gText_Stats_Gender_12_5, gender_x, gender_y, 1);
			break;
		case 63:
			nPrintStatsScreenText(WIN_STATS_NAME_GENDER, gText_Stats_Gender_25, gender_x, gender_y, 1);
			break;
		case 127:
			nPrintStatsScreenText(WIN_STATS_NAME_GENDER, gText_Stats_Gender_50, gender_x, gender_y, 1);
			break;
		case 191:
			nPrintStatsScreenText(WIN_STATS_NAME_GENDER, gText_Stats_Gender_75, gender_x, gender_y, 1);
			break;
		case 223:
			nPrintStatsScreenText(WIN_STATS_NAME_GENDER, gText_Stats_Gender_87_5, gender_x, gender_y, 1);
			break;
		case 254:
			nPrintStatsScreenText(WIN_STATS_NAME_GENDER, gText_Stats_Gender_100, gender_x, gender_y, 1);
			break;
        default:
            nPrintStatsScreenText(WIN_STATS_NAME_GENDER, gText_ThreeDashes, gender_x, gender_y, 1);
            break;
    }

    // Push to VRAM
    CopyWindowToVram(WIN_STATS_NAME_GENDER, COPYWIN_FULL);
}

static u8 PrintMonStatsToggle_DifferentEVsColumn(u8 differentEVs)
{
    if (differentEVs == 1 || differentEVs == 3)
        return 0;
    else
        return 1;
}

static u8 PrintMonStatsToggle_DifferentEVsRow(u8 differentEVs)
{
    if (differentEVs == 1 || differentEVs == 2)
        return 0;
    else
        return 1;
}

static u8* PrintMonStatsToggle_EV_Arrows(u8 *dest, u8 value)
{
    switch (value)
    {
        case 1:
            StringCopy(dest, gText_Stats_EV_Plus1);
            break;
        case 2:
            StringCopy(dest, gText_Stats_EV_Plus2);
            break;
        case 3:
            StringCopy(dest, gText_Stats_EV_Plus3);
            break;
    }
    return dest;
}

static void PrintStatsScreen_BaseStats(u8 taskId)
{
    u8 base_x = 8;
    u8 x_offset_column = 43;
    u8 x_offset_value = 26;
    u8 column = 0;
    u8 base_x_offset = 70;
    u8 base_x_first_row = 23;
    u8 base_x_second_row = 43;
    u8 base_y_offset = 11;
    u8 base_i = 0;
    u8 base_y = 5;
    u32 align_x;
    u8 total_x = 93;
    u8 strEV[25];
    u8 strBase[14];
    u8 EVs[6] = {sPokedexView->sPokemonStats.evYield_HP, sPokedexView->sPokemonStats.evYield_Speed, sPokedexView->sPokemonStats.evYield_Attack, sPokedexView->sPokemonStats.evYield_SpAttack, sPokedexView->sPokemonStats.evYield_Defense, sPokedexView->sPokemonStats.evYield_SpDefense};
    u8 differentEVs = 0;
    u8 i;
    u16 species = NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum);

    //Base stats
    if (gTasks[taskId].data[5] == 0)
    {
        PrintStatsScreenTextSmall(WIN_STATS_MOVES, gText_Stats_HP, base_x, base_y + base_y_offset*base_i);
        if ((gSpeciesInfo[species].baseHP_old != 0) && (gSaveBlock1Ptr->tx_Mode_New_Stats == 0))
            ConvertIntToDecimalStringN(strBase, sPokedexView->sPokemonStats.baseHP_old, STR_CONV_MODE_RIGHT_ALIGN, 3);
        else
            ConvertIntToDecimalStringN(strBase, sPokedexView->sPokemonStats.baseHP, STR_CONV_MODE_RIGHT_ALIGN, 3);
        PrintStatsScreenTextSmall(WIN_STATS_MOVES, strBase, base_x+base_x_first_row, base_y + base_y_offset*base_i);

        PrintStatsScreenTextSmall(WIN_STATS_MOVES, gText_Stats_Speed, base_x+base_x_second_row, base_y + base_y_offset*base_i);
        if ((gSpeciesInfo[species].baseSpeed_old != 0) && (gSaveBlock1Ptr->tx_Mode_New_Stats == 0))
            ConvertIntToDecimalStringN(strBase, sPokedexView->sPokemonStats.baseSpeed_old, STR_CONV_MODE_RIGHT_ALIGN, 3);
        else
            ConvertIntToDecimalStringN(strBase, sPokedexView->sPokemonStats.baseSpeed, STR_CONV_MODE_RIGHT_ALIGN, 3);
        PrintStatsScreenTextSmall(WIN_STATS_MOVES, strBase, base_x+base_x_offset, base_y + base_y_offset*base_i);

        base_i++;
        PrintStatsScreenTextSmall(WIN_STATS_MOVES, gText_Stats_Attack, base_x, base_y + base_y_offset*base_i);
        if ((gSpeciesInfo[species].baseAttack_old != 0) && (gSaveBlock1Ptr->tx_Mode_New_Stats == 0))
            ConvertIntToDecimalStringN(strBase, sPokedexView->sPokemonStats.baseAttack_old, STR_CONV_MODE_RIGHT_ALIGN, 3);
        else
            ConvertIntToDecimalStringN(strBase, sPokedexView->sPokemonStats.baseAttack, STR_CONV_MODE_RIGHT_ALIGN, 3);
        PrintStatsScreenTextSmall(WIN_STATS_MOVES, strBase, base_x+base_x_first_row, base_y + base_y_offset*base_i);

        PrintStatsScreenTextSmall(WIN_STATS_MOVES, gText_Stats_SpAttack, base_x+base_x_second_row, base_y + base_y_offset*base_i);
        if ((gSpeciesInfo[species].baseSpAttack_old != 0) && (gSaveBlock1Ptr->tx_Mode_New_Stats == 0))
            ConvertIntToDecimalStringN(strBase, sPokedexView->sPokemonStats.baseSpAttack_old, STR_CONV_MODE_RIGHT_ALIGN, 3);
        else
            ConvertIntToDecimalStringN(strBase, sPokedexView->sPokemonStats.baseSpAttack, STR_CONV_MODE_RIGHT_ALIGN, 3);
        PrintStatsScreenTextSmall(WIN_STATS_MOVES, strBase, base_x+base_x_offset, base_y + base_y_offset*base_i);

        base_i++;
        PrintStatsScreenTextSmall(WIN_STATS_MOVES, gText_Stats_Defense, base_x, base_y + base_y_offset*base_i);
        if ((gSpeciesInfo[species].baseDefense_old != 0) && (gSaveBlock1Ptr->tx_Mode_New_Stats == 0))
            ConvertIntToDecimalStringN(strBase, sPokedexView->sPokemonStats.baseDefense_old, STR_CONV_MODE_RIGHT_ALIGN, 3);
        else
            ConvertIntToDecimalStringN(strBase, sPokedexView->sPokemonStats.baseDefense, STR_CONV_MODE_RIGHT_ALIGN, 3);
        PrintStatsScreenTextSmall(WIN_STATS_MOVES, strBase, base_x+base_x_first_row, base_y + base_y_offset*base_i);

        PrintStatsScreenTextSmall(WIN_STATS_MOVES, gText_Stats_SpDefense, base_x+base_x_second_row, base_y + base_y_offset*base_i);
        if ((gSpeciesInfo[species].baseSpDefense_old != 0) && (gSaveBlock1Ptr->tx_Mode_New_Stats == 0))
            ConvertIntToDecimalStringN(strBase, sPokedexView->sPokemonStats.baseSpDefense_old, STR_CONV_MODE_RIGHT_ALIGN, 3);
        else
            ConvertIntToDecimalStringN(strBase, sPokedexView->sPokemonStats.baseSpDefense, STR_CONV_MODE_RIGHT_ALIGN, 3);
        PrintStatsScreenTextSmall(WIN_STATS_MOVES, strBase, base_x+base_x_offset, base_y + base_y_offset*base_i);
        base_i++;
    }
    else //EV increases
    {
        //If 1 or 2 EVs display with the same layout as the base stats
        if (sPokedexView->sPokemonStats.differentEVs < 3)
        {
            differentEVs = 0;
            //HP
            if (EVs[0] > 0)
            {
                differentEVs++;
                column = PrintMonStatsToggle_DifferentEVsColumn(differentEVs);
                base_i = PrintMonStatsToggle_DifferentEVsRow(differentEVs);
                StringCopy(gStringVar1, gText_Stats_HP);
                PrintMonStatsToggle_EV_Arrows(gStringVar2, EVs[0]);
                StringExpandPlaceholders(gStringVar3, gText_Stats_EvStr1Str2);
                PrintStatsScreenTextSmall(WIN_STATS_MOVES, gStringVar3, base_x + x_offset_column*column, base_y + base_y_offset*base_i);
            }
            //Speed
            if (EVs[1]> 0)
            {
                differentEVs++;
                column = PrintMonStatsToggle_DifferentEVsColumn(differentEVs);
                base_i = PrintMonStatsToggle_DifferentEVsRow(differentEVs);
                StringCopy(gStringVar1, gText_Stats_Speed);
                PrintMonStatsToggle_EV_Arrows(gStringVar2, EVs[1]);
                StringExpandPlaceholders(gStringVar3, gText_Stats_EvStr1Str2);
                PrintStatsScreenTextSmall(WIN_STATS_MOVES, gStringVar3, base_x + x_offset_column*column, base_y + base_y_offset*base_i);
            }
            //Attack
            if (EVs[2] > 0)
            {
                differentEVs++;
                column = PrintMonStatsToggle_DifferentEVsColumn(differentEVs);
                base_i = PrintMonStatsToggle_DifferentEVsRow(differentEVs);
                StringCopy(gStringVar1, gText_Stats_Attack);
                PrintMonStatsToggle_EV_Arrows(gStringVar2, EVs[2]);
                StringExpandPlaceholders(gStringVar3, gText_Stats_EvStr1Str2);
                PrintStatsScreenTextSmall(WIN_STATS_MOVES, gStringVar3, base_x + x_offset_column*column, base_y + base_y_offset*base_i);
            }
            //Special Attack
            if (EVs[3] > 0)
            {
                differentEVs++;
                column = PrintMonStatsToggle_DifferentEVsColumn(differentEVs);
                base_i = PrintMonStatsToggle_DifferentEVsRow(differentEVs);
                StringCopy(gStringVar1, gText_Stats_SpAttack);
                PrintMonStatsToggle_EV_Arrows(gStringVar2, EVs[3]);
                StringExpandPlaceholders(gStringVar3, gText_Stats_EvStr1Str2);
                PrintStatsScreenTextSmall(WIN_STATS_MOVES, gStringVar3, base_x + x_offset_column*column, base_y + base_y_offset*base_i);
            }
            //Defense
            if (EVs[4] > 0)
            {
                differentEVs++;
                column = PrintMonStatsToggle_DifferentEVsColumn(differentEVs);
                base_i = PrintMonStatsToggle_DifferentEVsRow(differentEVs);
                StringCopy(gStringVar1, gText_Stats_Defense);
                PrintMonStatsToggle_EV_Arrows(gStringVar2, EVs[4]);
                StringExpandPlaceholders(gStringVar3, gText_Stats_EvStr1Str2);
                PrintStatsScreenTextSmall(WIN_STATS_MOVES, gStringVar3, base_x + x_offset_column*column, base_y + base_y_offset*base_i);
            }
            //Special Defense
            if (EVs[5] > 0)
            {
                differentEVs++;
                column = PrintMonStatsToggle_DifferentEVsColumn(differentEVs);
                base_i = PrintMonStatsToggle_DifferentEVsRow(differentEVs);
                StringCopy(gStringVar1, gText_Stats_SpDefense);
                PrintMonStatsToggle_EV_Arrows(gStringVar2, EVs[5]);
                StringExpandPlaceholders(gStringVar3, gText_Stats_EvStr1Str2);
                PrintStatsScreenTextSmall(WIN_STATS_MOVES, gStringVar3, base_x + x_offset_column*column, base_y + base_y_offset*base_i);
            }
        }
        else //3 different EVs in 1 row
        {
            column = 0;
            //HP
            if (EVs[0] > 0)
            {
                StringCopy(gStringVar1, gText_Stats_HP);
                PrintMonStatsToggle_EV_Arrows(gStringVar2, EVs[0]);
                StringExpandPlaceholders(gStringVar3, gText_Stats_EvStr1Str2);
                PrintStatsScreenTextSmall(WIN_STATS_MOVES, gStringVar3, base_x + 29*column, base_y + base_y_offset*base_i);
                column++;
            }
            //Speed
            if (EVs[1] > 0)
            {
                StringCopy(gStringVar1, gText_Stats_Speed);
                PrintMonStatsToggle_EV_Arrows(gStringVar2, EVs[1]);
                StringExpandPlaceholders(gStringVar3, gText_Stats_EvStr1Str2);
                PrintStatsScreenTextSmall(WIN_STATS_MOVES, gStringVar3, base_x + 29*column, base_y + base_y_offset*base_i);
                column++;
            }
            //Attack
            if (EVs[2] > 0)
            {
                StringCopy(gStringVar1, gText_Stats_Attack);
                PrintMonStatsToggle_EV_Arrows(gStringVar2, EVs[2]);
                StringExpandPlaceholders(gStringVar3, gText_Stats_EvStr1Str2);
                PrintStatsScreenTextSmall(WIN_STATS_MOVES, gStringVar3, base_x + 29*column, base_y + base_y_offset*base_i);
                column++;
            }
            //Special Attack
            if (EVs[3] > 0)
            {
                StringCopy(gStringVar1, gText_Stats_SpAttack);
                PrintMonStatsToggle_EV_Arrows(gStringVar2, EVs[3]);
                StringExpandPlaceholders(gStringVar3, gText_Stats_EvStr1Str2);
                PrintStatsScreenTextSmall(WIN_STATS_MOVES, gStringVar3, base_x + 29*column, base_y + base_y_offset*base_i);
                column++;
            }
            //Defense
            if (EVs[4] > 0)
            {
                StringCopy(gStringVar1, gText_Stats_Defense);
                PrintMonStatsToggle_EV_Arrows(gStringVar2, EVs[4]);
                StringExpandPlaceholders(gStringVar3, gText_Stats_EvStr1Str2);
                PrintStatsScreenTextSmall(WIN_STATS_MOVES, gStringVar3, base_x + 29*column, base_y + base_y_offset*base_i);
                column++;
            }
            //Special Defense
            if (EVs[5] > 0)
            {
                StringCopy(gStringVar1, gText_Stats_SpDefense);
                PrintMonStatsToggle_EV_Arrows(gStringVar2, EVs[5]);
                StringExpandPlaceholders(gStringVar3, gText_Stats_EvStr1Str2);
                PrintStatsScreenTextSmall(WIN_STATS_MOVES, gStringVar3, base_x + 29*column, base_y + base_y_offset*base_i);
                column++;
            }
        }
        base_i++;
    }

    //TOGGLE--------------------------------------
    if (gTasks[taskId].data[5] == 0)
    {
        u32 catchRate = sPokedexView->sPokemonStats.catchRate;
        u32 growthRate = sPokedexView->sPokemonStats.growthRate;

        //Catch rate
        PrintStatsScreenTextSmall(WIN_STATS_MOVES, gText_Stats_CatchRate, base_x, base_y + base_y_offset*base_i);
        if (catchRate <= 10)
            PrintStatsScreenTextSmall(WIN_STATS_MOVES, gText_Stats_CatchRate_Legend, base_x + x_offset_column, base_y + base_y_offset*base_i);
        else if (catchRate <= 70)
            PrintStatsScreenTextSmall(WIN_STATS_MOVES, gText_Stats_CatchRate_VeryHard, base_x + x_offset_column, base_y + base_y_offset*base_i);
        else if (catchRate <= 100)
            PrintStatsScreenTextSmall(WIN_STATS_MOVES, gText_Stats_CatchRate_Difficult, base_x + x_offset_column, base_y + base_y_offset*base_i);
        else if (catchRate <= 150)
            PrintStatsScreenTextSmall(WIN_STATS_MOVES, gText_Stats_CatchRate_Medium, base_x + x_offset_column, base_y + base_y_offset*base_i);
        else if (catchRate <= 200)
            PrintStatsScreenTextSmall(WIN_STATS_MOVES, gText_Stats_CatchRate_Relaxed, base_x + x_offset_column, base_y + base_y_offset*base_i);
        else
            PrintStatsScreenTextSmall(WIN_STATS_MOVES, gText_Stats_CatchRate_Easy, base_x + x_offset_column, base_y + base_y_offset*base_i);
        base_i++;

        //Growth rate
        PrintStatsScreenTextSmall(WIN_STATS_MOVES, gText_Stats_Growthrate, base_x, base_y + base_y_offset*base_i);
        switch (growthRate)
        {
        case GROWTH_MEDIUM_FAST:
            StringCopy(strEV, gText_Stats_MEDIUM_FAST);
            break;
        case GROWTH_ERRATIC:
            StringCopy(strEV, gText_Stats_ERRATIC);
            break;
        case GROWTH_FLUCTUATING:
            StringCopy(strEV, gText_Stats_FLUCTUATING);
            break;
        case GROWTH_MEDIUM_SLOW:
            StringCopy(strEV, gText_Stats_MEDIUM_SLOW);
            break;
        case GROWTH_FAST:
            StringCopy(strEV, gText_Stats_FAST);
            break;
        case GROWTH_SLOW:
            StringCopy(strEV, gText_Stats_SLOW);
            break;
        default:
            break;
        }
        align_x = GetStringRightAlignXOffset(0, strEV, total_x);
        PrintStatsScreenTextSmall(WIN_STATS_MOVES, strEV, align_x, base_y + base_y_offset*base_i);
    }
    else
    {
        //Exp Yield
        PrintStatsScreenTextSmall(WIN_STATS_MOVES, gText_Stats_ExpYield, base_x, base_y + base_y_offset*base_i);
        ConvertIntToDecimalStringN(gStringVar1, sPokedexView->sPokemonStats.expYield, STR_CONV_MODE_RIGHT_ALIGN, 3);
        PrintStatsScreenTextSmall(WIN_STATS_MOVES, gStringVar1, base_x + base_x_offset, base_y + base_y_offset*base_i);
        base_i++;

        //Friendship
        PrintStatsScreenTextSmall(WIN_STATS_MOVES, gText_Stats_Friendship, base_x, base_y + base_y_offset*base_i);
        switch (sPokedexView->sPokemonStats.friendship)
        {
        case 35:
            StringCopy(strEV, gText_Stats_Friendship_BigAnger);
            break;
        case 70:
            StringCopy(strEV, gText_Stats_Friendship_Neutral);
            break;
        case 90:
            StringCopy(strEV, gText_Stats_Friendship_Happy);
            break;
        case 100:
            StringCopy(strEV, gText_Stats_Friendship_Happy);
            break;
        case 140:
            StringCopy(strEV, gText_Stats_Friendship_BigSmile);
            break;
        default:
            ConvertIntToDecimalStringN(strEV, sPokedexView->sPokemonStats.friendship, STR_CONV_MODE_RIGHT_ALIGN, 3);
            break;
        }
        align_x = GetStringRightAlignXOffset(0, strEV, total_x);
        PrintStatsScreenTextSmall(WIN_STATS_MOVES, strEV, align_x, base_y + base_y_offset*base_i);
        base_i++;

        //Egg cycles
        if (sPokedexView->sPokemonStats.eggGroup1 == EGG_GROUP_NO_EGGS_DISCOVERED || sPokedexView->sPokemonStats.eggGroup2 == EGG_GROUP_NO_EGGS_DISCOVERED) //Species without eggs (legendaries etc)
        {
            PrintStatsScreenTextSmall(WIN_STATS_MOVES, gText_Stats_EggCycles, base_x, base_y + base_y_offset*base_i);
            PrintStatsScreenTextSmall(WIN_STATS_MOVES, gText_ThreeDashes, 78, base_y + base_y_offset*base_i);
        }
        else
        {
            PrintStatsScreenTextSmall(WIN_STATS_MOVES, gText_Stats_EggCycles, base_x, base_y + base_y_offset*base_i);
            if (sPokedexView->sPokemonStats.eggCycles <= 10)
            {
                StringCopy(strEV, gText_Stats_EggCycles_VeryFast);
                align_x = 76;
            }
            else if (sPokedexView->sPokemonStats.eggCycles <= 20)
            {
                StringCopy(strEV, gText_Stats_EggCycles_Fast);
                align_x = 85;
            }
            else if (sPokedexView->sPokemonStats.eggCycles <= 30)
            {
                StringCopy(strEV, gText_Stats_EggCycles_Normal);
                align_x = 76;
            }
            else
            {
                StringCopy(strEV, gText_Stats_EggCycles_Slow);
                align_x = 67;
            }
            PrintStatsScreenTextSmall(WIN_STATS_MOVES, strEV, align_x, base_y + base_y_offset*base_i);
        }
        base_i++;

        //Egg group 1
        switch (sPokedexView->sPokemonStats.eggGroup1)
        {
        case EGG_GROUP_MONSTER     :
            StringCopy(gStringVar1, gText_Stats_eggGroup_MONSTER);
            break;
        case EGG_GROUP_WATER_1     :
            StringCopy(gStringVar1, gText_Stats_eggGroup_WATER_1);
            break;
        case EGG_GROUP_BUG         :
            StringCopy(gStringVar1, gText_Stats_eggGroup_BUG);
            break;
        case EGG_GROUP_FLYING      :
            StringCopy(gStringVar1, gText_Stats_eggGroup_FLYING);
            break;
        case EGG_GROUP_FIELD       :
            StringCopy(gStringVar1, gText_Stats_eggGroup_FIELD);
            break;
        case EGG_GROUP_FAIRY       :
            StringCopy(gStringVar1, gText_Stats_eggGroup_FAIRY);
            break;
        case EGG_GROUP_GRASS       :
            StringCopy(gStringVar1, gText_Stats_eggGroup_GRASS);
            break;
        case EGG_GROUP_HUMAN_LIKE  :
            StringCopy(gStringVar1, gText_Stats_eggGroup_HUMAN_LIKE);
            break;
        case EGG_GROUP_WATER_3     :
            StringCopy(gStringVar1, gText_Stats_eggGroup_WATER_3);
            break;
        case EGG_GROUP_MINERAL     :
            StringCopy(gStringVar1, gText_Stats_eggGroup_MINERAL);
            break;
        case EGG_GROUP_AMORPHOUS   :
            StringCopy(gStringVar1, gText_Stats_eggGroup_AMORPHOUS);
            break;
        case EGG_GROUP_WATER_2     :
            StringCopy(gStringVar1, gText_Stats_eggGroup_WATER_2);
            break;
        case EGG_GROUP_DITTO       :
            StringCopy(gStringVar1, gText_Stats_eggGroup_DITTO);
            break;
        case EGG_GROUP_DRAGON      :
            StringCopy(gStringVar1, gText_Stats_eggGroup_DRAGON);
            break;
        case EGG_GROUP_NO_EGGS_DISCOVERED:
            StringCopy(gStringVar1, gText_Stats_eggGroup_UNDISCOVERED);
            break;
        }
        //Egg group 2
        if (sPokedexView->sPokemonStats.eggGroup1 != sPokedexView->sPokemonStats.eggGroup2)
        {
            switch (sPokedexView->sPokemonStats.eggGroup2)
            {
            case EGG_GROUP_MONSTER     :
                StringCopy(gStringVar2, gText_Stats_eggGroup_MONSTER);
                break;
            case EGG_GROUP_WATER_1     :
                StringCopy(gStringVar2, gText_Stats_eggGroup_WATER_1);
                break;
            case EGG_GROUP_BUG         :
                StringCopy(gStringVar2, gText_Stats_eggGroup_BUG);
                break;
            case EGG_GROUP_FLYING      :
                StringCopy(gStringVar2, gText_Stats_eggGroup_FLYING);
                break;
            case EGG_GROUP_FIELD       :
                StringCopy(gStringVar2, gText_Stats_eggGroup_FIELD);
                break;
            case EGG_GROUP_FAIRY       :
                StringCopy(gStringVar2, gText_Stats_eggGroup_FAIRY);
                break;
            case EGG_GROUP_GRASS       :
                StringCopy(gStringVar2, gText_Stats_eggGroup_GRASS);
                break;
            case EGG_GROUP_HUMAN_LIKE  :
                StringCopy(gStringVar2, gText_Stats_eggGroup_HUMAN_LIKE);
                break;
            case EGG_GROUP_WATER_3     :
                StringCopy(gStringVar2, gText_Stats_eggGroup_WATER_3);
                break;
            case EGG_GROUP_MINERAL     :
                StringCopy(gStringVar2, gText_Stats_eggGroup_MINERAL);
                break;
            case EGG_GROUP_AMORPHOUS   :
                StringCopy(gStringVar2, gText_Stats_eggGroup_AMORPHOUS);
                break;
            case EGG_GROUP_WATER_2     :
                StringCopy(gStringVar2, gText_Stats_eggGroup_WATER_2);
                break;
            case EGG_GROUP_DITTO       :
                StringCopy(gStringVar2, gText_Stats_eggGroup_DITTO);
                break;
            case EGG_GROUP_DRAGON      :
                StringCopy(gStringVar2, gText_Stats_eggGroup_DRAGON);
                break;
            case EGG_GROUP_NO_EGGS_DISCOVERED:
                StringCopy(gStringVar2, gText_Stats_eggGroup_UNDISCOVERED);
                break;
            }
            StringExpandPlaceholders(gStringVar3, gText_Stats_eggGroup_Groups);
            align_x = GetStringRightAlignXOffset(0, gStringVar3, total_x);
            PrintStatsScreenTextSmall(WIN_STATS_MOVES, gStringVar3, base_x, base_y + base_y_offset*base_i);
        }
        else
        {
            align_x = GetStringRightAlignXOffset(0, gStringVar1, total_x);
            PrintStatsScreenTextSmall(WIN_STATS_MOVES, gStringVar1, base_x, base_y + base_y_offset*base_i);
        }
        base_i++;
    }
}

static void PrintStatsScreen_Abilities(u8 taskId)
{
    u16 species = NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum);
    u8 abilities_x = 6;
    u8 abilities_y = 0;
    u16 ability0;
    u16 ability1;
    u16 abilityHidden;

	// Abilitie(s)
    ability0 = sPokedexView->sPokemonStats.ability0;
    nPrintStatsScreenText(WIN_STATS_ABILITIES, gAbilityNames[ability0], abilities_x, abilities_y, 1);
    nPrintStatsScreenText(WIN_STATS_ABILITIES, gAbilityDescriptionPointers[ability0], abilities_x, abilities_y + 12, 0);

    ability1 = sPokedexView->sPokemonStats.ability1;
    if (ability1 != 0 && ability1 != ability0)
    {
        nPrintStatsScreenText(WIN_STATS_ABILITIES, gAbilityNames[ability1], abilities_x, abilities_y + 32, 1);
        nPrintStatsScreenText(WIN_STATS_ABILITIES, gAbilityDescriptionPointers[ability1], abilities_x, abilities_y + 44, 0);
    }
	
    PutWindowTilemap(WIN_STATS_ABILITIES);
    CopyWindowToVram(WIN_STATS_ABILITIES, COPYWIN_FULL);
}

static void Task_SwitchScreensFromStatsScreen(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        FreeSpriteTilesByTag(ITEM_TAG);                         //Destroy item icon
        FreeSpritePaletteByTag(ITEM_TAG);                       //Destroy item icon
        FreeSpriteOamMatrix(&gSprites[gTasks[taskId].data[3]]); //Destroy item icon
        DestroySprite(&gSprites[gTasks[taskId].data[3]]);       //Destroy item icon
        FreeMonIconPalettes();                                          //Destroy pokemon icon sprite
        FreeAndDestroyMonIconSprite(&gSprites[gTasks[taskId].data[4]]); //Destroy pokemon icon sprite

        FreeAndDestroyMonPicSprite(gTasks[taskId].tMonSpriteId);
        switch (sPokedexView->screenSwitchState)
        {
        case 1:
            FreeAllWindowBuffers();
            InitWindows(sInfoScreen_WindowTemplates);
            gTasks[taskId].func = Task_LoadAreaScreen;
            break;
        case 2:
            gTasks[taskId].func = Task_LoadCryScreen;
            break;
        case 3:
            FreeAllWindowBuffers();
            InitWindows(sInfoScreen_WindowTemplates);
            gTasks[taskId].func = Task_LoadEvolutionScreen;
            break;
        default:
            FreeAllWindowBuffers();
            InitWindows(sInfoScreen_WindowTemplates);
            gTasks[taskId].func = Task_LoadInfoScreen;
            break;
        }
    }
}

static void Task_ExitStatsScreen(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        FreeSpriteTilesByTag(ITEM_TAG);                         //Destroy item icon
        FreeSpritePaletteByTag(ITEM_TAG);                       //Destroy item icon
        FreeSpriteOamMatrix(&gSprites[gTasks[taskId].data[3]]); //Destroy item icon
        DestroySprite(&gSprites[gTasks[taskId].data[3]]);       //Destroy item icon
        FreeMonIconPalettes();                                          //Destroy pokemon icon sprite
        FreeAndDestroyMonIconSprite(&gSprites[gTasks[taskId].data[4]]); //Destroy pokemon icon sprite

        FreeAndDestroyMonPicSprite(gTasks[taskId].tMonSpriteId);
        FreeInfoScreenWindowAndBgBuffers();
        DestroyTask(taskId);
    }
}

static void Task_ExitStatsScreenToExternal(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        FreeSpriteTilesByTag(ITEM_TAG);                         //Destroy item icon
        FreeSpritePaletteByTag(ITEM_TAG);                       //Destroy item icon
        FreeSpriteOamMatrix(&gSprites[gTasks[taskId].data[3]]); //Destroy item icon
        DestroySprite(&gSprites[gTasks[taskId].data[3]]);       //Destroy item icon
        FreeMonIconPalettes();                                          //Destroy pokemon icon sprite
        FreeAndDestroyMonIconSprite(&gSprites[gTasks[taskId].data[4]]); //Destroy pokemon icon sprite

        FreeAndDestroyMonPicSprite(gTasks[taskId].tMonSpriteId);
        FreeInfoScreenWindowAndBgBuffers();
        DestroyTask(taskId);
        if (sExternalReturnCallback != NULL)
            SetMainCallback2(sExternalReturnCallback);
    }
}


//************************************
//*                                  *
//*        EVOS                      *
//*                                  *
//************************************
static const u8 sEvoFormsPageNavigationTextColor[] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY};

static void EvoFormsPage_PrintNavigationButtons(void)
{
    u8 x = 6;
    u8 y = 0;

    FillWindowPixelBuffer(WIN_NAV_BUTTONS, PIXEL_FILL(0));
	AddTextPrinterParameterized3(WIN_NAV_BUTTONS, 0, x+9, y, sStatsPageNavigationTextColor, 0, gText_EVO_Buttons);
    // DrawKeypadIcon(WIN_NAV_BUTTONS, 10, 5, 0); //(u8 windowId, u8 keypadIconId, u16 x, u16 y)
    PutWindowTilemap(WIN_NAV_BUTTONS);
    CopyWindowToVram(WIN_NAV_BUTTONS, 3);
}

static void ResetEvoScreenDataStruct(void)
{
    u8 i;
    sPokedexView->sEvoScreenData.numAllEvolutions = 0;
    sPokedexView->sEvoScreenData.numSeen = 0;
    sPokedexView->sEvoScreenData.menuPos = 0;
    for (i = 0; i < 10; i++)
    {
        sPokedexView->sEvoScreenData.targetSpecies[i] = 0;
        sPokedexView->sEvoScreenData.seen[i] = 0;
    }

}

static void GetSeenFlagTargetSpecies(void)
{
    u8 i;
    u16 species;
    for (i = 0; i < sPokedexView->sEvoScreenData.numAllEvolutions; i++)
    {
        species = sPokedexView->sEvoScreenData.targetSpecies[i];
        if (GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_GET_SEEN))
        {
            sPokedexView->sEvoScreenData.seen[i] = TRUE;
            sPokedexView->sEvoScreenData.numSeen += 1;
        }

    }
}

static void Task_LoadEvolutionScreen(u8 taskId)
{
    switch (gMain.state)
    {
    case 0:
    default:
        if (!gPaletteFade.active)
        {
            u16 r2;

            sPokedexView->currentPage = EVO_SCREEN;
            gPokedexVBlankCB = gMain.vblankCallback;
            SetVBlankCallback(NULL);
            r2 = 0;
            if (gTasks[taskId].data[1] != 0)
                r2 += DISPCNT_OBJ_ON;
            if (gTasks[taskId].data[2] != 0)
                r2 |= DISPCNT_BG1_ON;
            ResetOtherVideoRegisters(r2);
            gMain.state = 1;
        }
        break;
    case 1:
        LoadTilesetTilemapHGSS(EVO_SCREEN);
        FillWindowPixelBuffer(WIN_EVO_DISPLAY, PIXEL_FILL(0));
        PutWindowTilemap(WIN_EVO_DISPLAY);
        CopyWindowToVram(WIN_EVO_DISPLAY, 3);
        FillWindowPixelBuffer(WIN_NAV_BUTTONS, PIXEL_FILL(0));
        PutWindowTilemap(WIN_NAV_BUTTONS);
        CopyWindowToVram(WIN_NAV_BUTTONS, 3);
        CopyBgTilemapBufferToVram(1);
        CopyBgTilemapBufferToVram(2);
        CopyBgTilemapBufferToVram(3);
        gMain.state++;
        break;
    case 2:
        LoadScreenSelectBarMain(0xD);
        LoadPokedexBgPalette(sPokedexView->isSearchResults, false);
        gMain.state++;
        break;
    case 3:
        if (gTasks[taskId].data[1] == 0)
        {
            sPokedexView->selectedScreen = EVO_SCREEN;
            ResetEvoScreenDataStruct();
            //Icon
            FreeMonIconPalettes(); //Free space for new pallete
            LoadMonIconPalette(NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum)); //Loads pallete for current mon
            PrintPreEvolutions(taskId, NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum));
            #ifdef POKEMON_EXPANSION
                gTasks[taskId].data[4] = CreateMonIcon(NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum), SpriteCB_MonIcon, 18 + 32*sPokedexView->numPreEvolutions, 31, 4, 0); //Create pokemon sprite
            #else
                gTasks[taskId].data[4] = CreateMonIcon(NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum), SpriteCB_MonIcon, 18 + 32*sPokedexView->numPreEvolutions, 31, 4, 0, TRUE); //Create pokemon sprite
            #endif
            EvoFormsPage_PrintNavigationButtons(); //HGSS_Ui Navigation buttons
            gSprites[gTasks[taskId].data[4]].oam.priority = 0;
        }
        gMain.state++;
        break;
    case 4:
        //Print evo info and icons
        gTasks[taskId].data[3] = 0;
        PrintEvolutionTargetSpeciesAndMethod(taskId, NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum), 0, sPokedexView->numPreEvolutions);
        LoadSpritePalette(&sSpritePalette_Arrow);
        GetSeenFlagTargetSpecies();
        if (sPokedexView->sEvoScreenData.numAllEvolutions != 0 && sPokedexView->sEvoScreenData.numSeen != 0)
        {
            sPokedexView->sEvoScreenData.arrowSpriteId = CreateSprite(&sSpriteTemplate_Arrow, 7, 58, 0);
            gSprites[sPokedexView->sEvoScreenData.arrowSpriteId].animNum = 2;
        }
        gMain.state++;
        break;
    case 5:
        {
        u32 preservedPalettes = 0;

        if (gTasks[taskId].data[2] != 0)
            preservedPalettes = 0x14; // each bit represents a palette index
        if (gTasks[taskId].data[1] != 0)
            preservedPalettes |= (1 << (gSprites[gTasks[taskId].tMonSpriteId].oam.paletteNum + 16));
        BeginNormalPaletteFade(~preservedPalettes, 0, 16, 0, RGB_BLACK);
        SetVBlankCallback(gPokedexVBlankCB);
        gMain.state++;
        }
        break;
    case 6:
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        SetGpuReg(REG_OFFSET_BLDY, 0);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON);
        HideBg(0);
        ShowBg(1);
        ShowBg(2);
        ShowBg(3);
        gMain.state++;
        break;
    case 7:
        if (!gPaletteFade.active)
            gMain.state++;
        break;
    case 8:
        gMain.state++;
        break;
    case 9:
        sPokedexView->screenSwitchState = 0;
        gTasks[taskId].data[0] = 0;
        gTasks[taskId].data[1] = 0;
        gTasks[taskId].data[2] = 1;
        gTasks[taskId].func = Task_HandleEvolutionScreenInput;
        gMain.state = 0;
        break;
    }
}

static void Task_HandleEvolutionScreenInput(u8 taskId)
{
    if (sPokedexView->sEvoScreenData.numAllEvolutions != 0 && sPokedexView->sEvoScreenData.numSeen != 0)
    {
        u8 i;
        u8 base_y = 58;
        u8 base_y_offset = 9;
        u8 pos = sPokedexView->sEvoScreenData.menuPos;
        u8 max = sPokedexView->sEvoScreenData.numAllEvolutions;
        if (JOY_NEW(DPAD_DOWN))
        {
            while (TRUE)
            {
                pos += 1;
                if (pos >= max)
                    pos = 0;

                if (sPokedexView->sEvoScreenData.seen[pos] == TRUE)
                    break;
            }
            gSprites[sPokedexView->sEvoScreenData.arrowSpriteId].y = base_y + base_y_offset * pos;
            sPokedexView->sEvoScreenData.menuPos = pos;
        }
        else if (JOY_NEW(DPAD_UP))
        {
            if (sPokedexView->sEvoScreenData.menuPos == 0)
                sPokedexView->sEvoScreenData.menuPos = sPokedexView->sEvoScreenData.numAllEvolutions - 1;
            else
                sPokedexView->sEvoScreenData.menuPos -= 1;

            gSprites[sPokedexView->sEvoScreenData.arrowSpriteId].y = base_y + base_y_offset * sPokedexView->sEvoScreenData.menuPos;
        }

        if (JOY_NEW(A_BUTTON))
        {
            if (sPokedexView->isSearchResults && sPokedexView->originalSearchSelectionNum == 0)
                sPokedexView->originalSearchSelectionNum = sPokedexListItem->dexNum;

            u16 targetSpecies   = sPokedexView->sEvoScreenData.targetSpecies[sPokedexView->sEvoScreenData.menuPos];
            u16 dexNum          = SpeciesToNationalPokedexNum(targetSpecies);
            sPokedexListItem->dexNum = dexNum;
            sPokedexListItem->seen   = GetSetPokedexFlag(dexNum, FLAG_GET_SEEN);
            sPokedexListItem->owned  = GetSetPokedexFlag(dexNum, FLAG_GET_CAUGHT);

            sPokedexView->sEvoScreenData.fromEvoPage = TRUE;
            PlaySE(SE_PIN);
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
            gTasks[taskId].func = Task_LoadInfoScreenWaitForFade;
        }
    }

    //Exit to overview
    if (JOY_NEW(B_BUTTON))
    {
        BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
        // Use special exit handler if called from party menu
        if (sExternalReturnCallback != NULL)
            gTasks[taskId].func = Task_ExitEvolutionScreenToExternal;
        else
            gTasks[taskId].func = Task_ExitEvolutionScreen;
        PlaySE(SE_PC_OFF);
        return;
    }

    //Switch screens
    if ((JOY_NEW(DPAD_LEFT) || (JOY_NEW(L_BUTTON) && gSaveBlock2Ptr->optionsButtonMode == OPTIONS_BUTTON_MODE_LR)))
    {
        sPokedexView->selectedScreen = STATS_SCREEN;
        BeginNormalPaletteFade(0xFFFFFFEB, 0, 0, 0x10, RGB_BLACK);
        sPokedexView->screenSwitchState = 1;
        gTasks[taskId].func = Task_SwitchScreensFromEvolutionScreen;
        PlaySE(SE_PIN);
    }
    if ((JOY_NEW(DPAD_RIGHT) || (JOY_NEW(R_BUTTON) && gSaveBlock2Ptr->optionsButtonMode == OPTIONS_BUTTON_MODE_LR)))
    {
        if (!sPokedexListItem->owned)
            PlaySE(SE_FAILURE);
        else
        {
            sPokedexView->selectedScreen = CRY_SCREEN;
            BeginNormalPaletteFade(0xFFFFFFEB, 0, 0, 0x10, RGB_BLACK);
            sPokedexView->screenSwitchState = 2;
            gTasks[taskId].func = Task_SwitchScreensFromEvolutionScreen;
            PlaySE(SE_PIN);
        }
    }
}

static void HandleTargetSpeciesPrint(u8 taskId, u16 targetSpecies, u16 previousTargetSpecies, u8 base_x, u8 base_y, u8 base_y_offset, u8 base_i, bool8 isEevee)
{
    u8 iterations = 6;
    bool8 seen = GetSetPokedexFlag(SpeciesToNationalPokedexNum(targetSpecies), FLAG_GET_SEEN);

    if (seen || !HGSS_HIDE_UNSEEN_EVOLUTION_NAMES)
        StringCopy(gStringVar3, gSpeciesNames[targetSpecies]); //evolution mon name
    else
        StringCopy(gStringVar3, gText_ThreeQuestionMarks); //show questionmarks instead of name
    StringExpandPlaceholders(gStringVar3, gText_EVO_Name); //evolution mon name
    PrintInfoScreenTextSmall(gStringVar3, base_x, base_y + base_y_offset*base_i); //evolution mon name

    //Print mon icon in the top row
    if (isEevee)
    {
        iterations = 9;
        if (targetSpecies == previousTargetSpecies)
            return;
        /*else if (targetSpecies == SPECIES_GLACEON)
            base_i -= 1;
        else if (targetSpecies == SPECIES_SYLVEON)
            base_i -= 2;*/
    }

    if (base_i < iterations)
    {
        LoadMonIconPalette(targetSpecies); //Loads pallete for current mon
        if (isEevee)
            gTasks[taskId].data[4+base_i] = CreateMonIcon(targetSpecies, SpriteCB_MonIcon, 45 + 26*base_i, 31, 4, 0, TRUE); //Create pokemon sprite
        else
            gTasks[taskId].data[4+base_i] = CreateMonIcon(targetSpecies, SpriteCB_MonIcon, 50 + 32*base_i, 31, 4, 0, TRUE); //Create pokemon sprite
        gSprites[gTasks[taskId].data[4+base_i]].oam.priority = 0;
    }
}

static void CreateCaughtBallEvolutionScreen(u16 targetSpecies, u8 x, u8 y, u16 unused)
{
    bool8 owned = GetSetPokedexFlag(SpeciesToNationalPokedexNum(targetSpecies), FLAG_GET_CAUGHT);
    if (owned)
        BlitBitmapToWindow(0, sCaughtBall_Gfx, x, y-1, 8, 16);
    else
    {
        //FillWindowPixelRect(0, PIXEL_FILL(0), x, y, 8, 16); //not sure why this was even here
        PrintInfoScreenTextSmall(gText_OneDash, x+1, y-1);
    }
}

static void HandlePreEvolutionSpeciesPrint(u8 taskId, u16 preSpecies, u16 species, u8 base_x, u8 base_y, u8 base_y_offset, u8 base_i)
{
    bool8 seen = GetSetPokedexFlag(SpeciesToNationalPokedexNum(preSpecies), FLAG_GET_SEEN);

    StringCopy(gStringVar1, gSpeciesNames[species]); //evolution mon name

    #ifdef POKEMON_EXPANSION
    if (sPokedexView->sEvoScreenData.isMega)
        StringExpandPlaceholders(gStringVar3, gText_EVO_PreEvo_PE_Mega);
    else
    {
    #endif

        if (seen || !HGSS_HIDE_UNSEEN_EVOLUTION_NAMES)
            StringCopy(gStringVar2, gSpeciesNames[preSpecies]); //evolution mon name
        else
            StringCopy(gStringVar2, gText_ThreeQuestionMarks); //show questionmarks instead of name

        StringExpandPlaceholders(gStringVar3, gText_EVO_PreEvo); //evolution mon name

    #ifdef POKEMON_EXPANSION
    }
    #endif

    PrintInfoScreenTextSmall(gStringVar3, base_x, base_y + base_y_offset*base_i); //evolution mon name

    if (base_i < 3)
    {
        LoadMonIconPalette(preSpecies); //Loads pallete for current mon
        #ifndef POKEMON_EXPANSION
            gTasks[taskId].data[4+base_i] = CreateMonIcon(preSpecies, SpriteCB_MonIcon, 18 + 32*base_i, 31, 4, 0, TRUE); //Create pokemon sprite
        #endif
        #ifdef POKEMON_EXPANSION
            gTasks[taskId].data[4+base_i] = CreateMonIcon(preSpecies, SpriteCB_MonIcon, 18 + 32*base_i, 31, 4, 0); //Create pokemon sprite
        #endif
        gSprites[gTasks[taskId].data[4+base_i]].oam.priority = 0;
    }
}

static u8 PrintPreEvolutions(u8 taskId, u16 species)
{
    u16 i;
    u16 j;

    u8 base_x = 13+8;
    u8 base_y = 51;
    u8 base_y_offset = 9;
    u8 base_i = 0;
    u8 depth_x = 16;

    u16 preEvolutionOne = 0;
    u16 preEvolutionTwo = 0;
    u8 numPreEvolutions = 0;

    #ifdef POKEMON_EXPANSION
    bool8 isMega = FALSE;
    sPokedexView->sEvoScreenData.isMega = FALSE;
    #endif

    #ifdef TX_RANDOMIZER_AND_CHALLENGES
    if (gSaveBlock1Ptr->tx_Random_Evolutions || gSaveBlock1Ptr->tx_Random_EvolutionMethods)
        return 0;
    #endif


    //Calculate previous evolution
    for (i = 0; i < NUM_SPECIES; i++)
    {
        for (j = 0; j < EVOS_PER_MON; j++)
        {
            if (gEvolutionTable[i][j].targetSpecies == species)
            {
                preEvolutionOne = i;
                numPreEvolutions += 1;
                #ifdef POKEMON_EXPANSION
                    if (gEvolutionTable[i][j].method == EVO_MEGA_EVOLUTION)
                    {
                        CopyItemName(gEvolutionTable[i][j].param, gStringVar2); //item
                        isMega = TRUE;
                    }
                #endif
                break;
            }
        }
    }

    #ifdef POKEMON_EXPANSION
    if (isMega)
    {
        sPokedexView->numPreEvolutions = numPreEvolutions;
        sPokedexView->sEvoScreenData.numAllEvolutions += numPreEvolutions;
        sPokedexView->sEvoScreenData.isMega = isMega;

        CreateCaughtBallEvolutionScreen(preEvolutionOne, base_x - 9 - 8, base_y + base_y_offset*(numPreEvolutions - 1), 0);
        HandlePreEvolutionSpeciesPrint(taskId, preEvolutionOne, species, base_x - 8, base_y, base_y_offset, numPreEvolutions - 1);
        return numPreEvolutions;
    }
    #endif

    //Calculate if previous evolution also has a previous evolution
    if (numPreEvolutions != 0)
    {
        for (i = 0; i < NUM_SPECIES; i++)
        {
            for (j = 0; j < EVOS_PER_MON; j++)
            {
                if (gEvolutionTable[i][j].targetSpecies == preEvolutionOne)
                {
                    preEvolutionTwo = i;
                    numPreEvolutions += 1;
                    CreateCaughtBallEvolutionScreen(preEvolutionTwo, base_x - 9, base_y + base_y_offset*0, 0);
                    HandlePreEvolutionSpeciesPrint(taskId, preEvolutionTwo, preEvolutionOne, base_x, base_y, base_y_offset, 0);
                    break;
                }
            }
        }
    }

    //Print ball and name
    if (preEvolutionOne != 0)
    {
        CreateCaughtBallEvolutionScreen(preEvolutionOne, base_x - 9, base_y + base_y_offset*(numPreEvolutions - 1), 0);
        HandlePreEvolutionSpeciesPrint(taskId, preEvolutionOne, species, base_x, base_y, base_y_offset, numPreEvolutions - 1);
    }

    if (preEvolutionTwo != 0)
    {
        sPokedexView->sEvoScreenData.targetSpecies[0] = preEvolutionTwo;
        sPokedexView->sEvoScreenData.targetSpecies[1] = preEvolutionOne;
    }
    else if (preEvolutionOne != 0)
    {
        sPokedexView->sEvoScreenData.targetSpecies[0] = preEvolutionOne;
    }

    //vertical line
    //FillWindowPixelRect(0, PIXEL_FILL(5), 33 + 32*numPreEvolutions, 18, 1, 32); //PIXEL_FILL(15) for black

    sPokedexView->numPreEvolutions = numPreEvolutions;
    sPokedexView->sEvoScreenData.numAllEvolutions += numPreEvolutions;

    return numPreEvolutions;
}

#define EVO_SCREEN_LVL_DIGITS 2

static u8 PrintEvolutionTargetSpeciesAndMethod(u8 taskId, u16 species, u8 depth, u8 depth_i)
{
    u16 i;
    #ifdef POKEMON_EXPANSION
        u16 j;
        const struct MapHeader *mapHeader;
    #endif
    u16 targetSpecies = 0;
    u16 previousTargetSpecies = 0;

    u16 item;

    bool8 left = TRUE;
    u8 base_x = 13+8;
    u8 base_x_offset = 54+8;
    u8 base_y = 51;
    u8 base_y_offset = 9;
    u8 base_i = 0;
    u8 times = 0;
    u8 depth_x = 16;
    bool8 isEevee = FALSE;

    #ifdef POKEMON_EXPANSION
    if (sPokedexView->sEvoScreenData.isMega)
        return 0;
    #endif

    StringCopy(gStringVar1, gSpeciesNames[species]);

    if (species == SPECIES_EEVEE)
        isEevee = TRUE;

    #ifdef TX_RANDOMIZER_AND_CHALLENGES
    if (EvolutionBlockedByEvoLimit(species)) //No Evos already previously checked
        species = SPECIES_NONE;
    else if (gSaveBlock1Ptr->tx_Random_EvolutionMethods)
        species = GetSpeciesRandomSeeded(species, TX_RANDOM_T_EVO_METH, 0);
    #endif

    //Calculate number of possible direct evolutions (e.g. Eevee has 5 but torchic has 1)
    for (i = 0; i < EVOS_PER_MON; i++)
    {
        #ifndef POKEMON_EXPANSION
            if (gEvolutionTable[species][i].method != 0)
                times += 1;
        #endif
        #ifdef POKEMON_EXPANSION
            if (gEvolutionTable[species][i].method != 0 && gEvolutionTable[species][i].method != EVO_MEGA_EVOLUTION)
                times += 1;
        #endif
    }
    gTasks[taskId].data[3] = times;
    sPokedexView->sEvoScreenData.numAllEvolutions += times;

    //If there are no evolutions print text
    if (times == 0 && depth == 0)
    {
        StringExpandPlaceholders(gStringVar4, gText_EVO_NONE);
        PrintInfoScreenTextSmall(gStringVar4, base_x-7-7, base_y + base_y_offset*depth_i);
    }

    //If there are evolutions find out which and print them 1 by 1
    for (i = 0; i < times; i++)
    {
        base_i = i + depth_i;
        left = !left;

        previousTargetSpecies = targetSpecies;
        targetSpecies = gEvolutionTable[species][i].targetSpecies;
        sPokedexView->sEvoScreenData.targetSpecies[base_i] = targetSpecies;
        #ifdef TX_RANDOMIZER_AND_CHALLENGES
        if (gSaveBlock1Ptr->tx_Random_Evolutions && targetSpecies != SPECIES_NONE) //tx_difficulty_challenges
            targetSpecies = GetSpeciesRandomSeeded(targetSpecies, TX_RANDOM_T_EVO, 0);
        #endif
        CreateCaughtBallEvolutionScreen(targetSpecies, base_x + depth_x*depth-9, base_y + base_y_offset*base_i, 0);
        HandleTargetSpeciesPrint(taskId, targetSpecies, previousTargetSpecies, base_x + depth_x*depth, base_y, base_y_offset, base_i, isEevee); //evolution mon name

        switch (gEvolutionTable[species][i].method)
        {
        case EVO_FRIENDSHIP:
            ConvertIntToDecimalStringN(gStringVar2, 220, STR_CONV_MODE_LEADING_ZEROS, 3); //friendship value
            StringExpandPlaceholders(gStringVar4, gText_EVO_FRIENDSHIP );
            break;
        case EVO_FRIENDSHIP_DAY:
            StringExpandPlaceholders(gStringVar4, gText_EVO_FRIENDSHIP_DAY );
            break;
        case EVO_FRIENDSHIP_NIGHT:
            StringExpandPlaceholders(gStringVar4, gText_EVO_FRIENDSHIP_NIGHT );
            break;
        case EVO_LEVEL:
            ConvertIntToDecimalStringN(gStringVar2, gEvolutionTable[species][i].param, STR_CONV_MODE_LEADING_ZEROS, EVO_SCREEN_LVL_DIGITS); //level
            StringExpandPlaceholders(gStringVar4, gText_EVO_LEVEL );
            break;
        case EVO_TRADE:
            StringExpandPlaceholders(gStringVar4, gText_EVO_TRADE );
            break;
        case EVO_TRADE_ITEM:
            item = gEvolutionTable[species][i].param; //item
            CopyItemName(item, gStringVar2); //item
            StringExpandPlaceholders(gStringVar4, gText_EVO_TRADE_ITEM );
            break;
        case EVO_ITEM:
            item = gEvolutionTable[species][i].param;
            CopyItemName(item, gStringVar2);
            StringExpandPlaceholders(gStringVar4, gText_EVO_ITEM );
            break;
        case EVO_LEVEL_ATK_GT_DEF:
            ConvertIntToDecimalStringN(gStringVar2, gEvolutionTable[species][i].param, STR_CONV_MODE_LEADING_ZEROS, EVO_SCREEN_LVL_DIGITS); //level
            StringExpandPlaceholders(gStringVar4, gText_EVO_LEVEL_ATK_GT_DEF );
            break;
        case EVO_LEVEL_ATK_EQ_DEF:
            ConvertIntToDecimalStringN(gStringVar2, gEvolutionTable[species][i].param, STR_CONV_MODE_LEADING_ZEROS, EVO_SCREEN_LVL_DIGITS); //level
            StringExpandPlaceholders(gStringVar4, gText_EVO_LEVEL_ATK_EQ_DEF );
            break;
        case EVO_LEVEL_ATK_LT_DEF:
            ConvertIntToDecimalStringN(gStringVar2, gEvolutionTable[species][i].param, STR_CONV_MODE_LEADING_ZEROS, EVO_SCREEN_LVL_DIGITS); //level
            StringExpandPlaceholders(gStringVar4, gText_EVO_LEVEL_ATK_LT_DEF );
            break;
        case EVO_LEVEL_SILCOON:
            ConvertIntToDecimalStringN(gStringVar2, gEvolutionTable[species][i].param, STR_CONV_MODE_LEADING_ZEROS, EVO_SCREEN_LVL_DIGITS); //level
            StringExpandPlaceholders(gStringVar4, gText_EVO_LEVEL_SILCOON );
            break;
        case EVO_LEVEL_CASCOON:
            ConvertIntToDecimalStringN(gStringVar2, gEvolutionTable[species][i].param, STR_CONV_MODE_LEADING_ZEROS, EVO_SCREEN_LVL_DIGITS); //level
            StringExpandPlaceholders(gStringVar4, gText_EVO_LEVEL_CASCOON );
            break;
        case EVO_LEVEL_NINJASK:
            ConvertIntToDecimalStringN(gStringVar2, gEvolutionTable[species][i].param, STR_CONV_MODE_LEADING_ZEROS, EVO_SCREEN_LVL_DIGITS); //level
            StringExpandPlaceholders(gStringVar4, gText_EVO_LEVEL_NINJASK );
            break;
        case EVO_LEVEL_SHEDINJA:
            ConvertIntToDecimalStringN(gStringVar2, gEvolutionTable[species][i].param, STR_CONV_MODE_LEADING_ZEROS, EVO_SCREEN_LVL_DIGITS); //level
            StringExpandPlaceholders(gStringVar4, gText_EVO_LEVEL_SHEDINJA );
            break;
        case EVO_BEAUTY:
            ConvertIntToDecimalStringN(gStringVar2, gEvolutionTable[species][i].param, STR_CONV_MODE_LEADING_ZEROS, 3); //beauty
            StringExpandPlaceholders(gStringVar4, gText_EVO_BEAUTY );
            break;
        case EVO_LEVEL_FEMALE:
            ConvertIntToDecimalStringN(gStringVar2, gEvolutionTable[species][i].param, STR_CONV_MODE_LEADING_ZEROS, EVO_SCREEN_LVL_DIGITS); //level
            StringExpandPlaceholders(gStringVar4, gText_EVO_LEVEL_FEMALE );
            break;
        case EVO_LEVEL_MALE:
            ConvertIntToDecimalStringN(gStringVar2, gEvolutionTable[species][i].param, STR_CONV_MODE_LEADING_ZEROS, EVO_SCREEN_LVL_DIGITS); //level
            StringExpandPlaceholders(gStringVar4, gText_EVO_LEVEL_MALE );
            break;
        case EVO_LEVEL_NIGHT:
            ConvertIntToDecimalStringN(gStringVar2, gEvolutionTable[species][i].param, STR_CONV_MODE_LEADING_ZEROS, EVO_SCREEN_LVL_DIGITS); //level
            StringExpandPlaceholders(gStringVar4, gText_EVO_LEVEL_NIGHT );
            break;
        case EVO_LEVEL_DAY:
            ConvertIntToDecimalStringN(gStringVar2, gEvolutionTable[species][i].param, STR_CONV_MODE_LEADING_ZEROS, EVO_SCREEN_LVL_DIGITS); //level
            StringExpandPlaceholders(gStringVar4, gText_EVO_LEVEL_DAY );
            break;
        case EVO_ITEM_HOLD_DAY:
            item = gEvolutionTable[species][i].param; //item
            CopyItemName(item, gStringVar2); //item
            StringExpandPlaceholders(gStringVar4, gText_EVO_ITEM_HOLD_DAY );
            break;
        case EVO_ITEM_HOLD_NIGHT:
            item = gEvolutionTable[species][i].param; //item
            CopyItemName(item, gStringVar2); //item
            StringExpandPlaceholders(gStringVar4, gText_EVO_ITEM_HOLD_NIGHT );
            break;
        case EVO_MOVE:
            StringCopy(gStringVar2, gMoveNames[gEvolutionTable[species][i].param]);
            StringExpandPlaceholders(gStringVar4, gText_EVO_MOVE );
            break;
        case EVO_MOVE_TYPE:
            StringCopy(gStringVar2, gTypeNames[gEvolutionTable[species][i].param]);
            StringExpandPlaceholders(gStringVar4, gText_EVO_MOVE_TYPE );
            break;
        case EVO_LEVEL_FEMALE_MORNING:
            ConvertIntToDecimalStringN(gStringVar2, gEvolutionTable[species][i].param, STR_CONV_MODE_LEADING_ZEROS, EVO_SCREEN_LVL_DIGITS); //level
            StringExpandPlaceholders(gStringVar4, gText_EVO_LEVEL_TIME_FEMALE );
            break;
        case EVO_LEVEL_MALE_MORNING:
            ConvertIntToDecimalStringN(gStringVar2, gEvolutionTable[species][i].param, STR_CONV_MODE_LEADING_ZEROS, EVO_SCREEN_LVL_DIGITS); //level
            StringExpandPlaceholders(gStringVar4, gText_EVO_LEVEL_TIME_MALE );
            break;
        case EVO_ITEM_HOLD:
            item = gEvolutionTable[species][i].param; //item
            CopyItemName(item, gStringVar2); //item
            StringExpandPlaceholders(gStringVar4, gText_EVO_ITEM_HOLD );
            break;
        default:
            StringExpandPlaceholders(gStringVar4, gText_EVO_UNKNOWN );
            break;
        }//Switch end
        PrintInfoScreenTextSmall(gStringVar4, base_x + depth_x*depth+base_x_offset, base_y + base_y_offset*base_i); //Print actual instructions

        depth_i += PrintEvolutionTargetSpeciesAndMethod(taskId, targetSpecies, depth+1, base_i+1);
    }//For loop end

    return times;
}

static void Task_SwitchScreensFromEvolutionScreen(u8 taskId)
{
    u8 i;
    if (!gPaletteFade.active)
    {
        FreeMonIconPalettes();                                          //Destroy pokemon icon sprite
        FreeAndDestroyMonIconSprite(&gSprites[gTasks[taskId].data[4]]); //Destroy pokemon icon sprite
        for (i = 1; i <= gTasks[taskId].data[3]; i++)
        {
            FreeAndDestroyMonIconSprite(&gSprites[gTasks[taskId].data[4+i]]); //Destroy pokemon icon sprite
        }
        FreeAndDestroyMonPicSprite(gTasks[taskId].tMonSpriteId);

        switch (sPokedexView->screenSwitchState)
        {
        case 1:
            gTasks[taskId].func = Task_LoadStatsScreen;
            break;
        case 2:
            gTasks[taskId].func = Task_LoadCryScreen;
            break;
        #ifdef POKEMON_EXPANSION
            case 3:
                gTasks[taskId].func = Task_LoadFormsScreen;
                break;
        #endif
        default:
            FreeAllWindowBuffers();
            InitWindows(sInfoScreen_WindowTemplates);
            gTasks[taskId].func = Task_LoadInfoScreen;
            break;
        }
    }
}

static void Task_ExitEvolutionScreen(u8 taskId)
{
    u8 i;
    if (!gPaletteFade.active)
    {
        FreeMonIconPalettes();                                          //Destroy pokemon icon sprite
        FreeAndDestroyMonIconSprite(&gSprites[gTasks[taskId].data[4]]); //Destroy pokemon icon sprite
        for (i = 1; i <= gTasks[taskId].data[3]; i++)
        {
            FreeAndDestroyMonIconSprite(&gSprites[gTasks[taskId].data[4+i]]); //Destroy pokemon icon sprite
        }
        FreeAndDestroyMonPicSprite(gTasks[taskId].tMonSpriteId);

        FreeInfoScreenWindowAndBgBuffers();
        DestroyTask(taskId);
    }
}

static void Task_ExitEvolutionScreenToExternal(u8 taskId)
{
    u8 i;
    if (!gPaletteFade.active)
    {
        FreeMonIconPalettes();                                          //Destroy pokemon icon sprite
        FreeAndDestroyMonIconSprite(&gSprites[gTasks[taskId].data[4]]); //Destroy pokemon icon sprite
        for (i = 1; i <= gTasks[taskId].data[3]; i++)
        {
            FreeAndDestroyMonIconSprite(&gSprites[gTasks[taskId].data[4+i]]); //Destroy pokemon icon sprite
        }
        FreeAndDestroyMonPicSprite(gTasks[taskId].tMonSpriteId);

        FreeInfoScreenWindowAndBgBuffers();
        DestroyTask(taskId);
        if (sExternalReturnCallback != NULL)
            SetMainCallback2(sExternalReturnCallback);
    }
}

//************************************
//*                                  *
//*        Cry screen                *
//*                                  *
//************************************
#define tScrolling       data[0]
#define tMonSpriteDone   data[1]
#define tBgLoaded        data[2]
#define tSkipCry         data[3]
#define tMonSpriteId     data[4]
#define tTrainerSpriteId data[5]

static void Task_LoadCryScreen(u8 taskId)
{
    switch (gMain.state)
    {
    case 0:
    default:
        if (!gPaletteFade.active)
        {
            m4aMPlayStop(&gMPlayInfo_BGM);
            sPokedexView->currentPage = PAGE_CRY;
            gPokedexVBlankCB = gMain.vblankCallback;
            SetVBlankCallback(NULL);
            ResetOtherVideoRegisters(DISPCNT_BG1_ON);
            sPokedexView->selectedScreen = CRY_SCREEN;
            gMain.state = 1;
        }
        break;
    case 1:
        LoadTilesetTilemapHGSS(CRY_SCREEN);
        FillWindowPixelBuffer(WIN_EVO_DISPLAY, PIXEL_FILL(0));
        PutWindowTilemap(WIN_EVO_DISPLAY);
        PutWindowTilemap(WIN_VU_METER);
        PutWindowTilemap(WIN_CRY_WAVE);
        gMain.state++;
        break;
    case 2:
        LoadScreenSelectBarMain(0xD); //HGSS_Ui
        LoadPokedexBgPalette(sPokedexView->isSearchResults, false);
        gMain.state++;
        break;
    case 3:
        ResetPaletteFade();
        gMain.state++;
        break;
    case 4:
        PrintInfoScreenText(WIN_EVO_DISPLAY, gText_CryOf, 82, 33, 0);
        PrintCryScreenSpeciesName(0, sPokedexListItem->dexNum, 82, 49);
        gMain.state++;
        break;
    case 5:
        gTasks[taskId].tMonSpriteId = CreateMonSpriteFromNationalDexNumber(sPokedexListItem->dexNum, MON_PAGE_X, MON_PAGE_Y, 0);
        gSprites[gTasks[taskId].tMonSpriteId].oam.priority = 0;
        gDexCryScreenState = 0;
        gMain.state++;
        break;
    case 6:
        {
            struct CryScreenWindow waveformWindow;

            waveformWindow.unk0 = 0x4020;
            waveformWindow.unk2 = 31;
            waveformWindow.paletteNo = 8;
            waveformWindow.yPos = 30;
            waveformWindow.xPos = 12;
            if (LoadCryWaveformWindow(&waveformWindow, 2))
            {
                gMain.state++;
                gDexCryScreenState = 0;
            }
        }
        break;
    case 7:
        {
            struct CryScreenWindow cryMeter;

            cryMeter.paletteNo = 9;
            cryMeter.xPos = 18;
            cryMeter.yPos = 3;
            if (LoadCryMeter(&cryMeter, 3))
                gMain.state++;
            CopyWindowToVram(WIN_VU_METER, COPYWIN_GFX);
            CopyWindowToVram(WIN_EVO_DISPLAY, COPYWIN_FULL);
            CopyBgTilemapBufferToVram(0);
            CopyBgTilemapBufferToVram(1);
            CopyBgTilemapBufferToVram(2);
            CopyBgTilemapBufferToVram(3);
        }
        break;
    case 8:
        BeginNormalPaletteFade(PALETTES_ALL & ~(0x14), 0, 0x10, 0, RGB_BLACK);
        SetVBlankCallback(gPokedexVBlankCB);
        gMain.state++;
        break;
    case 9:
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        SetGpuReg(REG_OFFSET_BLDY, 0);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON);
        ShowBg(0);
        ShowBg(1);
        ShowBg(2);
        ShowBg(3);
        gMain.state++;
        break;
    case 10:
        sPokedexView->screenSwitchState = 0;
        gMain.state = 0;
        gTasks[taskId].func = Task_HandleCryScreenInput;
        break;
    }
}

static void Task_HandleCryScreenInput(u8 taskId)
{
    UpdateCryWaveformWindow(2);

    if (IsCryPlaying())
        LoadPlayArrowPalette(TRUE);
    else
        LoadPlayArrowPalette(FALSE);

    if (JOY_NEW(A_BUTTON))
    {
        LoadPlayArrowPalette(TRUE);
        CryScreenPlayButton(NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum));
        return;
    }
    else if (!gPaletteFade.active)
    {
        if (JOY_NEW(B_BUTTON))
        {
            BeginNormalPaletteFade(PALETTES_ALL & ~(0x14), 0, 0, 0x10, RGB_BLACK);
            m4aMPlayContinue(&gMPlayInfo_BGM);
            // Use special exit handler if called from party menu
            if (sExternalReturnCallback != NULL)
                gTasks[taskId].func = Task_ExitCryScreenToExternal;
            else
                gTasks[taskId].func = Task_ExitCryScreen;
            PlaySE(SE_PC_OFF);
            return;
        }
        if (JOY_NEW(DPAD_LEFT)
         || (JOY_NEW(L_BUTTON) && gSaveBlock2Ptr->optionsButtonMode == OPTIONS_BUTTON_MODE_LR))
        {
            BeginNormalPaletteFade(PALETTES_ALL & ~(0x14), 0, 0, 0x10, RGB_BLACK);
            m4aMPlayContinue(&gMPlayInfo_BGM);
            sPokedexView->screenSwitchState = 2;
            gTasks[taskId].func = Task_SwitchScreensFromCryScreen;
            PlaySE(SE_DEX_PAGE);
            return;
        }
        if (JOY_NEW(DPAD_RIGHT)
         || (JOY_NEW(R_BUTTON) && gSaveBlock2Ptr->optionsButtonMode == OPTIONS_BUTTON_MODE_LR))
        {
            if (!sPokedexListItem->owned)
            {
                PlaySE(SE_FAILURE);
            }
            else
            {
                BeginNormalPaletteFade(PALETTES_ALL & ~(0x14), 0, 0, 0x10, RGB_BLACK);
                m4aMPlayContinue(&gMPlayInfo_BGM);
                sPokedexView->screenSwitchState = 3;
                gTasks[taskId].func = Task_SwitchScreensFromCryScreen;
                PlaySE(SE_DEX_PAGE);
            }
            return;
        }
    }
}

static void Task_SwitchScreensFromCryScreen(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        FreeCryScreen();
        FreeAndDestroyMonPicSprite(gTasks[taskId].tMonSpriteId);
        switch (sPokedexView->screenSwitchState)
        {
        default:
        case 1:
            gTasks[taskId].func = Task_LoadInfoScreen;
            break;
        case 2:
            gTasks[taskId].func = Task_LoadEvolutionScreen;
            break;
        case 3:
            gTasks[taskId].func = Task_LoadSizeScreen;
            break;
        }
    }
}

static void Task_ExitCryScreen(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        FreeCryScreen();
        FreeAndDestroyMonPicSprite(gTasks[taskId].tMonSpriteId);
        FreeInfoScreenWindowAndBgBuffers();
        DestroyTask(taskId);
    }
}

static void Task_ExitCryScreenToExternal(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        FreeCryScreen();
        FreeAndDestroyMonPicSprite(gTasks[taskId].tMonSpriteId);
        FreeInfoScreenWindowAndBgBuffers();
        DestroyTask(taskId);
        if (sExternalReturnCallback != NULL)
            SetMainCallback2(sExternalReturnCallback);
    }
}



//************************************
//*                                  *
//*        Size screen               *
//*                                  *
//************************************
static void Task_LoadSizeScreen(u8 taskId)
{
    u8 spriteId;

    switch (gMain.state)
    {
    default:
    case 0:
        if (!gPaletteFade.active)
        {
            sPokedexView->currentPage = PAGE_SIZE;
            gPokedexVBlankCB = gMain.vblankCallback;
            SetVBlankCallback(NULL);
            ResetOtherVideoRegisters(DISPCNT_BG1_ON);
            sPokedexView->selectedScreen = SIZE_SCREEN;
            gMain.state = 1;
        }
        break;
    case 1:
        LoadTilesetTilemapHGSS(SIZE_SCREEN);
        FillWindowPixelBuffer(WIN_EVO_DISPLAY, PIXEL_FILL(0));
        PutWindowTilemap(WIN_EVO_DISPLAY);
        gMain.state++;
        break;
    case 2:
        LoadScreenSelectBarMain(0xD); //HGSS_Ui
        LoadPokedexBgPalette(sPokedexView->isSearchResults, false);
        gMain.state++;
        break;
    case 3:
        {
            u8 string[64];

            StringCopy(string, gText_SizeComparedTo);
            StringAppend(string, gSaveBlock2Ptr->playerName);
            PrintInfoScreenText(WIN_EVO_DISPLAY, string, GetStringCenterAlignXOffset(FONT_NORMAL, string, 0xF0), 0x79, 0);
            gMain.state++;
        }
        break;
    case 4:
        ResetPaletteFade();
        gMain.state++;
        break;
    case 5:
        spriteId = CreateSizeScreenTrainerPic(PlayerGenderToFrontTrainerPicId(gSaveBlock2Ptr->playerGender), 152, 56, 0);
        gSprites[spriteId].oam.affineMode = ST_OAM_AFFINE_NORMAL;
        gSprites[spriteId].oam.matrixNum = 1;
        gSprites[spriteId].oam.priority = 0;
        gSprites[spriteId].y2 = gPokedexEntries[sPokedexListItem->dexNum].trainerOffset;
        SetOamMatrix(1, gPokedexEntries[sPokedexListItem->dexNum].trainerScale, 0, 0, gPokedexEntries[sPokedexListItem->dexNum].trainerScale);
        LoadPalette(sSizeScreenSilhouette_Pal, OBJ_PLTT_ID2(gSprites[spriteId].oam.paletteNum), PLTT_SIZE_4BPP);
        gTasks[taskId].tTrainerSpriteId = spriteId;
        gMain.state++;
        break;
    case 6:
        spriteId = CreateMonSpriteFromNationalDexNumber(sPokedexListItem->dexNum, 88, 56, 1);
        gSprites[spriteId].oam.affineMode = ST_OAM_AFFINE_NORMAL;
        gSprites[spriteId].oam.matrixNum = 2;
        gSprites[spriteId].oam.priority = 0;
        gSprites[spriteId].y2 = gPokedexEntries[sPokedexListItem->dexNum].pokemonOffset;
        SetOamMatrix(2, gPokedexEntries[sPokedexListItem->dexNum].pokemonScale, 0, 0, gPokedexEntries[sPokedexListItem->dexNum].pokemonScale);
        LoadPalette(sSizeScreenSilhouette_Pal, OBJ_PLTT_ID2(gSprites[spriteId].oam.paletteNum), PLTT_SIZE_4BPP);
        gTasks[taskId].tMonSpriteId = spriteId;
        CopyWindowToVram(WIN_EVO_DISPLAY, COPYWIN_FULL);
        CopyBgTilemapBufferToVram(1);
        CopyBgTilemapBufferToVram(2);
        CopyBgTilemapBufferToVram(3);
        gMain.state++;
        break;
    case 7:
        BeginNormalPaletteFade(PALETTES_ALL & ~(0x14), 0, 0x10, 0, RGB_BLACK);
        SetVBlankCallback(gPokedexVBlankCB);
        gMain.state++;
        break;
    case 8:
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        SetGpuReg(REG_OFFSET_BLDY, 0);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON);
        HideBg(0);
        ShowBg(1);
        ShowBg(2);
        ShowBg(3);
        gMain.state++;
        break;
    case 9:
        if (!gPaletteFade.active)
        {
            sPokedexView->screenSwitchState = 0;
            gMain.state = 0;
            gTasks[taskId].func = Task_HandleSizeScreenInput;
        }
        break;
    }
}

static void LoadPlayArrowPalette(bool8 cryPlaying)
{
    u16 color;

    if (cryPlaying)
        color = RGB(18, 28, 0);
    else
        color = RGB(15, 21, 0);
    LoadPalette(&color, BG_PLTT_ID(5) + 13, PLTT_SIZEOF(1));
}

static void Task_HandleSizeScreenInput(u8 taskId)
{
    if (JOY_NEW(B_BUTTON))
    {
        BeginNormalPaletteFade(PALETTES_ALL & ~(0x14), 0, 0, 0x10, RGB_BLACK);
        // Use special exit handler if called from party menu
        if (sExternalReturnCallback != NULL)
            gTasks[taskId].func = Task_ExitSizeScreenToExternal;
        else
            gTasks[taskId].func = Task_ExitSizeScreen;
        PlaySE(SE_PC_OFF);
    }
    else if (JOY_NEW(DPAD_LEFT)
     || (JOY_NEW(L_BUTTON) && gSaveBlock2Ptr->optionsButtonMode == OPTIONS_BUTTON_MODE_LR))
    {
        BeginNormalPaletteFade(PALETTES_ALL & ~(0x14), 0, 0, 0x10, RGB_BLACK);
        sPokedexView->screenSwitchState = 2;
        gTasks[taskId].func = Task_SwitchScreensFromSizeScreen;
        PlaySE(SE_DEX_PAGE);
    }
}

static void Task_SwitchScreensFromSizeScreen(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        FreeAndDestroyMonPicSprite(gTasks[taskId].tMonSpriteId);
        FreeAndDestroyTrainerPicSprite(gTasks[taskId].tTrainerSpriteId);
        switch (sPokedexView->screenSwitchState)
        {
        default:
        case 1:
            gTasks[taskId].func = Task_LoadInfoScreen;
            break;
        case 2:
            gTasks[taskId].func = Task_LoadCryScreen;
            break;
        }
    }
}

static void Task_ExitSizeScreen(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        FreeAndDestroyMonPicSprite(gTasks[taskId].tMonSpriteId);
        FreeAndDestroyTrainerPicSprite(gTasks[taskId].tTrainerSpriteId);
        FreeInfoScreenWindowAndBgBuffers();
        DestroyTask(taskId);
    }
}

static void Task_ExitSizeScreenToExternal(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        FreeAndDestroyMonPicSprite(gTasks[taskId].tMonSpriteId);
        FreeAndDestroyTrainerPicSprite(gTasks[taskId].tTrainerSpriteId);
        FreeInfoScreenWindowAndBgBuffers();
        DestroyTask(taskId);
        if (sExternalReturnCallback != NULL)
            SetMainCallback2(sExternalReturnCallback);
    }
}

#undef tScrolling
#undef tMonSpriteDone
#undef tBgLoaded
#undef tSkipCry
#undef tMonSpriteId
#undef tTrainerSpriteId



//************************************
//*                                  *
//*        Search Screen             *
//*                                  *
//************************************
static void Task_WaitForExitSearch(u8 taskId)
{
    if (!gTasks[gTasks[taskId].tLoadScreenTaskId].isActive)
    {
        ClearMonSprites();

        // Search produced results
        if (sPokedexView->screenSwitchState != 0)
        {
            sPokedexView->selectedPokemon = 0;
            sPokedexView->pokeBallRotation = POKEBALL_ROTATION_TOP;
            gTasks[taskId].func = Task_OpenSearchResults;
        }
        // Search didn't produce results
        else
        {
            sPokedexView->pokeBallRotation = sPokedexView->pokeBallRotationBackup;
            sPokedexView->selectedPokemon = sPokedexView->selectedPokemonBackup;
            sPokedexView->dexMode = sPokedexView->dexModeBackup;
            if (!IsNationalPokedexEnabled())
                sPokedexView->dexMode = DEX_MODE_HOENN;
            sPokedexView->dexOrder = sPokedexView->dexOrderBackup;
            gTasks[taskId].func = Task_OpenPokedexMainPage;
        }
    }
}

static void Task_OpenSearchResults(u8 taskId)
{
    sPokedexView->isSearchResults = TRUE;
    if (LoadPokedexListPage(PAGE_SEARCH_RESULTS))
        gTasks[taskId].func = Task_HandleSearchResultsInput;
}

static void Task_HandleSearchResultsInput(u8 taskId)
{
    SetGpuReg(REG_OFFSET_BG0VOFS, sPokedexView->menuY);

    if (sPokedexView->menuY)
    {
        sPokedexView->menuY -= 8;
    }
    else
    {
        if (JOY_NEW(A_BUTTON) && sPokedexView->pokedexList[sPokedexView->selectedPokemon].seen)
        {
            u32 a;

            UpdateSelectedMonSpriteId();
            a = (1 << (gSprites[sPokedexView->selectedMonSpriteId].oam.paletteNum + 16));
            gSprites[sPokedexView->selectedMonSpriteId].callback = SpriteCB_MoveMonForInfoScreen;
            BeginNormalPaletteFade(~a, 0, 0, 0x10, RGB_BLACK);
            gTasks[taskId].func = Task_OpenSearchResultsInfoScreenAfterMonMovement;
            PlaySE(SE_PIN);
            FreeWindowAndBgBuffers();
        }
        else if (JOY_NEW(START_BUTTON))
        {
            sPokedexView->selectedPokemon = 0;
			sPokedexView->pokeBallRotation = POKEBALL_ROTATION_TOP;
			ClearMonSprites();
			CreateMonSpritesAtPos(sPokedexView->selectedPokemon, 0xE);
            PlaySE(SE_SELECT);
        }
        else if (JOY_NEW(SELECT_BUTTON))
        {
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 0x10, RGB_BLACK);
            gTasks[taskId].tLoadScreenTaskId = LoadSearchMenu();
            sPokedexView->screenSwitchState = 0;
            gTasks[taskId].func = Task_WaitForExitSearch;
            PlaySE(SE_PC_LOGIN);
            FreeWindowAndBgBuffers();
        }
        else if (JOY_NEW(B_BUTTON))
        {
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 0x10, RGB_BLACK);
            gTasks[taskId].func = Task_ReturnToPokedexFromSearchResults;
            PlaySE(SE_PC_OFF);
        }
        else
        {
            //Handle D-pad
            sPokedexView->selectedPokemon = TryDoPokedexScroll(sPokedexView->selectedPokemon, 0xE);
            gTasks[taskId].func = Task_WaitForSearchResultsScroll;
        }
    }
}

static void Task_WaitForSearchResultsScroll(u8 taskId)
{
    if (UpdateDexListScroll(sPokedexView->scrollDirection, sPokedexView->scrollMonIncrement, sPokedexView->maxScrollTimer))
        gTasks[taskId].func = Task_HandleSearchResultsInput;
}

static void Task_OpenSearchResultsInfoScreenAfterMonMovement(u8 taskId)
{
    if (gSprites[sPokedexView->selectedMonSpriteId].x == MON_PAGE_X && gSprites[sPokedexView->selectedMonSpriteId].y == MON_PAGE_Y)
    {
        sPokedexView->currentPageBackup = sPokedexView->currentPage;
        gTasks[taskId].tLoadScreenTaskId = LoadInfoScreen(&sPokedexView->pokedexList[sPokedexView->selectedPokemon], sPokedexView->selectedMonSpriteId);
        sPokedexView->selectedMonSpriteId = -1;
        gTasks[taskId].func = Task_WaitForExitSearchResultsInfoScreen;
    }
}

static void Task_WaitForExitSearchResultsInfoScreen(u8 taskId)
{
    if (gTasks[gTasks[taskId].tLoadScreenTaskId].isActive)
    {
        // While active, handle scroll input
        if (sPokedexView->currentPage == PAGE_INFO && !IsInfoScreenScrolling(gTasks[taskId].tLoadScreenTaskId) && TryDoInfoScreenScroll())
            StartInfoScreenScroll(&sPokedexView->pokedexList[sPokedexView->selectedPokemon], gTasks[taskId].tLoadScreenTaskId);
    }
    else
    {
        // Exiting, back to search results
        gTasks[taskId].func = Task_OpenSearchResults;
    }
}

static void Task_ReturnToPokedexFromSearchResults(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        sPokedexView->pokeBallRotation = sPokedexView->pokeBallRotationBackup;
        sPokedexView->selectedPokemon = sPokedexView->selectedPokemonBackup;
        sPokedexView->dexMode = sPokedexView->dexModeBackup;
        if (!IsNationalPokedexEnabled())
            sPokedexView->dexMode = DEX_MODE_HOENN;
        sPokedexView->dexOrder = sPokedexView->dexOrderBackup;
        gTasks[taskId].func = Task_OpenPokedexMainPage;
        ClearMonSprites();
        FreeWindowAndBgBuffers();
    }
}

#undef tLoadScreenTaskId


//************************************
//*                                  *
//*        Search code               *
//*                                  *
//************************************
static int DoPokedexSearch(u8 dexMode, u8 order, u8 abcGroup, u8 bodyColor, u8 type1, u8 type2)
{
    u16 species;
    u16 i,j;
    u16 resultsCount;
    u8 types[2];

    CreatePokedexList(dexMode, order);

    for (i = 0, resultsCount = 0; i < NATIONAL_DEX_COUNT; i++)
    {
        if (sPokedexView->pokedexList[i].seen)
        {
            sPokedexView->pokedexList[resultsCount] = sPokedexView->pokedexList[i];
            resultsCount++;
        }
    }
    sPokedexView->pokemonListCount = resultsCount;

    // Search by name
    if (abcGroup != 0xFF)
    {
        for (i = 0, resultsCount = 0; i < sPokedexView->pokemonListCount; i++)
        {
            u8 firstLetter;

            species = NationalPokedexNumToSpecies(sPokedexView->pokedexList[i].dexNum);
            firstLetter = gSpeciesNames[species][0];
            if (LETTER_IN_RANGE_UPPER(firstLetter, abcGroup) || LETTER_IN_RANGE_LOWER(firstLetter, abcGroup))
            {
                sPokedexView->pokedexList[resultsCount] = sPokedexView->pokedexList[i];
                resultsCount++;
            }
        }
        sPokedexView->pokemonListCount = resultsCount;
    }

    // Search by body color
    if (bodyColor != 0xFF)
    {
        for (i = 0, resultsCount = 0; i < sPokedexView->pokemonListCount; i++)
        {
            species = NationalPokedexNumToSpecies(sPokedexView->pokedexList[i].dexNum);

            if (bodyColor == gSpeciesInfo[species].bodyColor)
            {
                sPokedexView->pokedexList[resultsCount] = sPokedexView->pokedexList[i];
                resultsCount++;
            }
        }
        sPokedexView->pokemonListCount = resultsCount;
    }

    // Search by type
    if (type1 != TYPE_NONE || type2 != TYPE_NONE)
    {
        if (type1 == TYPE_NONE)
        {
            type1 = type2;
            type2 = TYPE_NONE;
        }

        if (type2 == TYPE_NONE)
        {
            for (i = 0, resultsCount = 0; i < sPokedexView->pokemonListCount; i++)
            {
                if (sPokedexView->pokedexList[i].owned)
                {
                    species = NationalPokedexNumToSpecies(sPokedexView->pokedexList[i].dexNum);

                    types[0] = GetTypeBySpecies(species, 1);
                    types[1] = GetTypeBySpecies(species, 2);
                    if (types[0] == type1 || types[1] == type1)
                    {
                        sPokedexView->pokedexList[resultsCount] = sPokedexView->pokedexList[i];
                        resultsCount++;
                    }
                }
            }
        }
        else
        {
            for (i = 0, resultsCount = 0; i < sPokedexView->pokemonListCount; i++)
            {
                if (sPokedexView->pokedexList[i].owned)
                {
                    species = NationalPokedexNumToSpecies(sPokedexView->pokedexList[i].dexNum);

                    types[0] = GetTypeBySpecies(species, 1);
                    types[1] = GetTypeBySpecies(species, 2);
                    if ((types[0] == type1 && types[1] == type2) || (types[0] == type2 && types[1] == type1))
                    {
                        sPokedexView->pokedexList[resultsCount] = sPokedexView->pokedexList[i];
                        resultsCount++;
                    }
                }
            }
        }
        sPokedexView->pokemonListCount = resultsCount;
    }

    if (sPokedexView->pokemonListCount != 0)
    {
        for (i = sPokedexView->pokemonListCount; i < NATIONAL_DEX_COUNT; i++)
        {
            sPokedexView->pokedexList[i].dexNum = 0xFFFF;
            sPokedexView->pokedexList[i].seen = FALSE;
            sPokedexView->pokedexList[i].owned = FALSE;
        }
    }

    return resultsCount;
}

static u8 LoadSearchMenu(void)
{
    return CreateTask(Task_LoadSearchMenu, 0);
}

static void PrintSearchText(const u8 *str, u32 x, u32 y)
{
    u8 color[3];

    color[0] = TEXT_COLOR_TRANSPARENT;
    color[1] = TEXT_DYNAMIC_COLOR_6;
    color[2] = TEXT_COLOR_DARK_GRAY;
    AddTextPrinterParameterized4(0, FONT_NORMAL, x, y, 0, 0, color, TEXT_SKIP_DRAW, str);
}

static void ClearSearchMenuRect(u32 x, u32 y, u32 width, u32 height)
{
    FillWindowPixelRect(0, PIXEL_FILL(0), x, y, width, height);
}

// Search task data
#define tTopBarItem             data[0]
#define tMenuItem               data[1]
#define tCursorPos_Mode         data[2]
#define tScrollOffset_Mode      data[3]
#define tCursorPos_Order        data[4]
#define tScrollOffset_Order     data[5]
#define tCursorPos_Name         data[6]
#define tScrollOffset_Name      data[7]
#define tCursorPos_Color        data[8]
#define tScrollOffset_Color     data[9]
#define tCursorPos_TypeLeft     data[10]
#define tScrollOffset_TypeLeft  data[11]
#define tCursorPos_TypeRight    data[12]
#define tScrollOffset_TypeRight data[13]
#define tCursorPos              data[14]
#define tScrollOffset           data[15]

static void Task_LoadSearchMenu(u8 taskId)
{
    u16 i;

    switch (gMain.state)
    {
    default:
    case 0:
        if (!gPaletteFade.active)
        {
            sPokedexView->currentPage = PAGE_SEARCH;
            ResetOtherVideoRegisters(0);
            ResetBgsAndClearDma3BusyFlags(0);
            InitBgsFromTemplates(0, sSearchMenu_BgTemplate, ARRAY_COUNT(sSearchMenu_BgTemplate));
            SetBgTilemapBuffer(3, AllocZeroed(BG_SCREEN_SIZE));
            SetBgTilemapBuffer(2, AllocZeroed(BG_SCREEN_SIZE));
            SetBgTilemapBuffer(1, AllocZeroed(BG_SCREEN_SIZE));
            SetBgTilemapBuffer(0, AllocZeroed(BG_SCREEN_SIZE));
            InitWindows(sSearchMenu_WindowTemplate);
            DeactivateAllTextPrinters();
            PutWindowTilemap(0);
			DecompressAndLoadBgGfxUsingHeap(3, gPokedexPlusHGSS_MenuSearch_Gfx, 0x2000, 0, 0);
            if (!IsNationalPokedexEnabled())
                CopyToBgTilemapBuffer(3, gPokedexPlusHGSS_ScreenSearchHoenn_Tilemap, 0, 0);
            else
                CopyToBgTilemapBuffer(3, gPokedexPlusHGSS_ScreenSearchNational_Tilemap, 0, 0);
			LoadPalette(gPokedexPlusHGSS_MenuSearch_Pal + 1, BG_PLTT_ID(0) + 1, PLTT_SIZEOF(4 * 16 - 1));
            gMain.state = 1;
        }
        break;
    case 1:
        LoadCompressedSpriteSheet(&sInterfaceSpriteSheet);
        LoadSpritePalettes(sInterfaceSpritePalette);
        CreateSearchParameterScrollArrows(taskId);
        for (i = 0; i < NUM_TASK_DATA; i++)
            gTasks[taskId].data[i] = 0;
        SetDefaultSearchModeAndOrder(taskId);
        HighlightSelectedSearchTopBarItem(SEARCH_TOPBAR_SEARCH);
        PrintSelectedSearchParameters(taskId);
        CopyWindowToVram(0, COPYWIN_FULL);
        CopyBgTilemapBufferToVram(1);
        CopyBgTilemapBufferToVram(2);
        CopyBgTilemapBufferToVram(3);
        gMain.state++;
        break;
    case 2:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        gMain.state++;
        break;
    case 3:
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        SetGpuReg(REG_OFFSET_BLDY, 0);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON);
        HideBg(0);
        ShowBg(1);
        ShowBg(2);
        ShowBg(3);
        gMain.state++;
        break;
    case 4:
        if (!gPaletteFade.active)
        {
            gTasks[taskId].func = Task_SwitchToSearchMenuTopBar;
            gMain.state = 0;
        }
        break;
    }
}

static void FreeSearchWindowAndBgBuffers(void)
{
    void *tilemapBuffer;

    FreeAllWindowBuffers();
    tilemapBuffer = GetBgTilemapBuffer(0);
    if (tilemapBuffer)
        Free(tilemapBuffer);
    tilemapBuffer = GetBgTilemapBuffer(1);
    if (tilemapBuffer)
        Free(tilemapBuffer);
    tilemapBuffer = GetBgTilemapBuffer(2);
    if (tilemapBuffer)
        Free(tilemapBuffer);
    tilemapBuffer = GetBgTilemapBuffer(3);
    if (tilemapBuffer)
        Free(tilemapBuffer);
}

static void Task_SwitchToSearchMenuTopBar(u8 taskId)
{
    HighlightSelectedSearchTopBarItem(gTasks[taskId].tTopBarItem);
    PrintSelectedSearchParameters(taskId);
    CopyWindowToVram(0, COPYWIN_GFX);
    CopyBgTilemapBufferToVram(3);
    gTasks[taskId].func = Task_HandleSearchTopBarInput;
}

static void Task_HandleSearchTopBarInput(u8 taskId)
{
    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_PC_OFF);
        gTasks[taskId].func = Task_ExitSearch;
        return;
    }
    if (JOY_NEW(A_BUTTON))
    {
        switch (gTasks[taskId].tTopBarItem)
        {
        case SEARCH_TOPBAR_SEARCH:
            PlaySE(SE_PIN);
            gTasks[taskId].tMenuItem = SEARCH_NAME;
            gTasks[taskId].func = Task_SwitchToSearchMenu;
            break;
        case SEARCH_TOPBAR_SHIFT:
            PlaySE(SE_PIN);
            gTasks[taskId].tMenuItem = SEARCH_ORDER;
            gTasks[taskId].func = Task_SwitchToSearchMenu;
            break;
        case SEARCH_TOPBAR_CANCEL:
            PlaySE(SE_PC_OFF);
            gTasks[taskId].func = Task_ExitSearch;
            break;
        }
        return;
    }
    if (JOY_NEW(DPAD_LEFT) && gTasks[taskId].tTopBarItem > SEARCH_TOPBAR_SEARCH)
    {
        PlaySE(SE_DEX_PAGE);
        gTasks[taskId].tTopBarItem--;
        HighlightSelectedSearchTopBarItem(gTasks[taskId].tTopBarItem);
        CopyWindowToVram(0, COPYWIN_GFX);
        CopyBgTilemapBufferToVram(3);
    }
    if (JOY_NEW(DPAD_RIGHT) && gTasks[taskId].tTopBarItem < SEARCH_TOPBAR_CANCEL)
    {
        PlaySE(SE_DEX_PAGE);
        gTasks[taskId].tTopBarItem++;
        HighlightSelectedSearchTopBarItem(gTasks[taskId].tTopBarItem);
        CopyWindowToVram(0, COPYWIN_GFX);
        CopyBgTilemapBufferToVram(3);
    }
}

static void Task_SwitchToSearchMenu(u8 taskId)
{
    HighlightSelectedSearchMenuItem(gTasks[taskId].tTopBarItem, gTasks[taskId].tMenuItem);
    PrintSelectedSearchParameters(taskId);
    CopyWindowToVram(0, COPYWIN_GFX);
    CopyBgTilemapBufferToVram(3);
    gTasks[taskId].func = Task_HandleSearchMenuInput;
}

// Input for main search menu
static void Task_HandleSearchMenuInput(u8 taskId)
{
    const u8 (*movementMap)[4];

    if (gTasks[taskId].tTopBarItem != SEARCH_TOPBAR_SEARCH)
    {
        if (!IsNationalPokedexEnabled())
            movementMap = sSearchMovementMap_ShiftHoennDex;
        else
            movementMap = sSearchMovementMap_ShiftNatDex;
    }
    else
    {
        if (!IsNationalPokedexEnabled())
            movementMap = sSearchMovementMap_SearchHoennDex;
        else
            movementMap = sSearchMovementMap_SearchNatDex;
    }

    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_BALL);
        SetDefaultSearchModeAndOrder(taskId);
        gTasks[taskId].func = Task_SwitchToSearchMenuTopBar;
        return;
    }
    if (JOY_NEW(A_BUTTON))
    {
        if (gTasks[taskId].tMenuItem == SEARCH_OK)
        {
            if (gTasks[taskId].tTopBarItem != SEARCH_TOPBAR_SEARCH)
            {
                sPokeBallRotation = POKEBALL_ROTATION_TOP;
                sPokedexView->pokeBallRotationBackup = POKEBALL_ROTATION_TOP;
                sLastSelectedPokemon = 0;
                sPokedexView->selectedPokemonBackup = 0;
                gSaveBlock2Ptr->pokedex.mode = GetSearchModeSelection(taskId, SEARCH_MODE);
                if (!IsNationalPokedexEnabled())
                    gSaveBlock2Ptr->pokedex.mode = DEX_MODE_HOENN;
                sPokedexView->dexModeBackup = gSaveBlock2Ptr->pokedex.mode;
                gSaveBlock2Ptr->pokedex.order = GetSearchModeSelection(taskId, SEARCH_ORDER);
                sPokedexView->dexOrderBackup = gSaveBlock2Ptr->pokedex.order;
                PlaySE(SE_PC_OFF);
                gTasks[taskId].func = Task_ExitSearch;
            }
            else
            {
                EraseAndPrintSearchTextBox(gText_SearchingPleaseWait);
                gTasks[taskId].func = Task_StartPokedexSearch;
                PlaySE(SE_DEX_SEARCH);
                CopyWindowToVram(0, COPYWIN_GFX);
            }
        }
        else
        {
            PlaySE(SE_PIN);
            gTasks[taskId].func = Task_SelectSearchMenuItem;
        }
        return;
    }

    if (JOY_NEW(DPAD_LEFT) && movementMap[gTasks[taskId].tMenuItem][0] != 0xFF)
    {
        PlaySE(SE_SELECT);
        gTasks[taskId].tMenuItem = movementMap[gTasks[taskId].tMenuItem][0];
        HighlightSelectedSearchMenuItem(gTasks[taskId].tTopBarItem, gTasks[taskId].tMenuItem);
        CopyWindowToVram(0, COPYWIN_GFX);
        CopyBgTilemapBufferToVram(3);
    }
    if (JOY_NEW(DPAD_RIGHT) && movementMap[gTasks[taskId].tMenuItem][1] != 0xFF)
    {
        PlaySE(SE_SELECT);
        gTasks[taskId].tMenuItem = movementMap[gTasks[taskId].tMenuItem][1];
        HighlightSelectedSearchMenuItem(gTasks[taskId].tTopBarItem, gTasks[taskId].tMenuItem);
        CopyWindowToVram(0, COPYWIN_GFX);
        CopyBgTilemapBufferToVram(3);
    }
    if (JOY_NEW(DPAD_UP) && movementMap[gTasks[taskId].tMenuItem][2] != 0xFF)
    {
        PlaySE(SE_SELECT);
        gTasks[taskId].tMenuItem = movementMap[gTasks[taskId].tMenuItem][2];
        HighlightSelectedSearchMenuItem(gTasks[taskId].tTopBarItem, gTasks[taskId].tMenuItem);
        CopyWindowToVram(0, COPYWIN_GFX);
        CopyBgTilemapBufferToVram(3);
    }
    if (JOY_NEW(DPAD_DOWN) && movementMap[gTasks[taskId].tMenuItem][3] != 0xFF)
    {
        PlaySE(SE_SELECT);
        gTasks[taskId].tMenuItem = movementMap[gTasks[taskId].tMenuItem][3];
        HighlightSelectedSearchMenuItem(gTasks[taskId].tTopBarItem, gTasks[taskId].tMenuItem);
        CopyWindowToVram(0, COPYWIN_GFX);
        CopyBgTilemapBufferToVram(3);
    }
}

static void Task_StartPokedexSearch(u8 taskId)
{
    u8 dexMode = GetSearchModeSelection(taskId, SEARCH_MODE);
    u8 order = GetSearchModeSelection(taskId, SEARCH_ORDER);
    u8 abcGroup = GetSearchModeSelection(taskId, SEARCH_NAME);
    u8 bodyColor = GetSearchModeSelection(taskId, SEARCH_COLOR);
    u8 type1 = GetSearchModeSelection(taskId, SEARCH_TYPE_LEFT);
    u8 type2 = GetSearchModeSelection(taskId, SEARCH_TYPE_RIGHT);

    DoPokedexSearch(dexMode, order, abcGroup, bodyColor, type1, type2);
    gTasks[taskId].func = Task_WaitAndCompleteSearch;
}

static void Task_WaitAndCompleteSearch(u8 taskId)
{
    if (!IsSEPlaying())
    {
        if (sPokedexView->pokemonListCount != 0)
        {
            PlaySE(SE_SUCCESS);
            EraseAndPrintSearchTextBox(gText_SearchCompleted);
        }
        else
        {
            PlaySE(SE_FAILURE);
            EraseAndPrintSearchTextBox(gText_NoMatchingPkmnWereFound);
        }
        gTasks[taskId].func = Task_SearchCompleteWaitForInput;
        CopyWindowToVram(0, COPYWIN_GFX);
    }
}

static void Task_SearchCompleteWaitForInput(u8 taskId)
{
    if (JOY_NEW(A_BUTTON))
    {
        if (sPokedexView->pokemonListCount != 0)
        {
            // Return to dex list and show search results
            sPokedexView->screenSwitchState = 1;
            sPokedexView->dexMode = GetSearchModeSelection(taskId, SEARCH_MODE);
            sPokedexView->dexOrder = GetSearchModeSelection(taskId, SEARCH_ORDER);
            gTasks[taskId].func = Task_ExitSearch;
            PlaySE(SE_PC_OFF);
        }
        else
        {
            gTasks[taskId].func = Task_SwitchToSearchMenu;
            PlaySE(SE_BALL);
        }
    }
}

static void Task_SelectSearchMenuItem(u8 taskId)
{
    u8 menuItem;
    u16 *cursorPos;
    u16 *scrollOffset;

    DrawOrEraseSearchParameterBox(FALSE);
    menuItem = gTasks[taskId].tMenuItem;
    cursorPos = &gTasks[taskId].data[sSearchOptions[menuItem].taskDataCursorPos];
    scrollOffset = &gTasks[taskId].data[sSearchOptions[menuItem].taskDataScrollOffset];
    gTasks[taskId].tCursorPos = *cursorPos;
    gTasks[taskId].tScrollOffset = *scrollOffset;
    PrintSearchParameterText(taskId);
    PrintSelectorArrow(*cursorPos);
    gTasks[taskId].func = Task_HandleSearchParameterInput;
    CopyWindowToVram(0, COPYWIN_GFX);
    CopyBgTilemapBufferToVram(3);
}

// Input for scrolling parameter box in right column
static void Task_HandleSearchParameterInput(u8 taskId)
{
    u8 menuItem;
    const struct SearchOptionText *texts;
    u16 *cursorPos;
    u16 *scrollOffset;
    u16 maxOption;
    bool8 moved;

    menuItem = gTasks[taskId].tMenuItem;
    texts = sSearchOptions[menuItem].texts;
    cursorPos = &gTasks[taskId].data[sSearchOptions[menuItem].taskDataCursorPos];
    scrollOffset = &gTasks[taskId].data[sSearchOptions[menuItem].taskDataScrollOffset];
    maxOption = sSearchOptions[menuItem].numOptions - 1;
    if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_PIN);
        ClearSearchParameterBoxText();
        DrawOrEraseSearchParameterBox(TRUE);
        gTasks[taskId].func = Task_SwitchToSearchMenu;
        CopyWindowToVram(0, COPYWIN_GFX);
        CopyBgTilemapBufferToVram(3);
        return;
    }
    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_BALL);
        ClearSearchParameterBoxText();
        DrawOrEraseSearchParameterBox(TRUE);
        *cursorPos = gTasks[taskId].tCursorPos;
        *scrollOffset = gTasks[taskId].tScrollOffset;
        gTasks[taskId].func = Task_SwitchToSearchMenu;
        CopyWindowToVram(0, COPYWIN_GFX);
        CopyBgTilemapBufferToVram(3);
        return;
    }
    moved = FALSE;
    if (JOY_REPEAT(DPAD_UP))
    {
        if (*cursorPos != 0)
        {
            // Move cursor up
            EraseSelectorArrow(*cursorPos);
            (*cursorPos)--;
            PrintSelectorArrow(*cursorPos);
            moved = TRUE;
        }
        else if (*scrollOffset != 0)
        {
            // Scroll up
            (*scrollOffset)--;
            PrintSearchParameterText(taskId);
            PrintSelectorArrow(*cursorPos);
            moved = TRUE;
        }
        if (moved)
        {
            PlaySE(SE_SELECT);
            EraseAndPrintSearchTextBox(texts[*cursorPos + *scrollOffset].description);
            CopyWindowToVram(0, COPYWIN_GFX);
        }
        return;
    }
    if (JOY_REPEAT(DPAD_DOWN))
    {
        if (*cursorPos < MAX_SEARCH_PARAM_CURSOR_POS && *cursorPos < maxOption)
        {
            // Move cursor down
            EraseSelectorArrow(*cursorPos);
            (*cursorPos)++;
            PrintSelectorArrow(*cursorPos);
            moved = TRUE;
        }
        else if (maxOption > MAX_SEARCH_PARAM_CURSOR_POS && *scrollOffset < maxOption - MAX_SEARCH_PARAM_CURSOR_POS)
        {
            // Scroll down
            (*scrollOffset)++;
            PrintSearchParameterText(taskId);
            PrintSelectorArrow(5);
            moved = TRUE;
        }
        if (moved)
        {
            PlaySE(SE_SELECT);
            EraseAndPrintSearchTextBox(texts[*cursorPos + *scrollOffset].description);
            CopyWindowToVram(0, COPYWIN_GFX);
        }
        return;
    }
}

static void Task_ExitSearch(u8 taskId)
{
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
    gTasks[taskId].func = Task_ExitSearchWaitForFade;
}

static void Task_ExitSearchWaitForFade(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        FreeSearchWindowAndBgBuffers();
        DestroyTask(taskId);
    }
}

static void SetSearchRectHighlight(u8 flags, u8 x, u8 y, u8 width)
{
    u16 i;
    u16 temp; //should be a pointer, but does not match as one
    u32 ptr = (u32)GetBgTilemapBuffer(3); //same as above

    for (i = 0; i < width; i++)
    {
        temp = *(u16 *)(ptr + (y + 0) * 64 + (x + i) * 2);
        temp &= 0x0fff;
        temp |= (flags << 12);
        *(u16 *)(ptr + (y + 0) * 64 + (x + i) * 2) = temp;

        temp = *(u16 *)(ptr + (y + 1) * 64 + (x + i) * 2);
        temp &= 0x0fff;
        temp |= (flags << 12);
        *(u16 *)(ptr + (y + 1) * 64 + (x + i) * 2) = temp;
    }
}


#define SEARCH_BG_SEARCH                SEARCH_TOPBAR_SEARCH
#define SEARCH_BG_SHIFT                 SEARCH_TOPBAR_SHIFT
#define SEARCH_BG_CANCEL                SEARCH_TOPBAR_CANCEL
#define SEARCH_BG_NAME                  (SEARCH_NAME + SEARCH_TOPBAR_COUNT)
#define SEARCH_BG_COLOR                 (SEARCH_COLOR + SEARCH_TOPBAR_COUNT)
#define SEARCH_BG_TYPE_SELECTION_LEFT   (SEARCH_TYPE_LEFT + SEARCH_TOPBAR_COUNT)
#define SEARCH_BG_TYPE_SELECTION_RIGHT  (SEARCH_TYPE_RIGHT + SEARCH_TOPBAR_COUNT)
#define SEARCH_BG_ORDER                 (SEARCH_ORDER + SEARCH_TOPBAR_COUNT)
#define SEARCH_BG_MODE                  (SEARCH_MODE + SEARCH_TOPBAR_COUNT)
#define SEARCH_BG_OK                    (SEARCH_OK + SEARCH_TOPBAR_COUNT)
#define SEARCH_BG_TYPE_TITLE            (SEARCH_COUNT + SEARCH_TOPBAR_COUNT)

static void DrawSearchMenuItemBgHighlight(u8 searchBg, bool8 unselected, bool8 disabled)
{
    u8 highlightFlags = (unselected & 1) | ((disabled & 1) << 1);

    switch (searchBg)
    {
    case SEARCH_BG_SEARCH:
    case SEARCH_BG_SHIFT:
    case SEARCH_BG_CANCEL:
        SetSearchRectHighlight(highlightFlags, sSearchMenuTopBarItems[searchBg].highlightX, sSearchMenuTopBarItems[searchBg].highlightY, sSearchMenuTopBarItems[searchBg].highlightWidth);
        break;
    case SEARCH_BG_NAME:
    case SEARCH_BG_COLOR:
    case SEARCH_BG_ORDER:
    case SEARCH_BG_MODE:
        SetSearchRectHighlight(highlightFlags, sSearchMenuItems[searchBg - SEARCH_TOPBAR_COUNT].titleBgX, sSearchMenuItems[searchBg - SEARCH_TOPBAR_COUNT].titleBgY, sSearchMenuItems[searchBg - SEARCH_TOPBAR_COUNT].titleBgWidth);
        // fall through, draw selectionBg for above
    case SEARCH_BG_TYPE_SELECTION_LEFT:
    case SEARCH_BG_TYPE_SELECTION_RIGHT:
        SetSearchRectHighlight(highlightFlags, sSearchMenuItems[searchBg - SEARCH_TOPBAR_COUNT].selectionBgX, sSearchMenuItems[searchBg - SEARCH_TOPBAR_COUNT].selectionBgY, sSearchMenuItems[searchBg - SEARCH_TOPBAR_COUNT].selectionBgWidth);
        break;
    case SEARCH_BG_TYPE_TITLE:
        SetSearchRectHighlight(highlightFlags, sSearchMenuItems[SEARCH_TYPE_LEFT].titleBgX, sSearchMenuItems[SEARCH_TYPE_LEFT].titleBgY, sSearchMenuItems[SEARCH_TYPE_LEFT].titleBgWidth);
        break;
    case SEARCH_BG_OK:
        if (!IsNationalPokedexEnabled())
            SetSearchRectHighlight(highlightFlags, sSearchMenuItems[searchBg - SEARCH_TOPBAR_COUNT].titleBgX, sSearchMenuItems[searchBg - SEARCH_TOPBAR_COUNT].titleBgY - 2, sSearchMenuItems[searchBg - SEARCH_TOPBAR_COUNT].titleBgWidth);
        else
            SetSearchRectHighlight(highlightFlags, sSearchMenuItems[searchBg - SEARCH_TOPBAR_COUNT].titleBgX, sSearchMenuItems[searchBg - SEARCH_TOPBAR_COUNT].titleBgY, sSearchMenuItems[searchBg - SEARCH_TOPBAR_COUNT].titleBgWidth);
        break;
    }
}

static void SetInitialSearchMenuBgHighlights(u8 topBarItem)
{
    switch (topBarItem)
    {
    case SEARCH_TOPBAR_SEARCH:
        DrawSearchMenuItemBgHighlight(SEARCH_BG_SEARCH, FALSE, FALSE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_SHIFT, TRUE, FALSE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_CANCEL, TRUE, FALSE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_NAME, TRUE, FALSE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_COLOR, TRUE, FALSE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_TYPE_TITLE, TRUE, FALSE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_TYPE_SELECTION_LEFT, TRUE, FALSE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_TYPE_SELECTION_RIGHT, TRUE, FALSE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_ORDER, TRUE, FALSE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_MODE, TRUE, FALSE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_OK, TRUE, FALSE);
        break;
    case SEARCH_TOPBAR_SHIFT:
        DrawSearchMenuItemBgHighlight(SEARCH_BG_SEARCH, TRUE, FALSE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_SHIFT, FALSE, FALSE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_CANCEL, TRUE, FALSE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_NAME, TRUE, TRUE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_COLOR, TRUE, TRUE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_TYPE_TITLE, TRUE, TRUE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_TYPE_SELECTION_LEFT, TRUE, TRUE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_TYPE_SELECTION_RIGHT, TRUE, TRUE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_ORDER, TRUE, FALSE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_MODE, TRUE, FALSE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_OK, TRUE, FALSE);
        break;
    case SEARCH_TOPBAR_CANCEL:
        DrawSearchMenuItemBgHighlight(SEARCH_BG_SEARCH, TRUE, FALSE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_SHIFT, TRUE, FALSE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_CANCEL, FALSE, FALSE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_NAME, TRUE, TRUE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_COLOR, TRUE, TRUE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_TYPE_TITLE, TRUE, TRUE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_TYPE_SELECTION_LEFT, TRUE, TRUE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_TYPE_SELECTION_RIGHT, TRUE, TRUE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_ORDER, TRUE, TRUE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_MODE, TRUE, TRUE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_OK, TRUE, TRUE);
        break;
    }
}

static void HighlightSelectedSearchTopBarItem(u8 topBarItem)
{
    SetInitialSearchMenuBgHighlights(topBarItem);
    EraseAndPrintSearchTextBox(sSearchMenuTopBarItems[topBarItem].description);
}

static void HighlightSelectedSearchMenuItem(u8 topBarItem, u8 menuItem)
{
    SetInitialSearchMenuBgHighlights(topBarItem);
    switch (menuItem)
    {
    case SEARCH_NAME:
        DrawSearchMenuItemBgHighlight(SEARCH_BG_NAME, FALSE, FALSE);
        break;
    case SEARCH_COLOR:
        DrawSearchMenuItemBgHighlight(SEARCH_BG_COLOR, FALSE, FALSE);
        break;
    case SEARCH_TYPE_LEFT:
        DrawSearchMenuItemBgHighlight(SEARCH_BG_TYPE_TITLE, FALSE, FALSE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_TYPE_SELECTION_LEFT, FALSE, FALSE);
        break;
    case SEARCH_TYPE_RIGHT:
        DrawSearchMenuItemBgHighlight(SEARCH_BG_TYPE_TITLE, FALSE, FALSE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_TYPE_SELECTION_RIGHT, FALSE, FALSE);
        break;
    case SEARCH_ORDER:
        DrawSearchMenuItemBgHighlight(SEARCH_BG_ORDER, FALSE, FALSE);
        break;
    case SEARCH_MODE:
        DrawSearchMenuItemBgHighlight(SEARCH_BG_MODE, FALSE, FALSE);
        break;
    case SEARCH_OK:
        DrawSearchMenuItemBgHighlight(SEARCH_BG_OK, FALSE, FALSE);
        break;
    }
    EraseAndPrintSearchTextBox(sSearchMenuItems[menuItem].description);
}

// Prints the currently selected search parameters in the search menu selection boxes
static void PrintSelectedSearchParameters(u8 taskId)
{
    u16 searchParamId;

    ClearSearchMenuRect(40, 16, 96, 80);

    searchParamId = gTasks[taskId].tCursorPos_Name + gTasks[taskId].tScrollOffset_Name;
    PrintSearchText(sDexSearchNameOptions[searchParamId].title, 0x2D, 0x11);

    searchParamId = gTasks[taskId].tCursorPos_Color + gTasks[taskId].tScrollOffset_Color;
    PrintSearchText(sDexSearchColorOptions[searchParamId].title, 0x2D, 0x21);

    searchParamId = gTasks[taskId].tCursorPos_TypeLeft + gTasks[taskId].tScrollOffset_TypeLeft;
    PrintSearchText(sDexSearchTypeOptions[searchParamId].title, 0x2D, 0x31);

    searchParamId = gTasks[taskId].tCursorPos_TypeRight + gTasks[taskId].tScrollOffset_TypeRight;
    PrintSearchText(sDexSearchTypeOptions[searchParamId].title, 0x5D, 0x31);

    searchParamId = gTasks[taskId].tCursorPos_Order + gTasks[taskId].tScrollOffset_Order;
    PrintSearchText(sDexOrderOptions[searchParamId].title, 0x2D, 0x41);

    if (IsNationalPokedexEnabled())
    {
        searchParamId = gTasks[taskId].tCursorPos_Mode + gTasks[taskId].tScrollOffset_Mode;
        PrintSearchText(sDexModeOptions[searchParamId].title, 0x2D, 0x51);
    }
}

static void DrawOrEraseSearchParameterBox(bool8 erase)
{
    u16 i;
    u16 j;
    u16 *ptr = GetBgTilemapBuffer(3);

    if (!erase)
    {
        *(ptr + 0x11) = 0xC0B;
        for (i = 0x12; i < 0x1F; i++)
            *(ptr + i) = 0x80D;
        for (j = 1; j < 13; j++)
        {
            *(ptr + 0x11 + j * 32) = 0x40A;
            for (i = 0x12; i < 0x1F; i++)
                *(ptr + j * 32 + i) = 2;
        }
        *(ptr + 0x1B1) = 0x40B;
        for (i = 0x12; i < 0x1F; i++)
            *(ptr + 0x1A0 + i) = 0xD;
    }
    else
    {
        for (j = 0; j < 14; j++)
        {
            for (i = 0x11; i < 0x1E; i++)
            {
                *(ptr + j * 32 + i) = 0x4F;
            }
        }
    }
}

// Prints the currently viewable search parameter titles in the right-hand text box
// and the currently selected search parameter description in the bottom text box
static void PrintSearchParameterText(u8 taskId)
{
    const struct SearchOptionText *texts = sSearchOptions[gTasks[taskId].tMenuItem].texts;
    const u16 *cursorPos = &gTasks[taskId].data[sSearchOptions[gTasks[taskId].tMenuItem].taskDataCursorPos];
    const u16 *scrollOffset = &gTasks[taskId].data[sSearchOptions[gTasks[taskId].tMenuItem].taskDataScrollOffset];
    u16 i;
    u16 j;

    ClearSearchParameterBoxText();

    for (i = 0, j = *scrollOffset; i < MAX_SEARCH_PARAM_ON_SCREEN && texts[j].title != NULL; i++, j++)
        PrintSearchParameterTitle(i, texts[j].title);

    EraseAndPrintSearchTextBox(texts[*cursorPos + *scrollOffset].description);
}

static u8 GetSearchModeSelection(u8 taskId, u8 option)
{
    const u16 *cursorPos = &gTasks[taskId].data[sSearchOptions[option].taskDataCursorPos];
    const u16 *scrollOffset = &gTasks[taskId].data[sSearchOptions[option].taskDataScrollOffset];
    u16 id = *cursorPos + *scrollOffset;

    switch (option)
    {
    default:
        return 0;
    case SEARCH_MODE:
        return sPokedexModes[id];
    case SEARCH_ORDER:
        return sOrderOptions[id];
    case SEARCH_NAME:
        if (id == 0)
            return 0xFF;
        else
            return id;
    case SEARCH_COLOR:
        if (id == 0)
            return 0xFF;
        else
            return id - 1;
    case SEARCH_TYPE_LEFT:
    case SEARCH_TYPE_RIGHT:
        return sDexSearchTypeIds[id];
    }
}

static void SetDefaultSearchModeAndOrder(u8 taskId)
{
    u16 selected;

    switch (sPokedexView->dexModeBackup)
    {
    default:
    case DEX_MODE_HOENN:
        selected = DEX_MODE_HOENN;
        break;
    case DEX_MODE_NATIONAL:
        selected = DEX_MODE_NATIONAL;
        break;
    }
    gTasks[taskId].tCursorPos_Mode = selected;

    switch (sPokedexView->dexOrderBackup)
    {
    default:
    case ORDER_NUMERICAL:
        selected = ORDER_NUMERICAL;
        break;
    case ORDER_ALPHABETICAL:
        selected = ORDER_ALPHABETICAL;
        break;
    case ORDER_HEAVIEST:
        selected = ORDER_HEAVIEST;
        break;
    case ORDER_LIGHTEST:
        selected = ORDER_LIGHTEST;
        break;
    case ORDER_TALLEST:
        selected = ORDER_TALLEST;
        break;
    case ORDER_SMALLEST:
        selected = ORDER_SMALLEST;
        break;
    }
    gTasks[taskId].tCursorPos_Order = selected;
}

static bool8 SearchParamCantScrollUp(u8 taskId)
{
    u8 menuItem = gTasks[taskId].tMenuItem;
    const u16 *scrollOffset = &gTasks[taskId].data[sSearchOptions[menuItem].taskDataScrollOffset];
    u16 lastOption = sSearchOptions[menuItem].numOptions - 1;

    if (lastOption > MAX_SEARCH_PARAM_CURSOR_POS && *scrollOffset != 0)
        return FALSE;
    else
        return TRUE;
}

static bool8 SearchParamCantScrollDown(u8 taskId)
{
    u8 menuItem = gTasks[taskId].tMenuItem;
    const u16 *scrollOffset = &gTasks[taskId].data[sSearchOptions[menuItem].taskDataScrollOffset];
    u16 lastOption = sSearchOptions[menuItem].numOptions - 1;

    if (lastOption > MAX_SEARCH_PARAM_CURSOR_POS && *scrollOffset < lastOption - MAX_SEARCH_PARAM_CURSOR_POS)
        return FALSE;
    else
        return TRUE;
}

#define sTaskId      data[0]

static void SpriteCB_SearchParameterScrollArrow(struct Sprite *sprite)
{
    if (gTasks[sprite->sTaskId].func == Task_HandleSearchParameterInput)
    {
        u8 val;

        if (sprite->sIsDownArrow)
        {
            if (SearchParamCantScrollDown(sprite->sTaskId))
                sprite->invisible = TRUE;
            else
                sprite->invisible = FALSE;
        }
        else
        {
            if (SearchParamCantScrollUp(sprite->sTaskId))
                sprite->invisible = TRUE;
            else
                sprite->invisible = FALSE;
        }
        val = sprite->data[2] + sprite->sIsDownArrow * 128;
        sprite->y2 = gSineTable[val] / 128;
        sprite->data[2] += 8;
    }
    else
    {
        sprite->invisible = TRUE;
    }
}

static void CreateSearchParameterScrollArrows(u8 taskId)
{
    u8 spriteId;

    spriteId = CreateSprite(&sScrollArrowSpriteTemplate, 184, 4, 0);
    gSprites[spriteId].sTaskId = taskId;
    gSprites[spriteId].sIsDownArrow = FALSE;
    gSprites[spriteId].callback = SpriteCB_SearchParameterScrollArrow;

    spriteId = CreateSprite(&sScrollArrowSpriteTemplate, 184, 108, 0);
    gSprites[spriteId].sTaskId = taskId;
    gSprites[spriteId].sIsDownArrow = TRUE;
    gSprites[spriteId].vFlip = TRUE;
    gSprites[spriteId].callback = SpriteCB_SearchParameterScrollArrow;
}

#undef sTaskId
#undef sIsDownArrow

static void EraseAndPrintSearchTextBox(const u8 *str)
{
    ClearSearchMenuRect(8, 120, 224, 32);
    PrintSearchText(str, 8, 121);
}

static void EraseSelectorArrow(u32 y)
{
    ClearSearchMenuRect(144, y * 16 + 8, 8, 16);
}

static void PrintSelectorArrow(u32 y)
{
    PrintSearchText(gText_SelectorArrow, 144, y * 16 + 9);
}

static void PrintSearchParameterTitle(u32 y, const u8 *str)
{
    PrintSearchText(str, 152, y * 16 + 9);
}

static void ClearSearchParameterBoxText(void)
{
    ClearSearchMenuRect(144, 8, 96, 96);
}