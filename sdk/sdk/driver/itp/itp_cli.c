/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL command line interface functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "itp_cfg.h"
#include "openrtos/FreeRTOS.h"
#include "openrtos/task.h"
#include "openrtos/semphr.h"
#include "openrtos/FreeRTOS_CLI.h"
#include <sys/stat.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* Dimensions the buffer into which input characters are placed. */
#define cmdMAX_INPUT_SIZE				200

/* Callbacks to handle the command line commands defined by the xTaskStats and
xRunTimeStats command definitions respectively.  These functions are not
necessarily reentrant!  They must be used from one task only - or at least by
only one task at a time. */
static portBASE_TYPE prvTaskStatsCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString );
static portBASE_TYPE prvRunTimeStatsCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString );

/*
 * Implements the file system "dir" command, accessible through a command
 * console.  See the definition of the xDirCommand command line input structure
 * below.  This function is not necessarily reentrant.  It must not be used by
 * more than one task at a time.
 */
static portBASE_TYPE prvDirCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString );

/*
 * Implements the file system "del" command, accessible through a command
 * console.  See the definition of the xDelCommand command line input structure
 * below.  This function is not necessarily reentrant.  It must not be used by
 * more than one task at a time.
 */
static portBASE_TYPE prvDelCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString );

/*
 * Implements the file system "copy" command, accessible through a command
 * console.  See the definition of the xCopyCommand command line input
 * structure below.  This function is not necessarily reentrant.  It must not
 * be used by more than one task at a time.
 */
static portBASE_TYPE prvCopyCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString );

/*-----------------------------------------------------------*/

/* Structure that defines the "run-time-stats" command line command. */
static const xCommandLineInput xRunTimeStats =
{
	( const int8_t * const ) "run-time-stats",
	( const int8_t * const ) "run-time-stats: Displays a table showing how much processing time each FreeRTOS task has used\r\n",
	prvRunTimeStatsCommand,
	0
};

/* Structure that defines the "task-stats" command line command. */
static const xCommandLineInput xTaskStats =
{
	( const int8_t * const ) "task-stats",
	( const int8_t * const ) "task-stats: Displays a table showing the state of each FreeRTOS task\r\n",
	prvTaskStatsCommand,
	0
};

static portBASE_TYPE prvTaskStatsCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString )
{
const int8_t *const pcHeader = ( int8_t * ) "Task          State  Priority  Stack	#\r\n************************************************\r\n";

	( void ) pcCommandString;
	configASSERT( pcWriteBuffer );

	/* This function assumes the buffer length is adequate. */
	( void ) xWriteBufferLen;

	/* Generate a table of task stats. */
	strcpy( ( char * ) pcWriteBuffer, ( char * ) pcHeader );
	vTaskList( pcWriteBuffer + strlen( ( char * ) pcHeader ) );

	/* There is no more data to return after this single string, so return
	pdFALSE. */
	return pdFALSE;
}
/*-----------------------------------------------------------*/

static portBASE_TYPE prvRunTimeStatsCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString )
{
const int8_t * const pcHeader = ( int8_t * ) "Task            Abs Time      % Time\r\n****************************************\r\n";

	( void ) pcCommandString;
	configASSERT( pcWriteBuffer );

	/* This function assumes the buffer length is adequate. */
	( void ) xWriteBufferLen;

	/* Generate a table of task stats. */
	strcpy( ( char * ) pcWriteBuffer, ( char * ) pcHeader );
	vTaskGetRunTimeStats( pcWriteBuffer + strlen( ( char * ) pcHeader ) );

	/* There is no more data to return after this single string, so return
	pdFALSE. */
	return pdFALSE;
}
/*-----------------------------------------------------------*/

static void* CliTask(void* arg)
{
int8_t cRxedChar, cInputIndex = 0, *pcOutputString;
static int8_t cInputString[ cmdMAX_INPUT_SIZE ];
portBASE_TYPE xReturned;

	/* Obtain the address of the output buffer.  Note there is no mutual
	exclusion on this buffer as it is assumed only one command console
	interface will be used at any one time. */
	pcOutputString = FreeRTOS_CLIGetOutputBuffer();

	for( ;; )
	{
		/* Only interested in reading one character at a time. */
		cRxedChar = getchar();

		if ( cRxedChar == EOF )
		{
		    usleep(33000);
		}
		else if( cRxedChar == '\n' )
		{

			/* Start to transmit a line separator, just to make the output
			easier to read. */
			puts("");

			/* Pass the received command to the command interpreter.  The
			command interpreter is called repeatedly until it returns
			pdFALSE as it might generate more than one string. */
			do
			{

				/* Get the string to write to the UART from the command
				interpreter. */
				xReturned = FreeRTOS_CLIProcessCommand( cInputString, pcOutputString, configCOMMAND_INT_MAX_OUTPUT_SIZE );

				/* Write the generated string to the UART. */
				puts(pcOutputString);

			} while( xReturned != pdFALSE );

			/* All the strings generated by the input command have been sent.
			Clear the input	string ready to receive the next command. */
			cInputIndex = 0;
			memset( cInputString, 0x00, cmdMAX_INPUT_SIZE );

			/* Start to transmit a line separator, just to make the output
			easier to read. */
			printf("\r\n>");
		}
		else
		{
			if( cRxedChar == '\r' )
			{
				/* Ignore the character. */
			}
			else if( cRxedChar == '\b' )
			{
				/* Backspace was pressed.  Erase the last character in the
				string - if any. */
				if( cInputIndex > 0 )
				{
					cInputIndex--;
					cInputString[ cInputIndex ] = '\0';
				}
			}
			else
			{
				/* A character was entered.  Add it to the string
				entered so far.  When a \n is entered the complete
				string will be passed to the command interpreter. */
				if( cInputIndex < cmdMAX_INPUT_SIZE )
				{
					cInputString[ cInputIndex ] = cRxedChar;
					cInputIndex++;
				}
			}
		}
	}
    return NULL;
}

