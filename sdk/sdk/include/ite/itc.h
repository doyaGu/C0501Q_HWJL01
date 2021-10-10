/*
 * Copyright (c) 2009 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * ITE Common Library.
 *
 * @author Jim Tan
 * @version 1.0
 * @date 2013
 * @copyright ITE Tech. Inc. All Rights Reserved.
 */
/** @defgroup itc ITE Common Library
 *  @{
 */
#ifndef ITE_ITC_H
#define ITE_ITC_H

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif

// Definitions
#define ITC_LOCK_FAIL   (-1)    ///< Locks fail error

/** @defgroup itc_tree Tree
 *  @{
 */

/**
 * Tree node definition.
 */
typedef struct ITCTreeTag
{
    struct ITCTreeTag* parent;  ///< Parent node
    struct ITCTreeTag* sibling; ///< Sibling node
    struct ITCTreeTag* child;   ///< Child node
} ITCTree;

/**
 * Pushes a child node to the front of tree.
 *
 * @param tree The tree.
 * @param node The node wiil be pushed.
 */
void itcTreePushFront(ITCTree* tree, void* node);

/**
 * Pushes a child node to the back of tree.
 *
 * @param tree The tree.
 * @param node The node wiil be pushed.
 */
void itcTreePushBack(ITCTree* tree, void* node);

/**
 * Removes a child node from a tree.
 *
 * @param node The node wiil be removed.
 */
void itcTreeRemove(void* node);

void* itcTreeGetChildAtImpl(ITCTree* tree, int index);
int itcTreeGetChildCountImpl(ITCTree* tree);

/**
 * Moves the last child to the first one of a tree.
 *
 * @param tree The tree.
 */
void itcTreeRotateFront(ITCTree* tree);

/**
 * Moves the first child to the last one.
 *
 * @param tree The tree.
 */
void itcTreeRotateBack(ITCTree* tree);

/**
* Swaps 2 nodes of tree.
*
* @param node1 The first node.
* @param node2 The second node.
*/
void itcTreeSwap(void* node1, void* node2);

// Macros to easily use ITCTree structure
/**
 * Gets a child node from a tree at specified index.
 *
 * @param tree The tree.
 * @param index The index.
 * @return The child node.
 */
#define itcTreeGetChildAt(tree, index)      itcTreeGetChildAtImpl((ITCTree*)(tree), (index))

/**
 * Get children count of a tree.
 *
 * @param tree The tree.
 * @return The children count.
 */
#define itcTreeGetChildCount(tree)          itcTreeGetChildCountImpl((ITCTree*)(tree))

/** @} */ // end of itc_tree

/** @defgroup itc_stream Stream
 *  @{
 */

/**
 * Stream interface definition.
 */
