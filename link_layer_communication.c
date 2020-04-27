/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 * This file is part of the Contiki operating system.
 *
 */


#include "contiki.h"
#include "os/net/nullnet/nullnet.h"
#include <string.h>
#include <stdio.h>
#include "os/sys/log.h"
#include "os/sys/etimer.h"
#include "os/net/netstack.h"
#include "net/linkaddr.h"

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define SEND_INTERVAL (5 * CLOCK_SECOND)
#define SEND_INTERVAL_BRO (13 * CLOCK_SECOND)
static linkaddr_t dest_addr = {{ 0x0001, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }};

static int uni_period = 0;
static int bro_period = 0;

PROCESS(comm_proc, "unicast broadcast example");
AUTOSTART_PROCESSES(&comm_proc);

static void input_callback(const void *data, uint16_t len, const linkaddr_t *src, const linkaddr_t *dest){
	char received_data[strlen((char *)data) + 1];
	if(len == strlen((char *)data) + 1) {
		memcpy(& received_data, data, strlen((char *)data) + 1);
		LOG_INFO("TIMESTAMP: %lu, Received \"%s\", from ", clock_seconds(), received_data); LOG_INFO_LLADDR(src);
		LOG_INFO_("\n");
	}
}


PROCESS_THREAD(comm_proc, ev, data){
	static struct etimer periodic_timer;
	static struct etimer broadcast_periodic_timer;

	static char uni_msg[70];
	static char bro_msg[70];

	sprintf(uni_msg, "Hello, I send you my msg at time: %d", uni_period);
	sprintf(bro_msg, "Hello, I am an annoying spammer, time: %d", bro_period);

	PROCESS_BEGIN();
	nullnet_set_input_callback(input_callback);
	if(linkaddr_node_addr.u8[0] == 2){
		nullnet_buf = (uint8_t *)bro_msg;
		etimer_set(&broadcast_periodic_timer, SEND_INTERVAL_BRO);
	}
	if(linkaddr_node_addr.u8[0] == 3 || linkaddr_node_addr.u8[0] == 4){
		nullnet_buf = (uint8_t *)uni_msg;
		etimer_set(&periodic_timer, SEND_INTERVAL);
	}
	while(1) {
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer)|| etimer_expired(&broadcast_periodic_timer));

		if(etimer_expired(&periodic_timer) && (linkaddr_node_addr.u8[0] == 3 || linkaddr_node_addr.u8[0] == 4)){
			LOG_INFO("TIMESTAMP: %lu, Sending UNICAST \"%s\", to ", clock_seconds(), uni_msg);
			LOG_INFO_LLADDR(&dest_addr);
			LOG_INFO_("\n");
			nullnet_len = strlen(uni_msg)+1;
			NETSTACK_NETWORK.output(&dest_addr); uni_period +=8; etimer_reset(&periodic_timer);
		}

		if(etimer_expired(&broadcast_periodic_timer) && linkaddr_node_addr.u8[0] == 2){
			LOG_INFO("TIMESTAMP: %lu, Sending BROADCAST \"%s\", to ", clock_seconds(), bro_msg);
			LOG_INFO_LLADDR(NULL);
			LOG_INFO_("\n");
			nullnet_len = strlen(bro_msg)+1;
			NETSTACK_NETWORK.output(NULL);
			etimer_reset(&broadcast_periodic_timer);
			bro_period+=13;
		}
	}
	PROCESS_END();
}