#if defined(CFG_FS_FAT) || defined(CFG_FS_NTFS)

/* These objects are large, so not stored on the stack.  They are used by
the functions that implement command line commands - and as only one command
line command can be executing at any one time - there is no need to protect
access to the variables using a mutex or other mutual exclusion method. */
static FILE *pxCommandLineFile1, *pxCommandLineFile2;

/* A buffer used to both create content to write to disk, and read content back
from a disk.  Note there is no mutual exclusion on this buffer, so it must not
be accessed outside of the task. */
static uint8_t cRAMBuffer[ 2000 ];

/* The RAM buffer is used by two tasks, so protect it using a mutex. */
static SemaphoreHandle_t xRamBufferMutex = NULL;

/* Structure that defines the "dir" command line command. */
static const xCommandLineInput xDirCommand =
{
	( const int8_t * const ) "dir",
	( const int8_t * const ) "dir: Displays the name and size of each file in the root directory\r\n",
	prvDirCommand,
	0
};

/* Structure that defines the "del" command line command. */
static const xCommandLineInput xDelCommand =
{
	( const int8_t * const ) "del",
	( const int8_t * const ) "del <filename>: Deletes <filename> from the disk\r\n",
	prvDelCommand,
	1
};

/* Structure that defines the "del" command line command. */
static const xCommandLineInput xCopyCommand =
{
	( const int8_t * const ) "copy",
	( const int8_t * const ) "copy <filename1> <filename2>: Copies <filename1> to <filename2>, creating or overwriting <filename2>\r\n",
	prvCopyCommand,
	2
};

/*-----------------------------------------------------------*/

static portBASE_TYPE prvDirCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString )
{
static portBASE_TYPE xDirectoryOpen = pdFALSE;
static struct dirent* pxFileInfo;
static DIR* xDirectory;
portBASE_TYPE xReturn;

	/* This assumes pcWriteBuffer is long enough. */
	( void ) pcCommandString;

	if( xDirectoryOpen == pdFALSE )
	{
		/* This is the first time the function has been called this run of the
		dir command.  Ensure the directory is open. */

		if( ( xDirectory = opendir( getcwd( cRAMBuffer, sizeof( cRAMBuffer ) ) ) ) != NULL )
		{
			xDirectoryOpen = pdTRUE;
		}
		else
		{
			xDirectoryOpen = pdFALSE;
			xReturn = pdFALSE;
		}
	}

	if( xDirectoryOpen == pdTRUE )
	{
		/* Read the next file. */
		if( ( pxFileInfo = readdir( xDirectory ) ) != NULL )
		{
			if( pxFileInfo->d_name[ 0 ] != '\0' )
			{
			    struct stat sbuf;
			    getcwd( cRAMBuffer, sizeof( cRAMBuffer ) );
			    strcat( cRAMBuffer, pxFileInfo->d_name );
			    stat( cRAMBuffer, &sbuf );

				/* There is at least one more file name to return. */
				snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "%s\t\t%d\r\n", pxFileInfo->d_name, sbuf.st_size );
				xReturn = pdTRUE;

			}
		}
		else
		{
			/* There are no more file names to return.   Reset the read
			pointer ready for the next time this directory is read. */
			xReturn = pdFALSE;
			xDirectoryOpen = pdFALSE;
			pcWriteBuffer[ 0 ] = 0x00;
		}
	}

	return xReturn;
}
/*-----------------------------------------------------------*/