typedef struct ITCStreamTag
{
    bool eof;   ///< Indicates the stream is on END-OF-FILE or not.
    int size;   ///< The stream total size.
    
    /**
     * Stream close method
     *
     * @param stream Pointer referring to the opened stream.
     * @return Returns 0 if the stream was successfully closed. A return non-zero value indicates an error.
     */
    int (*Close)(struct ITCStreamTag* stream);

    /**
     * Stream read method
     *
     * @param stream Pointer referring to the opened stream.
     * @param buf Storage location for data.
     * @param size Maximum number of data unit, in bytes.
     * @return Returns the number of data unit read. A return value of -1 indicates an error.
     */
    int (*Read)(struct ITCStreamTag* stream, void* buf, int size);

    /**
     * Stream write method
     *
     * @param stream Pointer referring to the opened stream.
     * @param buf Data to be written.
     * @param size Number of data unit, in bytes.
     * @return If successful, it returns the number of data unit actually written. A return value of -1 indicates an error.
     */
    int (*Write)(struct ITCStreamTag* stream, const void* buf, int size);

    /**
     * Set position in a stream.
     *
     * @param stream Pointer referring to the opened stream.
     * @param offset Number of data unit from origin, in bytes.
     * @param origin Initial position.
     * @return Returns the offset, in data unit, of the new position from the beginning of the stream. The function returns -1 to indicate an error.
     */
    int (*Seek)(struct ITCStreamTag* stream, long offset, int origin);

    /**
     * Gets the current position of a stream.
     *
     * @param stream Pointer referring to the opened stream.
     * @return Returns the current stream position.
     */
    long (*Tell)(struct ITCStreamTag* stream);
    
    /**
     * Gets the current available size of a stream.
     *
     * @param stream Pointer referring to the opened stream.
     * @return Returns the current available size.
     */
    int (*Available)(struct ITCStreamTag* stream);
    
    /**
     * Locks internal memory buffer of a stream for reading.
     *
     * @param stream Pointer referring to the opened stream.
     * @param bufptr Internal memory buffer address to retrieve.
     * @param size buffer size to lock.
     * @return If successful, it returns the locked size. A return value of ITC_LOCK_FAIL indicates lock fail error.
     */
    int (*ReadLock)(struct ITCStreamTag* stream, void** bufptr, int size);

    /**
     * Unlocks internal memory buffer of a stream for reading.
     *
     * @param stream Pointer referring to the opened stream.
     * @param size the locked buffer size.
     */
    void (*ReadUnlock)(struct ITCStreamTag* stream, int size);

    /**
     * Locks internal memory buffer of a stream for writing.
     *
     * @param stream Pointer referring to the opened stream.
     * @param bufptr Internal memory buffer address to retrieve.
     * @param size buffer size to lock.
     * @return If successful, it returns the locked size. A return value of ITC_LOCK_FAIL indicates lock fail error.
     */
    int (*WriteLock)(struct ITCStreamTag* stream, void** bufptr, int size);

    /**
     * Unlocks internal memory buffer of a stream for writing.
     *
     * @param stream Pointer referring to the opened stream.
     * @param size the locked buffer size.
     */
    void (*WriteUnlock)(struct ITCStreamTag* stream, int size);
} ITCStream;

// Macros to easily use ITCStream structure
/**
 * Closes a opened stream.
 *
 * @param stream Pointer referring to the opened stream.
 * @return Returns 0 if the stream was successfully closed. A return non-zero value indicates an error.
 */
#define itcStreamClose(stream)                          ((ITCStream*)(stream))->Close((ITCStream*)(stream))

/**
 * Sets Close callback function of stream.
 *
 * @param stream Pointer referring to the stream.
 * @param close The callback function to set.
 */
#define itcStreamSetClose(stream, close)                (((ITCStream*)(stream))->Close = (close))

/**
 * Reads a opened stream.
 *
 * @param stream Pointer referring to the opened stream.
 * @param buf Storage location for data.
 * @param size Maximum number of data unit, in bytes.
 * @return Returns the number of data unit read. A return value of -1 indicates an error.
 */
#define itcStreamRead(stream, buf, size)                ((ITCStream*)(stream))->Read((ITCStream*)(stream), (buf), (size))

/**
 * Sets Read callback function of stream.
 *
 * @param stream Pointer referring to the stream.
 * @param read The callback function to set.
 */
#define itcStreamSetRead(stream, read)                  (((ITCStream*)(stream))->Read = (read))

/**
 * Writes a opened stream.
 *
 * @param stream Pointer referring to the opened stream.
 * @param buf Data to be written.
 * @param size Number of data unit, in bytes.
 * @return If successful, it returns the number of data unit actually written. A return value of -1 indicates an error.
 */
#define itcStreamWrite(stream, buf, size)               ((ITCStream*)(stream))->Write((ITCStream*)(stream), (buf), (size))

/**
 * Sets Write callback function of stream.
 *
 * @param stream Pointer referring to the stream.
 * @param write The callback function to set.
 */
#define itcStreamSetWrite(stream, write)                (((ITCStream*)(stream))->Write = (write))

