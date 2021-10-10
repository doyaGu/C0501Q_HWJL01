#include "brCmd.h"

Byte BrCmd_sequence = 0;


Dword BrCmd_addChecksum (
    IN  Bridge*			bridge,
    OUT Dword*          bufferLength,
    OUT Byte*           buffer
) {
    Dword error  = BR_ERR_NO_ERROR;
    Dword loop   = (*bufferLength - 1) / 2;
    Dword remain = (*bufferLength - 1) % 2;
    Dword i;
    Word  checksum = 0;

	if(bridge == (void *)0)
		return (BR_ERR_NULL_HANDLE_PTR);

    for (i = 0; i < loop; i++)
        checksum = checksum + (Word) (buffer[2 * i + 1] << 8) + (Word) (buffer[2 * i + 2]);
    if (remain)
        checksum = checksum + (Word) (buffer[*bufferLength - 1] << 8);
    
    checksum = ~checksum;
    buffer[*bufferLength]     = (Byte) ((checksum & 0xFF00) >> 8);
    buffer[*bufferLength + 1] = (Byte) (checksum & 0x00FF);
    buffer[0]                 = (Byte) (*bufferLength + 1);
    *bufferLength            += 2;

    return (error);
}


Dword BrCmd_removeChecksum (
    IN  Bridge*			bridge,
    OUT Dword*          bufferLength,
    OUT Byte*           buffer
) {
    Dword error    = BR_ERR_NO_ERROR;
    Dword loop     = (*bufferLength - 3) / 2;
    Dword remain   = (*bufferLength - 3) % 2;
    Dword i;
    Word  checksum = 0;
	if(bridge == (void *)0)
		return (BR_ERR_NULL_HANDLE_PTR);

    for (i = 0; i < loop; i++)
        checksum = checksum + (Word) (buffer[2 * i + 1] << 8) + (Word) (buffer[2 * i + 2]);
    if (remain)
        checksum = checksum + (Word) (buffer[*bufferLength - 3] << 8);    
    
    checksum = ~checksum;
    if (((Word)(buffer[*bufferLength - 2] << 8) + (Word)(buffer[*bufferLength - 1])) != checksum) {
        error = BR_ERR_WRONG_CHECKSUM;
        goto exit;
    }
    if (buffer[2])
        error = BR_ERR_FIRMWARE_STATUS | buffer[2];
    
    buffer[0]      = (Byte) (*bufferLength - 3);
    *bufferLength -= 2;

exit :
    return (error);
}


Dword BrCmd_busTx (
    IN  Bridge*			bridge,
    IN  Byte			chip,
    IN  Dword           bufferLength,
    OUT Byte*           buffer
) {
    Dword     	error = BR_ERR_NO_ERROR;
    Dword	  	i;
    Endeavour* 	endeavour;
	Byte		i2cAddr;
    
    endeavour = (Endeavour*) bridge;
	i2cAddr = endeavour->gator[chip].i2cAddr;

    for (i = 0; i < IT9300User_RETRY_MAX_LIMIT; i++) {
        error = BrUser_busTx(bridge, i2cAddr, bufferLength, buffer);
        if (error == 0) goto exit;

        BrUser_delay (bridge, 1);
    }

exit:
    return (error);
}


Dword BrCmd_busRx (
    IN  Bridge*			bridge,
    IN  Byte			chip,
    IN  Dword           bufferLength,
    OUT Byte*           buffer
) {
    Dword     	error = BR_ERR_NO_ERROR;
    Dword	  	i;
    Endeavour* 	endeavour;
	Byte		i2cAddr;
    
    endeavour = (Endeavour*) bridge;
	i2cAddr = endeavour->gator[chip].i2cAddr;

    for (i = 0; i < IT9300User_RETRY_MAX_LIMIT; i++) {
        error = BrUser_busRx (bridge, i2cAddr, bufferLength, buffer);
        if (error == 0) goto exit;

        BrUser_delay (bridge, 1);
    }

exit:
    return (error);
}


