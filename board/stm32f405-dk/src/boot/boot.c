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
void board_reset(void);
void boot_monitor_task(const struct monitor_comm * comm, void * arg,
					   uintptr_t, struct thinkos_rt *);
int board_integrity_check(void);
extern const struct thinkos_mem_map board_mem_map;
extern const struct thinkos_board this_board;
extern const struct thinkos_flash_desc board_flash_desc;

extern const struct thinkos_comm stm32_uart1_comm_instance;
extern const struct thinkos_comm usb_cdc_comm_instance;


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
//	const struct monitor_comm * comm;
	int h;


#if DEBUG
	int i;

	DCC_LOG_INIT();
	DCC_LOG_CONNECT();
	mdelay(125);

	DCC_LOG(LOG_TRACE, "\n\n" VT_PSH VT_BRI VT_FBL);
	DCC_LOG(LOG_TRACE, "*************************************************");
	DCC_LOG(LOG_TRACE, "*    STM32F405-DK ThinkOS Custom Bootloader     *");
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

#if DEBUG
	DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR 
			"* 4. thinkos_krn_comm_init()." VT_POP);
	mdelay(125);
#endif
//	thinkos_krn_comm_init(krn, 0, &stm32_uart1_comm_instance, NULL);
	thinkos_krn_comm_init(krn, 0, &usb_cdc_comm_instance, (void *)&stm32f_otg_fs_dev);
  
#if DEBUG
	DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR 
			"* 5. usb_comm_init()." VT_POP);
	mdelay(125);
#endif
//	comm = usb_comm_init(&stm32f_otg_fs_dev);

#if DEBUG
	mdelay(125);
	DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR 
			"* 5. thinkos_krn_monitor_init()." VT_POP);
#endif
//	thinkos_krn_monitor_init(krn, comm, boot_monitor_task, (void *)&this_board);
	board_reset();

#if DEBUG
	mdelay(125);
	DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR 
			"* 6. thinkos_krn_sched_on()." VT_POP);
#endif
	thinkos_krn_sched_on(krn);

	DCC_LOG(LOG_TRACE, "thinkos_sleep()...");
	thinkos_sleep(2000);

	h = thinkos_comm_open(0);
	thinkos_comm_timedsend(h, "Hello world!\r\n", 14, 48);
	thinkos_comm_timedsend(h, "Many, but not all people.\r\n", 27, 0);


	btl_shell_env_init(env, "\r\n+++\r\nThinkOS\r\n", "boot# ");


//	monitor_signal(SIG_COMM_BRK); 
//	thinkos_sleep(1000);
//	monitor_signal(SIG_CONSOLE_CTRL); 
//	thinkos_sleep(1000);
//	monitor_signal(SIG_CONSOLE_CTRL); 
//	thinkos_sleep(1000);

	DCC_LOG(LOG_TRACE, "board_integrity_check(),,,");
	if (board_integrity_check()) {
		DCC_LOG(LOG_TRACE, "btl_flash_app_exec(APP)...");
		btl_flash_app_exec("APP", 0, 0);
	}

	DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR 
			"* 6. btl_flash_app_exec() fail." VT_POP);

	btl_console_shell(env);
}

