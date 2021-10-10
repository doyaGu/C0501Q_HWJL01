#ifndef __8195_DESC_H__
#define __8195_DESC_H__

#define PLATFORM_LITTLE_ENDIAN                  0
#define PLATFORM_BIG_ENDIAN                     1

#define SYSTEM_ENDIAN                           PLATFORM_LITTLE_ENDIAN

// define transmit packat type
#define TX_PACKET_802_3		(0x83)
#define TX_PACKET_802_11		(0x81)
#define TX_H2C_CMD				(0x11)
#define TX_MEM_READ			(0x51)
#define TX_MEM_WRITE			(0x53)
#define TX_MEM_SET				(0x55)
#define TX_FM_FREETOGO		(0x61)
#define TX_PACKET_USER			(0x41)

//define receive packet type
#define RX_PACKET_802_3		(0x82)
#define RX_PACKET_802_11		(0x80)
#define RX_C2H_CMD				(0x10)
#define RX_MEM_READ			(0x50)
#define RX_MEM_WRITE			(0x52)
#define RX_MEM_SET				(0x54)
#define RX_FM_FREETOGO		(0x60)
#define RX_PACKET_USER			(0x40)


typedef struct _TX_DESC_8195A{
	// u4Byte 0
	u32	txpktsize:16;
	u32	offset:8;    		// store the sizeof(TX_DESC)
	u32	bus_agg_num:8;		// the bus aggregation number
	
	// u4Byte 1
	/***********************************************/
	/**************transmit packet type****************/
	/*	0x83:	802.3 packet						      */
	/*	0x81:	802.11 packet						      */
	/*	0x11:	H2C command					      */
	/*	0x51:	Memory Read						      */
	/*	0x53:	Memory Write						      */
	/*	0x55:	Memory Set						      */
	/*	0x61:	Jump to firmware start				      */
	u32	type:8;//packet type
	u32	rsvd0:24;
	
	// u4Byte 2
	u32	rsvd1;
	
	// u4Byte 3
	u32	rsvd2;
	
	// u4Byte 4
	u32	rsvd3;

	// u4Byte 5
	u32	rsvd4;
} TXDESC_8195A, *PTXDESC_8195A;

#define	SIZE_TX_DESC_8195a	(sizeof(TXDESC_8195A))


typedef struct _RX_DESC_8195A{
	// u4Byte 0
	u32	pkt_len:16;
	u32	offset:8;    	
	u32	rsvd0:6;        //
	u32	icv:1; //icv error
	u32	crc:1; // crc error

	// u4Byte 1
	/************************************************/
	/*****************receive packet type*********************/
	/*	0x82:	802.3 packet						      */
	/*	0x80:	802.11 packet						      */
	/*	0x10:	C2H command					      */
	/*	0x50:	Memory Read						      */
	/*	0x52:	Memory Write						      */
	/*	0x54:	Memory Set						      */
	/*	0x60:	Indicate the firmware is started		      */
	u32 type:8;
	u32	rsvd1:24;
	
	// u4Byte 2
	u32	rsvd2;
	
	// u4Byte 3
	u32	rsvd3;
	
	// u4Byte 4
	u32	rsvd4;

	// u4Byte 5
	u32	rsvd5;
} RXDESC_8195A, *PRXDESC_8195A;
#define	SIZE_RX_DESC_8195a	(sizeof(RXDESC_8195A))
//typedef struct _RX_DESC RXDESC_8195A, *PRXDESC_8195A;