Dword BrCmd_busRxData (
    IN  Bridge*			bridge,
    IN  Dword           bufferLength,
    OUT Byte*           buffer
) {
    Dword     error = BR_ERR_NO_ERROR;
    Dword	  i;
    Endeavour* endeavour;
    
    endeavour = (Endeavour*) bridge;

    for (i = 0; i < IT9300User_RETRY_MAX_LIMIT; i++) {    
        error = BrUser_busRxData(bridge, bufferLength, buffer);
        if (error == 0) goto exit;

        BrUser_delay (bridge, 1);
    }

exit:
    return (error);
}


Dword BrCmd_writeRegisters (
    IN  Bridge*			bridge,
    IN  Byte            chip,
    IN  Dword           registerAddress,
    IN  Byte            registerAddressLength,
    IN  Dword           writeBufferLength,
    IN  Byte*           writeBuffer
) {
    Dword       error = BR_ERR_NO_ERROR;
    Word        command;
    Byte        buffer[255];
    Dword       bufferLength;
    Dword       remainLength;
    Dword       sendLength;
    Dword       i;
    Endeavour*  endeavour;

    BrUser_enterCriticalSection (bridge);

    if (writeBufferLength == 0) goto exit;
    if (registerAddressLength > 4) {
        error  = BR_ERR_PROTOCOL_FORMAT_INVALID;
        goto exit;
    }

    endeavour = (Endeavour*) bridge;
    if ((writeBufferLength + 12) > endeavour->maxBusTxSize) {
        error = BR_ERR_INVALID_DATA_LENGTH;
        goto exit;
    }


    /** add frame header */
    command   = BrCmd_buildCommand (Command_REG_DEMOD_WRITE, chip);
    buffer[1] = (Byte) (command >> 8);
    buffer[2] = (Byte) command;
    buffer[3] = (Byte) BrCmd_sequence++;
    buffer[4] = (Byte) writeBufferLength;
    buffer[5] = (Byte) registerAddressLength;
    buffer[6] = (Byte) ((registerAddress) >> 24); /** Get first byte of reg. address  */
    buffer[7] = (Byte) ((registerAddress) >> 16); /** Get second byte of reg. address */
    buffer[8] = (Byte) ((registerAddress) >> 8);  /** Get third byte of reg. address  */
    buffer[9] = (Byte) (registerAddress );        /** Get fourth byte of reg. address */

    /** add frame data */
    for (i = 0; i < writeBufferLength; i++) {    
        buffer[10 + i] = writeBuffer[i];
    }

    /** add frame check-sum */
    bufferLength = 10 + writeBufferLength;
    error = BrCmd_addChecksum (bridge, &bufferLength, buffer);
    if (error) goto exit;    

    /** send frame */
    i = 0;
    sendLength = 0;
    remainLength = bufferLength;
    while (remainLength > 0) {
        i     = (remainLength > IT9300User_MAX_PKT_SIZE) ? (IT9300User_MAX_PKT_SIZE) : (remainLength);
        error = BrCmd_busTx (bridge, chip, i, &buffer[sendLength]);
        if (error) goto exit;

        sendLength   += i;
        remainLength -= i;
    }

    /** get reply frame */
    bufferLength = 5;
    error = BrCmd_busRx (bridge, chip, bufferLength, buffer);
    if (error) goto exit;

    /** remove check-sum from reply frame */
    error = BrCmd_removeChecksum (bridge, &bufferLength, buffer);
    if (error) goto exit;

exit :
    BrUser_leaveCriticalSection (bridge);
    return (error);
}


