/*
 * Copyright (c) 2009 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * ITE Upgrade Library.
 *
 * @author Jim Tan
 * @version 1.0
 * @date 2013
 * @copyright ITE Tech. Inc. All Rights Reserved.
 */
/** @defgroup ug ITE Upgrade Library
 *  @{
 */
#ifndef ITE_UG_H
#define ITE_UG_H

#include "ite/itc.h"
#include "ite/itp.h"

#ifdef __cplusplus
extern "C"
{
#endif

// Error codes
#define UG_READ_FILE_FAIL      1    ///< Reads file fail error
#define UG_INVALID_FILE_FORMAT 2    ///< Invalid file format error
#define UG_OUT_OF_MEM          3    ///< Out of memory error
#define UG_NO_IMPL             4    ///< No implement error

// Upgrade
/** @defgroup ug_upgrade Upgrade
 *  @{
 */

/**
 * Checks last upgrade is success or not.
 *
 * @return 0 (success), or otherwise (failed).
 */
int ugUpgradeCheck(void);

/**
 * Do the upgrade procedure from the specified file stream.
 *
 * @param file the file stream.
 * @return 0 (OK), or otherwise (ERROR).
 */
int ugUpgradePackage(ITCStream* file);

/**
 * Checks the CRC value of the specified file stream.
 *
 * @param f the file stream.
 * @param buffer Retrieves file content if buffer address is available, null for ignore.
 * @return 0 (OK), or otherwise (ERROR).
 * @par Example:
 * @code
    ITCFileStream fileStream;
    int ret;

    // open package file to do the upgrade
    ret = itcFileStreamOpen(&fileStream, "B:/ITEPKG03.PKG", false);
    if (ret)
        return ret; // open file error
    
    ret = ugCheckCrc(&fileStream, NULL);
    if (ret)
        return ret; // check crc fail
        
    ret = ugUpgradePackage(&fileStream);
    if (ret)
        return ret; // upgrade fail
        
 * @endcode
 */
int ugCheckCrc(ITCStream *f, uint8_t* buffer);

/**
 * Restores files from /backup directory of private partition disk.
 *
 * @return 0 (OK), or otherwise (ERROR).
 */
int ugResetFactory(void);

/** @} */ // end of ug_upgrade

/** @defgroup ug_crc Files CRC check
 *  @{
 */

/**
 * Checks files CRCs by crc file at specified path.
 *
 * @param path the root path to check.
 * @param crcpath the saved crc file path to verify.
 * @return 0 (OK), or otherwise (ERROR).
 */
int ugCheckFilesCrc(char* path, char* crcpath);

/**
 * Sets file's CRC to crc file.
 *
 * @param filepath the relative path of file to calculate crc and save.
 * @param path The path to save to crc file.
 * @param crcpath the crc file path to save.
 * @return 0 (OK), or otherwise (ERROR).
 * @par Example:
 * @code
    int ret;
    
    ret = ugCheckFilesCrc("B:", "B:/ite_crc.dat");
    if (ret)
        return ret; // check crc fail

    ret = ugSetFileCrc("/font/DroidSans.ttf", "B:", "B:/ite_crc.dat");
    if (ret)
        return ret; // set crc fail
        
 * @endcode
 */
int ugSetFileCrc(char* filepath, char* path, char* crcpath);

/**
 * Upgrades the files' CRC of crc file.
 *
 * @param path The path to upgrade to crc file.
 * @param crcpath the crc file path to upgrade.
 * @return 0 (OK), or otherwise (ERROR).
 */
int ugUpgradeFilesCrc(char* path, char* crcpath);

/**
 * Copies file.
 *
 * @param destPath The path of destination file.
 * @param srcPath The path of source file.
 * @return 0 (OK), or otherwise (ERROR).
 */
int ugCopyFile(const char* destPath, const char* srcPath);

/**
 * Restores directory.
 *
 * @param dest The path of destination directory.
 * @param src The path of source directory.
 * @return 0 (OK), or otherwise (ERROR).
 */
int ugRestoreDir(const char* dest, const char* src);

/**
 * Copies directory recursively.
 *
 * @param dest The path of destination directory.
 * @param src The path of source directory.
 * @return 0 (OK), or otherwise (ERROR).
 */
int ugCopyDirectory(const char* dest, const char* src);

/**
 * Deletes directory recursively.
 *
 * @param dirname The path of destination directory to delete.
 */
void ugDeleteDirectory(char* dirname);

/** @} */ // end of ug_crc

// Internal functions

/**
 * Starts to upgrade. 
 * @return 0 (OK), or otherwise (ERROR).
 */
int ugUpgradeStart(void);

/**
 * Upgrades finished.
 * @return 0 (OK), or otherwise (ERROR).
 */
int ugUpgradeFinish(void);

int ugUpgradeDevice(ITCStream *f, ITPDisk* disk, bool partition, bool nopartition);
int ugUpgradeDirectory(ITCStream *f, char* drive);
int ugUpgradeEnd(ITCStream *f);
int ugUpgradeFile(ITCStream *f, char* drive);
int ugUpgradePartition(ITCStream *f, ITPDisk disk, int partition, char* drive, bool format);
int ugUpgradeRawData(ITCStream *f, ITPDisk disk);
int ugUpgradeUnformatted(ITCStream *f, ITPDisk disk);

int ugCalcFileCrc(char* filepath, uint32_t* crc);
uint32_t ugCalcBufferCrc(uint8_t* buffer, int size);
int ugGetProrgessPercentage(void);

#ifdef __cplusplus
}
#endif

#endif // ITE_UG_H
/** @} */ // end of ug
