/**
********************************************************************************
\file   veth-linuxkernel.c

\brief  Implementation of virtual Ethernet for Linux

This file contains the the virtual Ethernet driver for the Linux kernel
implementation.

\ingroup module_veth
*******************************************************************************/

/*------------------------------------------------------------------------------
Copyright (c) 2013, SYSTEC electronic GmbH
Copyright (c) 2013, Bernecker+Rainer Industrie-Elektronik Ges.m.b.H. (B&R)
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
#include <linux/version.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/if_arp.h>
#include <net/arp.h>

#include <net/protocol.h>
#include <net/pkt_sched.h>
#include <linux/if_ether.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/mm.h>
#include <linux/types.h>
#include <linux/skbuff.h>  /* for struct sk_buff */

#include <kernel/veth.h>
#include <kernel/dllkcal.h>
#include <kernel/dllk.h>


//============================================================================//
//            G L O B A L   D E F I N I T I O N S                             //
//============================================================================//

//------------------------------------------------------------------------------
// const defines
//------------------------------------------------------------------------------
#ifndef VETH_TX_TIMEOUT
//#define VETH_TX_TIMEOUT (2*HZ)
#define VETH_TX_TIMEOUT 0       // d.k.: we use no timeout
#endif

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


//------------------------------------------------------------------------------
// local types
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// local vars
//------------------------------------------------------------------------------
static struct net_device * pVEthNetDevice_g = NULL;

//------------------------------------------------------------------------------
// local function prototypes
//------------------------------------------------------------------------------
static int veth_open(struct net_device *pNetDevice_p);
static int veth_close(struct net_device *pNetDevice_p);
static int veth_xmit(struct sk_buff *pSkb_p, struct net_device *pNetDevice_p);
static struct net_device_stats* veth_getStats(struct net_device *pNetDevice_p);
static void veth_timeout(struct net_device *pNetDevice_p);
static tEplKernel veth_receiveFrame(tFrameInfo * pFrameInfo_p);

//------------------------------------------------------------------------------
// local vars
//------------------------------------------------------------------------------
static const struct net_device_ops epl_netdev_ops =
{
    .ndo_open               = veth_open,
    .ndo_stop               = veth_close,
    .ndo_get_stats          = veth_getStats,
    .ndo_start_xmit         = veth_xmit,
    .ndo_tx_timeout         = veth_timeout,
    .ndo_change_mtu         = eth_change_mtu,
    .ndo_set_mac_address    = eth_mac_addr,
    .ndo_validate_addr      = eth_validate_addr,
};

//============================================================================//
//            P U B L I C   F U N C T I O N S                                 //
//============================================================================//

//------------------------------------------------------------------------------
/**
\brief  Add virtual Ethernet instance

The function adds a virtual Ethernet instance.

\param  aSrcMac_p       MAC address to set for virtual Ethernet interface.

\return The function returns a tEplKernel error code.

\ingroup module_veth
*/
//------------------------------------------------------------------------------
tEplKernel veth_addInstance(const UINT8 aSrcMac_p[6])
{
    // allocate net device structure with priv pointing to stats structure
    pVEthNetDevice_g = alloc_netdev(sizeof (struct net_device_stats), EPL_VETH_NAME,
                                    ether_setup);

    if (pVEthNetDevice_g == NULL)
        return kEplNoResource;

    pVEthNetDevice_g->netdev_ops        = &epl_netdev_ops;
    pVEthNetDevice_g->watchdog_timeo    = VETH_TX_TIMEOUT;
    pVEthNetDevice_g->destructor        = free_netdev;

    // copy own MAC address to net device structure
    memcpy(pVEthNetDevice_g->dev_addr, aSrcMac_p, 6);

    //register VEth to the network subsystem
    if (register_netdev(pVEthNetDevice_g))
        EPL_DBGLVL_VETH_TRACE("veth_addInstance: Could not register VEth...\n");
    else
        EPL_DBGLVL_VETH_TRACE("veth_addInstance: Register VEth successfull...\n");

    return kEplSuccessful;
}

//------------------------------------------------------------------------------
/**
\brief  Delete virtual Ethernet instance

The function deletes a virtual ethernet instance.

\return The function returns a tEplKernel error code.

\ingroup module_veth
*/
//------------------------------------------------------------------------------
tEplKernel veth_delInstance(void)
{
    if (pVEthNetDevice_g != NULL)
    {
        //unregister VEth from the network subsystem
        unregister_netdev(pVEthNetDevice_g);
        // destructor was set to free_netdev,
        // so we do not need to call free_netdev here
        pVEthNetDevice_g = NULL;
    }

    return kEplSuccessful;
}

//============================================================================//
//            P R I V A T E   F U N C T I O N S                               //
//============================================================================//
/// \name Private Functions
/// \{

//------------------------------------------------------------------------------
/**
\brief  Open entry point of virtual Ethernet driver

The function contains the open routine of the virtual Ethernet driver.

\param  pNetDevice_p        Pointer to net device structure.

\return The function returns an error code.
*/
//------------------------------------------------------------------------------
static int veth_open(struct net_device *pNetDevice_p)
{
    tEplKernel  ret = kEplSuccessful;

    //open the device
    //start the interface queue for the network subsystem
    netif_start_queue(pNetDevice_p);

    // register callback function in DLL
    ret = dllk_regAsyncHandler(veth_receiveFrame);

    EPL_DBGLVL_VETH_TRACE("veth_open: EplDllkRegAsyncHandler returned 0x%02X\n", ret);
    return 0;
}

