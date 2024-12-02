/** @addtogroup littlefs
  * @{
*/

/**
  *******************************************************************************
  * @file           : littlefs.c
  * @author         : Massimo Casoni
  * @date           : 30/11/2024
  * @brief          : littlefs integration module
  ********************************************************************************
*/

#include "utilities.h"
#include "emulator.h"
#include "littlefs.h"
#include "stm32_assert.h"


#define CP23LFS_FILES_MAX       8u                                  /* Max number of opened files */


static cp23lfs_fileStructure_t cp23lsf_file[CP23LFS_FILES_MAX];     /* Files buffer pool */



static cp23lfs_file_t CP23_InitFileAttribute(void);
static void CP23_ReleaseFileStructure(cp23lfs_file_t cp23lfs_file);



static cp23lfs_file_t CP23_GetFileStructure(void)
{
    static uint32_t const parOffset[CP23LFS_ATTR_NUM] = {offsetof(cp23lfs_fileStructure_t, dId), offsetof(cp23lfs_fileStructure_t, date), offsetof(cp23lfs_fileStructure_t, time), offsetof(cp23lfs_fileStructure_t, flags), 
                                                        offsetof(cp23lfs_fileStructure_t, authorization), offsetof(cp23lfs_fileStructure_t, owner), offsetof(cp23lfs_fileStructure_t, company)};
    static uint32_t const parSize[CP23LFS_ATTR_NUM] = {sizeof(cp23lsf_file[0].dId), sizeof(cp23lsf_file[0].date), sizeof(cp23lsf_file[0].time), sizeof(cp23lsf_file[0].flags), 
                                                        sizeof(cp23lsf_file[0].authorization), sizeof(cp23lsf_file[0].owner), sizeof(cp23lsf_file[0].company)};
    cp23lfs_file_t retVal = NULL;   /* Default: not available */
    uint32_t cnt;

    for (cnt = 0 ; cnt < CP23LFS_FILES_MAX ; cnt++)
    {
        if (cp23lsf_file[cnt].system.allocated == false)
        {
            cp23lsf_file[cnt].system.allocated = true;
            retVal = &(cp23lsf_file[cnt]);
            retVal->system.allocated = true;
            break;
        }
    }
    if (retVal)
    {
        /* Clear the file structure */
        for (cnt = 0 ; cnt < (sizeof(cp23lfs_fileStructure_t) / sizeof(uint32_t)) ; cnt++)
        {
            *(((uint32_t *)(retVal)) + cnt) = 0u;
        }
        for (cnt = 0 ; cnt < (sizeof(cp23lfs_fileStructure_t) % sizeof(uint32_t)) ; cnt++)
        {
            *(((uint8_t *)(retVal)) + cnt) = 0u;
        }
        /* Init attributes description */
        for (cnt = 0 ; cnt < CP23LFS_ATTR_NUM ; cnt++)
        {
            retVal->system.descr[cnt].type = cnt;
            retVal->system.descr[cnt].buffer = (void *)(((char *)(retVal)) + parOffset[cnt]);
            retVal->system.descr[cnt].size = parSize[cnt];
        }
        /* Init file configuration */
        retVal->system.fileCfg.attrs = &(retVal->system.descr[0]);
        retVal->system.fileCfg.attr_count = CP23LFS_ATTR_NUM;
        retVal->system.fileCfg.buffer = (void *)(retVal->system.buffer);
    }
    return retVal;
}


static void CP23_ReleaseFileStructure(cp23lfs_file_t cp23lfs_file)
{
    assert_param(cp23lfs_file);

    cp23lfs_file->system.allocated = false;
}



/**
  * @}
*/
