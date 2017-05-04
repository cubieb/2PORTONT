/** \file vp_pcm_compute.c
 * vp_pcm_compute.c
 *
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 7116 $
 * $LastChangedDate: 2010-05-11 10:42:12 -0500 (Tue, 11 May 2010) $
 */

#include "vp_api_cfg.h"

#if (defined(VP_CC_890_SERIES) && defined(VP890_INCLUDE_TESTLINE_CODE)) \
    || (defined(VP_CC_880_SERIES) && defined(VP880_INCLUDE_TESTLINE_CODE))

#include "vp_api_types.h"
#include "vp_api_common.h"
#include "vp_api_int.h"

#include "vp_pcm_compute.h"

#define EZ_LOG_DEBUG "/var/ezpcmsamples" /* used to log the pcm samples to a file on the system */
#undef EZ_LOG_DEBUG

#ifdef EZ_LOG_DEBUG
  #include <stdio.h>
#endif

static void
VpPcmComputeConclude(
    VpPcmComputeTempType *pTempPcmData,
    VpPcmOperationResultsType *pResult);


EXTERN VpPcmComputeStatus
VpPcmComputeReset(
    VpPcmOperationMaskType operations,
    uint16 integrateTime,
    uint16 settlingTime,
    uint16 downSampleRate,
    bool b16bitSampBuf,
    VpPcmComputeTempType *pTempPcmData,
    VpPcmOperationResultsType *pResults)
{
    uint16 count = sizeof(VpPcmOperationResultsType);
    char *xs = (char *)pResults;

    /* make sure non-null pointers are passed in */
    if ((NULL == pTempPcmData) || (VP_NULL == pResults)) {
        return PCM_CAL_ERROR_INVALID_ARG;
    }
    /* initialize all results struct membvers to 0 */
/*    VpMemSet(pResults, 0, sizeof(VpPcmOperationResultsType)); */
    while (count--) {
        *xs++ = 0;
    }

    /* initialize the temp structure */
    pTempPcmData->zeroInARow = 0;
    pTempPcmData->downSampleRate = downSampleRate;
    pTempPcmData->operations = operations;
    pTempPcmData->curSampleNum = 0;
    pTempPcmData->max = VP_INT16_MIN;
    pTempPcmData->min = VP_INT16_MAX;
    pTempPcmData->sum = 0;
    pTempPcmData->sumOfSqrs = 0;
    pTempPcmData->shift = 0;
    pTempPcmData->freqSampTotal = 0;
    pTempPcmData->freqSampCount = 0;
    pTempPcmData->freqSampCycles = 0;
    pTempPcmData->newFreqCycle = FALSE;
    pTempPcmData->firstFreqCycle = TRUE;
    pTempPcmData->b16bitSampBuf =  b16bitSampBuf;

    /* collect at least one point */
    pTempPcmData->totalOpSamples = integrateTime/downSampleRate;
    pTempPcmData->totalSkipSamples = settlingTime/downSampleRate;
    if (0 == pTempPcmData->totalOpSamples) {
        pTempPcmData->totalOpSamples = 1;
    }

    return PCM_CAL_RUNNING;
}