Dword BrCmd_writeEepromValues (
    IN  Bridge*			bridge,
    IN  Byte            chip,
    IN  Byte            eepromAddress,
    IN  Word            registerAddress,
    IN  Byte            registerAddressLength,
    IN  Byte            writeBufferLength,
    IN  Byte*           writeBuffer
) {
    Dword       error = BR_ERR_NO_ERROR;
    Word        command;
    Byte        buffer[255];
    Dword       bufferLength;
    Dword       remainLength;
    Dword       sendLength;
    Dword       i;
    Endeavour*   endeavour;

    BrUser_enterCriticalSection (bridge);

    if (writeBufferLength == 0) goto exit;

    endeavour     = (Endeavour*) bridge;

    if ((Dword)(writeBufferLength + 11) > endeavour->maxBusTxSize) {
        error = BR_ERR_INVALID_DATA_LENGTH;
        goto exit;
    }

    /** add frame header */
    command   = BrCmd_buildCommand (Command_REG_EEPROM_WRITE, chip);
    buffer[1] = (Byte) (command >> 8);
    buffer[2] = (Byte) command;
    buffer[3] = (Byte) BrCmd_sequence++;
    buffer[4] = (Byte) writeBufferLength;
    buffer[5] = (Byte) eepromAddress;
    buffer[6] = (Byte) registerAddressLength;
    buffer[7] = (Byte) (registerAddress >> 8);  /** Get high byte of reg. address */
    buffer[8] = (Byte) registerAddress;         /** Get low byte of reg. address  */

    /** add frame data */
    for (i = 0; i < writeBufferLength; i++) {
        buffer[9 + i] = writeBuffer[i];		
    }

    /** add frame check-sum */
    bufferLength = 9 + writeBufferLength;
    error = BrCmd_addChecksum (bridge, &bufferLength, buffer);
    if (error) goto exit;

    /** send frame */
    i = 0;
    sendLength = 0;
    remainLength = bufferLength;
    while (remainLength > 0) {
        i     = (remainLength > IT9300User_MAX_PKT_SIZE) ? (IT9300User_MAX_PKT_SIZE) : (remainLength);
        error = BrCmd_busTx (bridge, chip, i, &buffer[sendLength]);
        if (error) goto exit;

        sendLength   += i;
        remainLength -= i;
    }

    /** get reply frame */
    bufferLength = 5;
    error = BrCmd_busRx (bridge, chip, bufferLength, buffer);
    if (error) goto exit;

    /** remove check-sum from reply frame */
    error = BrCmd_removeChecksum (bridge, &bufferLength, buffer);
    if (error) goto exit;

exit :
    BrUser_leaveCriticalSection (bridge);
    return (error);
}


Dword BrCmd_readRegisters (
    IN  Bridge*			bridge,
    IN  Byte            chip,
    IN  Dword           registerAddress,
    IN  Byte            registerAddressLength,
    IN  Dword           readBufferLength,
    OUT Byte*           readBuffer
) {
    Dword       error = BR_ERR_NO_ERROR;
    Word        command;
    Byte        buffer[255];
    Dword       bufferLength;
    Dword       sendLength;
    Dword       remainLength;
    Dword       i, k;
    Endeavour*   endeavour;

    BrUser_enterCriticalSection (bridge);

    if (readBufferLength == 0) goto exit;
    if (registerAddressLength > 4) {
        error  = BR_ERR_PROTOCOL_FORMAT_INVALID;
        goto exit;
    }

    endeavour     = (Endeavour*) bridge;

    if ((readBufferLength + 5) > IT9300User_MAX_PKT_SIZE) {
        error = BR_ERR_INVALID_DATA_LENGTH;
        goto exit;
    }

    if ((readBufferLength + 5) > endeavour->maxBusTxSize) {
        error = BR_ERR_INVALID_DATA_LENGTH;
        goto exit;
    }



    /** add frame header */
    command   = BrCmd_buildCommand (Command_REG_DEMOD_READ, chip);
    buffer[1] = (Byte) (command >> 8);
    buffer[2] = (Byte) command;
    buffer[3] = (Byte) BrCmd_sequence++;
    buffer[4] = (Byte) readBufferLength;
    buffer[5] = (Byte) registerAddressLength;
    buffer[6] = (Byte) (registerAddress >> 24); /** Get first byte of reg. address  */
    buffer[7] = (Byte) (registerAddress >> 16); /** Get second byte of reg. address */
    buffer[8] = (Byte) (registerAddress >> 8);  /** Get third byte of reg. address  */
    buffer[9] = (Byte) (registerAddress);       /** Get fourth byte of reg. address */

    /** add frame check-sum */
    bufferLength = 10;
    error = BrCmd_addChecksum (bridge, &bufferLength, buffer);
    if (error) goto exit;


    /** send frame */
    i = 0;
    sendLength   = 0;
    remainLength = bufferLength;
    while (remainLength > 0) {
        i     = (remainLength > IT9300User_MAX_PKT_SIZE) ? (IT9300User_MAX_PKT_SIZE) : (remainLength);        
        error = BrCmd_busTx (bridge, chip, i, &buffer[sendLength]);
        if (error) goto exit;

        sendLength   += i;
        remainLength -= i;
    }

    /** get reply frame */
    bufferLength = 5 + readBufferLength;
    error = BrCmd_busRx (bridge, chip, bufferLength, buffer);
    if (error) goto exit;

    /** remove check-sum from reply frame */
    error = BrCmd_removeChecksum (bridge, &bufferLength, buffer);
    if (error) goto exit;

    for (k = 0; k < readBufferLength; k++) {
        readBuffer[k] = buffer[k + 3];
    }

exit :
    BrUser_leaveCriticalSection (bridge);
    return (error);
}


