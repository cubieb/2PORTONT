/*
 * Copyright (c) 2012, Microsemi Corporation
 *
 * $Revision: 10393 $
 * $LastChangedDate: 2012-08-16 15:29:50 -0500 (Thu, 16 Aug 2012) $
 */

#include "vp_api_cfg.h"

#if defined (VP_CC_886_SERIES)
#if defined (VP886_INCLUDE_TESTLINE_CODE)

#include "vp886_testline_int.h"

/*******************************************************************************
 * Vp886TestVpMpiCmd
 *
 *  Sets up an internal API timer to generate a test complete.
 * 
 ******************************************************************************/
EXTERN VpStatusType
Vp886TestVpMpiCmd(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    const void *pArgsUntyped,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap,
    bool callback)
{

    const VpTestVpMpiCmdType *pMpiCmdArgs = (VpTestVpMpiCmdType *)pArgsUntyped;

    if (pMpiCmdArgs == NULL) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestVpMpiCmd NULL input struct"));
        return VP_STATUS_INVALID_ARG;
    }

    VpMemCpy(pLineObj->testResults.result.mpiCmd.buffer, pMpiCmdArgs->buffer,
        sizeof(pLineObj->testResults.result.mpiCmd.buffer));

    if (pMpiCmdArgs->cmd & 0x01) {
        /* read command */
        VpSlacRegRead(NULL, pLineCtx, pMpiCmdArgs->cmd, pMpiCmdArgs->cmdLen, \
            pLineObj->testResults.result.mpiCmd.buffer);
    } else {
        /* write command */
        VpSlacRegWrite(NULL, pLineCtx, pMpiCmdArgs->cmd, pMpiCmdArgs->cmdLen, \
            pLineObj->testResults.result.mpiCmd.buffer);
    }

    pLineObj->testResults.result.mpiCmd.cmd = pMpiCmdArgs->cmd;
    pLineObj->testResults.result.mpiCmd.cmdLen = pMpiCmdArgs->cmdLen;

    Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_SUCCESS);
    VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestVpMpiCmd(): complete"));


    return VP_STATUS_SUCCESS;
}

#endif /* VP886_INCLUDE_TESTLINE_CODE */
#endif /* VP_CC_886_SERIES */
