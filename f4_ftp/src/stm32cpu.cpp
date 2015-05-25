#include <cyg/hal/hal_io.h>
#include <cyg/kernel/diag.h>
#include <cyg/hal/var_io.h>
#include <cyg/kernel/kapi.h>
#include <string.h>
#include <stdlib.h>

#include "stm32cpu.h"
#include "logger.h"


 void stm32cpu::info()
 {
	 diag_printf("CPU Information:\n");

	 cyg_uint64 id = stm32cpu::uniqueID();
	 diag_printf(" - Unique ID: 0x%08X%08X\n", (cyg_uint32)(id >> 32) , (cyg_uint32)(id & 0xFFFFFFFF));

	 diag_printf(" - %6d kB FLASH\n", stm32cpu::flashSize());
	 diag_printf(" - %6d B RAM\n", stm32cpu::ramSize());

	 cyg_uint32 reg32;
	 HAL_READ_UINT32( CYGHWR_HAL_STM32_MCU_ID, reg32);
	 if(CYGHWR_HAL_STM32_MCU_ID_DEV(reg32) == CYGHWR_HAL_STM32_MCU_ID_DEV_MEDIUM)
		 diag_printf(" - Medium Density Line\n");
	 else if(CYGHWR_HAL_STM32_MCU_ID_DEV(reg32) == CYGHWR_HAL_STM32_MCU_ID_DEV_HIGH)
		 diag_printf(" - High Density Line\n");
	 else if(CYGHWR_HAL_STM32_MCU_ID_DEV(reg32) == 0x430)
		 diag_printf(" - XL Density Line\n");
	 else if(CYGHWR_HAL_STM32_MCU_ID_DEV(reg32) == 0x418)
		 diag_printf(" - Connectivity Line\n");

	 diag_printf(" - Revision: 0x%04X\n", CYGHWR_HAL_STM32_MCU_ID_REV(reg32));

 }

 cyg_uint64 stm32cpu::uniqueID()
{
	cyg_uint64 reg64;
	cyg_uint32 reg32;
	HAL_READ_UINT32( 0x1FFFF7E8, reg32);
	HAL_READ_UINT32( 0x1FFFF7E8 + 4, reg32);
	reg64 = reg32;
	HAL_READ_UINT32( 0x1FFFF7E8 + 8, reg32);
	reg64 |= (cyg_uint64)reg32 << 32;

	return reg64;
}

cyg_uint16 stm32cpu::flashSize()
{
	cyg_uint32 reg32;
	HAL_READ_UINT32(CYGHWR_HAL_STM32_DEV_SIG, reg32);

	return CYGHWR_HAL_STM32_DEV_SIG_FSIZE(reg32);
}

void stm32cpu::flashCmd(cTerm & term,int argc,char *argv[])
{
	diag_printf("Flash Command \n");
	if(!(strcmp(argv[1],"read")))
	{
		cyg_uint32 Address  = (cyg_uint32)strtoul(argv[2],NULL,16);

		cyg_uint32 reg32 = readflash(Address);

		diag_printf("Reading at Address : 0x%8X \n", Address);
		diag_printf("Data Read : 0x%8X \n", reg32);

	}
	else if(!(strcmp(argv[1],"write")))
	{
	    cyg_uint32 data     = (cyg_uint32)strtoul(argv[2],NULL,16);
	    cyg_uint32 Address  = (cyg_uint32)strtoul(argv[3],NULL,16);

	    diag_printf("Writing : 0x%8X at address 0x%8X \n", data, Address);

	    witeflash(data,Address);
	}
	else if(!(strcmp(argv[1],"unlock")))
	{
		flashunlock();
	}
	else if(!(strcmp(argv[1],"eraseall")))
	{
		eraseallsectors();
	}
	else if(!(strcmp(argv[1],"erase")))
	{
		cyg_uint32 Sector  = (cyg_uint32)strtoul(argv[2],NULL,16);
		eraseflash(Sector);
	}
	else if(!(strcmp(argv[1],"readlogs")))
	{

	    cyg_uint32 start_address_ = (cyg_uint32)strtoul(argv[2],NULL,16);
	    cyg_uint32 last_address_  = (cyg_uint32)strtoul(argv[3],NULL,16);

	    diag_printf("Reading logs from : 0x%8X to 0x%8X \n", start_address_, last_address_);

		cLOG::get()->read_logs(start_address_,last_address_);
	}
	else
	{
		diag_printf("Not a valid flash command\n");
	}
}

