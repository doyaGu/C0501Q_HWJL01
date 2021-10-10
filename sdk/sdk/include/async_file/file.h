/*
 * Copyright (c) 2004 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * The file functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#ifndef PAL_FILE_H
#define PAL_FILE_H

#include "async_file/def.h"

#ifdef __cplusplus
extern "C" {
#endif

/** File definition */
typedef void PAL_FILE;                           /**< File handle */
typedef void *PAL_FILE_FIND;                     /**< File find handle */

#define PAL_WEOF              (PAL_WINT)(0xFFFF) /**< End-of-file (Unicode) */
#define PAL_EOF               (-1)               /**< End-of-file */

#ifdef PAL_UNICODE
    #define PAL_TEOF          PAL_WEOF
#else
    #define PAL_TEOF          PAL_EOF
#endif // PAL_UNICODE

#define PAL_SEEK_SET          0 /**< Beginning of file */
#define PAL_SEEK_CUR          1 /**< Current position of file pointer */
#define PAL_SEEK_END          2 /**< End of file */

#define PAL_FILE_RB           0 /**< Opens for reading (binary) */
#define PAL_FILE_WB           1 /**< Opens an empty file for writing (binary) */
#define PAL_FILE_AB           2 /**< Opens for writing and appending (binary) */
#define PAL_FILE_RBP          3 /**< Opens for both reading and writing (binary) */
#define PAL_FILE_WBP          4 /**< Opens an empty file for both reading and writing (binary) */
#define PAL_FILE_ABP          5 /**< Opens for reading and appending (binary) */

#define PAL_FILE_DIR_DEFAULT  "/"
#define PAL_WFILE_DIR_DEFAULT L"/"
#define PAL_TFILE_DIR_DEFAULT PAL_T("/")    /**< Default directory */

//Device Type  -- depend on driver
typedef enum PAL_DEVICE_TYPE_TAG
{
    PAL_DEVICE_TYPE_NOR = 0,
    PAL_DEVICE_TYPE_NORPRIVATE,   // 1
    PAL_DEVICE_TYPE_NAND,         // 2
    PAL_DEVICE_TYPE_SD,           // 3
    PAL_DEVICE_TYPE_SD2,          // 4
    PAL_DEVICE_TYPE_xD,           // 5
    PAL_DEVICE_TYPE_MS,           // 6
    PAL_DEVICE_TYPE_MMC,          // 7
    PAL_DEVICE_TYPE_CF,           // 8
    PAL_DEVICE_TYPE_USB0,         // 9
    PAL_DEVICE_TYPE_USB1,         // 10
    PAL_DEVICE_TYPE_USB2,
    PAL_DEVICE_TYPE_USB3,
    PAL_DEVICE_TYPE_USB4,
    PAL_DEVICE_TYPE_USB5,
    PAL_DEVICE_TYPE_USB6,
    PAL_DEVICE_TYPE_USB7,
    PAL_DEVICE_TYPE_PTP,
    PAL_DEVICE_TYPE_COUNT
} PAL_DEVICE_TYPE;

#define PAL_FILE_OPEN_CALLBACK_DECL(func, param, result) \
    void func(PAL_FILE * param, MMP_ULONG result)

#define PAL_FILE_CLOSE_CALLBACK_DECL(func, param, result) \
    void func(PAL_FILE * param, MMP_ULONG result)

#define PAL_FILE_READ_CALLBACK_DECL(func, param, result) \
    void func(PAL_FILE * param, MMP_ULONG result)

#define PAL_FILE_WRITE_CALLBACK_DECL(func, param, result) \
    void func(PAL_FILE * param, MMP_ULONG result)

#define PAL_FILE_SEEK_CALLBACK_DECL(func, param, result) \
    void func(PAL_FILE * param, MMP_ULONG result)

#define PAL_FILE_TELL_CALLBACK_DECL(func, param, result) \
    void func(PAL_FILE * param, MMP_ULONG result)

#define PAL_FILE_EOF_CALLBACK_DECL(func, param, result) \
    void func(PAL_FILE * param, MMP_ULONG result)

#define PAL_FILE_DELETE_CALLBACK_DECL(func, param, result) \
    void func(PAL_FILE * param, MMP_ULONG result)

#define PAL_DISK_GET_FREE_SPACE_CALLBACK_DECL(func, param, result) \
    void func(PAL_FILE * param, MMP_ULONG result)

#define PAL_FILE_FIND_FIRST_CALLBACK_DECL(func, param, result) \
    void func(PAL_FILE * param, MMP_ULONG result)

#define PAL_FILE_FIND_NEXT_CALLBACK_DECL(func, param, result) \
    void func(PAL_FILE * param, MMP_ULONG result)