// TX Desc for Memory Write command
typedef struct _TX_DESC_MW{
	// u4Byte 0
#if (SYSTEM_ENDIAN==PLATFORM_LITTLE_ENDIAN)
	u32	txpktsize:16;       // bit[15:0]
	u32	offset:8;    		// bit[23:16], store the sizeof(TX_DESC)
	u32	bus_agg_num:8;		// bit[31:24], the bus aggregation number
#else
    u32 bus_agg_num:8;      // bit[31:24], the bus aggregation number
    u32 offset:8;           // bit[23:16], store the sizeof(TX_DESC)
    u32 txpktsize:16;       // bit[15:0]
#endif

	// u4Byte 1
#if (SYSTEM_ENDIAN==PLATFORM_LITTLE_ENDIAN)
    u32 type:8;             // bit[7:0], the packet type
    u32 reply:1;            // bit[8], request to send a reply message
    u32 rsvd0:23;
#else
    u32 rsvd0:23;
    u32 reply:1;            // bit[8], request to send a reply message
    u32 type:8;             // bit[7:0], the packet type
#endif

	// u4Byte 2
	u32	start_addr;         // memory write start address
	
	// u4Byte 3
#if (SYSTEM_ENDIAN==PLATFORM_LITTLE_ENDIAN)
    u32 write_len:16;       // bit[15:0], the length to write
    u32 rsvd2:16;           // bit[31:16]
#else
    u32 rsvd2:16;           // bit[31:16]
    u32 write_len:16;       // bit[15:0], the length to write
#endif
	
	// u4Byte 4
	u32	rsvd3;

	// u4Byte 5
	u32	rsvd4;
} TX_DESC_MW, *PTX_DESC_MW;

// TX Desc for Memory Read command
typedef struct _TX_DESC_MR{
	// u4Byte 0
#if (SYSTEM_ENDIAN==PLATFORM_LITTLE_ENDIAN)
	u32	txpktsize:16;       // bit[15:0]
	u32	offset:8;    		// bit[23:16], store the sizeof(TX_DESC)
	u32	bus_agg_num:8;		// bit[31:24], the bus aggregation number
#else
    u32 bus_agg_num:8;      // bit[31:24], the bus aggregation number
    u32 offset:8;           // bit[23:16], store the sizeof(TX_DESC)
    u32 txpktsize:16;       // bit[15:0]
#endif

	// u4Byte 1
#if (SYSTEM_ENDIAN==PLATFORM_LITTLE_ENDIAN)
    u32 type:8;             // bit[7:0], the packet type
    u32 rsvd0:24;
#else
    u32 rsvd0:24;
    u32 type:8;             // bit[7:0], the packet type
#endif

	// u4Byte 2
	u32	start_addr;         // memory write start address
	
	// u4Byte 3
#if (SYSTEM_ENDIAN==PLATFORM_LITTLE_ENDIAN)
    u32 read_len:16;        // bit[15:0], the length to read
    u32 rsvd2:16;           // bit[31:16]
#else
    u32 rsvd2:16;           // bit[31:16]
    u32 read_len:16;        // bit[15:0], the length to read
#endif
	
	// u4Byte 4
	u32	rsvd3;

	// u4Byte 5
	u32	rsvd4;
} TX_DESC_MR, *PTX_DESC_MR;

// TX Desc for Memory Set command
typedef struct _TX_DESC_MS{
	// u4Byte 0
#if (SYSTEM_ENDIAN==PLATFORM_LITTLE_ENDIAN)
	u32	txpktsize:16;       // bit[15:0]
	u32	offset:8;    		// bit[23:16], store the sizeof(TX_DESC)
	u32	bus_agg_num:8;		// bit[31:24], the bus aggregation number
#else
    u32 bus_agg_num:8;      // bit[31:24], the bus aggregation number
    u32 offset:8;           // bit[23:16], store the sizeof(TX_DESC)
    u32 txpktsize:16;       // bit[15:0]
#endif

	// u4Byte 1
#if (SYSTEM_ENDIAN==PLATFORM_LITTLE_ENDIAN)
    u32 type:8;             // bit[7:0], the packet type
    u32 data:8;             // bit[8:15], the value to be written to the memory
    u32 reply:1;            // bit[16], request to send a reply message
    u32 rsvd0:15;
#else
    u32 rsvd0:15;
    u32 reply:1;            // bit[16], request to send a reply message
    u32 data:8;             // bit[8:15], the value to be written to the memory
    u32 type:8;             // bit[7:0], the packet type
#endif

	// u4Byte 2
	u32	start_addr;         // memory write start address
	
	// u4Byte 3
#if (SYSTEM_ENDIAN==PLATFORM_LITTLE_ENDIAN)
    u32 write_len:16;       // bit[15:0], the length to write
    u32 rsvd2:16;           // bit[31:16]
#else
    u32 rsvd2:16;           // bit[31:16]
    u32 write_len:16;       // bit[15:0], the length to write
#endif
	
	// u4Byte 4
	u32	rsvd3;

	// u4Byte 5
	u32	rsvd4;
} TX_DESC_MS, *PTX_DESC_MS;

