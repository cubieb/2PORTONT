/** \lt_api_units.h
 * lt_api_units.h
 *
 * This file defines the basic physical parameter measurement types and their
 * units.
 *
 * This file assumes a certain include order to compile.
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 9115 $
 * $LastChangedDate: 2011-11-15 15:25:17 -0600 (Tue, 15 Nov 2011) $
 */

#ifndef _LT_API_UNITS_H_
#define _LT_API_UNITS_H_

/*******************************************************************************
 * Define the basic measurement types and their units                          *
 ******************************************************************************/
/* Line test impedance type; In units of tenth of a Ohm */
typedef int32 LtImpedanceType;

/* Line test voltage type; In units of milli Volts */
typedef int32 LtVoltageType;

/* Line test current type; In units of micro Amperes */
typedef int32 LtCurrentType;

/* Line test time type; In units of micro seconds */
typedef int32 LtTimeType;

/* Line test capacitance type; In units of pico Farads */
typedef int32 LtCapacitanceType;

/* Line test Inductance type; In units of nano Henries */
typedef int32 LtInductanceType;

/* Line test distance type; In units of milli meters */
typedef int32 LtDistanceType;

/* Line test dB type; In units of milli dB */
typedef int32 LtdBType;

/* Line test Frequency type; In units of milli Hertz */
typedef int32 LtFreqType;

/* Line test Ringer Equivalence Number (REN) type; In units of milli REN */
typedef int32 LtRenType;

/* Line test percent error type; in units of milli percent */
typedef int32 LtPercentType;

/******************************************************************************
 * Special meanings assigned for reprasenting certain physical conditions     *
 ******************************************************************************/
/*
 * The following definition defines a unique value for indicating open circuit.
 * Open circuit does not mean a specified or measured value that is indicated by
 * the following definition. It only means for the purposes of LT-API and
 * its measuring capabilities the impedance is high enough to be treated as
 * open circuit.
 */
#define LT_IMPEDANCE_OPEN_CKT (VP_INT32_MAX)

/*
 * The following definition defines a unique value for indicating short circuit
 * or fault condition. Short circuit does not mean a specified or measured value
 * that is indicated by the following definition. It only means for the
 * purposes of LT-API and its measuring capabilities the impedance is low
 * enough to be treated as short circuit.
 */
#define LT_IMPEDANCE_SHORT_CKT (0)

/*
 * The following definition is used to indicate an impedance parameter that
 * was not measured (or unspecified input).
 */
#define LT_IMPEDANCE_NOT_MEASURED (VP_INT32_MIN)

/*
 * The following definition is used to indicate an voltage parameter that
 * was not measured (or unspecified input).
 */
#define LT_VOLTAGE_NOT_MEASURED (VP_INT32_MIN)

/*
 * The following definition is used to indicate an REN parameter that
 * was not measured (or unspecified input).
 */
#define LT_REN_NOT_MEASURED (VP_INT32_MIN)

/*
 * The following definition is used to indicate a current parameter that
 * exceeded the device measurement capability.
 */
#define LT_MAX_REN (VP_INT32_MAX)

/*
 * The following definition is used to indicate an current parameter that
 * was not measured (or unspecified input).
 */
#define LT_CURRENT_NOT_MEASURED (VP_INT32_MIN)

/*
 * The following definition is used to indicate a current parameter that
 * exceeded the device measurement capability.
 */
#define LT_MAX_CURRENT (VP_INT32_MAX)

/*
 * The following definition is used to indicate a current parameter that
 * exceeded the device measurement capability.
 */
#define LT_MAX_CAPACITANCE (VP_INT32_MAX)

/*
 * The following definition is used to indicate an freq parameter that
 * was not measured (or unspecified input).
 */
#define LT_FREQ_NOT_MEASURED (VP_INT32_MIN)

/*
 * The following definition is used to indicate 0% and 100%
 */
#define LT_0_PERCENT (0)
#define LT_100_PERCENT (100 * 1000)

#endif /* !_LT_API_UNITS_H_ */