cyg_uint32 stm32cpu::readflash(cyg_uint32 Address)
{
	flashunlock();

	cyg_uint32 reg32;

	HAL_READ_UINT32(CYGHWR_HAL_STM32_FLASH + CYGHWR_HAL_STM32_FLASH_ACR, reg32);

	reg32 |= CYGHWR_HAL_STM32_FLASH_ACR_LATENCY(5);

	HAL_WRITE_UINT32(CYGHWR_HAL_STM32_FLASH + CYGHWR_HAL_STM32_FLASH_ACR, reg32);

	HAL_READ_UINT32(Address, reg32);

	return reg32;

	lock();
}
void stm32cpu::witeflash(cyg_uint32 data, cyg_uint32 Address)
{
	flashunlock();

	cyg_uint32 reg32;

    HAL_READ_UINT32(CYGHWR_HAL_STM32_FLASH + CYGHWR_HAL_STM32_FLASH_CR, reg32);

    reg32 &= ((cyg_uint32)0xFFFFFCFF);
    reg32 |= CYGHWR_HAL_STM32_FLASH_CR_PSIZE_32;
    reg32 |= CYGHWR_HAL_STM32_FLASH_CR_PG;

    HAL_WRITE_UINT32(CYGHWR_HAL_STM32_FLASH + CYGHWR_HAL_STM32_FLASH_CR, reg32);

    cyg_thread_delay(10);

    HAL_WRITE_UINT32(Address, data);

    cyg_thread_delay(10);

    reg32 &= (~CYGHWR_HAL_STM32_FLASH_CR_SER);

	lock();
}

void stm32cpu::eraseflash(cyg_uint32 Sector)
{
	cyg_uint32 reg32;

    HAL_READ_UINT32(CYGHWR_HAL_STM32_FLASH + CYGHWR_HAL_STM32_FLASH_CR, reg32);

    reg32 &= ((cyg_uint32)0xFFFFFCFF);
    reg32 |= CYGHWR_HAL_STM32_FLASH_CR_PSIZE_32;
    reg32 &= ((cyg_uint32)0xFFFFFF07);
    reg32 |= CYGHWR_HAL_STM32_FLASH_CR_SER;
    reg32 |= CYGHWR_HAL_STM32_FLASH_CR_SNB(Sector);
    reg32 |= CYGHWR_HAL_STM32_FLASH_CR_STRT;

    HAL_WRITE_UINT32(CYGHWR_HAL_STM32_FLASH + CYGHWR_HAL_STM32_FLASH_CR, reg32);

//    flash_status status = getflashstatus();
//
//    while(status == FLASH_BUSY)
//    {
//    	status = getflashstatus();
//    }

    cyg_thread_delay(100);

    reg32 &= (~CYGHWR_HAL_STM32_FLASH_CR_SER);
    reg32 &= ((cyg_uint32)0xFFFFFF07);
}

void stm32cpu::eraseallsectors()
{
	cyg_uint32 reg32;

	    HAL_READ_UINT32(CYGHWR_HAL_STM32_FLASH + CYGHWR_HAL_STM32_FLASH_CR, reg32);

	    reg32 &= ((cyg_uint32)0xFFFFFCFF);
	    reg32 |= CYGHWR_HAL_STM32_FLASH_CR_PSIZE_32;
	    reg32 |= CYGHWR_HAL_STM32_FLASH_CR_MER;
	    reg32 |= CYGHWR_HAL_STM32_FLASH_CR_STRT;

	    HAL_WRITE_UINT32(CYGHWR_HAL_STM32_FLASH + CYGHWR_HAL_STM32_FLASH_CR, reg32);

//	    flash_status status = getflashstatus();
//
//	    while(status == FLASH_BUSY)
//	    {
//	    	status = getflashstatus();
//	    }

	    cyg_thread_delay(200);

	    reg32 &= (~CYGHWR_HAL_STM32_FLASH_CR_MER);
}

