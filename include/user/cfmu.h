/**
********************************************************************************
\file   cfmu.h

\brief  Include file for configuration file manager (CFM) module

This file contains the definitions of the CFM module.
*******************************************************************************/

/*------------------------------------------------------------------------------
Copyright (c) 2013, Bernecker+Rainer Industrie-Elektronik Ges.m.b.H. (B&R)
Copyright (c) 2013, SYSTEC electronic GmbH
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holders nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
------------------------------------------------------------------------------*/

#ifndef _INC_EplCfmu_H_
#define _INC_EplCfmu_H_

//------------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------------
#include <nmt.h>
#include <cfm.h>

//------------------------------------------------------------------------------
// const defines
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// typedef
//------------------------------------------------------------------------------
typedef tEplKernel (*tCfmCbEventCnProgress) (tCfmEventCnProgress* pEventCnProgress_p);
typedef tEplKernel (*tCfmCbEventCnResult) (UINT nodeId_p, tNmtNodeCommand nodeCommand_p);

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

tEplKernel  cfmu_init(tCfmCbEventCnProgress pfnCbEventCnProgress_p, tCfmCbEventCnResult pfnCbEventCnResult_p);
tEplKernel  cfmu_exit(void);
tEplKernel  cfmu_processNodeEvent(UINT nodeId_p, tNmtNodeEvent nodeEvent_p);
BOOL        cfmu_isSdoRunning(UINT nodeId_p);

#ifdef __cplusplus
}
#endif

#endif /* _INC_EplCfmu_H_ */


