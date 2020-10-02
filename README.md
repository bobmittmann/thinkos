# ThinkOS
ThinkOS - Cortex-M Real Time Operating System

ThinkOS is a Real Time Operating System designed specifically for the ARM Cortex-M core. It takes advantage of some unique features of this processor to enable for very low task switch latency.

ThinkOS is a small configurable RTOS kernel that can either be linked directly into your application as a library or installed as a resident operating system independent of the application.

# Core

The core portion consists of a set of calls to perform krnel's initialization configuration and facility to debug Threads.

### Scheduler

ThinkOS scheduler was designed to reduce the context switch time to a minimum, so as to have the minimum possible latency.

### Threads

The ThinkOS is a preemptive time-sharing kernel, different tasks can be implemented in separate execution threads. The ThinkOS kernel holds minimum information about a thread, only what is neccessary for the proper system operation, thus reducing the memory footprint.
Tha absolute bare minimum configuration only the thread's stack pointer is managed by the kernel.

### Time Sharing

All threads with priority higher than 0 go into a round-robin time sharing scheduling policy, when this feature is enabled. Their execution time will be inversely proportional to their priority.

## Memory Management

# Interrupt Model

There are two types of interrupt handlers with ThinkOS:

  - Implicit ISR - Thread interrupt handlers that executes as normal threads. This is a simple call to thinkos_irq_wait(IRQ) inside a regular task or function. The thread will block until this interrupt is raised. This mechanism provides a convenient and powerful tool to design drivers. Devices that benefit most of this type of ISR are simplex I/O channels like I2C or SPI, hardware accelerators like cryptographic ...

  - Native ISR - Normal interrupt handlers that shares a common stack. These handlers are subject to to some constraints regarding API calls. It is recommended to use this type of interrupt handler when low latency is a must. But bare in mind that if the handler is too complex the actual latency due to loading registers and housekeeping may be equivalent to a context switch in which case it will be equivalent of a threaded handler in terms of performance. Another case will be a high priority interrupt you want to make sure it will won't be preempted...


# IPC

ThinkOS can be configured with a varaiety of IPC mechanisms to synchronize threads:

- Semaphores
- Mutexes
- Conditional variables
- Flags
- Events
- Gates

# Aditional Modules


## KRNSVC - Kernel Services

### Console
### Kernel pipes
### Kernel flash driver


## KRNMON - Kernel Monitor


### Console


### Application 


## KRNDBG - Kernel Debug


# Configuration

The file include/thinkos/kernel.h contains the definition of the structures used internally by the OS kernel. In this file there is a series of macros - prefixed by THINKOS_ - used to set default values for the configuration options.

It is not recommended to change the values directly in kernel.h, create a config.h file instead, and define the macro CONFIG_H at the compiler's: -DCONFIG_H.
 
## Configuration Profile 