Dword BrCmd_readEepromValues (
    IN  Bridge*			bridge,
    IN  Byte            chip,
    IN  Byte            eepromAddress,
    IN  Word            registerAddress,
    IN  Byte            registerAddressLength,
    IN  Byte            readBufferLength,
    OUT Byte*           readBuffer
) {
    Dword       error = BR_ERR_NO_ERROR;
    Word        command;
    Byte        buffer[255];
    Dword       bufferLength;
    Dword       remainLength;
    Dword       sendLength;
    Dword       i, k;
    Endeavour*   endeavour;

    BrUser_enterCriticalSection (bridge);

    if (readBufferLength == 0) goto exit;

    endeavour     = (Endeavour*) bridge;

    if ((Dword)(readBufferLength + 5) > IT9300User_MAX_PKT_SIZE) {
        error  = BR_ERR_INVALID_DATA_LENGTH;
        goto exit;
    }
        
	if ((Dword)(readBufferLength + 5) > endeavour->maxBusTxSize) {
        error  = BR_ERR_INVALID_DATA_LENGTH;
        goto exit;
    }

    /** add command header */
    command   = BrCmd_buildCommand (Command_REG_EEPROM_READ, chip);
    buffer[1] = (Byte) (command >> 8);
    buffer[2] = (Byte) command;
    buffer[3] = (Byte) BrCmd_sequence++;
    buffer[4] = (Byte) readBufferLength;
    buffer[5] = (Byte) eepromAddress;
    buffer[6] = (Byte) registerAddressLength;
    buffer[7] = (Byte) (registerAddress >> 8);  /** Get high byte of reg. address */
    buffer[8] = (Byte) registerAddress;         /** Get low byte of reg. address  */

    /** add frame check-sum */
    bufferLength = 9;
    error = BrCmd_addChecksum (bridge, &bufferLength, buffer);
    if (error) goto exit;

    /** send frame */
    i = 0;
    sendLength   = 0;
    remainLength = bufferLength;
    while (remainLength > 0) {
        i = (remainLength > IT9300User_MAX_PKT_SIZE) ? (IT9300User_MAX_PKT_SIZE) : (remainLength);        
        error = BrCmd_busTx (bridge, chip, i, &buffer[sendLength]);
        if (error) goto exit;

        sendLength   += i;
        remainLength -= i;
    }

    /** get reply frame */
    bufferLength = 5 + readBufferLength;
    error = BrCmd_busRx (bridge, chip, bufferLength, buffer);
    if (error) goto exit;

    /** remove frame check-sum */
    error = BrCmd_removeChecksum (bridge, &bufferLength, buffer);
    if (error) goto exit;

    for (k = 0; k < readBufferLength; k++) {
        readBuffer[k] = buffer[k + 3];
    }

exit :
    BrUser_leaveCriticalSection (bridge);
    return (error);
}