#define PAL_FILE_FIND_CLOSE_CALLBACK_DECL(func, param, result) \
    void func(PAL_FILE * param, MMP_ULONG result)

typedef void
(*PAL_FILE_OPEN_CALLBACK)(
    PAL_FILE  *file,
    MMP_ULONG result,
    void      *arg);

typedef void
(*PAL_FILE_CLOSE_CALLBACK)(
    PAL_FILE  *file,
    MMP_ULONG result,
    void      *arg);

typedef void
(*PAL_FILE_READ_CALLBACK)(
    PAL_FILE  *file,
    MMP_ULONG result,
    void      *arg);

typedef void
(*PAL_FILE_WRITE_CALLBACK)(
    PAL_FILE  *file,
    MMP_ULONG result,
    void      *arg);

typedef void
(*PAL_FILE_SEEK_CALLBACK)(
    PAL_FILE  *file,
    MMP_ULONG result,
    void      *arg);

typedef void
(*PAL_FILE_TELL_CALLBACK)(
    PAL_FILE  *file,
    MMP_ULONG result,
    void      *arg);

typedef void
(*PAL_FILE_EOF_CALLBACK)(
    PAL_FILE  *file,
    MMP_ULONG result,
    void      *arg);

typedef void
(*PAL_FILE_DELETE_CALLBACK)(
    PAL_FILE  *file,
    MMP_ULONG result,
    void      *arg);

typedef void
(*PAL_DISK_GET_FREE_SPACE_CALLBACK)(
    PAL_FILE  *file,
    MMP_ULONG result,
    void      *arg);

typedef void
(*PAL_FILE_FIND_FIRST_CALLBACK)(
    PAL_FILE  *file,
    MMP_ULONG result,
    void      *arg);

typedef void
(*PAL_FILE_FIND_NEXT_CALLBACK)(
    PAL_FILE  *file,
    MMP_ULONG result,
    void      *arg);

typedef void
(*PAL_FILE_FIND_CLOSE_CALLBACK)(
    PAL_FILE  *file,
    MMP_ULONG result,
    void      *arg);

/* Deprecated */
typedef void
(*PAL_FILE_CALLBACK)(
    PAL_FILE  *file,
    MMP_ULONG result,
    void      *arg);

/**
 * Open a file.
 *
 * @param filename Filename.
 * @param mode Type of access permitted.
 * @param callback Callback function in async mode. MMP_NULL for async mode.
 * @return
 *  sync mode: A pointer to the open file. A null pointer value indicates an error.
 *  async mode: 0 indicate success, fail otherwise.
 * @remark The following modes are allowed to open:
 *  PAL_FILE_RB Opens for reading. If the file does not exist or cannot be found, the fopen call fails.
 *  PAL_FILE_WB Opens an empty file for writing. If the given file exists, its contents are destroyed.
 *  PAL_FILE_AB Opens for writing at the end of the file (appending) without removing the EOF marker before writing new data to the file; creates the file first if it doesn't exist.
 *  PAL_FILE_RBP Opens for both reading and writing. (The file must exist.)
 *  PAL_FILE_WBP Opens an empty file for both reading and writing. If the given file exists, its contents are destroyed.
 *  PAL_FILE_ABP Opens for reading and appending; the appending operation includes the removal of the EOF marker before new data is written to the file and the EOF marker is restored after writing is complete; creates the file first if it doesn't exist.
 */
PAL_FILE *
PalFileOpen(
    const MMP_CHAR         *filename,
    MMP_UINT               mode,
    PAL_FILE_OPEN_CALLBACK callback,
    void                   *callback_arg);

MMP_INT    //Benson
PalFileOpenWriteClose(
    const MMP_CHAR    *filename,
    const void        *buffer,
    MMP_SIZE_T        size,
    MMP_SIZE_T        count,
    PAL_FILE_CALLBACK callback,
    void              *callback_arg);

MMP_INT
PalFileClose(
    PAL_FILE                *stream,
    PAL_FILE_CLOSE_CALLBACK callback,
    void                    *callback_arg);

MMP_SIZE_T
PalFileRead(
    void                   *buffer,
    MMP_SIZE_T             size,
    MMP_SIZE_T             count,
    PAL_FILE               *stream,
    PAL_FILE_READ_CALLBACK callback,
    void                   *callback_arg);

MMP_SIZE_T
PalFileWrite(
    const void              *ptr,
    MMP_SIZE_T              size,
    MMP_SIZE_T              nmemb,
    PAL_FILE                *stream,
    PAL_FILE_WRITE_CALLBACK callback,
    void                    *callback_arg);

MMP_INT
PalFileFlush(
    PAL_FILE          *stream,
    PAL_FILE_CALLBACK callback,
    void              *callback_arg);

