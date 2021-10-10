/*
 * Copyright (c) 2008 ITE Technology Corp. All Rights Reserved.
 */
/** @file protocol.h
 *
 * @author Irene Lin
 */
#ifndef PROTOCOL_H
#define PROTOCOL_H

/* Sub Classes */

#define US_SC_RBC    0x01       /* Typically, flash devices */
#define US_SC_8020   0x02       /* CD-ROM */
#define US_SC_QIC    0x03       /* QIC-157 Tapes */
#define US_SC_UFI    0x04       /* Floppy */
#define US_SC_8070   0x05       /* Removable media */
#define US_SC_SCSI   0x06       /* Transparent */
#define US_SC_ISD200 0x07       /* ISD200 ATA */
#define US_SC_MIN    US_SC_RBC
#define US_SC_MAX    US_SC_ISD200

#endif