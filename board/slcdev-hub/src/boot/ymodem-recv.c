/* cmd-vidc_ysend.c - VIDC realtime capture tool 
 * ----------------
 *
 *   ************************************************************************
 *   **            Company Confidential - For Internal Use Only            **
 *   **          Mircom Technologies Ltd. & Affiliates ("Mircom")          **
 *   **                                                                    **
 *   **   This information is confidential and the exclusive property of   **
 *   ** Mircom.  It is intended for internal use and only for the purposes **
 *   **   provided,  and may not be disclosed to any third party without   **
 *   **                prior written permission from Mircom.               **
 *   **                                                                    **
 *   **                        Copyright 2017-2018                         **
 *   ************************************************************************
 *
 */

/** 
 * @file cmd-vidc_ysend.c
 * @brief VI Meter DC capture tool
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */

#include <xmodem.h>

int ymodem_app_upload(FILE * f, const char * fname, unsigned int size)
{
	struct ymodem_rcv sy;
	struct comm_dev comm;
	uint32_t clk_begin;
	uint32_t clk_end;
	uint32_t seq;
	unsigned int cnt;
	int ret;

	fprintf(f, "YMODEM send ...");

	comm.arg = NULL;
	comm.op.send = console_write;
	comm.op.recv = console_read;

	ymodem_snd_init(&sy, &comm, XMODEM_SND_1K);

	thinkos_console_raw_mode(true);

	cnt = 0;
	clk_begin = 0;
	clk_end = 0;

	if ((ret = ymodem_snd_start(&sy, fname, size)) < 0) {
		ERR("SY: ymodem_snd_start() ret=%d", ret);
	} else {

		seq = vimeter_vidc_mon_get_first(vx, ix);

		clk_begin = thinkos_clock();

		while (cnt < size) {
			int n;

			n = NAMP_CKTS * sizeof(float);

			if ((ret = ymodem_snd_loop(&sy, vx, n)) < 0) {
				ERR("ymodem_snd_loop() ret=%d", ret);
				break;
			}

			if ((ret = ymodem_snd_loop(&sy, ix, n)) < 0) {
				ERR("ymodem_snd_loop() ret=%d", ret);
				break;
			}

			seq = vimeter_vidc_mon_get_next(vx, ix, seq);

			cnt += n * 2;
		} 

		clk_end = thinkos_clock();

		if (ret >= 0) {
			ymodem_snd_eot(&sy);
		}
	}

	thinkos_console_raw_mode(false);

	if (ret < 0) {
		fprintf(f, " failed!.\n");
	} else {
		int32_t dt_ms = clk_end - clk_begin;
		int32_t sps = (cnt * 1000) / dt_ms;
		int32_t sec = dt_ms / 1000;
		int32_t ms = dt_ms % 1000;
		
		fprintf(f, " ok. %d samples, %d.%03d sec = %d s/sec.\n",
				cnt, sec, ms, sps);
		ret = cnt;
	}

	fflush(f);

	return ret;
}