Dword BrCmd_loadFirmware (
    IN  Bridge*			bridge,
    IN  Byte 			chip,
    IN  Dword           length,
    IN  Byte*           firmware
) {
    Dword       error = BR_ERR_NO_ERROR;
    Word        command;
    Dword       loop;
    Dword       remain;
    Dword       i, j, k;
    Byte        buffer[255];
    Dword       payloadLength;
    Dword       bufferLength;
    Dword       remainLength;
    Dword       sendLength;
    Endeavour*  endeavour;
    Dword       maxFrameSize;

    BrUser_enterCriticalSection (bridge);

    endeavour     = (Endeavour*) bridge;
    maxFrameSize = endeavour->maxBusTxSize;

    payloadLength = (maxFrameSize - 6);
    loop   = length / payloadLength;
    remain = length % payloadLength;

    k = 0;
    command = BrCmd_buildCommand (Command_FW_DOWNLOAD, chip);
    for (i = 0; i < loop; i++) {
        buffer[1] = (Byte) (command >> 8);
        buffer[2] = (Byte) command;
        buffer[3] = (Byte) BrCmd_sequence++;

        for (j = 0; j < payloadLength; j++)
            buffer[4 + j] = firmware[j + i*payloadLength];

        bufferLength = 4 + payloadLength;
        error = BrCmd_addChecksum (bridge, &bufferLength, buffer);
        if (error) goto exit;

        sendLength = 0;
        remainLength = maxFrameSize;
        while (remainLength > 0) {
            k     = (remainLength > IT9300User_MAX_PKT_SIZE) ? (IT9300User_MAX_PKT_SIZE) : (remainLength);        
            error = BrCmd_busTx (bridge, chip, k, &buffer[sendLength]);
            if (error) goto exit;

            sendLength   += k;
            remainLength -= k;
        }
    }

    if (remain) {
        buffer[1] = (Byte) (command >> 8);
        buffer[2] = (Byte) command;
        buffer[3] = (Byte) BrCmd_sequence++;

        for (j = 0; j < remain; j++)
            buffer[4 + j] = firmware[j + i*payloadLength];

        bufferLength = 4 + remain;
        error = BrCmd_addChecksum (bridge, &bufferLength, buffer);
        if (error) goto exit;

        sendLength   = 0;
        remainLength = bufferLength;
        while (remainLength > 0)
        {
            k = (remainLength > IT9300User_MAX_PKT_SIZE) ? (IT9300User_MAX_PKT_SIZE) : (remainLength);        
            error = BrCmd_busTx (bridge, chip, k, &buffer[sendLength]);
            if (error) goto exit;

            sendLength   += k;
            remainLength -= k;
        }
    }

exit :
    BrUser_leaveCriticalSection (bridge);
    return (error);
}


Dword BrCmd_reboot (
    IN  Bridge*			bridge,
    IN  Byte            chip
) {
    Dword       error = BR_ERR_NO_ERROR;
    Word        command;
    Byte        buffer[255];
    Dword       bufferLength;
    Endeavour*  endeavour;

    BrUser_enterCriticalSection (bridge);

    endeavour     = (Endeavour*) bridge;

    command   = BrCmd_buildCommand (Command_REBOOT, chip);
    buffer[1] = (Byte) (command >> 8);
    buffer[2] = (Byte) command;
    buffer[3] = (Byte) BrCmd_sequence++;
    bufferLength = 4;
    error = BrCmd_addChecksum (bridge, &bufferLength, buffer);
    if (error) goto exit;

    error = BrCmd_busTx (bridge, chip, bufferLength, buffer);
    if (error) goto exit;

exit :
    BrUser_leaveCriticalSection (bridge);
    return (error);
}


