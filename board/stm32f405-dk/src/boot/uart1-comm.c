// uart5-comm.c
// ------------
//
//   ************************************************************************
//   **            Company Confidential - For Internal Use Only            **
//   **          Mircom Technologies Ltd. & Affiliates ("Mircom")          **
//   **                                                                    **
//   **   This information is confidential and the exclusive property of   **
//   ** Mircom.  It is intended for internal use and only for the purposes **
//   **   provided,  and may not be disclosed to any third party without   **
//   **                prior written permission from Mircom.               **
//   **                                                                    **
//   **                        Copyright 2017-2018                         **
//   ************************************************************************
//

/**
 * @file uart5-comm.c
 * @brief
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */

#define __THINKOS_FIFO__
#include <thinkos/fifo.h>

#define __THINKOS_COMM__
#include <thinkos/comm.h>

#include <sys/stm32f.h>

#include <sys/dcclog.h>
#include <vt100.h>

#if (THINKOS_COMM_MAX) > 0

/*
FIXME: RX fifo not implemented....
  */
#define UART_RX_FIFO_SIZE 0
#define UART_TX_FIFO_SIZE 64

struct stm32_uart_drv {
	struct stm32_usart *uart;
	uint16_t tx_wq;
	uint16_t rx_wq;

#if (UART_RX_FIFO_SIZE) > 8
	struct {
		struct thinkos_fifo8_hdr hdr;
		uint8_t dat[UART_RX_FIFO_SIZE];
	} rx_fifo;
#endif
#if (UART_TX_FIFO_SIZE) > 8
	struct {
		struct thinkos_fifo8_hdr hdr;
		uint8_t dat[UART_TX_FIFO_SIZE];
	} tx_fifo;
#endif
};


void stm32_uart_signal(struct stm32_uart_drv * drv, unsigned int sig)
{
	struct stm32_usart *uart = drv->uart;

	if (sig == COMM_TX_PEND) {
		/* enable TX empty interrupt */
		uart->cr1 |= USART_TXEIE;
		DCC_LOG(LOG_TRACE, "TX FIFO...");
	}
}

static void stm32_uart_init(struct stm32_usart *uart, unsigned int baudrate)
{
	uint32_t f_pclk;
	uint32_t div;
	uint32_t f;
	uint32_t m;
	uint32_t cr1;
	uint32_t cr2;
	uint32_t cr3;

	/* Disable peripheral */
	uart->cr1 = 0;
	uart->cr2 = 0;
	uart->cr3 = 0;
	/* Clear pending interrupts */
	uart->sr = 0;

	cr1 = 0;
	cr2 = 0;
	cr3 = 0;

	f_pclk = stm32_clk_hz(STM32_CLK_USART1);
	div = f_pclk / baudrate;
	m = div >> 4;
	f = div & 0x0f;
	uart->brr = (m << 4) | f;

	/* 8 bits */
	cr1 |= USART_M8;
	/* 1 stop bit */
	cr2 |= USART_STOP_1;

	/* enable UART */
	cr1 |= USART_UE;
	/* enable transfer complete interrupt */
	cr1 |= USART_TXEIE | USART_TCIE | USART_RXNEIE;

	/* write configuration registers */
	uart->cr1 = cr1;
	uart->cr2 = cr2;
	uart->cr3 = cr3;

	/* enable transmitter and receiver */
	cr1 |= USART_TE | USART_RE;
	uart->cr1 = cr1;
}

int stm32_uart_open(struct stm32_uart_drv * drv)
{
	struct stm32_usart *uart = drv->uart;

	DCC_LOG(LOG_TRACE, "Initializing UART...");
	stm32_uart_init(uart, 115200);

	return THINKOS_OK;
}

const struct thinkos_comm_drv_op stm32_uart_comm_drv_op  = {
	.open = (int (*)(const void *))stm32_uart_open,
	.send = (int (*)(const void *, const void *, unsigned int))NULL,
	.recv = (int (*)(const void *, void *, unsigned int))NULL,
	.ctrl = (int (*)(const void *, unsigned int))NULL,
	.signal = (void (*)(const void *, unsigned int))stm32_uart_signal
};

struct stm32_uart_drv stm32_uart1_drv_instance;

