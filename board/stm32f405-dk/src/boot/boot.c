/** 
 * @file boot.c
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 * @brief Bootloader descriptor and hardware initialization
 * 
 */

#define __THINKOS_FLASH__
#include <thinkos/flash.h>
#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>
#define __THINKOS_BOOTLDR__
#include <thinkos/bootldr.h>

#include <thinkos.h>

#include <trace.h>
#include <vt100.h>
#include <sys/delay.h>
#include <sys/dcclog.h>

#include "board.h"
#include "version.h"

extern const struct thinkos_board this_board;
extern const struct thinkos_mem_desc flash_mem;
extern const struct thinkos_mem_map mem_map;
#define FLASH_BLK_BOOT 0
#define FLASH_BLK_APP  2

void board_init(void);
void standby_monitor_task(const struct monitor_comm * comm, void * arg);
void boot_monitor_task(const struct monitor_comm * comm, void * arg);

void boot_monitor_task(const struct monitor_comm * comm, void * arg);
void __attribute__((noreturn)) board_default_task(void *ptr);
int board_integrity_check(void);

void __attribute__((noreturn)) main(int argc, char ** argv)
{
	struct thinkos_rt * krn = &thinkos_rt;
	const struct monitor_comm * comm;
	uintptr_t app_addr;

#if DEBUG
	DCC_LOG_INIT();
	DCC_LOG_CONNECT();
	mdelay(125);

	DCC_LOG(LOG_TRACE, "\n\n" VT_PSH VT_BRI VT_FBL);
	DCC_LOG(LOG_TRACE, "*************************************************");
	DCC_LOG(LOG_TRACE, "*    STM32F507-DK ThinkOS Custom Bootloader     *");
	DCC_LOG(LOG_TRACE, "*************************************************"
			VT_POP "\n\n");
	mdelay(125);

	DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR 
			"* 1. thinkos_krn_init()." VT_POP);
	mdelay(125);
#endif

	thinkos_krn_init(krn, THINKOS_OPT_PRIORITY(0) | THINKOS_OPT_ID(0) |
					 THINKOS_OPT_PRIVILEGED |
					 THINKOS_OPT_STACK_SIZE(32768), &mem_map);

#if DEBUG
	DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR 
			"* 2. board_init()." VT_POP);
	mdelay(125);
#endif
	board_init();

#if DEBUG
	DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR 
			"* 3. usb_comm_init()." VT_POP);
	mdelay(125);
#endif
	comm = usb_comm_init(&stm32f_otg_fs_dev);

#if DEBUG
	mdelay(125);
	DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR 
			"* 4. thinkos_krn_monitor_init()." VT_POP);
#endif
	thinkos_krn_monitor_init(comm, boot_monitor_task, (void *)&this_board);

#if DEBUG
	mdelay(125);
	DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR 
			"* 5. thinkos_krn_irq_on()." VT_POP);
#endif
	/* enable interrupts */
	thinkos_krn_irq_on();

	DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR 
			"* 6. board_integrity_check()..." VT_POP);
	if (!board_integrity_check()) {
		DCC_LOG(LOG_ERROR, VT_PSH VT_BRI VT_FRD
				"**** board_integrity_check() failed." VT_POP);
#if DEBUG
		mdelay(10000);
#endif
		thinkos_abort();
	}

	app_addr = flash_mem.base + flash_mem.blk[FLASH_BLK_APP].off;  

	DCC_LOG1(LOG_TRACE, VT_PSH VT_BRI VT_FGR 
			"* 7. thinkos_app_exec(%08x)..." VT_POP, app_addr);
	thinkos_app_exec(app_addr);
	DCC_LOG(LOG_ERROR, VT_PSH VT_BRI VT_FRD
			"**** thinkos_app_exec() failed." VT_POP);
#if DEBUG
	mdelay(10000);
#endif

	DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR 
			"* 8. board_default_task()..." VT_POP);
	board_default_task((void*)&this_board);
}