void stm32cpu::flashunlock()
{
	cyg_uint32 reg32;

    HAL_READ_UINT32(CYGHWR_HAL_STM32_FLASH + CYGHWR_HAL_STM32_FLASH_CR, reg32);

    if(reg32 & CYGHWR_HAL_STM32_FLASH_CR_LOCK)
    {
//    	diag_printf("flash locked, writing unlock keys\n");
    	HAL_WRITE_UINT32(CYGHWR_HAL_STM32_FLASH + CYGHWR_HAL_STM32_FLASH_KEYR, CYGHWR_HAL_STM32_FLASH_KEYR_KEY1);
    	HAL_WRITE_UINT32(CYGHWR_HAL_STM32_FLASH + CYGHWR_HAL_STM32_FLASH_KEYR, CYGHWR_HAL_STM32_FLASH_KEYR_KEY2);

        HAL_READ_UINT32(CYGHWR_HAL_STM32_FLASH + CYGHWR_HAL_STM32_FLASH_CR, reg32);
        if(reg32 & CYGHWR_HAL_STM32_FLASH_CR_LOCK)
        {
        	diag_printf("Failed to unlock the flash\n");
        }
    }

}

void stm32cpu::lock()
{
	cyg_uint32 reg32;
	HAL_READ_UINT32(CYGHWR_HAL_STM32_FLASH + CYGHWR_HAL_STM32_FLASH_CR, reg32);
	reg32 |= CYGHWR_HAL_STM32_FLASH_CR_LOCK;

	HAL_WRITE_UINT32(CYGHWR_HAL_STM32_FLASH + CYGHWR_HAL_STM32_FLASH_CR, reg32);

	HAL_READ_UINT32(CYGHWR_HAL_STM32_FLASH + CYGHWR_HAL_STM32_FLASH_CR, reg32);
    if(reg32 & CYGHWR_HAL_STM32_FLASH_CR_LOCK)
    {
    	diag_printf("Flash successfully locked\n");
    }
    else
    {
    	diag_printf("Failed to lock the flash\n");
    }
}

flash_status stm32cpu::getflashstatus()
{

	 flash_status flashstatus = FLASH_COMPLETE;

	 cyg_uint32 reg32;
	 HAL_READ_UINT32(CYGHWR_HAL_STM32_FLASH + CYGHWR_HAL_STM32_FLASH_CR, reg32);

	  if((CYGHWR_HAL_STM32_FLASH_CR & CYGHWR_HAL_STM32_FLASH_SR_BSY) == 1)
	  {
	    flashstatus = FLASH_BUSY;
	  }
	  else
	  {
	    if((CYGHWR_HAL_STM32_FLASH_CR & CYGHWR_HAL_STM32_FLASH_SR_WRPERR) != (cyg_uint32)0x00)
	    {
	      flashstatus = FLASH_ERROR_WRP;
	    }
	    else
	    {
	      if((CYGHWR_HAL_STM32_FLASH_CR & (cyg_uint32)0xEF) != (cyg_uint32)0x00)
	      {
	        flashstatus = FLASH_ERROR_PROGRAM;
	      }
	      else
	      {
	        if((CYGHWR_HAL_STM32_FLASH_CR & CYGHWR_HAL_STM32_FLASH_SR_OPERR) != (cyg_uint32)0x00)
	        {
	          flashstatus = FLASH_ERROR_OPERATION;
	        }
	        else
	        {
	          flashstatus = FLASH_COMPLETE;
	        }
	      }
	    }
	  }
	  /* Return the FLASH Status */
	  return flashstatus;
}