MMP_INT
PalFileSeek(
    PAL_FILE               *stream,
    MMP_LONG               offset,
    MMP_INT                origin,
    PAL_FILE_SEEK_CALLBACK callback,
    void                   *callback_arg);

MMP_INT
PalFileSeekEx(
    PAL_FILE               *stream,
    MMP_INT64              offset,
    MMP_INT                origin,
    PAL_FILE_SEEK_CALLBACK callback,
    void                   *callback_arg);

MMP_LONG
PalFileTell(
    PAL_FILE               *stream,
    PAL_FILE_TELL_CALLBACK callback,
    void                   *callback_arg);

MMP_INT64
PalFileTellEx(
    PAL_FILE               *stream,
    PAL_FILE_TELL_CALLBACK callback,
    void                   *callback_arg);

MMP_INT
PalFileEof(
    PAL_FILE              *stream,
    PAL_FILE_EOF_CALLBACK callback,
    void                  *callback_arg);

MMP_INT
PalFileDelete(
    const MMP_CHAR           *filename,
    PAL_FILE_DELETE_CALLBACK callback,
    void                     *callback_arg);

#if 0
MMP_INT
PalWFileDelete(
    const MMP_WCHAR          *filename,
    PAL_FILE_DELETE_CALLBACK callback);

MMP_ULONG
PalDiskGetFreeSpace(
    const MMP_INT                    dirnum,
    MMP_UINT32                       *free_h,
    MMP_UINT32                       *free_l,
    PAL_DISK_GET_FREE_SPACE_CALLBACK callback);

MMP_ULONG
PalDiskGetTotalSpace(
    const MMP_INT                    dirnum,
    MMP_UINT32                       *total_h,
    MMP_UINT32                       *total_l,
    PAL_DISK_GET_FREE_SPACE_CALLBACK callback);

PAL_FILE_FIND
PalFileFindFirst(
    const MMP_CHAR               *filename,
    PAL_FILE_FIND_FIRST_CALLBACK callback);

PAL_FILE_FIND
PalWFileFindFirst(
    const MMP_WCHAR              *filename,
    PAL_FILE_FIND_FIRST_CALLBACK callback);

MMP_INT
PalFileFindNext(
    PAL_FILE_FIND               find,
    PAL_FILE_FIND_NEXT_CALLBACK callback);

MMP_INT
PalWFileFindNext(
    PAL_FILE_FIND               find,
    PAL_FILE_FIND_NEXT_CALLBACK callback);

MMP_INT
PalFileFindClose(
    PAL_FILE_FIND                find,
    PAL_FILE_FIND_CLOSE_CALLBACK callback);

const MMP_CHAR *
PalFileFindGetName(
    PAL_FILE_FIND find);

const MMP_WCHAR *
PalWFileFindGetName(
    PAL_FILE_FIND find);

const MMP_ULONG
PalFileFindGetSize(
    PAL_FILE_FIND find);

const MMP_UINT64
PalFileFindGetSizeEx(
    PAL_FILE_FIND find);

const MMP_UINT16
PalFileFindGetTime(
    PAL_FILE_FIND find);

const MMP_UINT16
PalFileFindGetDate(
    PAL_FILE_FIND find);

MMP_INT
PalFindAttrIsDirectory(
    PAL_FILE_FIND find);

void *
PalWFileFindGetPos(
    PAL_FILE_FIND find);

PAL_FILE_FIND
PalWFileFindByPos(
    int       drivenum,
    MMP_WCHAR *path,
    void      *pos);

MMP_INT
PalInitVolume(
    MMP_UINT drvNumber,
    void     *drvInit,
    MMP_UINT cardType,
    MMP_UINT partitionNum);

MMP_INT
PalDelVolume(
    MMP_UINT drvnumber);

MMP_INT
PalDelDriver(
    MMP_UINT cardtype);

MMP_INT
PalGetPartitionCount(
    MMP_UINT cardType,
    void     *drvInit);

MMP_INT
PalGetTimeDate(
    const MMP_CHAR *filename,
    MMP_UINT16     *sec,
    MMP_UINT16     *minute,
    MMP_UINT16     *hour,
    MMP_UINT16     *day,
    MMP_UINT16     *month,
    MMP_UINT16     *year);

MMP_INT
PalWGetTimeDate(
    const MMP_WCHAR *filename,
    MMP_UINT16      *sec,
    MMP_UINT16      *minute,
    MMP_UINT16      *hour,
    MMP_UINT16      *day,
    MMP_UINT16      *month,
    MMP_UINT16      *year);

