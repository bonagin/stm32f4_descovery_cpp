#include <cyg/kernel/diag.h>
#include <cyg/kernel/kapi.h>
#include <cyg/hal/var_io.h>
#include <stdlib.h>

#include "F4_RTC.h"
#include "utils.h"

F4RTC::F4RTC()
{
}

bool F4RTC::enterInitMode()
{
	cyg_uint32 cnt = 9000;
	cyg_uint32 reg32;
	//unlock write protection
	reg32 = 0xCA;
	HAL_WRITE_UINT32(CYGHWR_HAL_STM32_RTC + CYGHWR_HAL_STM32_RTC_WPR, reg32);
	reg32 = 0x53;
	HAL_WRITE_UINT32(CYGHWR_HAL_STM32_RTC + CYGHWR_HAL_STM32_RTC_WPR, reg32);

	reg32 = CYGHWR_HAL_STM32_RTC_ISR_INIT;
	HAL_WRITE_UINT32(CYGHWR_HAL_STM32_RTC + CYGHWR_HAL_STM32_RTC_ISR, reg32);
	//wait for initialization
	do
	{
		HAL_READ_UINT32(CYGHWR_HAL_STM32_RTC + CYGHWR_HAL_STM32_RTC_ISR, reg32);
	}while(!(reg32 & CYGHWR_HAL_STM32_RTC_ISR_INITF) && (cnt-- > 0));

	if(!(reg32 & CYGHWR_HAL_STM32_RTC_ISR_INITF))
		return false;

	return true;
}

bool F4RTC::exitInitMode()
{
	cyg_uint32 cnt = 9000;
	cyg_uint32 reg32 = 0;
	HAL_WRITE_UINT32(CYGHWR_HAL_STM32_RTC + CYGHWR_HAL_STM32_RTC_ISR, reg32);

	do
	{
		HAL_READ_UINT32(CYGHWR_HAL_STM32_RTC + CYGHWR_HAL_STM32_RTC_ISR, reg32);
	}while((reg32 & CYGHWR_HAL_STM32_RTC_ISR_INITF) && (cnt-- > 0));

	if(reg32 & CYGHWR_HAL_STM32_RTC_ISR_INITF)
		return false;

	return true;
}

bool F4RTC::disableBKProtect()
{
	cyg_uint32 cnt = 9000;
	cyg_uint32 bkpReg;
	HAL_READ_UINT32(CYGHWR_HAL_STM32_PWR + CYGHWR_HAL_STM32_PWR_CR, bkpReg);

	do
	{
	bkpReg |= CYGHWR_HAL_STM32_PWR_CR_DBP;
	HAL_WRITE_UINT32(CYGHWR_HAL_STM32_PWR + CYGHWR_HAL_STM32_PWR_CR, bkpReg);

	HAL_READ_UINT32(CYGHWR_HAL_STM32_PWR + CYGHWR_HAL_STM32_PWR_CR, bkpReg);
	}while(!(bkpReg & CYGHWR_HAL_STM32_PWR_CR_DBP) && (cnt-- >0));

	if(!(bkpReg & CYGHWR_HAL_STM32_PWR_CR_DBP))
		return false;

	return true;

}

void F4RTC::enableBKProtect()
{
	cyg_uint32 bkpReg;

	HAL_READ_UINT32(CYGHWR_HAL_STM32_PWR + CYGHWR_HAL_STM32_PWR_CR, bkpReg);
	bkpReg &= ~(CYGHWR_HAL_STM32_PWR_CR_DBP);
	HAL_WRITE_UINT32(CYGHWR_HAL_STM32_PWR + CYGHWR_HAL_STM32_PWR_CR, bkpReg);

}

