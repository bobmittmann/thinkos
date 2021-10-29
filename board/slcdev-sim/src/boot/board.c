/** 
 * @file board.c
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 * @brief Bootloader descriptor and hardware initialization
 * 
 */

#include <sys/stm32f.h>
#include <sys/delay.h>
#include <sys/dcclog.h>
#define __THINKOS_FLASH__
#include <thinkos/flash.h>
#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>
#define __THINKOS_BOOTLDR__
#include <thinkos/bootldr.h>
#include <thinkos.h>
#include <trace.h>
#include <vt100.h>

#include "board.h"
#include "version.h"

#define PWM_FREQ 100000

void __puts(const char *s)
{
	int rem = __thinkos_strlen(s, 64);

	while (rem) {
		int n = thinkos_console_write(s, rem);
		s += n;
		rem -= n;
	}
}

void isink(int val)
{
	if (val & (1 << 0))
		stm32_gpio_set(IO_SINK1);
	else
		stm32_gpio_clr(IO_SINK1);

	if (val & (1 << 1))
		stm32_gpio_set(IO_SINK2);
	else
		stm32_gpio_clr(IO_SINK2);

	if (val & (1 << 2))
		stm32_gpio_set(IO_SINK3);
	else
		stm32_gpio_clr(IO_SINK3);

	if (val & (1 << 3))
		stm32_gpio_set(IO_SINK4);
	else
		stm32_gpio_clr(IO_SINK4);
}

void irarte(int val)
{
	struct stm32f_dac * dac = STM32_DAC;

	dac->dhr12r2 = 4095 * val / 4;
}

void isink_test(void)
{
	int i;

	for (i = 0; i < 4; ++i) {
		irarte(i);
		thinkos_sleep(4);
		isink(2);
		thinkos_sleep(1);
		isink(6);
		thinkos_sleep(1);
		isink(11);
		thinkos_sleep(1);
		isink(15);
		thinkos_sleep(1);
		isink(0);
	}
}

/* enable RX IDLE and errors interrupt */
#define USART_CR1 (USART_UE | USART_RE | USART_TE | USART_IDLEIE | \
				   USART_TCIE | USART_EIE)


#define RX_DMA_CHAN STM32_DMA_CHANNEL6
#define TX_DMA_CHAN STM32_DMA_CHANNEL7

void __attribute__((noinline)) serdrv_xmit(unsigned int len)
{
	struct stm32f_dma * dma = STM32_DMA1;
	struct stm32_usart * uart = STM32_USART2;
	uint32_t ccr;
	uint32_t cr;

	/* Pulse TE to generate an IDLE Frame */
	cr = uart->cr1;
	uart->cr1 = cr & ~USART_TE;
	uart->cr1 = cr | USART_TE;

	/* Disable DMA */
	while ((ccr = dma->ch[TX_DMA_CHAN].ccr) & DMA_EN)
		dma->ch[TX_DMA_CHAN].ccr = ccr & ~DMA_EN; 
	/* Program DMA transfer */
	dma->ch[TX_DMA_CHAN].cndtr = len;
	dma->ch[TX_DMA_CHAN].ccr = ccr | DMA_EN;	
}

void stm32_usart2_isr(void)
{
	struct stm32_usart * uart = STM32_USART2;
	uint32_t sr;
	
	sr = uart->sr & uart->cr1;

	if (sr & USART_RXNE) {
		DCC_LOG(LOG_TRACE, "RXNE");
	}	

	if (sr & USART_IDLE) {
		DCC_LOG(LOG_TRACE, "IDLE!");
	}

	if (sr & USART_TXE) {
		DCC_LOG(LOG_TRACE, "TXE");
		uart->cr1 &= ~USART_TXEIE;
	}

	if (sr & USART_TC) {
//		DCC_LOG(LOG_TRACE, "TC");
		/* TC interrupt is cleared by writing 0 back to the SR register */
	//	uart->sr = sr & ~USART_TC;
		serdrv_xmit(4);
	}
}

void stm32_dma1_channel7_isr(void)
{
	struct stm32f_dma * dma = STM32_DMA1;

	if (dma->isr & DMA_TCIF7) {
//		DCC_LOG(LOG_TRACE, "TCIF");
		/* clear the DMA transfer complete flag */
		dma->ifcr = DMA_CTCIF7;
	}

	if (dma->isr & DMA_TEIF7) {
		DCC_LOG(LOG_TRACE, "TEIF");
		/* clear the DMA transfer complete flag */
		dma->ifcr = DMA_CTEIF7;
	}
}

