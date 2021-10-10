/*
 * Copyright (c) 2001,2002 Florian Schulze.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the authors nor the names of the contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * pktdrv.c - This file is part of lwIP pktif
 *
 ****************************************************************************
 *
 * This file is derived from an example in lwIP with the following license:
 *
 * Copyright (c) 2001, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
 #if 0

#include "pktdrv.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/** @todo use the lwip header file */
#define ETHARP_HWADDR_LEN      6

#define MAX_NUM_ADAPTERS       10
#define ADAPTER_NAME_LEN       4096
#define PACKET_ADAPTER_BUFSIZE 512000
#define PACKET_INPUT_BUFSIZE   256000

#define PACKET_OID_DATA_SIZE   255

/* Packet Adapter informations */
struct packet_adapter {
  input_fn         input;
  void            *input_fn_arg;

  /* buffer to hold the data coming from the driver */
  char             buffer[PACKET_INPUT_BUFSIZE];
};

/** Get a list of adapters
 *
 * @param adapter_list void* array: list where the adapters are stored
 * @param list_len size of adapter_list (number of void*)
 * @param buffer here the actual data is stored, adapter_list points into this buffer
 * @param buf_len size of buffer in bytes
 * @return number of adapters found or negative on error
 */
static int
get_adapter_list(char** adapter_list, int list_len, void* buffer, size_t buf_len)
{
  int i;
  char *temp, *start;

  memset(adapter_list, 0, list_len*sizeof(void*));
  memset(buffer, 0, buf_len);

  /* obtain the name of the adapters installed on this machine
     (a list of strings separated by '\0') */

  /* get the start of each adapter name in the list and put it into
   * the AdapterList array */
  i = 0;
  temp = (char*)buffer;
  start = (char*)buffer;
  while ((*temp != '\0') || (*(temp - 1) != '\0')) {
    if (*temp == '\0') {
      adapter_list[i] = start;
      start = temp + 1;
      i++;
      if (i >= list_len) {
        break;
      }
    }
    temp++;
  }
  return i;
}

/** Get the index of an adapter by its GUID
 *
 * @param adapter_guid GUID of the adapter
 * @return index of the adapter or negative on error
 */
int
get_adapter_index(const char* adapter_guid)
{
  char *AdapterList[MAX_NUM_ADAPTERS];
  int i;
  char AdapterName[ADAPTER_NAME_LEN]; /* string that contains a list of the network adapters */
  int AdapterNum;

  if ((adapter_guid != NULL) && (adapter_guid[0] != 0)) {
    AdapterNum = get_adapter_list(AdapterList, MAX_NUM_ADAPTERS, AdapterName, ADAPTER_NAME_LEN);
    if (AdapterNum > 0) {
      for (i = 0; i < AdapterNum; i++) {
        if(strstr((char*)AdapterList[i], adapter_guid)) {
          return i;
        }
      }
    }
  }
  return -1;
}

/**
 * Open a network adapter and set it up for packet input
 *
 * @param adapter_num the index of the adapter to use
 * @param mac_addr the MAC address of the adapter is stored here (if != NULL)
 * @param input a function to call to receive a packet
 * @param arg argument to pass to input
 * @param linkstate the initial link state
 * @return an adapter handle on success, NULL on failure
 */
void*
init_adapter(int adapter_num, char *mac_addr, input_fn input, void *arg, enum link_adapter_event *linkstate)
{
  char *AdapterList[MAX_NUM_ADAPTERS];
  char AdapterName[ADAPTER_NAME_LEN]; /* string that contains a list of the network adapters */
  int AdapterNum;
  unsigned char ethaddr[ETHARP_HWADDR_LEN];
  struct packet_adapter *pa;
  
  pa = (struct packet_adapter *)malloc(sizeof(struct packet_adapter));
  if (!pa) {
    printf("Unable to alloc the adapter!\n");
    return NULL;
  }

  memset(pa, 0, sizeof(struct packet_adapter));
  pa->input = input;
  pa->input_fn_arg = arg;

  AdapterNum = get_adapter_list(AdapterList, MAX_NUM_ADAPTERS, AdapterName, ADAPTER_NAME_LEN);

  /* print all adapter names */
  if (AdapterNum <= 0) {
    free(pa);
    return NULL; /* no adapters found */
  }

  /* invalid adapter index -> check this after printing the adapters */
  if (adapter_num < 0) {
    printf("Invalid adapter_num: %d\n", adapter_num);
    free(pa);
    return NULL;
  }
  /* adapter index out of range */
  if (adapter_num >= AdapterNum) {
    printf("Invalid adapter_num: %d\n", adapter_num);
    free(pa);
    return NULL;
  }

  /* set up the selected adapter */

  /* alloc the OID packet  */

  /* get the description of the selected adapter */

  /* get the MAC address of the selected adapter */

  /* copy the MAC address */

  /* some more adapter settings */

  return pa;
}

/**
 * Close the adapter (no more packets can be sent or received)
 *
 * @param adapter adapter handle received by a call to init_adapter, invalid on return
 */
void
shutdown_adapter(void *adapter)
{
  struct packet_adapter *pa = (struct packet_adapter*)adapter;
  if (pa != NULL) {

    free(pa);
  }
}

/**
 * Send a packet
 *
 * @param adapter adapter handle received by a call to init_adapter
 * @param buffer complete packet to send (including ETH header; without CRC)
 * @param len length of the packet (including ETH header; without CRC)
 */
int
packet_send(void *adapter, void *buffer, int len)
{
  struct packet_adapter *pa = (struct packet_adapter*)adapter;

  if (pa == NULL) {
    return -1;
  }

  return 0;
}

/**
 * Check for newly received packets. Called in the main loop: 'interrupt' mode is not
 * really supported :(
 *
 * @param adapter adapter handle received by a call to init_adapter
 */
void
update_adapter(void *adapter)
{
  struct packet_adapter *pa = (struct packet_adapter*)adapter;

  /* print the capture statistics */
}

/**
 * Check for link state changes. Called in the main loop: 'interrupt' mode is not
 * really supported :(
 *
 * @param adapter adapter handle received by a call to init_adapter
 * @return one of the link_adapter_event values
 */
enum link_adapter_event
link_adapter(void *adapter)
{
  struct packet_adapter *pa = (struct packet_adapter*)adapter;

  return LINKEVENT_UNCHANGED;
}
#endif