cyg_uint16 stm32cpu::ramSize()
{
	cyg_uint32 reg32;
	HAL_READ_UINT32(CYGHWR_HAL_STM32_DEV_SIG, reg32);

	return CYGHWR_HAL_STM32_DEV_SIG_RSIZE(reg32);
}

void stm32cpu::sysResetStatus()
{
	cyg_uint32 reg32;

	HAL_READ_UINT32(CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_CSR, reg32);

//	if(reg32 & CYGHWR_HAL_STM32_RCC_CSR_PINRSTF)
//	{
//		diag_printf("Reset from NRST pin\n");
//	}

	if(reg32 & CYGHWR_HAL_STM32_RCC_CSR_SFTRSTF)
	{
		diag_printf(" - Reset from Software\n");
	}
	if(reg32 & CYGHWR_HAL_STM32_RCC_CSR_IWDGRSTF)
	{
		diag_printf(" - Reset from Independent Watchdog\n");
	}
	if(reg32 & CYGHWR_HAL_STM32_RCC_CSR_WWDGRSTF)
	{
		diag_printf(" - Reset from Window Watchdog\n");
	}
	if(reg32 & CYGHWR_HAL_STM32_RCC_CSR_LPWRRSTF)
	{
		diag_printf(" - Reset from Low power \n");
	}


	//Clear reset flags
	HAL_WRITE_UINT32(CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_CSR, CYGHWR_HAL_STM32_RCC_CSR_RMVF);
}

