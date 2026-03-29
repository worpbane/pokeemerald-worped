#include "global.h"
#include "event_data.h"
#include "rtc.h"
#include "time_events.h"
#include "field_specials.h"
#include "lottery_corner.h"
#include "dewford_trend.h"
#include "tv.h"
#include "field_weather.h"
#include "berry.h"
#include "main.h"
#include "overworld.h"
#include "wallclock.h"
#include "random.h"
#include "constants/weather.h"

static void UpdatePerDay(struct Time *localTime);
static void UpdatePerMinute(struct Time *localTime);
static void setWeatherSeason(void);

static void InitTimeBasedEvents(void)
{
    FlagSet(FLAG_SYS_CLOCK_SET);
    RtcCalcLocalTime();
    gSaveBlock2Ptr->lastBerryTreeUpdate = gLocalTime;
    VarSet(VAR_DAYS, gLocalTime.days);
}

void DoTimeBasedEvents(void)
{
    if (FlagGet(FLAG_SYS_CLOCK_SET) && !InPokemonCenter())
    {
        RtcCalcLocalTime();
        UpdatePerDay(&gLocalTime);
        UpdatePerMinute(&gLocalTime);
    }
}

static void UpdatePerDay(struct Time *localTime)
{
    u16 *days = GetVarPointer(VAR_DAYS);
    u16 daysSince;
	u16 rndWthr = Random() % 100;

    if (*days != localTime->days && *days <= localTime->days)
    {
		if (rndWthr > 25)
                VarSet(VAR_SEASON_WEATHER, WEATHER_NONE);
            else
                setWeatherSeason();
        daysSince = localTime->days - *days;
        ClearDailyFlags();
        UpdateDewfordTrendPerDay(daysSince);
        UpdateTVShowsPerDay(daysSince);
        UpdateWeatherPerDay(daysSince);
        UpdatePartyPokerusTime(daysSince);
        UpdateMirageRnd(daysSince);
        UpdateBirchState(daysSince);
        UpdateFrontierManiac(daysSince);
        UpdateFrontierGambler(daysSince);
        SetShoalItemFlag(daysSince);
        SetRandomLotteryNumber(daysSince);
        *days = localTime->days;
    }
    else 
    {
        // Clock moved backward resync baseline;
        *days = localTime->days;
    }
}

static void UpdatePerMinute(struct Time *localTime)
{
    struct Time difference;
    int minutes;

    CalcTimeDifference(&difference, &gSaveBlock2Ptr->lastBerryTreeUpdate, localTime);
    minutes = 24 * 60 * difference.days + 60 * difference.hours + difference.minutes;
    if (minutes > 0)
    {
        BerryTreeTimeUpdate(minutes);
        gSaveBlock2Ptr->lastBerryTreeUpdate = *localTime;
    }
    else if (minutes < 0)
    {
        gSaveBlock2Ptr->lastBerryTreeUpdate = *localTime;
    }
}

static void ReturnFromStartWallClock(void)
{
    InitTimeBasedEvents();
    SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
}

void StartWallClock(void)
{
    SetMainCallback2(CB2_StartWallClock);
    gMain.savedCallback = ReturnFromStartWallClock;
}


void FastForwardTime(s16 daysToUpdateDay, s16 hoursToGrowBerries){
// Runs the UpdatePerDay function as if daysToUpdateDay days have passed and grows the berries by hoursToGrowBerries
        s16 daysBerry = hoursToGrowBerries / 24;
        s8 hoursBerry = hoursToGrowBerries % 24;
        struct Time localTimeOffset;
        localTimeOffset.days = *GetVarPointer(VAR_DAYS) + daysToUpdateDay;
        UpdatePerDay(&localTimeOffset);
        localTimeOffset = gSaveBlock2Ptr->lastBerryTreeUpdate;
        localTimeOffset.days += daysBerry;
        localTimeOffset.hours += hoursBerry;
        UpdatePerMinute(&localTimeOffset);
}

static void setWeatherSeason(void)
{
    int temp = 0;
    u8 currentSeason = VarGet(VAR_SEASON_STATE);
    u16 rngval = (Random() % 100);

    switch (currentSeason)
    {
        case SEASON_SPRING:
            if (rngval < 1)              // 1%
                temp = WEATHER_SHADE;
            else if (rngval < 11)        // 10%
                temp = WEATHER_RAIN_THUNDERSTORM;
            else if (rngval < 41)        // 30%
                temp = WEATHER_NONE;
            else                         // 59%
                temp = WEATHER_RAIN;
            break;

        case SEASON_SUMMER:
            if (rngval < 1)              // 1%
                temp = WEATHER_SHADE;
            else if (rngval < 4)         // 3%
                temp = WEATHER_RAIN_THUNDERSTORM;
            else if (rngval < 10)        // 6%
                temp = WEATHER_RAIN;
            else if (rngval < 25)        // 15%
                temp = WEATHER_DROUGHT;
            else                         // 75%
                temp = WEATHER_NONE;
            break;

        case SEASON_AUTUMN:
            if (rngval < 2)              // 2%
                temp = WEATHER_SHADE;
            else if (rngval < 42)        // 40%
                temp = WEATHER_RAIN;
            else if (rngval < 82)        // 40%
                temp = WEATHER_NONE;
            else                         // 18%
                temp = WEATHER_RAIN_THUNDERSTORM;
            break;

        case SEASON_WINTER:
            if (rngval < 4)              // 4%
                temp = WEATHER_SHADE;
            else if (rngval < 14)        // 10%
                temp = WEATHER_RAIN_THUNDERSTORM;
            else if (rngval < 29)        // 15%
                temp = WEATHER_NONE;
            else if (rngval < 55)        // 26%
                temp = WEATHER_RAIN;
            else                         // 45%
                temp = WEATHER_SNOW;
            break;
    }
    VarSet(VAR_SEASON_WEATHER, temp);
}