void stm32_dma1_channel6_isr(void)
{
	struct stm32f_dma * dma = STM32_DMA1;

	DCC_LOG(LOG_TRACE, "...");

	if (dma->isr & DMA_TCIF6) {
		DCC_LOG(LOG_TRACE, "TCIF");
		/* clear the DMA transfer complete flag */
		dma->ifcr = DMA_CTCIF6;
	}

	if (dma->isr & DMA_TEIF6) {
		DCC_LOG(LOG_TRACE, "TEIF");
		/* clear the DMA transfer error flag */
		dma->ifcr = DMA_CTEIF6;
	}

}

uint32_t __attribute__((noinline)) __crc32(void  * buf, unsigned int len)
{
	struct stm32_crc * crc = STM32_CRC;
	uint8_t * src = (uint8_t *)buf;
	uint8_t * mrk;

	crc->cr = CRC_RESET;

	mrk = src + (len & ~3);
	while (src != mrk) {
		crc->dr = src[0] + (src[1] << 8) + (src[2] << 16) + (src[3] << 24);
		src += 4;
	}
	
	switch (len & 3) {
	case 3:
		crc->dr = src[0] + (src[1] << 8) + (src[2] << 16);
		break;
	case 2:
		crc->dr = src[0] + (src[1] << 8);
		break;
	case 1:
		crc->dr = src[0];
		break;
	}

	return crc->dr;
}

struct serdrv {
	uint32_t tx_buf[520 / 4];
} serdrv;

void __attribute__((noinline)) serdrv_enqueue(uint32_t opc, void  * buf, 
											  unsigned int len)
{
	struct stm32_crc * crc = STM32_CRC;
	uint32_t * src = (uint32_t *)buf;
	uint32_t * dst = (uint32_t *)serdrv.tx_buf;
	uint32_t data;
	uint32_t * mrk;

	crc->cr = CRC_RESET;

	crc->dr = opc;
	*dst++ = opc;

	mrk = src + (len >> 2);
	while (src != mrk) {
		data = *src++;
		crc->dr = data;
		*dst++ = data;
	}

	data = *src;
	switch (len & 3) {
	case 3:
		data &= 0x00ffffff;
		crc->dr = data;
		*dst++ = data | ((crc->dr & 0xff) << 24);
		*dst = (crc->dr >> 8) & 0xff;
		break;
	case 2:
		data &= 0x0000ffff;
		crc->dr = data;
		*dst = data | ((crc->dr & 0xffff) << 16);
		break;
	case 1:
		data &= 0x000000ff;
		crc->dr = data;
		*dst = data | ((crc->dr & 0xffff) << 8);
		break;
	case 0:
		*dst = crc->dr & 0xffff;
	}
}

int serdrv_init(void)
{
	struct stm32f_dma * dma = STM32_DMA1;
	struct stm32_usart * uart = STM32_USART2;

	/* -- UART ----------------------------------------------------------- */
	stm32_clk_enable(STM32_RCC, STM32_CLK_USART2);

	//stm32_usart_init(uart);
	//stm32_usart_baudrate_set(uart, 50000);

	/* configure the UART for DMA transfer */
	uart->cr3 = USART_DMAT | USART_DMAR;
	/* Configure 8N1 */
	uart->cr2 = USART_STOP_1;
	/* enable RX IDLE and errors interrupt */
	uart->cr1 = USART_CR1;

	stm32_clk_enable(STM32_RCC, STM32_CLK_DMA1);
	/* -------------------------------------------------------
	   Configure TX DMA */
	dma->ch[TX_DMA_CHAN].ccr = 0;
	while (dma->ch[TX_DMA_CHAN].ccr & DMA_EN); 
	dma->ch[TX_DMA_CHAN].cpar = &uart->dr;
	dma->ch[TX_DMA_CHAN].cmar = serdrv.tx_buf;
	dma->ch[TX_DMA_CHAN].ccr = DMA_MSIZE_8 | DMA_PSIZE_8 | 
		DMA_MINC | DMA_DIR_MTP | DMA_TEIE | DMA_TCIE;

	DCC_LOG1(LOG_TRACE, "DMA_CCR7=%08x", &dma->ch[TX_DMA_CHAN].ccr);

#ifdef CM3_RAM_VECTORS
	thinkos_irq_register(STM32_IRQ_USART2, IRQ_PRIORITY_LOW, 
						 stm32_usart2_isr);
	thinkos_irq_register(STM32_IRQ_DMA1_CH7, IRQ_PRIORITY_LOW, 
						 stm32_dma1_channel7_isr);
//	thinkos_irq_register(STM32_IRQ_DMA1_CH6, IRQ_PRIORITY_LOW, 
//						 stm32_dma1_channel6_isr);
#else
	cm3_irq_pri_set(STM32_IRQ_USART2, IRQ_PRIORITY_LOW);
	cm3_irq_enable(STM32_IRQ_USART2);
	cm3_irq_pri_set(STM32_IRQ_DMA1_CH7, IRQ_PRIORITY_LOW);
	cm3_irq_enable(STM32_IRQ_DMA1_CH7);
	cm3_irq_pri_set(STM32_IRQ_DMA1_CH6, IRQ_PRIORITY_LOW);
	cm3_irq_enable(STM32_IRQ_DMA1_CH6);
#endif

	return 0;
}