MMP_INT
PalWSetTimeDate(
    const MMP_WCHAR *filename,
    MMP_UINT16      sec,
    MMP_UINT16      minute,
    MMP_UINT16      hour,
    MMP_UINT16      day,
    MMP_UINT16      month,
    MMP_UINT16      year);
#endif

MMP_INT32
PalGetFileLength(
    const MMP_CHAR *filename);

MMP_INT64
PalGetFileLengthEx(
    const MMP_CHAR *filename);

MMP_INT32
PalWGetFileLength(
    const MMP_WCHAR *filename);

MMP_INT32
PalMakeDir(
    const MMP_CHAR *filename);

MMP_INT32
PalWMakeDir(
    const MMP_WCHAR *filename);

MMP_INT
PalRemoveDir(
    const MMP_CHAR           *dirname,
    PAL_FILE_DELETE_CALLBACK callback);

MMP_INT
PalWRemoveDir(
    const MMP_WCHAR          *dirname,
    PAL_FILE_DELETE_CALLBACK callback);

MMP_INT
PalFormat(
    MMP_UINT drvnumber,
    MMP_UINT cardType,
    MMP_INT  partitionIdx,
    void     *drvInit);

MMP_INT
PalGetAttr(
    const MMP_CHAR *filename,
    MMP_UINT8      *attr);

MMP_INT
PalWGetAttr(
    const MMP_WCHAR *filename,
    MMP_UINT8       *attr);

MMP_INT
PalSetAttr(
    const MMP_CHAR *filename,
    MMP_UINT8      attr);

MMP_INT
PalWSetAttr(
    const MMP_WCHAR *filename,
    MMP_UINT8       attr);

#ifdef PAL_UNICODE
    #define PalTFileOpen            PalWFileOpen
    #define PalTFileDelete          PalWFileDelete
    #define PalTRemoveDir           PalWRemoveDir
    #define PalTDiskGetFreeSpace    PalDiskGetFreeSpace
    #define PalTDiskGetTotalSpace   PalDiskGetTotalSpace
    #define PalTFileFindFirst       PalWFileFindFirst
    #define PalTFileFindNext        PalWFileFindNext
    #define PalTFileFindGetName     PalWFileFindGetName
    #define PalTGetFileLength       PalWGetFileLength
    #define PalTMakeDir             PalWMakeDir
    #define PalTGetAttr             PalWGetAttr
    #define PalTSetAttr             PalWSetAttr
    #define PalTFileClose           PalFileClose
    #define PalTFileRead            PalFileRead
    #define PalTFileWrite           PalFileWrite
    #define PalTFileSeek            PalFileSeek
    #define PalTFileTell            PalFileTell
    #define PalTFileEof             PalFileEof
    #define PalTFileFindClose       PalFileFindClose
    #define PalTFileFindGetSize     PalFileFindGetSize
    #define PalTFileFindGetTime     PalFileFindGetTime
    #define PalTFileFindGetDate     PalFileFindGetDate
    #define PalTGetTimeDate         PalWGetTimeDate
    #define PalTSetTimeDate         PalWSetTimeDate
    #define PalTGetFindAttr         PalGetFindAttr
    #define PalTFindAttrIsDirectory PalFindAttrIsDirectory

#else
    #define PalTFileOpen            PalFileOpen
    #define PalTFileDelete          PalFileDelete
    #define PalTRemoveDir           PalRemoveDir
    #define PalTDiskGetFreeSpace    PalDiskGetFreeSpace
    #define PalTDiskGetTotalSpace   PalDiskGetTotalSpace
    #define PalTFileFindFirst       PalFileFindFirst
    #define PalTFileFindNext        PalFileFindNext
    #define PalTFileFindGetName     PalFileFindGetName
    #define PalTGetFileLength       PalGetFileLength
    #define PalTMakeDir             PalMakeDir
    #define PalTGetAttr             PalGetAttr
    #define PalTSetAttr             PalSetAttr
    #define PalTFileClose           PalFileClose
    #define PalTFileRead            PalFileRead
    #define PalTFileWrite           PalFileWrite
    #define PalTFileSeek            PalFileSeek
    #define PalTFileTell            PalFileTell
    #define PalTFileEof             PalFileEof
    #define PalTFileFindClose       PalFileFindClose
    #define PalTFileFindGetSize     PalFileFindGetSize
    #define PalTFileFindGetTime     PalFileFindGetTime
    #define PalTFileFindGetDate     PalFileFindGetDate
    #define PalTGetTimeDate         PalGetTimeDate
    #define PalTGetFindAttr         PalGetFindAttr
    #define PalTFindAttrIsDirectory PalFindAttrIsDirectory

#endif /* PAL_UNICODE */

#ifdef __cplusplus
}
#endif

#endif /* PAL_FILE_H */