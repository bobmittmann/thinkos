/** 
 * @file boot.c
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 * @brief Bootloader descriptor and hardware initialization
 * 
 */

#define __THINKOS_FLASH__
#include <thinkos/flash.h>
#define __THINKOS_BOOTLDR__
#include <thinkos/bootldr.h>
#define __THINKOS_CONSOLE__
#include <thinkos/console.h>

#include <thinkos.h>

#include <trace.h>
#include <vt100.h>
#include <sys/delay.h>
#include <sys/dcclog.h>

#include "board.h"
#include "version.h"

void board_init(void);
void boot_monitor_task(const struct monitor_comm * comm, void * arg);
int board_integrity_check(void);
extern const struct thinkos_mem_map board_mem_map;
extern const struct thinkos_board this_board;
extern const struct thinkos_flash_desc board_flash_desc;

void usb_vbus_connect(bool connect)
{
	if (connect)
		stm32_gpio_mode(OTG_FS_VBUS, ALT_FUNC, SPEED_LOW);
	else
		stm32_gpio_mode(OTG_FS_VBUS, INPUT, 0);
}

void main(int argc, char ** argv)
{
	struct btl_shell_env * env = btl_shell_env_getinstance();
	struct thinkos_rt * krn = &thinkos_rt;
	const struct monitor_comm * comm;


#if DEBUG
	int i;

	DCC_LOG_INIT();
	DCC_LOG_CONNECT();
	mdelay(125);

	DCC_LOG(LOG_TRACE, "\n\n" VT_PSH VT_BRI VT_FBL);
	DCC_LOG(LOG_TRACE, "*************************************************");
	DCC_LOG(LOG_TRACE, "*    STM32F507-DK ThinkOS Custom Bootloader     *");
	DCC_LOG(LOG_TRACE, "*************************************************"
			VT_POP "\n\n");
	mdelay(125);


	for (i = 0; i < 100; ++i) {
		DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR 
				"* +++" VT_POP);
		mdelay(125);
	}

	DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR 
			"* 1. thinkos_krn_init()." VT_POP);
	mdelay(125);
#endif

	thinkos_krn_init(krn, THINKOS_OPT_PRIORITY(0) | THINKOS_OPT_ID(0) |
					 THINKOS_OPT_PRIVILEGED |
					 THINKOS_OPT_STACK_SIZE(32768), &board_mem_map);

#if DEBUG
	DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR 
			"* 2. board_init()." VT_POP);
	mdelay(125);
#endif
	board_init();

#if DEBUG
	DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR 
			"* 3. thinkos_krn_flash_drv_init()." VT_POP);
	mdelay(125);
#endif
	thinkos_krn_flash_drv_init(krn, 0, &board_flash_desc);

	btl_shell_env_init(env, "\r\n+++\r\nThinkOS\r\n", "boot# ");

#if DEBUG
	DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR 
			"* 4. usb_comm_init()." VT_POP);
	mdelay(125);
#endif
	comm = usb_comm_init(&stm32f_otg_fs_dev);

#if DEBUG
	mdelay(125);
	DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR 
			"* 5. thinkos_krn_monitor_init()." VT_POP);
#endif
	thinkos_krn_monitor_init(krn, comm, boot_monitor_task, 
							 (void *)&this_board);

	thinkos_sleep(200);

//	monitor_signal(SIG_COMM_BRK); 
//	thinkos_sleep(1000);
//	monitor_signal(SIG_CONSOLE_CTRL); 
//	thinkos_sleep(1000);
//	monitor_signal(SIG_CONSOLE_CTRL); 
//	thinkos_sleep(1000);

	if (board_integrity_check()) {
		btl_flash_app_exec("APP");
	}

	btl_console_shell(env);
}