void board_test(void)
{
	struct stm32f_tim * tim = STM32_TIM2;
	struct stm32f_dac * dac = STM32_DAC;
	uint32_t div;
	uint32_t pre;
	uint8_t tx_buf[516];
	unsigned int crc;
	uint32_t n;
	int i;

	/* DAC clock enable */
	stm32_clk_enable(STM32_RCC, STM32_CLK_DAC);
	/* DAC channel 2 initial value */
	dac->dhr12r2 = 0;
	/* DAC configure */
	dac->cr = DAC_EN2;

	/* Timer clock enable */
	stm32_clk_enable(STM32_RCC, STM32_CLK_TIM2);
	/* get the total divisior */
	div = (stm32f_tim1_hz + (PWM_FREQ / 2)) / PWM_FREQ;
	/* get the minimum pre scaler */
	pre = (div / 65536) + 1;
	/* get the reload register value */
	n = (div * 2 + pre) / (2 * pre);
	/* Timer configuration */
	tim->psc = pre - 1;
	tim->arr = n - 1;
	tim->ccmr1 = TIM_OC1M_PWM_MODE1 | TIM_OC2M_PWM_MODE1;
	tim->ccmr2 = TIM_OC3M_PWM_MODE1 | TIM_OC4M_PWM_MODE1;
	tim->ccer = TIM_CC1E | TIM_CC2E | TIM_CC3E | TIM_CC4E;
	tim->ccr1 = (tim->arr * 4) / 8;
	tim->ccr2 = (tim->arr * 4) / 8;
	tim->ccr3 = (tim->arr * 14) / 16; /* White LED */
	tim->ccr4 = (tim->arr * 10) / 16; /* Blue LED */
	tim->cr1 = TIM_URS | TIM_CEN; /* Enable counter */

	serdrv_init();

	for (i = 0; i < 512; ++i)
		tx_buf[i] = i;

//	crc = crc16ccitt(0, tx_buf, 512);
//	tx_buf[512] = crc;
//	tx_buf[513] = crc >> 8;

	crc = __crc32(tx_buf, 512);
	tx_buf[512] = crc;
	tx_buf[513] = crc >> 8;

	serdrv_enqueue(0xaaaaffff, tx_buf, 512);
	serdrv_xmit(512 + 6);

	for (;;) {
		dac->dhr12r2 = 0;

		isink_test();

		thinkos_sleep(50);

		__led_on(IO_LED1);

		thinkos_sleep(100);

		__led_off(IO_LED1);
		__led_on(IO_LED2);
		
		thinkos_sleep(100);

		__led_off(IO_LED2);
		__led_on(IO_LED3);

		thinkos_sleep(100);

		__led_off(IO_LED3);
		__led_on(IO_LED4);

		thinkos_sleep(100);

		__led_off(IO_LED4);
	}	
}