//------------------------------------------------------------------------------
/**
\brief  Close entry point of virtual Ethernet driver

The function contains the close routine of the virtual Ethernet driver.

\param  pNetDevice_p        Pointer to net device structure.

\return The function returns an error code.
*/
//------------------------------------------------------------------------------
static int veth_close(struct net_device *pNetDevice_p)
{
    EPL_DBGLVL_VETH_TRACE("VEthClose\n");

    dllk_deregAsyncHandler(veth_receiveFrame);
    netif_stop_queue(pNetDevice_p);     //stop the interface queue for the network subsystem
    return 0;
}

//------------------------------------------------------------------------------
/**
\brief  Transmit entry point of virtual Ethernet driver

The function contains the transmit function for the virtual Ethernet driver.

\param  pSkb_p          Pointer to packet which should be transmitted.
\param  pNetDevice_p    Pointer to net device structure of interface.

\return The function returns an error code.
*/
//------------------------------------------------------------------------------
static int veth_xmit(struct sk_buff *pSkb_p, struct net_device *pNetDevice_p)
{
    tEplKernel      ret = kEplSuccessful;
    tFrameInfo      frameInfo;

    //transmit function
    struct net_device_stats* pStats = netdev_priv(pNetDevice_p);

    //save timestemp
    pNetDevice_p->trans_start = jiffies;

    frameInfo.pFrame = (tEplFrame *)pSkb_p->data;
    frameInfo.frameSize = pSkb_p->len;

    //call send fkt on DLL
    ret = dllkcal_sendAsyncFrame(&frameInfo, kDllAsyncReqPrioGeneric);
    if (ret != kEplSuccessful)
    {
        EPL_DBGLVL_VETH_TRACE("veth_xmit: dllkcal_sendAsyncFrame returned 0x%02X\n", ret);
        netif_stop_queue(pNetDevice_p);
        goto Exit;
    }
    else
    {
        EPL_DBGLVL_VETH_TRACE("veth_xmit: frame passed to DLL\n");
        dev_kfree_skb(pSkb_p);

        //set stats for the device
        pStats->tx_packets++;
        pStats->tx_bytes += frameInfo.frameSize;
    }

Exit:
    return 0;
}

//------------------------------------------------------------------------------
/**
\brief  Get statistics

The function gets the statistics of the interface.

\param  pNetDevice_p        Pointer to net device structure of interface.

\return The function returns a pointer to a net_device_stats structure.
*/
//------------------------------------------------------------------------------
static struct net_device_stats* veth_getStats(struct net_device *pNetDevice_p)
{
    EPL_DBGLVL_VETH_TRACE("veth_getStats\n");
    return netdev_priv(pNetDevice_p);
}

//------------------------------------------------------------------------------
/**
\brief  TX timeout entry point

The function provides the TX timeout entry point of the driver.

\param  pNetDevice_p        Pointer to net device structure of interface.
*/
//------------------------------------------------------------------------------
static void veth_timeout(struct net_device *pNetDevice_p)
{
    EPL_DBGLVL_VETH_TRACE("veth_timeout(\n");
    // $$$ d.k.: move to extra function, which is called by DLL when new space is available in TxFifo
    if (netif_queue_stopped (pNetDevice_p))
    {
        netif_wake_queue (pNetDevice_p);
    }
}

//------------------------------------------------------------------------------
/**
\brief  Receive frame from virtual Ethernet interface

The function receives a frame from the virtual Ethernet interface.

\param  pFrameInfo_p        Pointer to frame information of received frame.

\return The function returns a tEplKernel error code.
*/
//------------------------------------------------------------------------------
static tEplKernel veth_receiveFrame(tFrameInfo * pFrameInfo_p)
{
    tEplKernel  ret = kEplSuccessful;
    struct net_device* pNetDevice = pVEthNetDevice_g;
    struct net_device_stats* pStats = netdev_priv(pNetDevice);
    struct sk_buff *pSkb;

    EPL_DBGLVL_VETH_TRACE("veth_receiveFrame: FrameSize=%u\n", pFrameInfo_p->frameSize);

    if ((pSkb = dev_alloc_skb(pFrameInfo_p->frameSize + 2)) == NULL)
    {
        pStats->rx_dropped++;
        goto Exit;
    }
    pSkb->dev = pNetDevice;

    skb_reserve(pSkb, 2);

    memcpy((void *)skb_put(pSkb, pFrameInfo_p->frameSize), pFrameInfo_p->pFrame, pFrameInfo_p->frameSize);

    pSkb->protocol = eth_type_trans(pSkb, pNetDevice);
    pSkb->ip_summed = CHECKSUM_UNNECESSARY;

    netif_rx(pSkb);         // call netif_rx with skb

    EPL_DBGLVL_VETH_TRACE("veth_receiveFrame: SrcMAC=0x%llx\n", AmiGetQword48FromBe(pFrameInfo_p->pFrame->m_be_abSrcMac));

    // update receive statistics
    pStats->rx_packets++;
    pStats->rx_bytes += pFrameInfo_p->frameSize;

Exit:
    return ret;
}

///\}