void stm32f_usart1_isr(void)
{
	struct stm32_uart_drv * drv = &stm32_uart1_drv_instance;
#if (UART_RX_FIFO_SIZE) > 0
	struct thinkos_fifo8 * rx_fifo = THINKOS_FIFO8(&drv->rx_fifo);
#endif
#if (UART_TX_FIFO_SIZE) > 0
	struct thinkos_fifo8 * tx_fifo = THINKOS_FIFO8(&drv->tx_fifo);
#endif
	struct stm32_usart * uart = drv->uart;
	struct thinkos_rt * krn = &thinkos_rt;
	uint32_t cr;
	uint32_t sr;
	int c;
	cr = uart->cr1;
	sr = uart->sr & (cr | USART_ORE | USART_LBD);

	if (sr & USART_TXE) {
#if (UART_TX_FIFO_SIZE) > 0
		uint32_t head = tx_fifo->head;
		uint32_t tail = tx_fifo->tail;
		uint32_t size = tx_fifo->size;
		uint32_t mask = size - 1;
		int n;
		
		DCC_LOG2(LOG_TRACE, "TXE head=%d tail=%d", tx_fifo->head, tx_fifo->tail);

		if ((n = (head - tail)) == 0) {
			/* FIFO empty, process a tx request */
			uint8_t * dst = &tx_fifo->buf[head & mask];
			int cnt = size - (head & mask);
			n = krn_comm_tx_wq_req_process(krn, drv->tx_wq, dst, cnt);
			tx_fifo->head = head + n; 
			DCC_LOG2(LOG_TRACE, "TXE cnt=%d n=%d", cnt, n);
		}

		if (n == 0) {
			/* Nothing to be sent */
			uart->cr1 = cr & ~USART_TXEIE;
		} else {
			int c = tx_fifo->buf[tail & mask];
			tx_fifo->tail = tail + 1;
			DCC_LOG2(LOG_TRACE, "TXE '%c' n=%d", c, n);
			uart->tdr = c;
		}

#else
		if ((c = krn_comm_tx_getc(krn, drv->tx_wq)) < 0 )  {
			uart->cr1 = cr & ~USART_TXEIE;
		} else { 
			uart->tdr = c;
		} 
#endif
	}

	if (sr & USART_RXNE) {
		DCC_LOG(LOG_TRACE, "RXNE");
		c = uart->rdr;
#if (UART_RX_FIFO_SIZE) > 0
		__krn_fifo8_putc(rx_fifo, c);
#else
		krn_comm_rx_putc(krn, drv->rx_wq, c);
#endif
	}

	if (sr & USART_TC) {
		/* TC interrupt is cleared by writing 0 btail to the SR register */
		uart->sr = sr & ~USART_TC;
		/* disable the transfer complete interrupt */
		cr &= ~USART_TCIE;
	}

	/* break detection */
	if (sr & USART_LBD) {
		/* clear the break detection interrupt flag */
		uart->sr = sr & ~(USART_ORE | USART_LBD);
	}

	/* idle detection */
	if (sr & USART_IDLE) {
		c = uart->rdr;
		(void)c;
	}

	if (sr & USART_ORE) {
		c = uart->rdr;
		(void)c;
	}
	
	if (sr & USART_ORE) {
		c = uart->rdr;
		(void)c;
	}
}

int stm32_uart1_comm_init(const struct thinkos_comm * comm, void * param, 
						  int tx_wq, int rx_wq)
{
//	const struct thinkos_comm * comm = &stm32_uart5_comm_instance;
	struct stm32_uart_drv * drv = (struct stm32_uart_drv *)comm->drv;
	struct stm32_usart *uart = STM32_USART1;
#if (UART_RX_FIFO_SIZE) > 0
	struct thinkos_fifo8 * rx_fifo = THINKOS_FIFO8(&drv->rx_fifo);
#endif
#if (UART_TX_FIFO_SIZE) > 0
	struct thinkos_fifo8 * tx_fifo = THINKOS_FIFO8(&drv->tx_fifo);
#endif

	/* Enable peripheral clock */
	stm32_clk_enable(STM32_RCC, STM32_CLK_USART1);

	drv->uart = uart;
	drv->tx_wq = tx_wq;
	drv->rx_wq = rx_wq;
#if (UART_RX_FIFO_SIZE) > 0
	__krn_fifo8_init(rx_fifo, UART_RX_FIFO_SIZE);
#endif
#if (UART_TX_FIFO_SIZE) > 0
	__krn_fifo8_init(tx_fifo, UART_TX_FIFO_SIZE);
#endif

	DCC_LOG2(LOG_TRACE, "tx_wq=%d rx_wq=%d", tx_wq, rx_wq);

	return 0;
}

void stm32_uart1_comm_reset(const struct thinkos_comm * comm, int priority)
{
	DCC_LOG1(LOG_TRACE, "priority=%d", priority);

	/* configure interrupts */
	cm3_irq_pri_set(STM32_IRQ_USART1, priority);
	/* enable interrupts */
	cm3_irq_enable(STM32_IRQ_USART1);
}

struct thinkos_comm_krn_op stm32_uart1_comm_krn_op = {
	.init = (int (*)(const struct thinkos_comm *, void *, 
					 int, int))stm32_uart1_comm_init,
	.reset = (void (*)(const struct thinkos_comm *, int))stm32_uart1_comm_reset,
	.done = (int (*)(const struct thinkos_comm *))NULL
};

const struct thinkos_comm stm32_uart1_comm_instance = {
	.drv = (void *)&stm32_uart1_drv_instance,
	.drv_op = &stm32_uart_comm_drv_op,
	.krn_op = &stm32_uart1_comm_krn_op,
};

#endif /* (THINKOS_COMM_MAX) > 0 */