void io_init(void)
{
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOA);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOB);

	/* LEDs */
	stm32_gpio_mode(IO_LED1, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_mode(IO_LED2, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_mode(IO_LED3, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_mode(IO_LED4, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_LED1);
	stm32_gpio_clr(IO_LED2);
	stm32_gpio_clr(IO_LED3);
	stm32_gpio_clr(IO_LED4);

	/* Switches */
	stm32_gpio_mode(IO_SW1UP,  INPUT, PULL_UP | SPEED_LOW);
	stm32_gpio_mode(IO_SW1DWN, INPUT, PULL_UP | SPEED_LOW);
	stm32_gpio_mode(IO_SW2UP,  INPUT, PULL_UP | SPEED_LOW);
	stm32_gpio_mode(IO_SW2DWN, INPUT, PULL_UP | SPEED_LOW);

	/* USART2_TX */
	stm32_gpio_mode(IO_UART_TX, ALT_FUNC, PUSH_PULL | SPEED_LOW);
	stm32_gpio_af(IO_UART_TX, GPIO_AF7);
	/* USART2_RX */
	stm32_gpio_mode(IO_UART_RX, ALT_FUNC, 0);
	stm32_gpio_af(IO_UART_RX, GPIO_AF7);

	/* Comparator */ 
	stm32_gpio_mode(IO_COMP, INPUT, SPEED_MED);

	/* Negative volatge supply */
	stm32_gpio_mode(IO_VNEG_SW, ALT_FUNC, PUSH_PULL | SPEED_MED);
	stm32_gpio_af(IO_VNEG_SW, GPIO_AF1);

	/* Current rate control */
	stm32_gpio_mode(IO_IRATE, ANALOG, 0);
	stm32_gpio_clr(IO_IRATE);

	/* Current sink */
	stm32_gpio_mode(IO_SINK1, OUTPUT, PUSH_PULL | SPEED_MED);
	stm32_gpio_mode(IO_SINK2, OUTPUT, PUSH_PULL | SPEED_MED);
	stm32_gpio_mode(IO_SINK3, OUTPUT, PUSH_PULL | SPEED_MED);
	stm32_gpio_mode(IO_SINK4, OUTPUT, PUSH_PULL | SPEED_MED);
	stm32_gpio_clr(IO_SINK1);
	stm32_gpio_clr(IO_SINK2);
	stm32_gpio_clr(IO_SINK3);
	stm32_gpio_clr(IO_SINK4);

}

#ifndef SOFTRESET_DISABLE_DMA
#define SOFTRESET_DISABLE_DMA 0
#endif

void board_on_softreset(void)
{
//	struct stm32_rcc *rcc = STM32_RCC;
#if SOFTRESET_DISABLE_DMA
	struct stm32f_dma *dma;
	unsigned int j;
#endif

	DCC_LOG(LOG_TRACE, VT_PSH VT_FMG VT_BRI "^^^^ Soft Reset ^^^^" VT_POP);

	/* disable all peripherals clock sources except USB_FS, 
	   GPIOA and GPIOB */

#if SOFTRESET_DISABLE_DMA
	dma = STM32F_DMA1;
	for (j = 0; j < 7; ++j) {
		dma->ch[j].ccr = 0;
		while (dma->ch[j].ccr & DMA_EN);
	}
	dma = STM32F_DMA2;
	for (j = 0; j < 7; ++j) {
		dma->ch[j].ccr = 0;
		while (dma->ch[j].ccr & DMA_EN);
	}
#endif
#if 0
	/* Reset all peripherals except USB_FS, GPIOA, GPIOB, FLASH and RTC */
	rcc->ahbrstr = ~((1 << RCC_FLASH) | (1 << RCC_RTC));
	rcc->ahb2rstr = ~((1 << RCC_GPIOA) | (1 << RCC_GPIOB));
	rcc->ahb3rstr = ~(0);
	rcc->apb1rstr1 = ~((1 << RCC_USBFS) | (1 << RCC_PWR));
	rcc->apb1rstr2 = ~(0);
	rcc->apb2rstr = ~(0);

	rcc->ahb1rstr = 0;
	rcc->ahb2rstr = 0;
	rcc->ahb3rstr = 0;
	rcc->apb1rstr1 = 0;
	rcc->apb1rstr2 = 0;
	rcc->apb2rstr = 0;

	/* enable peripherals clock sources for FLASH, RTC, SPI1, USB_FS, 
	   GPIOA and GPIOB */
	rcc->ahb1enr = (1 << RCC_FLASH) | (RCC_RTC);
	rcc->ahb2enr = (1 << RCC_GPIOA) | (1 << RCC_GPIOB);
	rcc->ahb3enr = 0;
	rcc->apb1enr1 = (1 << RCC_USBFS) | (1 << RCC_PWR);
	rcc->apb1enr2 = 0;
	rcc->apb2enr = (1 << RCC_SPI1);
#endif
	/* initialize IO's */
	io_init();

	serdrv_init();
}

int board_init(void)
{
//	board_on_softreset();

	return 0;
}

#if 0
extern const uint8_t otg_xflash_pic[];
extern const unsigned int sizeof_otg_xflash_pic;

struct magic {
	uint32_t cnt;
	struct {
		uint32_t addr;
		uint32_t mask;
		uint32_t comp;
	} rec[];
};

void board_upgrade(struct dmon_comm * comm)
{
	uint32_t * xflash_code = (uint32_t *)(0x20001000);
	int (* xflash_ram)(uint32_t, uint32_t, const struct magic *) = 
		((void *)xflash_code) + 1;

	cm3_cpsid_f();
	__thinkos_memcpy(xflash_code, otg_xflash_pic, sizeof_otg_xflash_pic);
	xflash_ram(0, 65536, &bootloader_magic);
}

#endif

/* ----------------------------------------------------------------------------
 * Preboot: this task runs once at power up only.
 * It's used to delay booting up the application ... 
 * ----------------------------------------------------------------------------
 */

#define PREBOOT_TIME_SEC 5

int board_preboot_task(void *ptr)
{
	uint32_t tick;

	/* Time window autoboot */
	for (tick = 0; tick < 4*PREBOOT_TIME_SEC; ++tick) {
		thinkos_sleep(250);

		__puts(".");

		switch (tick & 0x3) {
		case 0:
			break;
		case 1:
			break;
		case 2:
			break;
		case 3:
			break;
		}
	}

	thinkos_sleep(250);

	__puts(".\r\n");

	return 0;
}

int board_configure_task(void *ptr)
{
	return 0;
}

int board_selftest_task(void *ptr)
{
	return 0;
}

/* ----------------------------------------------------------------------------
 * This function runs as the main thread's task when the bootloader 
 * fails to run the application ... 
 * ----------------------------------------------------------------------------
 */

int board_default_task(void *ptr)
{
	uint32_t tick;

	for (tick = 0;; ++tick) {
		thinkos_sleep(128);
	}
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

const struct magic_blk thinkos_10_app_magic = {
	.hdr = {
		.pos = 0,
		.cnt = 3},
	.rec = {
		{0xffffffff, 0x0a0de004},
		{0xffffffff, 0x6e696854},
		{0xffffffff, 0x00534f6b},
		{0x00000000, 0x00000000}
		}
};

/* Bootloader board description  
   Bootloader and debugger, memory description  
 */

#define RAM_BOOT  0
#define RAM_APP   1

const struct thinkos_mem_desc sram_desc = {
	.tag = "ram",
	.base = 0x00000000,
	.cnt = 2,
	.blk = {
		/* Bootloader: 2iB */
		[RAM_BOOT] = {.tag = "boot",  .off = 0x20000000, M_RO, SZ_1K, 2},   
		/* Application: 8KiB */
		[RAM_APP] = {.tag = "app",    .off = 0x20001000, M_RW, SZ_1K, 8},   
	}
};


#define FLASH_BOOT 0
#define FLASH_APP  1

const struct thinkos_mem_desc flash_desc = {
	.tag = "flash",
	.base = 0x08000000,
	.cnt = 2,
	.blk = {
		/* Bootloader: 16 KiB */
		[FLASH_BOOT] = {.tag = "boot", .off= 0x00000000, M_RO, SZ_256, 64}, 
		/* Application: 112 KiB */
		[FLASH_APP] = {.tag = "app", .off = 0x0000a000, M_RW, SZ_256, 448}
	}
};

#define MEM_FLASH  0
#define MEM_SRAM   1

const struct thinkos_mem_map mem_map = {
	.tag = "MEM",
	.cnt = 2,
	.desc = {
		[MEM_FLASH] = &flash_desc,
		[MEM_SRAM] = &sram_desc
	}
};


int board_on_break(const struct monitor_comm * comm)
{
	struct btl_shell_env * env = btl_shell_env_getinstance();

	btl_shell_env_init(env, "\r\n+++\r\nThinkOS\r\n", "boot# ");

	return monitor_thread_create(comm, C_TASK(btl_console_shell), 
								 C_ARG(env), true);
}

/* Bootloader board description  */
const struct thinkos_board this_board = {
	.name = "ISINK",
	.desc = "Current Sink Device",
	.hw = {
	       .tag = "ISINK",
	       .ver = {.major = 0,.minor = 1}
	       },
	.sw = {
	       .tag = "ThinkOS",
	       .ver = {
		       .major = VERSION_MAJOR,
		       .minor = VERSION_MINOR,
		       .build = VERSION_BUILD}
	       },
	.memory = &mem_map,
	.on_softreset = board_on_softreset,
	.on_break = board_on_break
};

extern const struct flash_dev stm32l1x_flash_dev;


#pragma GCC diagnostic pop


