============================
 ThinkOS Kernel - Scheduler
============================
-------------------------
 ARM Cortex M Exceptions
-------------------------

Exception return behavior
==========================


**EXC_RETURN definition of exception return behavior, no FP extension**

=========== ============== ============= ============
 EXC_RETURN    Return to   Return stack   Frame type
=========== ============== ============= ============
 0xFFFFFFF1  Handler mode          Main        Basic
 0xFFFFFFF9   Thread mode          Main        Basic
 0xFFFFFFFD   Thread mode       Process        Basic
=========== ============== ============= ============

**EXC_RETURN definition of exception return behavior, with FP extension**

=========== ============== ============= ============
 EXC_RETURN    Return to   Return stack   Frame type
=========== ============== ============= ============
 0xFFFFFFE1  Handler mode          Main     Extended
 0xFFFFFFE9   Thread mode          Main     Extended
 0xFFFFFFED   Thread mode       Process     Extended
 0xFFFFFFF1  Handler mode          Main        Basic
 0xFFFFFFF9   Thread mode          Main        Basic
 0xFFFFFFFD   Thread mode       Process        Basic
=========== ============== ============= ============

Integrity checks on exception return
------------------------------------

The ARMv7-M architecture provides a number of integrity checks on an exception return. These provide a guard against errors in the system software. Incorrect exception return information might be inconsistent with the state of execution that the processor holds in hardware or, or inconsistent with other state stored by the exception mechanisms.

The hardware-related integrity checks ensure that the tracking of active exceptions in the NVIC and SCB hardware is consistent with the exception return.

The integrity checks test the following on an exception return:

- The Exception number being returned from, as held in the IPSR at the start of the return, is listed in the SCB as being active.

- Normally, if at least one exception other than the returning exception is active, the return must be to Handler mode. This checks for a mismatch of the number of exception returns. Software can use the CCR.NONBASETHRDENA to disable this check, see Configuration and Control Register, CCR on page B3-660.

- On a return to Thread mode, the value restored to the IPSR Exception number field must be 0.

- On a return to Handler mode, the value restored to the IPSR Exception number field must not be 0.

- EXC_RETURN[3:0] must not be a reserved value, see Table B1-8 on page B1-595.


Any failed check causes an INVPC UsageFault, with the EXC_RETURN value in the LR.

An exception return where HardFault is active and NMI is inactive always makes HardFault inactive and clears FAULTMASK



=========== ======= ====== =======
 Ret Codes    Value  Inv    ^e1
=========== ======= ====== =======
 0xFFFFFFFD      -3     2       7
 0xFFFFFFF9      -7     6       6
 0xFFFFFFF1     -15    14       4
 0xFFFFFFED     -19    18       3
 0xFFFFFFE9     -23    22       2
 0xFFFFFFE1     -31    30       0
=========== ======= ====== =======


=========== ============== ============= ============ ===== =====
 EXC_RETURN    Return to   Return stack   Frame type  Priv   Ctrl
=========== ============== ============= ============ ===== =====
 0xFFFFFFE1  Handler mode          Main     Extended          xxx
 0xFFFFFFE9   Thread mode          Main     Extended  Err     xxx
 0xFFFFFFED   Thread mode       Process     Extended  Usr     101
 0xFFFFFFF1  Handler mode          Main        Basic          xxx
 0xFFFFFFF9   Thread mode          Main        Basic  Krn     000
 0xFFFFFFFD   Thread mode       Process        Basic  Usr     001
=========== ============== ============= ============ ===== =====




The special-purpose CONTROL register
------------------------------------

The special-purpose CONTROL register is a 2-bit or 3-bit register defined as follows:

**nPRIV, bit[0]** Defines the execution privilege in Thread mode:
0 Thread mode has privileged access.
1 Thread mode has unprivileged access.

**SPSEL, bit[1]** Defines the stack to be used:
0 Use SP_main as the current stack.
1 In Thread mode, use SP_process as the current stack.
In Handler mode, this value is reserved.

**FPCA, bit[2], if the processor includes the FP extension**
Defines whether the FP extension is active in the current context:
0 FP extension not active.
1 FP extension is active.
If FPCCR.ASPEN is set to 1, enabling automatic FP state preservation, then the processor sets this bit to 1 on successful completion of any FP instruction.

A reset clears the CONTROL register to zero. Software can use the MRS instruction to read the register, and the MSR instruction to write to the register. The processor ignores unprivileged write accesses.

Software can update the SPSEL bit in Thread mode. In Handler mode, the processor ignores explicit writes to the SPSEL bit.

On an exception entry or exception return, the processor updates the SPSEL bit and, if it implements the FP extension, the FPCA bit. 

Software must use an ISB barrier instruction to ensure a write to the CONTROL register takes effect before the next instruction is executed.





Modes, privilege and stacks
---------------------------

Mode, privilege and stack pointer are key concepts used in ARMv7-M.

**Mode** An M-profile processor supports two operating modes:
- **Thread mode** Is entered on reset, and can be entered as a result of an exception return.
- **Handler mode** Is entered as a result of an exception. The processor must be in Handler mode to issue an exception return.

**Privilege**
Code can execute as privileged or unprivileged. Unprivileged execution limits or excludes access to some resources. Privileged execution has access to all resources.

Execution in Handler mode is always privileged. Execution in Thread mode can be privileged or unprivileged.

**Stack pointer**
The processor implements a banked pair of stack pointers, the Main stack pointer, and the Process
stack pointer. See The SP registers on page B1-572 for more information.
In Handler mode, the processor uses the Main stack pointer. In Thread mode it can use either stack

==== ========= ============= ====================
Mode Privilege Stack pointer Typical usage model
==== ========= ============= ====================
Handler Privileged Main Exception handling.
Thread Privileged Main Execution of a privileged process or thread using a common stack in a system
that only supports privileged access.
Process Execution of a privileged process or thread using a stack reserved for that
process or thread in a system that only supports privileged access, or that
supports a mix of privileged and unprivileged threads.
Thread Unprivileged Main Execution of an unprivileged process or thread using a common stack in a
system that supports privileged and unprivileged access.
Process Execution of an unprivileged process or thread using a stack reserved for that
process or thread in a system that supports privileged and unprivileged access.
==== ========= ============= ====================

