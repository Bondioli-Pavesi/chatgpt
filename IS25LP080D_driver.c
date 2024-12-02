/** @addtogroup IS25LP080D_driver
  * @{
*/

/**
  *******************************************************************************
  * @file           : IS25LP080D_driver.c
  * @author         : Massimo Casoni
  * @date           : 29/11/2024
  * @brief          : IS25LP080D serial memory driver
  ********************************************************************************
*/

#include "utilities.h"  
#include "spi.h"
#include "gpio.h"
#include "e_emulator.h"
#include "stm32_assert.h"
#include "swtimer.h"
#include "IS25LP080D_driver.h"


// Define IS25LP080D commands
#define CMD_READ             0x03
#define CMD_WRITE_ENABLE     0x06
#define CMD_PAGE_PROGRAM     0x02
#define CMD_SECTOR_ERASE     0x20
#define CMD_BLOCK_ERASE      0xD8
#define CMD_READ_STATUS      0x05
#define CMD_WRITE_DISABLE    0x04


#define IS25LP080D_SPI_LINE             SPI1_ID     // SPI line for the memory
#define IS25LP080D_ERROR                -5          // Memory (LFS) error code 
#define IS25LP080D_BUSY_TIMEOUT_MSEC    2000        // Memory busy timeout (mSec)


static int IS25LP080D_WaitWhileBusy(uint8_t memOpcode);
/* static void DelayNOP(uint32_t cycles); */


void IS25LP080D_Init(void) 
{
    SPIn_Init(IS25LP080D_SPI_LINE);
}


int IS25LP080D_Read(const void *context, uint32_t addr, void *buffer, uint32_t size) 
{
    assert_param(buffer);
    assert_param(addr < 0x100000); 
    assert_param(size <= 0x100000); // 8 Mbit memory (1 MByte)
    NOT_USED(context);

    uint8_t cmd[4] = {CMD_READ, ((split32_t)addr).b[SPLIT_T2], ((split32_t)addr).b[SPLIT_T1], ((split32_t)addr).b[SPLIT_T0]};

    SPI_CS_Enable(SPI1_ID);
    if (!SPI_Transmit(IS25LP080D_SPI_LINE, cmd, sizeof(cmd))) 
    {
        SPI_CS_Disable(SPI1_ID);
        return IS25LP080D_ERROR;
    }
    if (!SPI_Receive(IS25LP080D_SPI_LINE, buffer, size))
    {
        SPI_CS_Disable(SPI1_ID);
        return IS25LP080D_ERROR;
    }
    SPI_CS_Disable(SPI1_ID);
    return 0;
}


int IS25LP080D_Program(const void *context, uint32_t addr, const void *buffer, uint32_t size) 
{
    assert_param(buffer);
    assert_param(addr < 0x100000); 
    assert_param(size <= 0x100000); // 8 Mbit memory (1 MByte)
    NOT_USED(context);

    uint8_t cmd[4] = {CMD_PAGE_PROGRAM, ((split32_t)addr).b[SPLIT_T2], ((split32_t)addr).b[SPLIT_T1], ((split32_t)addr).b[SPLIT_T0]};
    uint8_t wren = CMD_WRITE_ENABLE;

    /* Enable write */
    SPI_CS_Enable(SPI1_ID);
    if (!SPI_Transmit(IS25LP080D_SPI_LINE, &wren, 1)) 
    {
        SPI_CS_Disable(SPI1_ID);
        return IS25LP080D_ERROR;
    }
    SPI_CS_Disable(SPI1_ID);
    /* Send program command */
    SPI_CS_Enable(SPI1_ID);
    if (!SPI_Transmit(IS25LP080D_SPI_LINE, cmd, sizeof(cmd))) 
    {
        SPI_CS_Disable(SPI1_ID);
        return IS25LP080D_ERROR;
    }
    if (!SPI_Transmit(IS25LP080D_SPI_LINE, (void *)buffer, size)) 
    {
        SPI_CS_Disable(SPI1_ID);
        return IS25LP080D_ERROR;
    }
    SPI_CS_Disable(SPI1_ID);
    /* Wait for completion, and return result */
    return (IS25LP080D_WaitWhileBusy(CMD_PAGE_PROGRAM)); 
}