// TX Desc for Jump to Start command
typedef struct _TX_DESC_JS{
	// u4Byte 0
#if (SYSTEM_ENDIAN==PLATFORM_LITTLE_ENDIAN)
	u32	txpktsize:16;       // bit[15:0]
	u32	offset:8;    		// bit[23:16], store the sizeof(TX_DESC)
	u32	bus_agg_num:8;		// bit[31:24], the bus aggregation number
#else
    u32 bus_agg_num:8;      // bit[31:24], the bus aggregation number
    u32 offset:8;           // bit[23:16], store the sizeof(TX_DESC)
    u32 txpktsize:16;       // bit[15:0]
#endif

	// u4Byte 1
#if (SYSTEM_ENDIAN==PLATFORM_LITTLE_ENDIAN)
    u32 type:8;             // bit[7:0], the packet type
    u32 rsvd0:24;
#else
    u32 rsvd0:24;
    u32 type:8;             // bit[7:0], the packet type
#endif

	// u4Byte 2
	u32	start_fun;         // the pointer of the startup function 
	
	// u4Byte 3
	u32	rsvd2;
	
	// u4Byte 4
	u32	rsvd3;

	// u4Byte 5
	u32	rsvd4;
} TX_DESC_JS, *PTX_DESC_JS;

// For memory read command
typedef struct _RX_DESC_MR{
	// u4Byte 0
#if (SYSTEM_ENDIAN==PLATFORM_LITTLE_ENDIAN)
	u32	pkt_len:16;     // bit[15:0], the packet size
	u32	offset:8;    	// bit[23:16], the offset from the packet start to the buf start, also means the size of RX Desc
	u32	rsvd0:8;        // bit[31:24]
#else
	u32	rsvd0:8;        // bit[31:24]
	u32	offset:8;    	// bit[23:16], the offset from the packet start to the buf start, also means the size of RX Desc
	u32	pkt_len:16;     // bit[15:0], the packet size
#endif

	// u4Byte 1
#if (SYSTEM_ENDIAN==PLATFORM_LITTLE_ENDIAN)
	u32	type:8;         // bit[7:0], the type of this packet
	u32	rsvd1:24;       // bit[31:8]
#else
    u32 rsvd1:24;       // bit[31:8]
    u32 type:8;         // bit[7:0], the type of this packet
#endif

	// u4Byte 2
	u32	start_addr;
	
	// u4Byte 3
	u32	rsvd2;
	
	// u4Byte 4
	u32	rsvd3;

	// u4Byte 5
	u32	rsvd4;
} RX_DESC_MR, *PRX_DESC_MR;

