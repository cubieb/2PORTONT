/** \file vp_api_cslac_seq.h
 * vp_api_cslac_seq.h
 *
 * Header file for the API-II c files.
 *
 * This file contains the all of the VoicePath API-II function prototypes
 * required to run the CSLAC sequencer.
 *
 * $Revision: 9115 $
 * $LastChangedDate: 2011-11-15 15:25:17 -0600 (Tue, 15 Nov 2011) $
 */

#ifndef VP_API_CSLAC_SEQ_H
#define VP_API_CSLAC_SEQ_H

#include "vp_CSLAC_types.h"

#ifdef VP_CSLAC_SEQ_EN

/*
 * Converting from bool to enumeration. So make sure there exists a string that
 * maps directly to previous "FALSE" value. Other strings can take on any valid
 * enumeration value.
 */
typedef enum VpCliEncodedDataType {
    VP_CLI_ENCODE_END = 0,
    VP_CLI_ENCODE_DATA = 1,
    VP_CLI_ENCODE_MARKOUT = 2
} VpCliEncodedDataType;

VpStatusType
VpSeq(
    VpLineCtxType *pLineCtx,
    VpProfilePtrType pProfile);

VpStatusType
VpBranchInstruction(
    VpLineCtxType *pLineCtx,
    VpProfilePtrType pSeqData);

VpStatusType
VpTimeInstruction(
    VpLineCtxType *pLineCtx,
    VpProfilePtrType pSeqData);

#if (defined (VP_CC_890_SERIES) && defined (VP890_FXS_SUPPORT)) || \
    (defined (VP_CC_880_SERIES) && defined (VP880_FXS_SUPPORT))
bool
VpCSLACHowlerInit(
    VpSeqDataType *cadence,
    uint16 tickRate);

uint16
VpDecimalMultiply(
    uint16 value,
    uint16 byteMask);

bool
VpCSLACProcessRampGenerators(
    VpSeqDataType *cadence);
#endif

bool
VpServiceSeq(
    VpDevCtxType *pDevCtx);

void
VpCSLACInitCidStruct(
    VpCallerIdType *pCidStruct,
    uint8 sequenceData);

VpStatusType
VpCidSeq(
    VpLineCtxType *pLineCtx);

void
VpCliStopCli(
    VpLineCtxType *pLineCtx);

VpCliEncodedDataType
VpCSLACCliGetEncodedByte(
    uint8 *pByte,
    VpCallerIdType *pCidStruct,
    uint16 *pProcessData,
    VpOptionEventMaskType *pLineEvents,
    uint8 checkSumIndex);

VpStatusType
VpCSLACInitMeter(
    VpLineCtxType *pLineCtx,
    VpProfilePtrType pMeterProfile);

VpStatusType
VpCSLACStartMeter(
    VpLineCtxType *pLineCtx,
    uint16 onTime,
    uint16 offTime,
    uint16 numMeters);

#if defined (VP_CC_790_SERIES)
VpStatusType
Vp790CommandInstruction(
    VpLineCtxType *pLineCtx,
    VpProfilePtrType pSeqData);
#endif

#if defined (VP_CC_880_SERIES)
VpStatusType
Vp880CommandInstruction(
    VpLineCtxType *pLineCtx,
    VpProfilePtrType pSeqData);
#endif

#if defined (VP_CC_580_SERIES)
VpStatusType
Vp580CommandInstruction(
    VpLineCtxType *pLineCtx,
    VpProfilePtrType pSeqData);
#endif

#endif

#endif