EXTERN VpPcmComputeStatus
VpPcmCompute(
    void *pSampleBuffer,
    uint16 bufferLength,
    VpPcmComputeTempType *pTempPcmData,
    VpPcmOperationResultsType *pResults)
{
    uint16 i = 0;
    int16 pcmSample;

#ifdef EZ_LOG_DEBUG
FILE *pFile = fopen(EZ_LOG_DEBUG, "a");
#endif

    /* If the bufferLength is 0 to much time in the row -> ERROR and bail out */
    if (bufferLength == 0) {
        (pTempPcmData->zeroInARow)++;
        if (pTempPcmData->zeroInARow >= MAX_INVALID_PCM_MEASUREMENT) {
            pResults->error = TRUE;
            goto pcm_cal_done;
        }
    }

    for (i = 0; i < bufferLength; i++) {

        pTempPcmData->zeroInARow = 0;   /* A non empty buffer reset the counter */

        if (TRUE == pTempPcmData->b16bitSampBuf) {
            pcmSample = ((int16 *)pSampleBuffer)[i];
        } else {
            #ifdef VP_PCM_COMPUTE_LITTLE_ENDIAN
            pcmSample = (((char *)pSampleBuffer)[i+1] << 8) | ((char *)pSampleBuffer)[i];
            #else
            pcmSample = (((char *)pSampleBuffer)[i] << 8) | ((char *)pSampleBuffer)[i+1];
            #endif
            i++;
        }

        /* don't perform any operations until all skip samples have been handled */
        if (0 < pTempPcmData->totalSkipSamples) {
            pTempPcmData->totalSkipSamples--;
            continue;
        }

        /* update the current sample number count */
        pTempPcmData->curSampleNum++;

        #ifdef EZ_LOG_DEBUG
        fprintf(pFile, "%i ", pcmSample);
        fflush(NULL);
        #endif

        /* If APP_SPECIFIC operation is requested no calculations are done!!*/
        if (pTempPcmData->operations & VP_PCM_OPERATION_APP_SPECIFIC) {
            if (pTempPcmData->curSampleNum >= pTempPcmData->totalOpSamples) {
                goto pcm_cal_done;
            } else {
                continue;
            }
        }

        /* add to the running sum */
        pTempPcmData->sum += (int32)pcmSample;

        /* correct min and max */
        if (pcmSample > pTempPcmData->max) {
            pTempPcmData->max = pcmSample;
        }
        if (pcmSample < pTempPcmData->min) {
            pTempPcmData->min = pcmSample;
        }

        /* freq measurment */
        if (pTempPcmData->operations & VP_PCM_OPERATION_FREQ) {
            if ((pcmSample > VP_PCM_COMPUTE_HYST) &&
                pTempPcmData->newFreqCycle) {

                if (FALSE == pTempPcmData->firstFreqCycle) {
                    pTempPcmData->freqSampTotal += pTempPcmData->freqSampCount;
                    pTempPcmData->freqSampCycles++;
                } else {
                    pTempPcmData->firstFreqCycle = FALSE;
                }
                pTempPcmData->freqSampCount = 0;
                pTempPcmData->newFreqCycle = FALSE;
            }

            if ((pcmSample < -VP_PCM_COMPUTE_HYST) &&
                !pTempPcmData->newFreqCycle) {

                pTempPcmData->newFreqCycle = TRUE;
            }
            pTempPcmData->freqSampCount++;
        }


        /* sum of squares measrument (only needed for RMS) */
        if (pTempPcmData->operations & VP_PCM_OPERATION_RMS) {
            /*add shifted square to sum of squares */
            pTempPcmData->sumOfSqrs +=  ((uint32)pcmSample * (uint32)pcmSample) >>
                pTempPcmData->shift;

            /* if the sum is too large, halve it and increase shift by 1 */
            if (pTempPcmData->sumOfSqrs > VP_PCM_COMPUTE_MAX_ACCUMULATE) {
                pTempPcmData->sumOfSqrs >>= 1;
                pTempPcmData->shift++;
            }
        }

        /* return if all samples have been collected */
        if (pTempPcmData->curSampleNum >= pTempPcmData->totalOpSamples) {
            VpPcmComputeConclude(pTempPcmData, pResults);
            goto pcm_cal_done;
        }
    }

    #ifdef EZ_LOG_DEBUG
    fclose(pFile);
    #endif

    return PCM_CAL_RUNNING;

    pcm_cal_done:
        #ifdef EZ_LOG_DEBUG
        fprintf(pFile, "\n\n");
        fflush(NULL);
        fclose(pFile);
        #endif

        return PCM_CAL_DONE;
}

static void VpPcmComputeConclude(
    VpPcmComputeTempType *pTempPcmData,
    VpPcmOperationResultsType *pResults)
{
    int32 average, temp, aveOfSquares, totalSamples;
    average = temp = 0;
    totalSamples =  (int32)pTempPcmData->totalOpSamples;

    /* range calculations */
    pResults->range = pTempPcmData->max - pTempPcmData->min;
    pResults->min = pTempPcmData->min;
    pResults->max = pTempPcmData->max;

    /* calculate and store the average value */
    average  = pTempPcmData->sum / totalSamples;
    pResults->average = (int16)average;

    /* Only perform following if freq is requested */
    if (pTempPcmData->operations & VP_PCM_OPERATION_FREQ) {
        if (pTempPcmData->freqSampTotal != 0) {
            pResults->freq = 100 * ((8000 * 10 * pTempPcmData->freqSampCycles) /
                (pTempPcmData->freqSampTotal * pTempPcmData->downSampleRate));
        }
    }

    /* Only perform following if rms is requested */
    if (pTempPcmData->operations & VP_PCM_OPERATION_RMS) {

        /* if there are no samples then make average of squares 0 */
        if (0 >= totalSamples) {
            aveOfSquares = 0;
        } else {
            /* calculate the average of the squares */
            aveOfSquares = (pTempPcmData->sumOfSqrs + (totalSamples >> 1 )) /
                totalSamples;
            aveOfSquares <<= pTempPcmData->shift;
        }

        /* abs (squrt( average^2 - aveOfSquares )) */
        temp = average * average;
        temp -= aveOfSquares;
        temp = (temp < 0) ? -temp : temp;
        pResults->rms = VpComputeSquareRoot((uint32)temp);
    }
    #ifdef EZ_LOG_DEBUG
      {
        FILE *pFile = fopen(EZ_LOG_DEBUG, "a");
        fprintf(pFile, "\n\n range=%i  min=%i  max=%i  average=%i  freq=%li  rms=%i\n\n",
            pResults->range, pResults->min, pResults->max, pResults->average, pResults->freq,
            pResults->rms);
        fflush(NULL);
        fclose(pFile);
      }
    #endif

    return;
}

#endif




