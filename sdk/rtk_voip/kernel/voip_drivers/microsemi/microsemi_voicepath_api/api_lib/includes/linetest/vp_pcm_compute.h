/** file vp_pcm_compute.h
 * vp_pcm_compute.h
 *
 * Provides declarations for pcm compute algorithms
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 9115 $
 * $LastChangedDate: 2011-11-15 15:25:17 -0600 (Tue, 15 Nov 2011) $
 */

#ifndef VP_PCM_COMPUTE_H
#define VP_PCM_COMPUTE_H

#include "vp_api_types.h"
#include "vp_api_common.h"

#define VP_PCM_COMPUTE_HYST 512          /* freq hysteresis */
#define VP_PCM_COMPUTE_MAX_ACCUMULATE 0x3FFFFFFF   /* max accumulate sum*/
#define MAX_INVALID_PCM_MEASUREMENT 4   /* max faillures to read the PCM buffer */

/*
 * if the data is collected as 8 bit samples this tells the
 * RunningPcmCalculate() how to combine the samples to form 16 bit
 * linear samples
 */
#undef VP_PCM_COMPUTE_LITTLE_ENDIAN

typedef enum VpPcmComputeStatus {
    PCM_CAL_RUNNING,
    PCM_CAL_DONE,
    PCM_CAL_ERROR_INVALID_ARG,
    PCM_CAL_ERROR_CODES
} VpPcmComputeStatus;

typedef struct VpPcmComputeTempType {
    VpPcmOperationMaskType operations;
    uint8 zeroInARow;       /** < Number of bufferlength = 0 in the row */
    uint16 downSampleRate;
    uint16 totalOpSamples;  /** < Number of requested pcm samples at downsample rate*/
    uint16 totalSkipSamples;/** < Number of samples to skip */
    uint16 curSampleNum;    /** < Current sample number */
    int16 max;              /** < Running max pcm val during range calc*/
    int16 min;              /** < Running min pcm val during range calc*/
    int32 sum;              /** < Running pcm val total during avg calc*/
    int32 sumOfSqrs;        /** < Running sum of squares total */
    int32 shift;            /** < Running sum of squares shift operator*/
    int32 freqSampTotal;    /** < Total number of freq samples (not opSamples)*/
    int32 freqSampCount;    /** < Current Freq cycle sample count*/
    int32 freqSampCycles;   /** < Running number of freq cycles found */
    bool  newFreqCycle;     /** < New freq cycle is starting */
    bool  firstFreqCycle;   /** < First freq cycle indication*/
    bool  b16bitSampBuf;     /** < True indicates that void *pSampleBuffer is
                             *    an array of int16.
                             *    False indicates that void *pSampleBuffer is
                             *    an array of chars */
} VpPcmComputeTempType;

EXTERN VpPcmComputeStatus
VpPcmComputeReset(
    VpPcmOperationMaskType operations,
    uint16 integrateTime,
    uint16 settlingTime,
    uint16 downSampleRate,
    bool b16bitSampBuf,
    VpPcmComputeTempType *pTempPcmData,
    VpPcmOperationResultsType *pResults
);

EXTERN VpPcmComputeStatus
VpPcmCompute(
    void *pSampleBuffer,
    uint16 bufferLength,
    VpPcmComputeTempType *pTempPcmData,
    VpPcmOperationResultsType *pResults);

#endif