/**
 * Set position in a stream.
 *
 * @param stream Pointer referring to the opened stream.
 * @param offset Number of data unit from origin, in bytes.
 * @param origin Initial position.
 * @return Returns the offset, in data unit, of the new position from the beginning of the stream. The function returns -1 to indicate an error.
 */
#define itcStreamSeek(stream, offset, origin)           ((ITCStream*)(stream))->Seek((ITCStream*)(stream), (offset), (origin))

/**
 * Sets Seek callback function of stream.
 *
 * @param stream Pointer referring to the stream.
 * @param seek The callback function to set.
 */
#define itcStreamSetSeek(stream, seek)                  (((ITCStream*)(stream))->Seek = (seek))

/**
 * Gets the current position of a stream.
 *
 * @param stream Pointer referring to the opened stream.
 * @return Returns the current stream position.
 */
#define itcStreamTell(stream)                           ((ITCStream*)(stream))->Tell((ITCStream*)(stream))

/**
 * Sets Tell callback function of stream.
 *
 * @param stream Pointer referring to the stream.
 * @param tell The callback function to set.
 */
#define itcStreamSetTell(stream, tell)                  (((ITCStream*)(stream))->Tell = (tell))

/**
 * Gets the current available size of a stream.
 *
 * @param stream Pointer referring to the opened stream.
 * @return Returns the current available size.
 */
#define itcStreamAvailable(stream)                      ((ITCStream*)(stream))->Available((ITCStream*)(stream))

/**
 * Sets Available callback function of stream.
 *
 * @param stream Pointer referring to the stream.
 * @param avail The callback function to set.
 */
#define itcStreamSetAvailable(stream, avail)            (((ITCStream*)(stream))->Available = (avail))

/**
 * Locks internal memory buffer of a stream for reading.
 *
 * @param stream Pointer referring to the opened stream.
 * @param bufptr Internal memory buffer address to retrieve.
 * @param size buffer size to lock.
 * @return If successful, it returns the locked size. A return value of ITC_LOCK_FAIL indicates lock fail error.
 */
#define itcStreamReadLock(stream, bufptr, size)         ((ITCStream*)(stream))->ReadLock((ITCStream*)(stream), (void**)(bufptr), (size))

/**
 * Sets ReadLock callback function of stream.
 *
 * @param stream Pointer referring to the stream.
 * @param readlock The callback function to set.
 */
#define itcStreamSetReadLock(stream, readlock)          (((ITCStream*)(stream))->ReadLock = (readlock))

/**
 * Unlocks internal memory buffer of a stream for reading.
 *
 * @param stream Pointer referring to the opened stream.
 * @param size the locked buffer size.
 */
#define itcStreamReadUnlock(stream, size)               ((ITCStream*)(stream))->ReadUnlock((ITCStream*)(stream), (size))

/**
 * Sets ReadUnlock callback function of stream.
 *
 * @param stream Pointer referring to the stream.
 * @param readunlock The callback function to set.
 */
#define itcStreamSetReadUnlock(stream, readunlock)      (((ITCStream*)(stream))->ReadUnlock = (readunlock))

/**
 * Locks internal memory buffer of a stream for writing.
 *
 * @param stream Pointer referring to the opened stream.
 * @param bufptr Internal memory buffer address to retrieve.
 * @param size buffer size to lock.
 * @return If successful, it returns the locked size. A return value of ITC_LOCK_FAIL indicates lock fail error.
 */
#define itcStreamWriteLock(stream, bufptr, size)        ((ITCStream*)(stream))->WriteLock((ITCStream*)(stream), (void**)(bufptr), (size))

/**
 * Sets WriteLock callback function of stream.
 *
 * @param stream Pointer referring to the stream.
 * @param writelock The callback function to set.
 */
#define itcStreamSetWriteLock(stream, writelock)        (((ITCStream*)(stream))->WriteLock = (writelock))