static portBASE_TYPE prvDelCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString )
{
int8_t *pcParameter;
portBASE_TYPE xParameterStringLength;
const unsigned portBASE_TYPE uxFirstParameter = 1U;

	/* This assumes pcWriteBuffer is long enough. */
	( void ) xWriteBufferLen;

	/* Obtain the name of the file being deleted. */
	pcParameter = ( int8_t * ) FreeRTOS_CLIGetParameter( pcCommandString, uxFirstParameter, &xParameterStringLength );

	/* Terminate the parameter string. */
	pcParameter[ xParameterStringLength ] = 0x00;

	if( unlink( ( const char * ) pcParameter ) != 0 )
	{
		snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "Could not delete %s\r\n\r\n", pcParameter );
	}

	/* There is only ever one string to return. */
	return pdFALSE;
}
/*-----------------------------------------------------------*/

static portBASE_TYPE prvCopyCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString )
{
int8_t *pcParameter1, *pcParameter2;
portBASE_TYPE xParameter1StringLength, xParameter2StringLength, xFinished = pdFALSE;
const unsigned portBASE_TYPE uxFirstParameter = 1U, uxSecondParameter = 2U;
unsigned int xBytesRead, xBytesWritten;
const TickType_t xMaxDelay = 500UL / portTICK_PERIOD_MS;

	( void ) xWriteBufferLen;

	/* Obtain the name of the source file, and the length of its name. */
	pcParameter1 = ( int8_t * ) FreeRTOS_CLIGetParameter( pcCommandString, uxFirstParameter, &xParameter1StringLength );

	/* Obtain the name of the destination file, and the length of its name. */
	pcParameter2 = ( int8_t * ) FreeRTOS_CLIGetParameter( pcCommandString, uxSecondParameter, &xParameter2StringLength );

	/* Terminate both file names. */
	pcParameter1[ xParameter1StringLength ] = 0x00;
	pcParameter2[ xParameter2StringLength ] = 0x00;

	/* Open the source file. */
	if( ( pxCommandLineFile1 = fopen( ( const char * ) pcParameter1, "rb" ) ) != NULL )
	{
		/* Open the destination file. */
		if( ( pxCommandLineFile2 = fopen( ( const char * ) pcParameter2, "wb" ) ) != NULL )
		{
			while( xFinished == pdFALSE )
			{
				/* About to use the RAM buffer, ensure this task has exclusive access
				to it while it is in use. */
				if( xSemaphoreTake( xRamBufferMutex, xMaxDelay ) == pdPASS )
				{
					if( ( xBytesRead = fread( cRAMBuffer, 1, sizeof( cRAMBuffer ),  pxCommandLineFile1 ) ) != EOF )
					{
						if( xBytesRead != 0U )
						{
							if( ( xBytesWritten = fwrite( cRAMBuffer, 1, xBytesRead, pxCommandLineFile2 ) ) == xBytesRead )
							{
								if( xBytesWritten < xBytesRead )
								{
									snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "Error writing to %s, disk full?\r\n\r\n", pcParameter2 );
									xFinished = pdTRUE;
								}
							}
							else
							{
								snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "Error during copy\r\n\r\n" );
								xFinished = pdTRUE;
							}
						}
						else
						{
							/* EOF. */
							xFinished = pdTRUE;
						}
					}
					else
					{
						snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "Error during copy\r\n\r\n" );
						xFinished = pdTRUE;
					}
					
					/* Must give the mutex back! */
					xSemaphoreGive( xRamBufferMutex );
				}				
			}

			/* Close both files. */
			fclose( pxCommandLineFile1 );
			fclose( pxCommandLineFile2 );
		}
		else
		{
			snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "Could not open or create %s\r\n\r\n", pcParameter2 );

			/* Close the source file. */
			fclose( pxCommandLineFile1 );
		}
	}
	else
	{
		snprintf( ( char * ) pcWriteBuffer, xWriteBufferLen, "Could not open %s\r\n\r\n", pcParameter1 );
	}

	return pdFALSE;
}
/*-----------------------------------------------------------*/

#endif // defined(CFG_FS_FAT) || defined(CFG_FS_NTFS)

void itpCliInit(void)
{
    pthread_t task;
        
	/* Register two command line commands to show task stats and run time stats
	respectively. */
	FreeRTOS_CLIRegisterCommand( &xTaskStats );
	FreeRTOS_CLIRegisterCommand( &xRunTimeStats );

#if defined(CFG_FS_FAT) || defined(CFG_FS_NTFS)
	/* Register the file commands with the command interpreter. */
	FreeRTOS_CLIRegisterCommand( &xDirCommand );
	FreeRTOS_CLIRegisterCommand( &xDelCommand );
	FreeRTOS_CLIRegisterCommand( &xCopyCommand );

	/* Create the mutex that protects the shared RAM buffer. */
	xRamBufferMutex = xSemaphoreCreateMutex();
	configASSERT( xRamBufferMutex );

#endif // defined(CFG_FS_FAT) || defined(CFG_FS_NTFS)

    // create command line interface task
    pthread_create(&task, NULL, CliTask, NULL);
}
