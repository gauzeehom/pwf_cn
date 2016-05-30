#include <stdio.h>
#include "LunarSolarConverter.h"

int main()
{
    int solarDay;
    int solarMonth;
    int solarYear;
    
    solarYear = 2016;
    solarMonth = 11;
    solarDay = 28;

    Solar solar;

    solar.solarYear = solarYear;
    solar.solarMonth = solarMonth;
    solar.solarDay = solarDay;

    Lunar lunar = SolarToLunar(solar);

    printf("Lunar : %d-%d-%d\n",
	   lunar.lunarYear, lunar.lunarMonth, lunar.lunarDay);
       
    const char * lunarStr = LunarToString(lunar);
    printf("%s\n", lunarStr);

    return 0;
}