/**
 * Unlocks internal memory buffer of a stream for writing.
 *
 * @param stream Pointer referring to the opened stream.
 * @param size the locked buffer size.
 */
#define itcStreamWriteUnlock(stream, size)              ((ITCStream*)(stream))->WriteUnlock((ITCStream*)(stream), (size))

/**
 * Sets WriteUnlock callback function of stream.
 *
 * @param stream Pointer referring to the stream.
 * @param writeunlock The callback function to set.
 */
#define itcStreamSetWriteUnlock(stream, writeunlock)    (((ITCStream*)(stream))->WriteUnlock = (writeunlock))

/**
 * Opens a stream.
 *
 * @param stream Pointer referring to the stream.
 */
void itcStreamOpen(ITCStream* stream);

// ArrayStream
/**
 * Array stream is a read-only, memory-based stream. Don't support write related functions.
 */
typedef struct
{
    ITCStream stream;   ///< Base stream definition
    const char* array;  ///< Array buffer
    int pos;            ///< Current position
} ITCArrayStream;

/**
 * Opens a array stream.
 *
 * @param astream Pointer referring to the array stream.
 * @param array Array buffer.
 * @param size Array size.
 * @return Returns 0 if the array stream was successfully opened. A return non-zero value indicates an error.
 */
int itcArrayStreamOpen(ITCArrayStream* astream, const char* array, int size);

// BufferStream
/**
 * Buffer stream is a ring-buffer, memory-based stream.
 */
typedef struct
{
    ITCStream stream;       ///< Base stream definition
    unsigned char* buf;     ///< Memory buffer
    int size;               ///< Buffer size
    int readpos;            ///< Read position
    int writepos;           ///< Written position
    pthread_mutex_t mutex;  ///< Mutex for locking
} ITCBufferStream;

/**
 * Opens a buffer stream.
 *
 * @param bstream Pointer referring to the buffer stream.
 * @param size Buffer size.
 * @return Returns 0 if the buffer stream was successfully opened. A return non-zero value indicates an error.
 */
int itcBufferStreamOpen(ITCBufferStream* bstream, int size);

// FileStream
/**
 * File stream is a file-based stream. Don't support lock related functions.
 */
typedef struct
{
    ITCStream stream;   ///< Base stream definition
    FILE* file;         ///< File handle
} ITCFileStream;

/**
 * Opens a file stream.
 *
 * @param fstream Pointer referring to the file stream.
 * @param filename The file path to open.
 * @param write Indicates the stream is for reading or writting.
 * @return Returns 0 if the file stream was successfully opened. A return non-zero value indicates an error.
 */
int itcFileStreamOpen(ITCFileStream* fstream, const char* filename, bool write);

/**
 * Closes a file stream.
 *
 * @param stream Pointer referring to the file stream.
 * @return Returns 0 if the file stream was successfully closed. A return non-zero value indicates an error.
 */
int itcFileStreamClose(ITCStream* stream);

/** @} */ // end of itc_stream

/** @defgroup itc_url URL
*  @{
*/
/**
* Returns a url-encoded version of str.
* IMPORTANT: be sure to free() the returned string after use
*
* @param str url to encode.
* @return a url-encoded version of str.
*/
char *itcUrlEncode(char *str);

/**
* Returns a url-decoded version of str.
* IMPORTANT: be sure to free() the returned string after use
*
* @param str url to encode.
* @return a url-decoded version of str.
*/
char *itcUrlDecode(char *str);

/** @} */ // end of itc_url

/** @defgroup itc_crc CRC
*  @{
*/
/**
* Calculates CRC-16 code.
*
* @param data the data to calculate.
* @param data the data size.
* @return the crc-16 code.
*/
uint16_t itcCrc16(const uint8_t* data, uint16_t size);

/** @} */ // end of itc_crc

#ifdef __cplusplus
}
#endif

#endif // ITE_ITC_H
/** @} */ // end of itc