int IS25LP080D_Erase(const void *context, uint32_t addr, uint32_t size) 
{
    assert_param(addr < 0x100000); 
    assert_param(size <= 0x100000); // 8 Mbit memory (1 MByte)    
    NOT_USED(context);

    uint8_t cmd[4] = {0, ((split32_t)addr).b[SPLIT_T2], ((split32_t)addr).b[SPLIT_T1], ((split32_t)addr).b[SPLIT_T0]};
    uint8_t wren = CMD_WRITE_ENABLE;

    // Determine command based on size
    if (size == 4096) 
    {
        cmd[0] = CMD_SECTOR_ERASE;
    } else if (size == 65536) 
    {
        cmd[0] = CMD_BLOCK_ERASE;
    } 
    else 
    {
        return IS25LP080D_ERROR; // Unsupported size
    }
    // Enable write
    SPI_CS_Enable(SPI1_ID);
    if (!SPI_Transmit(IS25LP080D_SPI_LINE, &wren, 1)) 
    {
        SPI_CS_Disable(SPI1_ID);
        return IS25LP080D_ERROR;
    }
    SPI_CS_Disable(SPI1_ID);
    // Send erase command
    SPI_CS_Enable(SPI1_ID);
    if (!SPI_Transmit(IS25LP080D_SPI_LINE, cmd, sizeof(cmd))) 
    {
        SPI_CS_Disable(SPI1_ID);
        return IS25LP080D_ERROR;
    }
    SPI_CS_Disable(SPI1_ID);
    // Wait for completion, and return result
    return (IS25LP080D_WaitWhileBusy(cmd[0]));
}


int IS25LP080D_Sync(const void *context) 
{
    NOT_USED(context);

    return 0; // No action needed for blocking operations
}


/**
  * @brief Waits while the memory is busy.
  * @param memOpcode The memory operation code.
  * 
  * This function waits while the memory is busy performing an operation.
  * 
  * @return 0 if the memory is ready, a negative number if an error occurred.
  */
static int IS25LP080D_WaitWhileBusy(uint8_t memOpcode) 
{
    uint8_t status = 0;
    uint8_t cmd = CMD_READ_STATUS;
    swtimer_t busyTimeout;

    LoadSWTimer(&busyTimeout);
    do 
    {
        if (SWTimerTimeout(&busyTimeout, IS25LP080D_BUSY_TIMEOUT_MSEC, mSec, NULL)) 
        {
            RTT_Printf(RTT_EC_IS25LP080D_TIMEOUT, memOpcode);
            ManageEventError(EC_IS25LP080D_TIMEOUT, true, memOpcode);
            return IS25LP080D_ERROR;
        }
        SPI_CS_Enable(SPI1_ID);
        if (!SPI_Transmit(IS25LP080D_SPI_LINE, &cmd, 1))
        {
            SPI_CS_Disable(SPI1_ID);
            return IS25LP080D_ERROR;
        }
        if (!SPI_Receive(IS25LP080D_SPI_LINE, &status, 1)) 
        {
            SPI_CS_Disable(SPI1_ID);
            return IS25LP080D_ERROR;
        }
        SPI_CS_Disable(SPI1_ID);
    } while (status & 0x01);    // WIP bit is set
    return 0;   // Success
}


/**
 * @brief Delays execution for a specified number of cycles using NOP instructions.
 *
 * This function introduces a delay by executing a specified number of NOP (No Operation) 
 * instructions. The delay duration is about 6.25 nSec multiplied by the number of cycles.
 *
 * @param cycles The number of NOP (6.25 nSec) cycles to delay.
 */
/*
static void DelayNOP(uint32_t cycles) 
{
    while (cycles--) 
    {
        __asm__("NOP");
    }
}
*/

/**
  * @}
*/
 