bool F4RTC::init()
{
	diag_printf("F4RTC: Initializing\n");


	CYGHWR_HAL_STM32_CLOCK_ENABLE(CYGHWR_HAL_STM32_CLOCK(AHB1, BKPSRAM));
	CYGHWR_HAL_STM32_CLOCK_ENABLE(CYGHWR_HAL_STM32_CLOCK(APB1, PWR));

	cyg_uint32 reg32;
	HAL_READ_UINT32(CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_BDCR, reg32);

	if(reg32 & CYGHWR_HAL_STM32_RCC_BDCR_LSERDY)
	{
		diag_printf("F4RTC: LSE Ready\n");
		HAL_READ_UINT32(CYGHWR_HAL_STM32_RTC + CYGHWR_HAL_STM32_RTC_ISR, reg32);
		if(reg32 & CYGHWR_HAL_STM32_RTC_ISR_INITS)
		{
			diag_printf("F4RTC: RTC was initialized\n");

			if(syncTime())
			{
				time_t now = time(NULL);
				diag_printf("F4RTC: wallclock set - "GREEN("%s"), asctime(localtime(&now)));
			}
		}
	}
	else
	{
		diag_printf("F4RTC: Switching the LSE on\n");

		if(!disableBKProtect())
		{
			diag_printf("F4RTC: Could not disable Backup write protect\n");
			return false;
		}

		HAL_READ_UINT32(CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_BDCR, reg32);
		reg32 |= CYGHWR_HAL_STM32_RCC_BDCR_LSEON;
		HAL_WRITE_UINT32(CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_BDCR, reg32);

		do
		{
			HAL_READ_UINT32(CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_BDCR, reg32);
		}while(!(reg32 & CYGHWR_HAL_STM32_RCC_BDCR_LSERDY));
		diag_printf("F4RTC: LSE READY\n");

		//Select RTC Clock and enable
		HAL_READ_UINT32(CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_BDCR, reg32);
		reg32 |= CYGHWR_HAL_STM32_RCC_BDCR_RTCSEL_LSE | CYGHWR_HAL_STM32_RCC_BDCR_RTCEN;
		HAL_WRITE_UINT32(CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_BDCR, reg32);

//		HAL_READ_UINT32(CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_BDCR, reg32);

		enableBKProtect();
	}

	return true;
}

bool F4RTC::syncTime(time_t t)
{
	if(!t)
		t = getTime();

	if(!cyg_libc_time_settime(t))
		return true;

	return false;
}

bool F4RTC::hasBeenSet()
{
	cyg_uint32 reg32;
	HAL_READ_UINT32(CYGHWR_HAL_STM32_RTC + CYGHWR_HAL_STM32_RTC_ISR, reg32);
			if(reg32 & CYGHWR_HAL_STM32_RTC_ISR_INITS)
				return true;

			return false;
}

time_t F4RTC::getTime()
{
	cyg_uint32 reg32;
	struct tm tStruct;

	HAL_READ_UINT32(CYGHWR_HAL_STM32_RTC + CYGHWR_HAL_STM32_RTC_TR, reg32);
	tStruct.tm_hour = (((reg32 & CYGHWR_HAL_STM32_RTC_TR_HT) >> CYGHWR_HAL_STM32_RTC_TR_HT_SHIFT) * 10) + ((reg32 & CYGHWR_HAL_STM32_RTC_TR_HU) >> CYGHWR_HAL_STM32_RTC_TR_HU_SHIFT);
	tStruct.tm_min = (((reg32 & CYGHWR_HAL_STM32_RTC_TR_MNT) >> CYGHWR_HAL_STM32_RTC_TR_MNT_SHIFT) * 10) + ((reg32 & CYGHWR_HAL_STM32_RTC_TR_MNU) >> CYGHWR_HAL_STM32_RTC_TR_MNU_SHIFT);
	tStruct.tm_sec = (((reg32 & CYGHWR_HAL_STM32_RTC_TR_ST) >> CYGHWR_HAL_STM32_RTC_TR_ST_SHIFT) * 10) + ((reg32 & CYGHWR_HAL_STM32_RTC_TR_SU) >> CYGHWR_HAL_STM32_RTC_TR_SU_SHIFT);
	//diag_printf("%02d:%02d:%02d\n", tStruct.tm_hour, tStruct.tm_min, tStruct.tm_sec);

	HAL_READ_UINT32(CYGHWR_HAL_STM32_RTC + CYGHWR_HAL_STM32_RTC_DR, reg32);
	tStruct.tm_year = (((reg32 & CYGHWR_HAL_STM32_RTC_DR_YT) >> CYGHWR_HAL_STM32_RTC_DR_YT_SHIFT) * 10) + ((reg32 & CYGHWR_HAL_STM32_RTC_DR_YU) >> CYGHWR_HAL_STM32_RTC_DR_YU_SHIFT);
	tStruct.tm_mon = (((reg32 & CYGHWR_HAL_STM32_RTC_DR_MT) >> CYGHWR_HAL_STM32_RTC_DR_MT_SHIFT) * 10) + ((reg32 & CYGHWR_HAL_STM32_RTC_DR_MU) >> CYGHWR_HAL_STM32_RTC_DR_MU_SHIFT);
	tStruct.tm_mday = (((reg32 & CYGHWR_HAL_STM32_RTC_DR_DT) >> CYGHWR_HAL_STM32_RTC_DR_DT_SHIFT) * 10) + ((reg32 & CYGHWR_HAL_STM32_RTC_DR_DU) >> CYGHWR_HAL_STM32_RTC_DR_DU_SHIFT);
	//diag_printf("%04d-%02d-%02d\n", tStruct.tm_year, tStruct.tm_mon, tStruct.tm_mday);

	return mktime(&tStruct);
}

