/* main program of Free Software for Calypso Phone */

/* (C) 2010 by Harald Welte <laforge@gnumonks.org>
 *
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include <stdint.h>
#include <stdio.h>

#include <debug.h>
#include <memory.h>
#include <string.h>
#include <delay.h>
#include <rffe.h>
#include <keypad.h>
#include <board.h>

#include <abb/twl3025.h>
#include <rf/trf6151.h>

#include <comm/sercomm.h>
#include <comm/timer.h>

#include <calypso/clock.h>
#include <calypso/tpu.h>
#include <calypso/tsp.h>
#include <calypso/irq.h>
#include <calypso/misc.h>
#include <calypso/sim.h>

#include <layer1/sync.h>
#include <layer1/async.h>
#include <layer1/tpu_window.h>
#include <layer1/l23_api.h>

#include <fb/framebuffer.h>
/////////////////////////////////////////////////////////////////////////
//mod here
#include <osmocom/bb/common/osmocom_data.h>
#include <osmocom/bb/common/l1ctl.h>
#include <osmocom/bb/common/l1l2_interface.h>
#include <osmocom/bb/common/sap_interface.h>
#include <osmocom/bb/misc/layer3.h>
#include <osmocom/bb/common/logging.h>
#include <osmocom/bb/common/l23_app.h>

#include <osmocom/core/msgb.h>
#include <osmocom/core/talloc.h>
#include <osmocom/core/select.h>
#include <osmocom/core/linuxlist.h>
#include <osmocom/core/gsmtap_util.h>
#include <osmocom/core/gsmtap.h>
#include <osmocom/core/utils.h>
////////////////////////////////////////////////////////

struct log_target *stderr_target;

void *l23_ctx = NULL;


struct llist_head ms_list;
static struct osmocom_ms *ms = NULL;
static struct osmocom_ms *ms1 = NULL;
int rc;
int (*l23_app_work) (struct osmocom_ms *ms) = NULL;
int (*l23_app_exit) (struct osmocom_ms *ms) = NULL;
int one=0;


//24nov12
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char *hr = "======================================================================\n";

/* MAIN program **************************************************************/

static void key_handler(enum key_codes code, enum key_states state);

int main(void)
{
	uint8_t atr[20];
	uint8_t atrLength = 0;

	board_init();

	puts("\n\nOsmocomBB Layer 1 (revision " GIT_REVISION ")\n");
	puts(hr);

	/* Dump device identification */
	dump_dev_id();
	puts(hr);

	keypad_set_handler(&key_handler);

	/* Dump clock config after PLL set */
	calypso_clk_dump();
	puts(hr);

	fb_clear();

	fb_setfg(FB_COLOR_BLACK);
	fb_setbg(FB_COLOR_WHITE);
	fb_setfont(FB_FONT_HELVB14);

	fb_gotoxy(2,20);
	fb_putstr("Layer 1",framebuffer->width-4);

	fb_setfg(FB_COLOR_RED);
	fb_setbg(FB_COLOR_BLUE);

	fb_gotoxy(2,25);
	fb_boxto(framebuffer->width-3,38);

	fb_setfg(FB_COLOR_WHITE);
	fb_setfont(FB_FONT_HELVR08);
	fb_gotoxy(8,33);
	fb_putstr("osmocom-bb",framebuffer->width-4);

	fb_flush();

	/* initialize SIM */
	calypso_sim_init();

	puts("Power up simcard:\n");
	memset(atr,0,sizeof(atr));
	atrLength = calypso_sim_powerup(atr);
	puts(hr);
	//initialise_queue();
	layer1_init();
	//puts("TP2.0\n");--working
	tpu_frame_irq_en(1, 1);
	//puts("TP2.1\n");
//	puts(hr);--working
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//init_queue();
	//initialize_queue();

	INIT_LLIST_HEAD(&ms_list);
		l23_ctx = talloc_named_const(NULL, 1, "layer2 context");
	//	puts("TP2.2\n");
		ms = talloc_zero(l23_ctx, struct osmocom_ms);
		ms1= ms;
	//	puts("TP2.3\n");

		if (!ms) {			puts("error");			exit(1);		}

	//	print_copyright();

		llist_add_tail(&ms->entity, &ms_list);

		sprintf(ms->name, "1");

//		puts("TP2.4\n");--working
	//struct msgb *msgx;
	//msgx = msgb_alloc_headroom(512+64,64, "SIM");
	//initqueue();
	//msgx->data="akib is here ";
	//message_enqueue(msgx);
	//message_dequeue();





	//exit(1);

		ms->test_arfcn = 116;
		//printf("asdasd%d",ms1->test_arfcn);
		//puts("TP2.5\n");--working


		//rc = layer2_open(ms, "asdsa");



		ms->lapdm_channel.lapdm_dcch.l1_ctx = ms;
		ms->lapdm_channel.lapdm_dcch.l3_ctx = ms;
		ms->lapdm_channel.lapdm_acch.l1_ctx = ms;
		ms->lapdm_channel.lapdm_acch.l3_ctx = ms;
		lapdm_channel_init(&ms->lapdm_channel, LAPDM_MODE_MS);
		lapdm_channel_set_l1(&ms->lapdm_channel, l1ctl_ph_prim_cb, ms);
	//	puts("TP2.6\n");
	//	puts("TP2.6\n");
		rc = l23_app_init(ms);
	//	puts("TP2.7\n");
		if (rc < 0)
			exit(1);


int test=0;
///////////////////////////////////////////////////////////////////////////////////
	//	puts("ok working");
	while (1) {
//printf("\nl1-compl-exe");
		l1a_compl_execute();
//printf("\nosmo-timer-update");
		osmo_timers_update();
//printf("\nl1a_l23_handl--------1");
		l1a_l23_handler();
//printf("\ndelay");
//printf("\ndelay-2");
//if (test<=10)
//{
//	delay_ms(100);

//}
//printf("==============|");
//printf("\nl2_l1_hand");

		l2_l1_handler(ms);

		//printf("\ncount-%d",test);
	//	printf("l2 completed \n \n");
		test=test+1;
	}

	/* NOT REACHED */

	twl3025_power_off();
}

static int afcout = 0;

static void tspact_toggle(uint8_t num)
{
	printf("TSPACT%u toggle\n", num);
	tsp_act_toggle((1 << num));
	tpu_enq_sleep();
	tpu_enable(1);
	tpu_wait_idle();
}

static void key_handler(enum key_codes code, enum key_states state)
{
	if (state != PRESSED)
		return;

	switch (code) {
	case KEY_4:
		tspact_toggle(6);	/* TRENA (RFFE) */
		break;
	case KEY_5:
		tspact_toggle(8);	/* GSM_TXEN (RFFE) */
		break;
	case KEY_6:
		tspact_toggle(1);	/* PAENA (RFFE) */
		break;
	case KEY_7:			/* decrement AFC OUT */
		afcout -= 100;
		if (afcout < -4096)
			afcout = -4096;
		twl3025_afc_set(afcout);
		printf("AFC OUT: %u\n", twl3025_afcout_get());
		break;
	case KEY_9:			/* increase AFC OUT */
		afcout += 100;
		if (afcout > 4095)
			afcout = 4095;
		twl3025_afc_set(afcout);
		printf("AFC OUT: %u\n", twl3025_afcout_get());
		break;
	default:
		break;
	}
	/* power down SIM, TODO:  this will happen with every key pressed,
       put it somewhere else ! */
	calypso_sim_powerdown();
}


