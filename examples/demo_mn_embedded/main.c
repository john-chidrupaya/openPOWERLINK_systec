/**
********************************************************************************
\file   main.c

\brief  Main file of console MN demo application

This file contains the main file of the openPOWERLINK MN console demo
application.

\ingroup module_demo_mn_embedded
*******************************************************************************/

/*------------------------------------------------------------------------------
Copyright (c) 2013, Bernecker+Rainer Industrie-Elektronik Ges.m.b.H. (B&R)
Copyright (c) 2013, SYSTEC electronic GmbH
Copyright (c) 2013, Kalycito Infotech Private Ltd.All rights reserved.
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
#include <Epl.h>
#include <EplTarget.h>

#include <gpio.h>
#include <lcd.h>

#include "app.h"
#include "event.h"

//============================================================================//
//            G L O B A L   D E F I N I T I O N S                             //
//============================================================================//

//------------------------------------------------------------------------------
// const defines
//------------------------------------------------------------------------------
#define CYCLE_LEN               -1
#define NODEID                  0xF0                //=> MN
#define IP_ADDR                 0xc0a86401          // 192.168.100.1
#define SUBNET_MASK             0xFFFFFF00          // 255.255.255.0
#define MAC_ADDR                0x00, 0x12, 0x34, 0x56, 0x78, NODEID
#define HOSTNAME                "openPOWERLINK Stack    "
#define CHECK_KERNEL_TIMEOUT    100000

//------------------------------------------------------------------------------
// module global vars
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// global function prototypes
//------------------------------------------------------------------------------

//============================================================================//
//            P R I V A T E   D E F I N I T I O N S                           //
//============================================================================//

//------------------------------------------------------------------------------
// const defines
//------------------------------------------------------------------------------
const unsigned char aCdcBuffer[] =
{
    #include "mnobd.txt"
};

//------------------------------------------------------------------------------
// local types
//------------------------------------------------------------------------------
typedef struct sInstace
{
    UINT8           aMacAddr[6];    ///< Mac address
    UINT8           nodeId;         ///< Node ID
    UINT32          cycleLen;       ///< Cycle length
    BOOL            fShutdown;      ///< User flag to shutdown the stack
    BOOL            fGsOff;         ///< NMT State GsOff reached
    unsigned char*  pCdcBuffer;     ///< Pointer to CDC buffer
    UINT            cdcBufferSize;  ///< Size of CDC buffer
} tInstance;

//------------------------------------------------------------------------------
// local vars
//------------------------------------------------------------------------------
static tInstance instance_l;

//------------------------------------------------------------------------------
// local function prototypes
//------------------------------------------------------------------------------
static tEplKernel initPowerlink(tInstance* pInstance_p);
static tEplKernel loopMain(tInstance* pInstance_p);
static void shutdownPowerlink(tInstance* pInstance_p);

//============================================================================//
//            P U B L I C   F U N C T I O N S                                 //
//============================================================================//

//------------------------------------------------------------------------------
/**
\brief  main function

This is the main function of the openPOWERLINK console MN demo application.

\return Returns an exit code

\ingroup module_demo_mn_embedded
*/
//------------------------------------------------------------------------------
int main(void)
{
    tEplKernel  ret = kEplSuccessful;
    const UINT8 aMacAddr[] = {MAC_ADDR};
    UINT8       nodeid;

    target_init();
    lcd_init();

    // get node ID from input
    nodeid = gpio_getNodeid();

    // initialize instance
    EPL_MEMSET(&instance_l, 0, sizeof(instance_l));

    instance_l.cycleLen         = CYCLE_LEN;
    instance_l.nodeId           = (nodeid != 0) ? nodeid : NODEID;
    instance_l.fShutdown        = FALSE;
    instance_l.fGsOff           = FALSE;
    instance_l.pCdcBuffer       = (unsigned char*)aCdcBuffer;
    instance_l.cdcBufferSize    = sizeof(aCdcBuffer);

    // set mac address (last byte is set to node ID)
    EPL_MEMCPY(instance_l.aMacAddr, aMacAddr, sizeof(aMacAddr));
    instance_l.aMacAddr[5]  = instance_l.nodeId;

    initEvents(&instance_l.fGsOff);

    PRINTF("----------------------------------------------------\n");
    PRINTF("openPOWERLINK console MN DEMO application\n");
    PRINTF("----------------------------------------------------\n");

    PRINTF("NODEID=0x%02X\n", instance_l.nodeId);
    lcd_printNodeId((WORD)instance_l.nodeId);

    if((ret = initPowerlink(&instance_l)) != kEplSuccessful)
        goto Exit;

    if((ret = initApp()) != kEplSuccessful)
        goto Exit;

    loopMain(&instance_l);

Exit:
    shutdownPowerlink(&instance_l);
    shutdownApp();
    target_cleanup();

    return 0;
}

//============================================================================//
//            P R I V A T E   F U N C T I O N S                               //
//============================================================================//
/// \name Private Functions
/// \{