bool F4RTC::setTime(cyg_uint8 yy, cyg_uint8 mm, cyg_uint8 dd, cyg_uint8 h, cyg_uint8 m, cyg_uint8 s)
{

	cyg_uint32 dateReg =   (yy/10 << CYGHWR_HAL_STM32_RTC_DR_YT_SHIFT)
									| (yy%10 << CYGHWR_HAL_STM32_RTC_DR_YU_SHIFT)
									| (mm/10 << CYGHWR_HAL_STM32_RTC_DR_MT_SHIFT)
									| (mm%10 << CYGHWR_HAL_STM32_RTC_DR_MU_SHIFT)
									| (dd/10 << CYGHWR_HAL_STM32_RTC_DR_DT_SHIFT)
									| (dd%10 << CYGHWR_HAL_STM32_RTC_DR_DU_SHIFT)
									;

	cyg_uint32 timeReg =   (h/10 << CYGHWR_HAL_STM32_RTC_TR_HT_SHIFT)		//HOUR TENS
							| (h%10 << CYGHWR_HAL_STM32_RTC_TR_HU_SHIFT)	//HOUR UNITS
							| (m/10 << CYGHWR_HAL_STM32_RTC_TR_MNT_SHIFT)	//MINUTES TENS
							| (m%10 << CYGHWR_HAL_STM32_RTC_TR_MNU_SHIFT)	//MINUTES UNITS
							| (s/10 << CYGHWR_HAL_STM32_RTC_TR_ST_SHIFT)	//SECONDS TENS
							| (s%10 << CYGHWR_HAL_STM32_RTC_TR_SU_SHIFT)	//SECONDS UNITS
							;
	bool status;
	disableBKProtect();
	{
		status = enterInitMode();
		if(status)
		{
			HAL_WRITE_UINT32(CYGHWR_HAL_STM32_RTC + CYGHWR_HAL_STM32_RTC_TR, timeReg);
			HAL_WRITE_UINT32(CYGHWR_HAL_STM32_RTC + CYGHWR_HAL_STM32_RTC_DR, dateReg);

			status = exitInitMode();
		}
	}
	enableBKProtect();

	if(status)
	{
		HAL_DELAY_US(1000);
		return syncTime();
	}

	return false;
}

bool F4RTC::setTime(time_t* t)
{
	struct tm* tStruct = localtime(t);

	diag_printf("%d-", tStruct->tm_year + 1900);
	diag_printf("%02d-", tStruct->tm_mon + 1);
	diag_printf("%02d ", tStruct->tm_mday);
	diag_printf("%02d:", tStruct->tm_hour);
	diag_printf("%02d:", tStruct->tm_min);
	diag_printf("%02d\n", tStruct->tm_sec);

	return setTime(tStruct->tm_year + 1900, tStruct->tm_mon + 1, tStruct->tm_mday, tStruct->tm_hour, tStruct->tm_min, tStruct->tm_sec);
}


F4RTC::~F4RTC()
{

}