// For memory write reply command
typedef struct _RX_DESC_MW{
	// u4Byte 0
#if (SYSTEM_ENDIAN==PLATFORM_LITTLE_ENDIAN)
	u32	pkt_len:16;     // bit[15:0], the packet size
	u32	offset:8;    	// bit[23:16], the offset from the packet start to the buf start, also means the size of RX Desc
	u32	rsvd0:8;        // bit[31:24]
#else
	u32	rsvd0:8;        // bit[31:24]
	u32	offset:8;    	// bit[23:16], the offset from the packet start to the buf start, also means the size of RX Desc
	u32	pkt_len:16;     // bit[15:0], the packet size
#endif

	// u4Byte 1
#if (SYSTEM_ENDIAN==PLATFORM_LITTLE_ENDIAN)
	u32	type:8;         // bit[7:0], the type of this packet
	u32	rsvd1:24;       // bit[31:8]
#else
    u32 rsvd1:24;       // bit[31:8]
    u32 type:8;         // bit[7:0], the type of this packet
#endif

	// u4Byte 2
	u32	start_addr;
	
	// u4Byte 3
#if (SYSTEM_ENDIAN==PLATFORM_LITTLE_ENDIAN)
    u32 write_len:16;   // bit[15:0], the type of this packet
    u32 result:8;      // bit[23:16], the result of memory write command
    u32 rsvd2:8;       // bit[31:24]
#else
    u32 rsvd2:8;       // bit[31:24]
    u32 result:8;      // bit[23:16], the result of memory write command
    u32 write_len:16;   // bit[15:0], the type of this packet
#endif
	
	// u4Byte 4
	u32	rsvd3;

	// u4Byte 5
	u32	rsvd4;
} RX_DESC_MW, *PRX_DESC_MW;

// For memory set reply command
typedef struct _RX_DESC_MS{
	// u4Byte 0
#if (SYSTEM_ENDIAN==PLATFORM_LITTLE_ENDIAN)
	u32	pkt_len:16;     // bit[15:0], the packet size
	u32	offset:8;    	// bit[23:16], the offset from the packet start to the buf start, also means the size of RX Desc
	u32	rsvd0:8;        // bit[31:24]
#else
	u32	rsvd0:8;        // bit[31:24]
	u32	offset:8;    	// bit[23:16], the offset from the packet start to the buf start, also means the size of RX Desc
	u32	pkt_len:16;     // bit[15:0], the packet size
#endif

	// u4Byte 1
#if (SYSTEM_ENDIAN==PLATFORM_LITTLE_ENDIAN)
	u32	type:8;         // bit[7:0], the type of this packet
	u32	rsvd1:24;       // bit[31:8]
#else
    u32 rsvd1:24;       // bit[31:8]
    u32 type:8;         // bit[7:0], the type of this packet
#endif

	// u4Byte 2
	u32	start_addr;
	
	// u4Byte 3
#if (SYSTEM_ENDIAN==PLATFORM_LITTLE_ENDIAN)
    u32 write_len:16;   // bit[15:0], the type of this packet
    u32 result:8;      // bit[23:16], the result of memory write command
    u32 rsvd2:8;       // bit[31:24]
#else
    u32 rsvd2:8;       // bit[31:24]
    u32 result:8;      // bit[23:16], the result of memory write command
    u32 write_len:16;   // bit[15:0], the type of this packet
#endif
	
	// u4Byte 4
	u32	rsvd3;

	// u4Byte 5
	u32	rsvd4;
} RX_DESC_MS, *PRX_DESC_MS;

// For firmware ready reply command
typedef struct _RX_DESC_FS{
	// u4Byte 0
#if (SYSTEM_ENDIAN==PLATFORM_LITTLE_ENDIAN)
	u32	pkt_len:16;     // bit[15:0], the packet size
	u32	offset:8;    	// bit[23:16], the offset from the packet start to the buf start, also means the size of RX Desc
	u32	rsvd0:8;        // bit[31:24]
#else
	u32	rsvd0:8;        // bit[31:24]
	u32	offset:8;    	// bit[23:16], the offset from the packet start to the buf start, also means the size of RX Desc
	u32	pkt_len:16;     // bit[15:0], the packet size
#endif

	// u4Byte 1
#if (SYSTEM_ENDIAN==PLATFORM_LITTLE_ENDIAN)
	u32	type:8;         // bit[7:0], the type of this packet
	u32	rsvd1:24;       // bit[31:8]
#else
    u32 rsvd1:24;       // bit[31:8]
    u32 type:8;         // bit[7:0], the type of this packet
#endif

	// u4Byte 2
	u32	rsvd2;
	
	// u4Byte 3
	u32	rsvd3;
	
	// u4Byte 4
	u32	rsvd4;

	// u4Byte 5
	u32	rsvd5;
} RX_DESC_FS, *PRX_DESC_FS;

#endif