//------------------------------------------------------------------------------
/**
\brief  Initialize the openPOWERLINK stack

The function initializes the openPOWERLINK stack.

\param  pInstance_p             Pointer to demo instance

\return The function returns a tEplKernel error code.
*/
//------------------------------------------------------------------------------
static tEplKernel initPowerlink(tInstance* pInstance_p)
{
    tEplKernel                  ret = kEplSuccessful;
    static tEplApiInitParam     initParam;
    char*                       sHostname = HOSTNAME;

    PRINTF ("Initializing openPOWERLINK stack...\n");

    EPL_MEMSET(&initParam, 0, sizeof (initParam));
    initParam.m_uiSizeOfStruct = sizeof (initParam);

    initParam.m_uiNodeId = pInstance_p->nodeId;
    initParam.m_dwIpAddress = (0xFFFFFF00 & IP_ADDR) | initParam.m_uiNodeId;

    EPL_MEMCPY(initParam.m_abMacAddress, pInstance_p->aMacAddr, sizeof (initParam.m_abMacAddress));

    initParam.m_fAsyncOnly = FALSE;

    initParam.m_dwFeatureFlags            = -1;
    initParam.m_dwCycleLen                = pInstance_p->cycleLen;  // required for error detection
    initParam.m_uiIsochrTxMaxPayload      = 256;                    // const
    initParam.m_uiIsochrRxMaxPayload      = 256;                    // const
    initParam.m_dwPresMaxLatency          = 50000;                  // const; only required for IdentRes
    initParam.m_uiPreqActPayloadLimit     = 36;                     // required for initialisation (+28 bytes)
    initParam.m_uiPresActPayloadLimit     = 36;                     // required for initialisation of Pres frame (+28 bytes)
    initParam.m_dwAsndMaxLatency          = 150000;                 // const; only required for IdentRes
    initParam.m_uiMultiplCycleCnt         = 0;                      // required for error detection
    initParam.m_uiAsyncMtu                = 1500;                   // required to set up max frame size
    initParam.m_uiPrescaler               = 2;                      // required for sync
    initParam.m_dwLossOfFrameTolerance    = 500000;
    initParam.m_dwAsyncSlotTimeout        = 3000000;
    initParam.m_dwWaitSocPreq             = -1;
    initParam.m_dwDeviceType              = -1;                     // NMT_DeviceType_U32
    initParam.m_dwVendorId                = -1;                     // NMT_IdentityObject_REC.VendorId_U32
    initParam.m_dwProductCode             = -1;                     // NMT_IdentityObject_REC.ProductCode_U32
    initParam.m_dwRevisionNumber          = -1;                     // NMT_IdentityObject_REC.RevisionNo_U32
    initParam.m_dwSerialNumber            = -1;                     // NMT_IdentityObject_REC.SerialNo_U32

    initParam.m_dwSubnetMask              = SUBNET_MASK;
    initParam.m_dwDefaultGateway          = 0;
    EPL_MEMCPY(initParam.m_sHostname, sHostname, sizeof(initParam.m_sHostname));
    initParam.m_uiSyncNodeId              = EPL_C_ADR_SYNC_ON_SOC;
    initParam.m_fSyncOnPrcNode            = FALSE;

    // set callback functions
    initParam.m_pfnCbEvent =    processEvents;
    initParam.m_pfnCbSync  =    processSync;

    // initialize POWERLINK stack
    ret = oplk_init(&initParam);
    if(ret != kEplSuccessful)
    {
        PRINTF("oplk_init() failed (Error:0x%x!)\n", ret);
        return ret;
    }

    ret = oplk_setCdcBuffer(pInstance_p->pCdcBuffer, pInstance_p->cdcBufferSize);
    if(ret != kEplSuccessful)
    {
        PRINTF("oplk_setCdcBuffer() failed (Error:0x%x!)\n", ret);
        return ret;
    }

    return kEplSuccessful;
}

//------------------------------------------------------------------------------
/**
\brief  Main loop of demo application

This function implements the main loop of the demo application.
- It sends a NMT command to start the stack

\param  pInstance_p             Pointer to demo instance

\return The function returns a tEplKernel error code.
*/
//------------------------------------------------------------------------------
static tEplKernel loopMain(tInstance* pInstance_p)
{
    tEplKernel  ret = kEplSuccessful;
    UINT        checkStack = 0;

    // start processing
    if((ret = oplk_execNmtCommand(kNmtEventSwReset)) != kEplSuccessful)
        return ret;

    while(1)
    {
        // do background tasks
        if((ret = oplk_process()) != kEplSuccessful)
            break;

        // check the kernel part from time to time
        if(checkStack++ >= CHECK_KERNEL_TIMEOUT)
        {
            checkStack = 0;
            if(oplk_checkKernelStack() == FALSE)
            {
                PRINTF("Kernel stack has gone! Exiting...\n");
                instance_l.fShutdown = TRUE;
            }
        }

        // trigger switch off
        if(pInstance_p->fShutdown != FALSE)
        {
            oplk_execNmtCommand(kNmtEventSwitchOff);

            // reset shutdown flag to generate only one switch off command
            pInstance_p->fShutdown = FALSE;
        }

        // exit loop if NMT is in off state
        if(pInstance_p->fGsOff != FALSE)
            break;
    }

    return ret;
}

//------------------------------------------------------------------------------
/**
\brief  Shutdown the demo application

The function shut's down the demo application.

\param  pInstance_p             Pointer to demo instance
*/
//------------------------------------------------------------------------------
static void shutdownPowerlink(tInstance* pInstance_p)
{
    UNUSED_PARAMETER(pInstance_p);

    PRINTF("Shut down DEMO\n");

    oplk_shutdown();
}

///\}