void stm32cpu::pStatus(cTerm & term, int argc,char * argv[])
{
	diag_printf("Peripheral power status\n");

		cyg_uint32 reg32 = 0;
		//TODO HAL_READ_UINT32(CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_AHBENR, reg32);
		diag_printf(" AHB peripheral clock enable register:\n  - 0x%08X\n", reg32);
		if(reg32 & CYGHWR_HAL_STM32_RCC_AHBENR_DMA1)
		{
			diag_printf("   - DMA1 enabled\n");
		}
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_AHBENR_DMA2))
		{
			diag_printf("   - DMA2 enabled\n");
		}
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_AHBENR_SRAM))
		{
			diag_printf("   - SRAM enabled\n");
		}
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_AHBENR_FLITF))
		{
			diag_printf("   - FLITF enabled\n");
		}
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_AHBENR_CRC))
		{
			diag_printf("   - CRC enabled\n");
		}
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_AHBENR_FSMC))
		{
			diag_printf("   - FSMC enabled\n");
		}
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_AHBENR_SDIO))
		{
			diag_printf("   - SDIO enabled\n");
		}
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_AHBENR_OTGFS))
		{
			diag_printf("   - USB OTG enabled\n");
		}

		HAL_READ_UINT32(CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_AHB1ENR, reg32);
		diag_printf(" AHB1 peripheral clock enable register:\n  - 0x%08X\n", reg32);
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_AHB1ENR_BKPSRAM))
		{
			diag_printf("   - Back Up Interface enabled\n");
		}
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_AHB1ENR_GPIOA))
		{
			diag_printf("   - GPIOA enabled\n");
		}
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_AHB1ENR_GPIOB))
		{
			diag_printf("   - GPIOB enabled\n");
		}
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_AHB1ENR_GPIOC))
		{
			diag_printf("   - GPIOC enabled\n");
		}
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_AHB1ENR_GPIOD))
		{
			diag_printf("   - GPIOD enabled\n");
		}
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_AHB1ENR_GPIOE))
		{
			diag_printf("   - GPIOE enabled\n");
		}
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_AHB1ENR_GPIOF))
		{
			diag_printf("   - GPIOF enabled\n");
		}
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_AHB1ENR_OTGHS))
		{
			diag_printf("   - OTG HS enabled\n");
		}
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_AHB1ENR_OTGHSULPI))
		{
			diag_printf("   - OTG HS ULPI enabled\n");
		}

		HAL_READ_UINT32(CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_AHB2ENR, reg32);
		diag_printf(" AHB2 peripheral clock enable register:\n  - 0x%08X\n", reg32);
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_AHB2ENR_OTGFS))
		{
			diag_printf("   - OTG FS enabled\n");
		}
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_AHB2ENR_RNG))
		{
			diag_printf("   - Random Number generator enabled\n");
		}

		HAL_READ_UINT32(CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_APB1ENR, reg32);
		diag_printf(" APB1 peripheral clock enable register:\n  - 0x%08X\n", reg32);
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_APB1ENR_TIM2))
		{
			diag_printf("   - Timer2 enabled\n");
		}
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_APB1ENR_TIM3))
		{
			diag_printf("   - Timer3 enabled\n");
		}
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_APB1ENR_TIM4))
		{
			diag_printf("   - Timer4 enabled\n");
		}
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_APB1ENR_TIM5))
		{
			diag_printf("   - Timer5 enabled\n");
		}
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_APB1ENR_TIM6))
		{
			diag_printf("   - Timer6 enabled\n");
		}
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_APB1ENR_TIM7))
		{
			diag_printf("   - Timer7 enabled\n");
		}
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_APB1ENR_WWDG))
		{
			diag_printf("   - Window Watchdog enabled\n");
		}
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_APB1ENR_SPI2))
		{
			diag_printf("   - SPI2 enabled\n");
		}
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_APB1ENR_SPI3))
		{
			diag_printf("   - SPI3 enabled\n");
		}
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_APB1ENR_UART2))
		{
			diag_printf("   - UART2 enabled\n");
		}
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_APB1ENR_UART3))
		{
			diag_printf("   - UART3 enabled\n");
		}
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_APB1ENR_UART4))
		{
			diag_printf("   - UART4 enabled\n");
		}
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_APB1ENR_UART5))
		{
			diag_printf("   - UART5 enabled\n");
		}
		if(reg32 & (CYGHWR_HAL_STM32_RCC_APB1ENR_I2C1))
		{
			diag_printf("   - I2C1 enabled\n");
		}
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_APB1ENR_I2C2))
		{
			diag_printf("   - I2C2 enabled\n");
		}
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_APB1ENR_CAN1))
		{
			diag_printf("   - CAN1 enabled\n");
		}
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_APB1ENR_CAN2))
		{
			diag_printf("   - CAN2 enabled\n");
		}
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_APB1ENR_PWR))
		{
			diag_printf("   - Power Interface  enabled\n");
		}
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_APB1ENR_DAC))
		{
			diag_printf("   - DAC enabled\n");
		}

		HAL_READ_UINT32(CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_APB2ENR, reg32);
		diag_printf(" APB2 peripheral clock enable register:\n  - 0x%08X\n", reg32);
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_APB2ENR_SPI1))
		{
			diag_printf("   - SPI1 enabled\n");
		}
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_APB2ENR_UART1))
		{
			diag_printf("   - UART1 enabled\n");
		}
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_APB2ENR_UART6))
		{
			diag_printf("   - UART6 enabled\n");
		}
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_APB2ENR_TIM1))
		{
			diag_printf("   - TIM1 enabled\n");
		}
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_APB2ENR_TIM8))
		{
			diag_printf("   - TIM8 enabled\n");
		}
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_APB2ENR_TIM9))
		{
			diag_printf("   - TIM9 enabled\n");
		}
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_APB2ENR_TIM10))
		{
			diag_printf("   - TIM10 enabled\n");
		}
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_APB2ENR_TIM11))
		{
			diag_printf("   - TIM11 enabled\n");
		}
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_APB2ENR_ADC1))
		{
			diag_printf("   - ADC1 enabled\n");
		}
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_APB2ENR_ADC2))
		{
			diag_printf("   - ADC2 enabled\n");
		}
		if(reg32 & (1 << CYGHWR_HAL_STM32_RCC_APB2ENR_ADC3))
		{
			diag_printf("   - ADC3 enabled\n");
		}
}
