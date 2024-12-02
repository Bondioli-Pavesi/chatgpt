/** @addtogroup IS25LP080D_driver
  * @{
*/

/**
  *******************************************************************************
  * @file           : IS25LP080D_driver.h
  * @author         : Massimo Casoni
  * @date           : 29/11/2024
  * @brief          : IS25LP080D serial memory driver header
  ********************************************************************************
*/

#ifndef __IS25LP080D_DRIVER_HEADER
#define __IS25LP080D_DRIVER_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>


/**
 * @brief Initializes the memory.
 * 
 * This function initializes the memory.
 * 
 * @param None
 * @return Nothing
 */
void IS25LP080D_Init(void);


/**
 * @brief Reads data from the memory.
 * 
 * This function reads data from the memory starting from the specified address.
 * 
 * @param context The memory context (not used).
 * @param addr The memory address to start reading from.
 * @param buffer The buffer to store the read data.
 * @param size The number of bytes to read.
 * 
 * @return 0 if the operation was successful, IS25LP080D_ERROR (-5) if an error occurred.
 */
int IS25LP080D_Read(const void *context, uint32_t addr, void *buffer, uint32_t size);


/**
 * @brief Programs data into the memory.
 * 
 * This function programs data into the memory starting from the specified address.
 * 
 * @param context The memory context (not used).
 * @param addr The memory address to start programming from.
 * @param buffer The buffer containing the data to program.
 * @param size The number of bytes to program.
 * 
 * @return 0 if the operation was successful, IS25LP080D_ERROR (-5) if an error occurred.
 */
int IS25LP080D_Program(const void *context, uint32_t addr, const void *buffer, uint32_t size);


/**
 * @brief Erases data from the memory.
 * 
 * This function erases data from the memory starting from the specified address.
 * 
 * @param context The memory context (not used).
 * @param addr The memory address to start erasing from.
 * @param size The number of bytes to erase.
 * 
 * @return 0 if the operation was successful, IS25LP080D_ERROR (-5) if an error occurred.
 */
int IS25LP080D_Erase(const void *context, uint32_t addr, uint32_t size);


/**
 * @brief Synchronizes the memory.
 * 
 * This function synchronizes the memory.
 * 
 * @param context The memory context (not used).
 * 
 * @return 0 if the operation was successful, IS25LP080D_ERROR (-5) if an error occurred.
 */
int IS25LP080D_Sync(const void *context);


#ifdef __cplusplus
}
#endif

#endif /* __IS25LP080D_DRIVER_HEADER */

/**
  * @}
*/
 