/**
********************************************************************************
\file   ctrlkcal-linuxkernel.c

\brief  Kernel control CAL module for Linux kernel

This file contains an implementation of the kernel control CAL module for
the Linux kernel implementation.

\ingroup module_ctrlkcal
*******************************************************************************/

/*------------------------------------------------------------------------------
Copyright (c) 2012, Bernecker+Rainer Industrie-Elektronik Ges.m.b.H. (B&R)
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

//------------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------------
#include <stddef.h>

#include <ctrl.h>
#include <ctrlcal.h>

//============================================================================//
//            G L O B A L   D E F I N I T I O N S                             //
//============================================================================//

//------------------------------------------------------------------------------
// const defines
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// module global vars
//------------------------------------------------------------------------------
tCtrlInitParam      kernelInitParam_g;
UINT16              status_g;

//------------------------------------------------------------------------------
// global function prototypes
//------------------------------------------------------------------------------

//============================================================================//
//            P R I V A T E   D E F I N I T I O N S                           //
//============================================================================//

//------------------------------------------------------------------------------
// const defines
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// local types
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// local vars
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// local function prototypes
//------------------------------------------------------------------------------

//============================================================================//
//            P U B L I C   F U N C T I O N S                                 //
//============================================================================//

//------------------------------------------------------------------------------
/**
\brief  Initialize kernel control CAL module

The function initializes the kernel control CAL module. For the direct call
implementation nothing has to be done!

\return The function returns a tEplKernel error code.

\ingroup module_ctrlkcal
*/
//------------------------------------------------------------------------------
tEplKernel ctrlkcal_init (void)
{
    status_g = kCtrlStatusReady;
    return kEplSuccessful;
}

//------------------------------------------------------------------------------
/**
\brief  Cleanup kernel control CAL module

The function cleans up the kernel control CAL module. It resets the control
memory block and cleans up the underlaying CAL module used for implementing
the memory block access functions.

\ingroup module_ctrlkcal
*/
//------------------------------------------------------------------------------
void ctrlkcal_exit (void)
{
    status_g = kCtrlStatusUnavailable;
}

//------------------------------------------------------------------------------
/**
\brief  Process kernel control CAL module

This function provides processing time for the CAL module.

\return The function returns a tEplKernel error code.

\ingroup module_ctrlkcal
*/
//------------------------------------------------------------------------------
tEplKernel ctrlkcal_process (void)
{
    return kEplSuccessful;
}

//------------------------------------------------------------------------------
/**
\brief  Get control command

The function reads a control command stored by the user in the control memory
block to execute a kernel control function.

\note The function is only implemented to provide the interface but is not used,
      because in the direct implemtation the kernel control command could
      directly be called by the user control module.

\param  pCmd_p            The command to be executed.

\return The function returns a tEplKernel error code.

\ingroup module_ctrlkcal
*/
//------------------------------------------------------------------------------
tEplKernel ctrlkcal_getCmd (tCtrlCmdType *pCmd_p)
{
    UNUSED_PARAMETER(pCmd_p);
    return kEplSuccessful;
}

//------------------------------------------------------------------------------
/**
\brief  Send a return value

The function sends the return value of an executed command to the user stack
by storing it in the control memory block.

\note The function is only implemented to provide the interface but is not used,
      because in the direct implemtation the kernel control command could
      directly be called by the user control module.

\param  retval_p            Return value to send.

\ingroup module_ctrlkcal
*/
//------------------------------------------------------------------------------
void ctrlkcal_sendReturn(UINT16 retval_p)
{
    UNUSED_PARAMETER(retval_p);
}

//------------------------------------------------------------------------------
/**
\brief  Set the kernel stack status

The function stores the status of the kernel stack in the control memory block.

\param  status_p                Status to set.

\ingroup module_ctrlkcal
*/
//------------------------------------------------------------------------------
void ctrlkcal_setStatus (UINT16 status_p)
{
    status_g = status_p;
}

//------------------------------------------------------------------------------
/**
\brief  Get the kernel stack status

The function gets the status of the kernel stack.

\return The function returns the status of the kernel stack.

\ingroup module_ctrlkcal
*/
//------------------------------------------------------------------------------
UINT16 ctrlkcal_getStatus (void)
{
    return status_g;
}

//------------------------------------------------------------------------------
/**
\brief  Update the heartbeat counter

The function updates it's heartbeat counter in the control memory block which
can be used by the user stack to detect if the kernel stack is still running.

\param  heartbeat_p         Heartbeat counter to store in the control memory
                            block.
\ingroup module_ctrlkcal
*/
//------------------------------------------------------------------------------
void ctrlkcal_updateHeartbeat (UINT16 heartbeat_p)
{
    // nothing to do
}

//------------------------------------------------------------------------------
/**
\brief  Store the init parameters for user kernel

The function stores the openPOWERLINK initialization parameter so that they
can be accessed by the user stack. It is used to notify the user stack about
parameters modified in the kernel stack.

\param  pInitParam_p        Specifies where to read the init parameters.

\ingroup module_ctrlkcal
*/
//------------------------------------------------------------------------------
void ctrlkcal_storeInitParam(tCtrlInitParam* pInitParam_p)
{
    EPL_MEMCPY(&kernelInitParam_g, pInitParam_p, sizeof(tCtrlInitParam));
}

//------------------------------------------------------------------------------
/**
\brief  Read the init parameters from user stack

The function reads the initialization parameter from the user stack.

\param  pInitParam_p        Specifies where to store the read init parameters.

\return The function returns a tEplKernel error code. It returns always
        kEplSuccessful!

\ingroup module_ctrlkcal
*/
//------------------------------------------------------------------------------
tEplKernel ctrlkcal_readInitParam(tCtrlInitParam* pInitParam_p)
{
    EPL_MEMCPY(pInitParam_p, &kernelInitParam_g, sizeof(tCtrlInitParam));
    return kEplSuccessful;
}

