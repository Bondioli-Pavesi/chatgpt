/** @addtogroup littlefs
  * @{
*/

/**
  *******************************************************************************
  * @file           : littlefs.h
  * @author         : Massimo Casoni
  * @date           : 30/11/2024
  * @brief          : Header for the littlefs integration 
  * 
  *     Attributes for the CP23LFS file system:
  *     - DID: 
  *     - Data IDentifier. Includes encryption level (yes/no/type) and default authorization rules
  *     -
  *     - DATE: 
  *     - File creation date (dd-mm-yyyy format). If not available report "NA"
  *     - 
  *     - TIME: 
  *     - File creation time (HH:MM:SS format). If not available report "NA"
  *     -
  *     - GROUP: 
  *     - The group of the owner (mandatory)
  *     -   0: USER - End Customer/Dealer/Maintenance (No identification and authorization required. Name optional)
  *     -   1: MNF - Vehicle Manufacturer/B&P Customer (specify name in ECUtuner license) 
  *     -   2: BP - B&P Manufacturer/B&P Engineering-Production-Testing (specify name in ECUtuner license)
  *     -   3: SYS - Electronic system (Self-Generated, or from another B&P device: Node, Gateway, Server)
  *     -
  *     - AUTHORIZATION: 
  *     - Read/Write permissions. OWNERS of GROUP 2 and 3 can modify this attribute. All others must maintain default DID values
  *     - Authorization syntax: 2 bits per group, where bit0 = r and Bit1 = w (1 = enabled). LSB field for USER GROUP, then MNF, BP and finally SYS (8 bits total)
  *     - Example: wrwr-r-- (0xF4) --> SYS and BP can read/write, MNF can read, USER has no access
  *     - When the OWNER has no access rights (rw) on a specific file, this automatically becomes a hidden file
  *     -
  *     - OWNER: 
  *     - File owner name (who's sending or receiving. Optional only for GROUP 0 owners). If not available leave blank.
  *     -   For USER GROUP use: Optional, requested by ECUTuner (leave blank if not available). Character "*" forbidden
  *     -   For MNF GROUP use: Name of the ECUtuner license (mandatory). Character "*" forbidden
  *     -   For BP GROUP use: Name of the ECUtuner license (mandatory). Character "*" forbidden
  *     -   For SYS GROUP use (Assigned by receiver!): "*Local" (Same ECU), "*Can_XXX" (Another CAN ECU, where xxx = node address), or "*Server" (Non-ECU device, remote server)
  *     -
  *     - COMPANY: 
  *     - Company name of the owner. Optional only for GROUP 0 owners. If not available leave blank.
  *     -   For USER GROUP use: Optional, requested by ECUTuner (leave blank if not available)
  *     -   For MNF GROUP use: Company of the ECUtuner license (mandatory)
  *     -   For BP GROUP use: Company of the ECUtuner license (mandatory)
  *     -   For SYS GROUP use: "Bondioli-Pavesi"
  * 
  ********************************************************************************
*/

#ifndef __LITTLEFS_HEADER
#define __LITTLEFS_HEADER

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>
#include "lfs.h"

/* Attribute keys */
#define CP23LFS_ATTR_DID            0u                          /* File DID (Data IDentifier) */
#define CP23LFS_ATTR_DATE           1u                          /* File creation date (dd-mm-yyyy format) */
#define CP23LFS_ATTR_TIME           2u                          /* File creation time (HH:MM:SS format) */
#define CP23LFS_ATTR_GROUP          3u                          /* File owner group */
#define CP23LFS_ATTR_AUTH           4u                          /* File authorization */
#define CP23LFS_ATTR_OWNER          5u                          /* File owner name */
#define CP23LFS_ATTR_COMPANY        6u                          /* File owner company */

#define CP23LFS_ATTR_NUM            7u                          /* Number of file attributes */

#define CP23LFS_DATE_LEN            11u                         /* Maximum date length */
#define CP23LFS_TIME_LEN            9u                          /* Maximum time length */
#define CP23LFS_OWNER_LEN           32u                         /* Maximum owner length */
#define CP23LFS_COMPANY_LEN         32u                         /* Maximum company length */

#define LfsOwnerGroup(x)            ((x) & 0x03)                /* Owner group position */
#define LfsUserAuth(x)              ((x) & 0x03)                /* User authorization position */
#define LfsMNFAuth(x)               (((x) >> 2) & 0x03)         /* (Vehicle) Manufacturers authorization position */
#define LfsBPAuth(x)                (((x) >> 4) & 0x03)         /* BP authorization position */
#define LfsSysAuth(x)               (((x) >> 6) & 0x03)         /* System authorization position */

/* CP23LFS error code offset */
typedef uint32_t cp23lfs_errorcode_t;                           /* Error code type (native error code + CP23LFS_ERRORCODE_OFFSET)*/

#define CP23LFS_ERRORCODE(lfsec)    (cp23lfs_errorcode_t)(((int32_t)(CP23LFS_ERRORCODE_OFFSET))+((int32_t)(lfsec)))                        
#define CP23LFS_OK                  CP23LFS_ERRORCODE_OFFSET    /* No error */


typedef struct 
{
    uint16_t dId;                                               /* Data IDentifier */
    uint8_t date[CP23LFS_DATE_LEN];                             /* File creation date (dd-mm-yyyy format) - When len = 0 the date is missing */
    uint8_t time[CP23LFS_TIME_LEN];                             /* File creation time (HH:MM:SS format) - When Len = 0 the time is missing */
    uint8_t flags;                                              /* Flags: 
                                                                    - Bits 0-1: Owner's Group (00=USER, 01=MNF, 10=BP, 11=SYS)
                                                                    - Bits 2-7: Not used (= 0) 
                                                                */
    uint8_t authorization;                                      /* Authorization flags. r or w = 1 (enabled), r or w = 0 (disabled).
                                                                    - Bits 0-1: USER Group (r/w)
                                                                    - Bits 2-3: MNF Group (r/w)
                                                                    - Bits 4-5: BP Group (r/w)
                                                                    - Bits 6-7: SYS Group (r/w)
                                                                */
    uint8_t owner[CP23LFS_OWNER_LEN];                           /* File Owner name */
    uint8_t company[CP23LFS_COMPANY_LEN];                       /* File Owner company */
    uint32_t size;                                              /* File size (read only) */
    struct 
    {
        bool allocated;                                         /* File structure allocated (true). File structure available (false) */
        uint8_t buffer[CP23LFS_CACHE_SIZE];                     /* Service buffer */
        struct lfs_attr descr[CP23LFS_ATTR_NUM];                /* Attributes description */
        struct lfs_file_config fileCfg;                         /* File configuration */
        lfs_file_t file;                                        /* File object */
    } system;                                                   /* System attributes - Do not access from Application */
}cp23lfs_fileStructure_t;

typedef cp23lfs_fileStructure_t *cp23lfs_fileStructure_tPtr;
typedef cp23lfs_fileStructure_tPtr cp23lfs_file_t;






#ifdef __cplusplus
}
#endif

#endif /* __LITTLEFS_HEADER */

/**
  * @}
*/
 