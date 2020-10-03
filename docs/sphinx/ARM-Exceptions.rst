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