Dword BrCmd_sendCommand (
    IN  Bridge*			bridge,
    IN  Word            command,
    IN  Byte            chip,
    IN  Dword           writeBufferLength,
    IN  Byte*           writeBuffer,
    IN  Dword           readBufferLength,
    OUT Byte*           readBuffer
) {
    Dword       error = BR_ERR_NO_ERROR;
    Byte        buffer[255];
    Dword       bufferLength;
    Dword       remainLength;
    Dword       sendLength;
    Dword       i, k;
    Endeavour*  endeavour;
    Dword       maxFrameSize;

    BrUser_enterCriticalSection (bridge);

    endeavour    = (Endeavour*) bridge;
    maxFrameSize = endeavour->maxBusTxSize;

    if ((writeBufferLength + 6) > maxFrameSize) {
        error = BR_ERR_INVALID_DATA_LENGTH;
        goto exit;
    }

    if ((readBufferLength + 5) > IT9300User_MAX_PKT_SIZE) {
        error  = BR_ERR_INVALID_DATA_LENGTH;
        goto exit;
    }

    if ((readBufferLength + 5) > maxFrameSize) {
        error = BR_ERR_INVALID_DATA_LENGTH;
        goto exit;
    }


    if (writeBufferLength == 0) {
        command   = BrCmd_buildCommand (command, chip);
        buffer[1] = (Byte) (command >> 8);
        buffer[2] = (Byte) command;
        buffer[3] = (Byte) BrCmd_sequence++;
        bufferLength = 4;
        error = BrCmd_addChecksum (bridge, &bufferLength, buffer);
        if (error) goto exit;

        // send command packet
        i = 0;
        sendLength = 0;
        remainLength = bufferLength;
        while (remainLength > 0) {
            i = (remainLength > IT9300User_MAX_PKT_SIZE) ? (IT9300User_MAX_PKT_SIZE) : (remainLength);        
            error = BrCmd_busTx (bridge, chip, i, &buffer[sendLength]);
            if (error) goto exit;

            sendLength   += i;
            remainLength -= i;
        }
    } else {
        command   = BrCmd_buildCommand (command, chip);
        buffer[1] = (Byte) (command >> 8);
        buffer[2] = (Byte) command;
        buffer[3] = (Byte) BrCmd_sequence++;
        for (k = 0; k < writeBufferLength; k++)
            buffer[k + 4] = writeBuffer[k];
        
        
        bufferLength = 4 + writeBufferLength;
        error = BrCmd_addChecksum (bridge, &bufferLength, buffer);
        if (error) goto exit;

        
        /** send command */
        i = 0;
        sendLength = 0;
        remainLength = bufferLength;
        while (remainLength > 0) {
            i     = (remainLength > IT9300User_MAX_PKT_SIZE) ? (IT9300User_MAX_PKT_SIZE) : (remainLength);        
            error = BrCmd_busTx (bridge, chip, i, &buffer[sendLength]);
            if (error) goto exit;

            sendLength   += i;
            remainLength -= i;
        }
    }

    bufferLength = 5 + readBufferLength;

    error = BrCmd_busRx (bridge, chip, bufferLength, buffer);
    if (error) goto exit;

    error = BrCmd_removeChecksum (bridge, &bufferLength, buffer);
    if (error) goto exit;

    if (readBufferLength) {
        for (k = 0; k < readBufferLength; k++) {
            readBuffer[k] = buffer[k + 3];
        }
    }

exit :
    BrUser_leaveCriticalSection (bridge);
    return (error);
}


Dword BrCmd_receiveData (
    IN  Bridge*			bridge,	
	IN	Byte			chip,
    IN  Dword           registerAddress,
    IN  Dword           readBufferLength,
    OUT Byte*           readBuffer
) {
    Dword       error = BR_ERR_NO_ERROR;
    Word        command;
    Byte        buffer[255];
    Dword       bufferLength;
    Endeavour*  endeavour;

    if (readBufferLength == 0) goto exit;

    endeavour     = (Endeavour*) bridge;

    BrUser_enterCriticalSection (bridge);

    command   = BrCmd_buildCommand (Command_DATA_READ, chip);
    buffer[1] = (Byte) (command >> 8);
    buffer[2] = (Byte) command;
    buffer[3] = (Byte) BrCmd_sequence++;
    buffer[4] = (Byte) ((readBufferLength >> 16)  & 0xFF);
    buffer[5] = (Byte) ((readBufferLength >> 8)  & 0xFF);
    buffer[6] = (Byte) (readBufferLength  & 0xFF);
    buffer[7] = (Byte) ((registerAddress >> 16)  & 0xFF);
    buffer[8] = (Byte) ((registerAddress >> 8)  & 0xFF);
    buffer[9] = (Byte) (registerAddress  & 0xFF);

    bufferLength = 10;
    error = BrCmd_addChecksum (bridge, &bufferLength, buffer);
    if (error) goto exit;

    error = BrCmd_busTx (bridge, chip, bufferLength, buffer);
    if (error) goto exit;

    error = BrCmd_busRxData (bridge, readBufferLength, readBuffer);

exit :
    BrUser_leaveCriticalSection (bridge);
    return (error);
}
