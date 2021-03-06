/*
 * Copyright (c) 2017-2018 Arm Limited
 *
 * Licensed under the Apache License Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * \file platform_retarget_dev.h
 * \brief The structure definitions in this file are exported based on the peripheral
 * definitions from device_cfg.h.
 * This retarget file is meant to be used as a helper for baremetal
 * applications and/or as an example of how to configure the generic
 * driver structures.
 */

#ifndef __MUSCA_B1_RETARGET_DEV_H__
#define __MUSCA_B1_RETARGET_DEV_H__

#include "device_cfg.h"

/* ======= Peripheral configuration structure declarations ======= */

/* ARM SCC driver structures */
#ifdef MUSCA_B1_SCC_S
#include "musca_b1_scc_drv.h"
extern struct musca_b1_scc_dev_t MUSCA_B1_SCC_DEV_S;
#endif

/* ARM MPC SIE 200 driver structures */
#ifdef MPC_ISRAM0_S
#include "mpc_sie200_drv.h"
extern struct mpc_sie200_dev_t MPC_ISRAM0_DEV_S;
#endif
#ifdef MPC_ISRAM1_S
#include "mpc_sie200_drv.h"
extern struct mpc_sie200_dev_t MPC_ISRAM1_DEV_S;
#endif
#ifdef MPC_ISRAM2_S
#include "mpc_sie200_drv.h"
extern struct mpc_sie200_dev_t MPC_ISRAM2_DEV_S;
#endif
#ifdef MPC_ISRAM3_S
#include "mpc_sie200_drv.h"
extern struct mpc_sie200_dev_t MPC_ISRAM3_DEV_S;
#endif
#ifdef MPC_CODE_SRAM_S
#include "mpc_sie200_drv.h"
extern struct mpc_sie200_dev_t MPC_CODE_SRAM_DEV_S;
#endif
#ifdef MPC_QSPI_S
#include "mpc_sie200_drv.h"
extern struct mpc_sie200_dev_t MPC_QSPI_DEV_S;
#endif

/* ARM UART PL011 driver structures */
#ifdef UART0_PL011_S
#include "uart_pl011_drv.h"
extern struct uart_pl011_dev_t UART0_PL011_DEV_S;
#endif
#ifdef UART0_PL011_NS
#include "uart_pl011_drv.h"
extern struct uart_pl011_dev_t UART0_PL011_DEV_NS;
#endif
#ifdef UART1_PL011_S
#include "uart_pl011_drv.h"
extern struct uart_pl011_dev_t UART1_PL011_DEV_S;
#endif
#ifdef UART1_PL011_NS
#include "uart_pl011_drv.h"
extern struct uart_pl011_dev_t UART1_PL011_DEV_NS;
#endif

/* CMSDK Timer driver structures */
#ifdef CMSDK_TIMER0_S
#include "timer_cmsdk_drv.h"
extern struct timer_cmsdk_dev_t CMSDK_TIMER0_DEV_S;
#endif
#ifdef CMSDK_TIMER0_NS
#include "timer_cmsdk_drv.h"
extern struct timer_cmsdk_dev_t CMSDK_TIMER0_DEV_NS;
#endif

#ifdef CMSDK_TIMER1_S
#include "timer_cmsdk_drv.h"
extern struct timer_cmsdk_dev_t CMSDK_TIMER1_DEV_S;
#endif
#ifdef CMSDK_TIMER1_NS
#include "timer_cmsdk_drv.h"
extern struct timer_cmsdk_dev_t CMSDK_TIMER1_DEV_NS;
#endif

/* ARM PPC driver structures */
#ifdef AHB_PPC0_S
#include "ppc_sse200_drv.h"
extern struct ppc_sse200_dev_t AHB_PPC0_DEV_S;
#endif

#ifdef AHB_PPCEXP0_S
#include "ppc_sse200_drv.h"
extern struct ppc_sse200_dev_t AHB_PPCEXP0_DEV_S;
#endif

#ifdef AHB_PPCEXP1_S
#include "ppc_sse200_drv.h"
extern struct ppc_sse200_dev_t AHB_PPCEXP1_DEV_S;
#endif

#ifdef AHB_PPCEXP2_S
#include "ppc_sse200_drv.h"
extern struct ppc_sse200_dev_t AHB_PPCEXP2_DEV_S;
#endif

#ifdef AHB_PPCEXP3_S
#include "ppc_sse200_drv.h"
extern struct ppc_sse200_dev_t AHB_PPCEXP3_DEV_S;
#endif

#ifdef APB_PPC0_S
#include "ppc_sse200_drv.h"
extern struct ppc_sse200_dev_t APB_PPC0_DEV_S;
#endif

#ifdef APB_PPC1_S
#include "ppc_sse200_drv.h"
extern struct ppc_sse200_dev_t APB_PPC1_DEV_S;
#endif

#ifdef APB_PPCEXP0_S
#include "ppc_sse200_drv.h"
extern struct ppc_sse200_dev_t APB_PPCEXP0_DEV_S;
#endif

#ifdef APB_PPCEXP1_S
#include "ppc_sse200_drv.h"
extern struct ppc_sse200_dev_t APB_PPCEXP1_DEV_S;
#endif

#ifdef APB_PPCEXP2_S
#include "ppc_sse200_drv.h"
extern struct ppc_sse200_dev_t APB_PPCEXP2_DEV_S;
#endif

#ifdef APB_PPCEXP3_S
#include "ppc_sse200_drv.h"
extern struct ppc_sse200_dev_t APB_PPCEXP3_DEV_S;
#endif

#endif  /* __MUSCA_B1_RETARGET_DEV_H__ */
