

#include		"ctypes.h"
#include		"local.h"
#include		"debug.h"
#include		"smp.h"
#include		"mix.h"
#include		"avl.h"
#include		"mis.h"
#include		"aps.h"
#include		"asn.h"
#include		"asndefs.h"

typedef		struct			SmpRecTag {

		SmpHandlerType		smpRecUpCall;
		SmpSendFnType		smpRecSendFn;
		SmpSocketType		smpRecPeer;
		AsnIdType		smpRecAsn;
		SmpStatusType		smpRecStatus;

		}			SmpRecType;

typedef		SmpRecType		*SmpRecPtrType;

#define		smpIdToPtr(x)		((SmpRecPtrType)((SmpIdType)(x)))
#define		smpPtrToId(x)		((SmpIdType)((SmpRecPtrType)(x)))

#define		smpMaxBindSize		(20)

#ifndef		SERVER

static		SmpKindType		smpUniversalVector [] = {

		smpKindNone,
		smpKindNone,
		smpKindInteger,
		smpKindNone,
		smpKindOctetString,
		smpKindNull,
		smpKindObjectId
};

static		SmpKindType		smpApplicationVector [] = {

		smpKindIPAddr,
		smpKindCounter,
		smpKindGuage,
		smpKindTimeTicks,
		smpKindOpaque
};

#define		smpAsnToKind(asn)	\
			((asnClass ((asn)) == asnClassUniversal) ?	\
			smpUniversalVector [ (int) asnTag ((asn)) ] :	\
			smpApplicationVector [ (int) asnTag ((asn)) ])

#endif		/*	SERVER	*/

static		AsnTagType		smpTagVector [] = {

		(AsnTagType)	-1,	/* smpKindNone	*/
		(AsnTagType)	2,
		(AsnTagType)	4,
		(AsnTagType)	0,
		(AsnTagType)	4,
		(AsnTagType)	1,
		(AsnTagType)	2,
		(AsnTagType)	3,
		(AsnTagType)	6,
		(AsnTagType)	5
};

static		AsnClassType		smpClassVector [] = {

		asnClassPrivate,	/* smpKindNone	*/
		asnClassUniversal,
		asnClassUniversal,
		asnClassApplication,
		asnClassApplication,
		asnClassApplication,
		asnClassApplication,
		asnClassApplication,
		asnClassUniversal,
		asnClassUniversal

};

#define		smpKindToTag(kind)	\
			(smpTagVector [ (int) (kind) ])

#define		smpKindToClass(kind)	\
			(smpClassVector [ (int) (kind) ])

#define		smpAsnToCmd(tag)	\
			((SmpCommandType) (tag))

CVoidType		smpInit (void)
{
}

SmpIdType		smpNew (SmpSocketType peer, SmpSendFnType sendFn, SmpHandlerType upcall)
{
	SmpRecPtrType		sp;

	sp = (SmpRecPtrType) malloc ((unsigned) sizeof (*sp));
	if (sp != (SmpRecPtrType) 0) {
		(void) bzero ((char *) sp, (int) sizeof (*sp));
		sp->smpRecPeer = peer;
		sp->smpRecSendFn = sendFn;
		sp->smpRecUpCall = upcall;
		sp->smpRecAsn = (AsnIdType) 0;
	}
	return (smpPtrToId (sp));
}


SmpIdType		smpFree (SmpIdType smp)
{
	SmpRecPtrType		sp;

	if (smp != (SmpIdType) 0) {
		sp = smpIdToPtr (smp);
		sp->smpRecAsn = asnFree (sp->smpRecAsn);
		(void) free ((char *) sp);
	}
	return ((SmpIdType) 0);
}

static	SmpStatusType	smpSend (SmpIdType smp, ApsIdType aps, AsnIdType asn)
{
	AsnLengthType		n;
	AsnIdType		msg;
	SmpRecPtrType		sp;
	CByteType		buffer [ 2048 ];
	SmpStatusType		status;
 

	DEBUG0 ("smpSend:\n");
	msg = apsEncode (aps, asn);
	if (msg == (AsnIdType) 0) {
		return (errBad);
	}
	DEBUGASN (msg);
	DEBUG0 ("\n");
	n = asnEncode(msg, buffer, (AsnLengthType) sizeof(buffer));
	DEBUGBYTES (buffer, n);
	DEBUG0 ("\n");
	sp = smpIdToPtr (smp);
	status = (*sp->smpRecSendFn) (sp->smpRecPeer,
		buffer, (CIntfType) n);
	msg = asnFree (msg);
	return (status);
}

static	AsnStatusType	smpSishKabob (AsnIdType skewer, AsnClassType class, AsnTagType tag, CUnslType value)
{
	AsnStatusType		status;
	AsnIdType		item;

	if ((item = asnUnsl (class, tag, value)) == (AsnIdType) 0) {
		return (asnStatusBad);
	}
        status = asnAppend (skewer, item);
	item = asnFree (item);
	return (status);
}

static	AsnIdType	smpBuildBind2 (AsnIdType name, AsnIdType value)
{
	AsnIdType		seq;

	if ((seq = asnSequence (asnClassUniversal, (AsnTagType) 0x10,
		asnTypeSequence)) != (AsnIdType) 0) {
		if (asnAppend (seq, name) != asnStatusOk) {
			seq = asnFree (seq);
		}
		else if (asnAppend (seq, value) != asnStatusOk) {
			seq = asnFree (seq);
		}
	}
	return (seq);
}

static	AsnIdType	smpBuildBind1 (CBytePtrType name, AsnLengthType namelen, AsnIdType value)
{
	AsnIdType		oid;
	AsnIdType		result;

	if ((oid = asnObjectId (asnClassUniversal, (AsnTagType) 6,
		name, namelen)) != (AsnIdType) 0) {
		result =  smpBuildBind2 (oid, value);
		oid = asnFree (oid);
		return (result);
	}
	else {
		return (oid);
	}
}

static	AsnIdType	smpBuildValue (SmpBindPtrType list)
{
	AsnIdType		result;

	if (list == (SmpBindPtrType) 0) {
		result = (AsnIdType) 0;
	}
	else {
		switch (list->smpBindKind) {

		case smpKindInteger:
			result = asnIntl (asnClassUniversal,
				(AsnTagType) 2,
				(CIntlType) list->smpBindNumber);
			break;

		case smpKindGuage:
		case smpKindCounter:
		case smpKindTimeTicks:
			result = asnUnsl (asnClassApplication,
				smpKindToTag (list->smpBindKind),
				list->smpBindNumber);
			break;

		case smpKindOctetString:
		case smpKindOpaque:
		case smpKindNull:
			result = asnOctetString (
				smpKindToClass (list->smpBindKind),
				smpKindToTag (list->smpBindKind),
				(CBytePtrType) list->smpBindValue,
				(AsnLengthType) list->smpBindValueLen);
			break;

		case smpKindIPAddr:
			if (list->smpBindValueLen != (SmpLengthType) 4) {
				result = (AsnIdType) 0;
			}
			else {
				result = asnOctetString (asnClassApplication,
					(AsnTagType) 0,
					(CBytePtrType) list->smpBindValue,
					(AsnLengthType) 4);
			}
			break;

		case smpKindObjectId:
			result = asnObjectId (asnClassUniversal,
				(AsnTagType) 6,
				(CBytePtrType) list->smpBindValue,
				(AsnLengthType) list->smpBindValueLen);
			break;

		default:
			result = (AsnIdType) 0;
			break;

		}
	}
	
	return (result);
}

static	AsnIdType	smpBuildBind0 (SmpBindPtrType bind)
{
	AsnIdType		value;
	AsnIdType		result;

	if ((value = smpBuildValue (bind)) != (AsnIdType) 0) {
		result =  smpBuildBind1 ((CBytePtrType) bind->smpBindName,
			(AsnLengthType) bind->smpBindNameLen, value);
		value = asnFree (value);
		return (result);
	}
	else {
		return (value);
	}
}

static	AsnIdType	smpBuildMsg (AsnTagType tag, AsnIdType reqid, SmpErrorType error, CUnsfType index, AsnIdType list)
{
	AsnIdType		result;

	if (reqid == (AsnIdType) 0) {
		result = (AsnIdType) 0;
	}
	else if ((result = asnSequence (asnClassContext, tag,
		asnTypeSequence)) == (AsnIdType) 0) {
		result = (AsnIdType) 0;
	}
	else if (asnAppend (result, reqid) != asnStatusOk) {
		result = asnFree (result);
	}
	else if (smpSishKabob (result, asnClassUniversal, (AsnTagType) 2,
		(CUnslType) error) != asnStatusOk) {
		result = asnFree (result);
	}
	else if (smpSishKabob (result, asnClassUniversal, (AsnTagType) 2,
		(CUnslType) index) != asnStatusOk) {
		result = asnFree (result);
	}
	else if (asnAppend (result, list) != asnStatusOk) {
			result = asnFree (result);
	}

	return (result);
}

#ifndef		SERVER
#endif		/*	SERVER	*/

#ifndef		CLIENT
#endif		/*	CLIENT	*/

#ifndef		CLIENT

static	AsnIdType	smpReply (AsnIdType asn0, SmpErrorType error, CUnsfType index, AsnIdType asn1)
{
	AsnIdType		result;
	AsnIdType		reqid;
	AsnIdType		seq;

	DEBUG0 ("smpReply:\n");
	if (error == smpErrorNone) {
		index = (CUnsfType) 0;
	}

	if ((reqid = asnComponent (asn0, (AsnIndexType) 1)) ==
		(AsnIdType) 0) {
		result = (AsnIdType) 0;
	}
	else if (error == smpErrorNone) {
		result = smpBuildMsg ((AsnTagType) 2, reqid, error,
			index, asn1);
		reqid = asnFree (reqid);
	}
	else if ((seq = asnComponent (asn0, (AsnIndexType) 4)) ==
		(AsnIdType) 0) {
		result = (AsnIdType) 0;
		reqid = asnFree (reqid);
	}
	else {
		result = smpBuildMsg ((AsnTagType) 2,
			reqid, error, index, seq);
		reqid = asnFree (reqid);
		seq = asnFree (seq);
	}
	return (result);
}

#endif		/*	CLIENT	*/

#ifndef		SERVER

static	SmpIndexType	smpCrackList (AsnIdType seq, SmpBindPtrType result, SmpIndexType n)
{
	SmpIndexType		count;
	AsnIdType		bind;
	AsnIdType		name;
	AsnIdType		value;
	AsnIndexType		sons;
	AsnIndexType		component;

	sons = asnSons (seq);
	count = (SmpIndexType) 0;
	bind = (AsnIdType) 0;
	for (component = 1; (component <= sons) && (count < n);
		bind = asnFree (bind)) {
		bind = asnComponent (seq, component);
		name = asnComponent (bind, (AsnIndexType) 1);
		result->smpBindName = asnValue (name);
		result->smpBindNameLen = asnLength (name);
		name = asnFree (name);
		value = asnComponent (bind, (AsnIndexType) 2);
		result->smpBindKind = smpAsnToKind (value);
		/* support only primitive encodings */
		result->smpBindValue = (SmpValueType) asnValue (value);
		result->smpBindValueLen = (SmpLengthType) asnLength (value);
		switch (result->smpBindKind) {

		case smpKindInteger:
		case smpKindCounter:
		case smpKindGuage:
		case smpKindTimeTicks:
			result->smpBindNumber =
				(SmpNumberType) asnNumber (
				(CBytePtrType) result->smpBindValue,
				(AsnLengthType)
				result->smpBindValueLen);
			break;

		default:
			result->smpBindNumber = (SmpNumberType) 0;
			break;

		}
		value = asnFree (value);
		count++;
		result++;
		component += (asnSons (bind) + 1);
	}
	return (count);
}

static	SmpStatusType	smpRspOp (SmpIdType smp, ApsIdType aps, AsnIdType asn)
{
	SmpRequestType		p;
	SmpBindType		bindvec [ smpMaxBindSize ];
	AsnIdType		seq;
	AsnIdType		value;

	(void) bzero ((char *) & p, sizeof (p));
	p.smpRequestCmd = smpAsnToCmd (asnTag (asn));
	value = asnComponent (asn, (AsnIndexType) 1);
	p.smpRequestId = (SmpSequenceType) asnNumber (asnValue (value),
		asnLength (value));
	value = asnFree (value);
	value = asnComponent (asn, (AsnIndexType) 2);
	p.smpRequestError = (SmpErrorType) asnNumber (asnValue (value),
		asnLength (value));
	value = asnFree (value);
	value = asnComponent (asn, (AsnIndexType) 3);
	p.smpRequestIndex = (SmpIndexType) asnNumber (asnValue (value),
		asnLength (value));
	value = asnFree (value);
	seq = asnComponent (asn, (AsnIndexType) 4);
	p.smpRequestCount = smpCrackList (seq, bindvec,
		(SmpIndexType) smpMaxBindSize);
	seq = asnFree (seq);
	p.smpRequestBinds = bindvec;
	p.smpRequestCommunity = aps;
	return ((*((smpIdToPtr (smp))->smpRecUpCall)) (smp, & p));
}

static	SmpStatusType	smpTrapOp (SmpIdType smp, ApsIdType aps, AsnIdType asn)
{
	SmpRequestType		p;
	SmpBindType		bindvec [ smpMaxBindSize ];
	AsnIdType		seq;
	AsnIdType		value;

	(void) bzero ((char *) & p, sizeof (p));
	p.smpRequestCmd = smpCommandTrap;
	value = asnComponent (asn, (AsnIndexType) 1);
	p.smpRequestEnterprise = (SmpValueType) asnValue (value);
	p.smpRequestEnterpriseLen = (SmpLengthType) asnLength (value);
	value = asnFree (value);
	value = asnComponent (asn, (AsnIndexType) 2);
	p.smpRequestAgent = (SmpValueType) asnValue (value);
	p.smpRequestAgentLen = (SmpLengthType) asnLength (value);
	value = asnFree (value);
	value = asnComponent (asn, (AsnIndexType) 3);
	p.smpRequestGenericTrap = (SmpTrapType) asnNumber (asnValue (value),
		asnLength (value));
	value = asnFree (value);
	value = asnComponent (asn, (AsnIndexType) 4);
	p.smpRequestSpecificTrap = (SmpNumberType) asnNumber (asnValue (value),
		asnLength (value));
	value = asnFree (value);
	value = asnComponent (asn, (AsnIndexType) 5);
	p.smpRequestTimeStamp = (SmpNumberType) asnNumber (asnValue (value),
		asnLength (value));
	value = asnFree (value);
	seq = asnComponent (asn, (AsnIndexType) 6);
	p.smpRequestCount = smpCrackList (seq, bindvec,
		(SmpIndexType) smpMaxBindSize);
	seq = asnFree (seq);
	p.smpRequestBinds = bindvec;
	p.smpRequestCommunity = aps;
	return ((*((smpIdToPtr (smp))->smpRecUpCall)) (smp, & p));
}

static	SmpStatusType	smpTrapV2Op (SmpIdType smp, ApsIdType aps, AsnIdType asn)
{
	SmpRequestType		p;
	SmpBindType		bindvec [ smpMaxBindSize ];
	AsnIdType		seq;
	AsnIdType		value;

	(void) bzero ((char *) & p, sizeof (p));
	p.smpRequestCmd = smpCommandTrap;
	value = asnComponent (asn, (AsnIndexType) 1);
	p.smpRequestEnterprise = (SmpValueType) asnValue (value);
	p.smpRequestEnterpriseLen = (SmpLengthType) asnLength (value);
	value = asnFree (value);
	value = asnComponent (asn, (AsnIndexType) 2);
	p.smpRequestAgent = (SmpValueType) asnValue (value);
	p.smpRequestAgentLen = (SmpLengthType) asnLength (value);
	value = asnFree (value);
	value = asnComponent (asn, (AsnIndexType) 3);
	p.smpRequestGenericTrap = (SmpTrapType) asnNumber (asnValue (value),
		asnLength (value));
	value = asnFree (value);
	value = asnComponent (asn, (AsnIndexType) 4);
	p.smpRequestSpecificTrap = (SmpNumberType) asnNumber (asnValue (value),
		asnLength (value));
	value = asnFree (value);
	value = asnComponent (asn, (AsnIndexType) 5);
	p.smpRequestTimeStamp = (SmpNumberType) asnNumber (asnValue (value),
		asnLength (value));
	value = asnFree (value);
	seq = asnComponent (asn, (AsnIndexType) 6);
	p.smpRequestCount = smpCrackList (seq, bindvec,
		(SmpIndexType) smpMaxBindSize);
	seq = asnFree (seq);
	p.smpRequestBinds = bindvec;
	p.smpRequestCommunity = aps;
	return ((*((smpIdToPtr (smp))->smpRecUpCall)) (smp, & p));
}

#else		/*	SERVER	*/

#define		smpRspOp(smp, aps, asn)		(errOk)
#define		smpTrapOp(smp, aps, asn)	(errOk)
#define		smpTrapV2Op(smp, aps, asn)	(errOk)

#endif		/*	SERVER	*/

#ifndef		CLIENT

static	SmpStatusType	smpSetOp (SmpIdType smp, ApsIdType aps, AsnIdType asn)
{
	AsnIdType		reply;
	AsnIdType		name;
	AsnIdType		bind;
	AsnIdType		seq;
	AsnIdType		val;
	AsnIdType		save;
	AsnIndexType		sons;
	CUnsfType		index;
	CUnsfType		count;
	AsnIndexType		component;
	MixIdType		mib;
	SmpErrorType		error;
	SmpStatusType		result;
	MixNamePtrType		path;
	MixLengthType		pathlen;
	MixNamePtrType		names [ smpMaxBindSize ];
	MixLengthType		sizes [ smpMaxBindSize ];
	AsnIdType		values [ smpMaxBindSize ];

	DEBUG0 ("smpSetOp\n");
	DEBUG0 ("smpSetOp: asn:\n");
	DEBUGASN (asn);
	seq = asnComponent (asn, (AsnIndexType) 4);
	DEBUG0 ("smpSetOp: seq:\n");
	DEBUGASN (seq);

	sons = asnSons (seq);
	mib = misCommunityToMib (aps);
	index = (CUnsfType) 1;
	if (misCommunityToAccess (aps)) {
		error = smpErrorNone;
		reply = (AsnIdType) 0;
		name = (AsnIdType) 0;
		bind = (AsnIdType) 0;
		val = (AsnIdType) 0;
		count = (CUnsfType) 0;
		for (component = 1; (component <= sons) &&
			(error == smpErrorNone) && (count < smpMaxBindSize);
			component += (asnSons (bind) + 1)) {
			bind = asnFree (bind);
			bind = asnComponent (seq, component);
			DEBUG0 ("smpSetOp: bind:\n");
			DEBUGASN (bind);
			name = asnComponent (bind, (AsnIndexType) 1);
			DEBUG0 ("smpSetOp: name:\n");
			DEBUGASN (name);
			path = (MixNamePtrType) asnValue (name);
			pathlen = (MixLengthType) asnLength (name);
			val = asnComponent (bind, (AsnIndexType) 2);
			if ((save = mixGet (mib, path, pathlen)) ==
				(AsnIdType) 0) {
				error = smpErrorNoSuch;
				DEBUG0 ("smpSetOp 1\n");
			}
			else if ((error = mixSet (mib, path, pathlen,
				val)) == smpErrorNone) {
				DEBUG0 ("smpSetOp 7\n");
				name = asnFree (name);
				val = asnFree (val);
				values [ count ] = save;
				names [ count ] = path;
				sizes [ count ] = pathlen;
				index++;
				count++;
			}
			else {
				save = asnFree (save);
			}
		}

		name = asnFree (name);
		bind = asnFree (bind);
		val = asnFree (val);

		if ((error == smpErrorNone) && (count >= smpMaxBindSize)) {
			error = smpErrorTooBig;
		}
		while (count != 0) {
			count--;
			if (error != smpErrorNone) {
				(void) mixSet (mib, names [ count ],
					sizes [ count ], values [ count ]);
			}
			values [ count ] = asnFree (values [ count ]);
		}
	}
	else {
		error = smpErrorReadOnly;
	}

	DEBUG1 ("smpSetOp: error %d\n", error);
	if  ((reply = smpReply (asn, error, index, seq)) !=
		(AsnIdType) 0) {
		result = smpSend (smp, aps, reply);
	}
	else {
		result = errBad;
	}
	seq = asnFree (seq);
	reply = asnFree (reply);
	return (result);
}

static	SmpStatusType	smpNextOp (SmpIdType smp, ApsIdType aps, AsnIdType asn)
{
	AsnIdType		reply;
	AsnIdType		name;
	AsnIdType		bind;
	AsnIdType		seq;
	AsnIdType		rval;
	AsnIdType		rbind;
	AsnIdType		rseq;
	AsnIndexType		sons;
	CUnsfType		index;
	AsnIndexType		component;
	MixIdType		mib;
	CByteType		path [ mixMaxPathLen ];
	MixLengthType		pathlen;
	AsnLengthType		namelen;
	SmpErrorType		error;
	SmpStatusType		result;

	DEBUG0 ("smpNextOp\n");
	rseq = asnSequence (asnClassUniversal, (AsnTagType) 0x10,
		asnTypeSequenceOf);
	if (rseq == (AsnIdType) 0) {
		return (errBad);
	}

	DEBUG0 ("smpNextOp: asn:\n");
	DEBUGASN (asn);
	seq = asnComponent (asn, (AsnIndexType) 4);
	DEBUG0 ("smpNextOp: seq:\n");
	DEBUGASN (seq);
	sons = asnSons (seq);
	mib = misCommunityToMib (aps);
	error = smpErrorNone;
	reply = (AsnIdType) 0;
	name = (AsnIdType) 0;
	bind = (AsnIdType) 0;
	rval = (AsnIdType) 0;
	rbind = (AsnIdType) 0;
	index = 1;
	for (component = 1; (component <= sons) &&
		(error == smpErrorNone);
		component += (asnSons (bind) + 1)) {
		bind = asnFree (bind);
		bind = asnComponent (seq, component);
		DEBUG0 ("smpNextOp: bind:\n");
		DEBUGASN (bind);
		name = asnComponent (bind, (AsnIndexType) 1);
		DEBUG0 ("smpNextOp: name:\n");
		DEBUGASN (name);
		namelen = asnContents (name, path,
			(AsnLengthType) mixMaxPathLen);
		pathlen = (MixLengthType) namelen;
		if (namelen < (AsnLengthType) 0) {
			error = smpErrorNoSuch;
			DEBUG0 ("smpNextOp 0\n");
		}
		else if ((rval = mixNext (mib, (MixNamePtrType) path,
			& pathlen, NULL)) == (AsnIdType) 0) {
			DEBUG0 ("smpNextOp 1\n");
			error = smpErrorNoSuch;
		}
		else if ((rbind = smpBuildBind1 (path,
			(AsnLengthType) pathlen, rval)) == (AsnIdType) 0) {
			DEBUG0 ("smpNextOp 2\n");
			error = smpErrorTooBig;
		}
		else if (asnAppend (rseq, rbind) != asnStatusOk) {
			DEBUG0 ("smpNextOp 6\n");
			error = smpErrorTooBig;
		}
		else {
			DEBUG0 ("smpNextOp 7\n");
			name = asnFree (name);
			rval = asnFree (rval);
			rbind = asnFree (rbind);
			index++;
		}
	}

	DEBUG1 ("smpNextOp: error %d\n", error);
	DEBUG0 ("smpNextOp: rseq:\n");
	DEBUGASN (rseq);
	name = asnFree (name);
	seq = asnFree (seq);
	bind = asnFree (bind);
	rval = asnFree (rval);
	rbind = asnFree (rbind);
	if  ((reply = smpReply (asn, error, index, rseq)) !=
		(AsnIdType) 0) {
		result = smpSend (smp, aps, reply);
	}
	else {
		result = errBad;
	}
	rseq = asnFree (rseq);
	reply = asnFree (reply);
	return (result);
}

static	SmpStatusType	smpGetOp (SmpIdType smp, ApsIdType aps, AsnIdType asn)
{
	AsnIdType		reply;
	AsnIdType		name;
	AsnIdType		bind;
	AsnIdType		seq;
	AsnIdType		rval;
	AsnIdType		rbind;
	AsnIdType		rseq;
	AsnIndexType		sons;
	CUnsfType		index;
	AsnIndexType		component;
	MixIdType		mib;
	SmpErrorType		error;
	SmpStatusType		result;

	DEBUG0 ("smpGetOp\n");
	rseq = asnSequence (asnClassUniversal, (AsnTagType) 0x10,
		asnTypeSequenceOf);
	if (rseq == (AsnIdType) 0) {
		return (errBad);
	}

	DEBUG0 ("smpGetOp: asn:\n");
	DEBUGASN (asn);
	seq = asnComponent (asn, (AsnIndexType) 4);
	DEBUG0 ("smpGetOp: seq:\n");
	DEBUGASN (seq);
	sons = asnSons (seq);
	mib = misCommunityToMib (aps);
	error = smpErrorNone;
	reply = (AsnIdType) 0;
	name = (AsnIdType) 0;
	bind = (AsnIdType) 0;
	rval = (AsnIdType) 0;
	rbind = (AsnIdType) 0;
	index = 1;
	for (component = 1; (component <= sons) &&
		(error == smpErrorNone);
		component += (asnSons (bind) + 1)) {
		bind = asnFree (bind);
		bind = asnComponent (seq, component);
		DEBUG0 ("smpGetOp: bind:\n");
		DEBUGASN (bind);
		name = asnComponent (bind, (AsnIndexType) 1);
		DEBUG0 ("smpGetOp: name:\n");
		DEBUGASN (name);
		if ((rval = mixGet (mib, (MixNamePtrType) asnValue (name),
			(MixLengthType) asnLength (name))) ==
			(AsnIdType) 0) {
			DEBUG0 ("smpGetOp 1\n");
			error = smpErrorNoSuch;
		}
		else if ((rbind = smpBuildBind2 (name, rval)) ==
			(AsnIdType) 0) {
			DEBUG0 ("smpGetOp 2\n");
			error = smpErrorTooBig;
		}
		else if (asnAppend (rseq, rbind) != asnStatusOk) {
			DEBUG0 ("smpGetOp 6\n");
			error = smpErrorTooBig;
		}
		else {
			DEBUG0 ("smpGetOp 7\n");
			name = asnFree (name);
			rval = asnFree (rval);
			rbind = asnFree (rbind);
			index++;
		}
	}

	DEBUG1 ("smpGetOp: error %d\n", error);
	DEBUG0 ("smpGetOp: rseq:\n");
	DEBUGASN (rseq);
	name = asnFree (name);
	seq = asnFree (seq);
	bind = asnFree (bind);
	rval = asnFree (rval);
	rbind = asnFree (rbind);
	if  ((reply = smpReply (asn, error, index, rseq)) !=
		(AsnIdType) 0) {
		result = smpSend (smp, aps, reply);
	}
	else {
		result = errBad;
	}
	rseq = asnFree (rseq);
	reply = asnFree (reply);
	return (result);
}

static	SmpStatusType	smpGetBulkOp (SmpIdType smp, ApsIdType aps, AsnIdType asn)
{
	AsnIdType		reply;
	AsnIdType		name;
	AsnIdType		bind;
	AsnIdType		seq;
	AsnIdType		rval;
	AsnIdType		rbind;
	AsnIdType		rseq;
	AsnIndexType		sons;
	CUnsfType		index;
	AsnIndexType		component;
	MixIdType		mib;
	CByteType		path [ mixMaxPathLen ];
	MixLengthType		pathlen;
	AsnLengthType		namelen;
	SmpErrorType		error;
	SmpStatusType		result;
	//for get bulk 
	AsnIdType		asnRpt;
	AsnLengthType			n;
	int			nonRepeaters, maxRepetitions, tmpnonRepeaters;
	int 			i, j, v2EOMV, vbNum;
#define MR_NUM	128	
	AsnIdType		currentBind, MRarray[MR_NUM];
	AsnIndexType		tmpcomponent;

	DEBUG0 ("smpGetBulkOp\n");
	asnRpt  = asnComponent (asn, (AsnIndexType) 2);
	if (asnRpt == (AsnIdType) 0) {
		return ((ApsIdType) 0);
	}
	n = asnLength (asnRpt);
	if (n > (AsnLengthType) 2)
		return ((ApsIdType) 0);
	tmpnonRepeaters = nonRepeaters = asnNumber (asnValue (asnRpt), n);
	if (nonRepeaters<0) {
		tmpnonRepeaters = nonRepeaters = 0;
	};

	asnRpt  = asnComponent (asn, (AsnIndexType) 3);
	if (asnRpt == (AsnIdType) 0) {
		return ((ApsIdType) 0);
	}
	n = asnLength (asnRpt);
	if (n > (AsnLengthType) 2)
		return ((ApsIdType) 0);
	maxRepetitions = asnNumber (asnValue (asnRpt), n);
	if (maxRepetitions<0) maxRepetitions = 0;
		
	rseq = asnSequence (asnClassUniversal, (AsnTagType) 0x10,
		asnTypeSequenceOf);
	if (rseq == (AsnIdType) 0) {
		return (errBad);
	}

	DEBUG0 ("smpGetBulkOp: asn:\n");
	DEBUGASN (asn);
	seq = asnComponent (asn, (AsnIndexType) 4);
	DEBUG0 ("smpGetBulkOp: seq:\n");
	DEBUGASN (seq);
	sons = asnSons (seq);
	mib = misCommunityToMib (aps);
	error = smpErrorNone;
	reply = (AsnIdType) 0;
	name = (AsnIdType) 0;
	bind = (AsnIdType) 0;
	rval = (AsnIdType) 0;
	rbind = (AsnIdType) 0;
	index = 1;
	//nonRepeaters
	for (component = 1; (component <= sons) &&
		(error == smpErrorNone);
		component += (asnSons (bind) + 1)) {
			
		if (tmpnonRepeaters==0)
	    		goto process_maxRepetitions;
	    	tmpnonRepeaters--;
	    
		bind = asnFree (bind);
		bind = asnComponent (seq, component);
		DEBUG0 ("smpGetBulkOp: bind:\n");
		DEBUGASN (bind);
		name = asnComponent (bind, (AsnIndexType) 1);
		DEBUG0 ("smpGetBulkOp: name:\n");
		DEBUGASN (name);
		namelen = asnContents (name, path,
			(AsnLengthType) mixMaxPathLen);
		pathlen = (MixLengthType) namelen;
		if (namelen < (AsnLengthType) 0) {
			error = smpErrorNoSuch;
			DEBUG0 ("smpGetBulkOp 0\n");
		}
		else if ((rval = mixNext (mib, (MixNamePtrType) path,
			& pathlen, NULL)) == (AsnIdType) 0) {
			DEBUG0 ("smpGetBulkOp 1\n");
			error = smpErrorNoSuch;
		}
		else if ((rbind = smpBuildBind1 (path,
			(AsnLengthType) pathlen, rval)) == (AsnIdType) 0) {
			DEBUG0 ("smpGetBulkOp 2\n");
			error = smpErrorTooBig;
		}
		else if (asnAppend (rseq, rbind) != asnStatusOk) {
			DEBUG0 ("smpGetBulkOp 6\n");
			error = smpErrorTooBig;
		}
		else {
			DEBUG0 ("smpGetBulkOp 7\n");
			name = asnFree (name);
			rval = asnFree (rval);
			rbind = asnFree (rbind);
			index++;
		}
	}
	
process_maxRepetitions:
	name = asnFree (name);
	rval = asnFree (rval);
	rbind = asnFree (rbind);
	if (!((component <= sons) && (error == smpErrorNone)))  //error or no more to process
		goto stop_bulk_get;
	if (maxRepetitions==0)  //M=0
		goto stop_bulk_get;
	//maxRepetitions
	memset(MRarray, 0, sizeof(MRarray));
	
	i=0; vbNum=0;
	for ( ; (component <= sons) &&
		(error == smpErrorNone);
		component += (asnSons (bind) + 1)) {
	
	    vbNum++;
	    bind = asnFree (bind);
	    bind = asnComponent (seq, component);  //get OID record from PDU Var Bind list
	    DEBUG0 ("smpGetRptBulkOp: bind:\n");
	    DEBUGASN (bind);
	    rbind = (AsnIdType) 0;	    
	    for (j=0;(j<maxRepetitions)&&(i<MR_NUM);j++) {  //execute maxRepetitions getNext of component
	    	if (rbind==0) {  //1st getNext of this component
	    		currentBind = bind;
	    	} else {  //next var bind
	    		currentBind = rbind;
	    	}
		name = asnComponent (currentBind, (AsnIndexType) 1);  //get name for OID record
		DEBUG0 ("smpGetRptBulkOp: name:\n");
		DEBUGASN (name);
		namelen = asnContents (name, path,
			(AsnLengthType) mixMaxPathLen);
		pathlen = (MixLengthType) namelen;
		if (namelen < (AsnLengthType) 0) {
			error = smpErrorNoSuch;
			DEBUG0 ("smpGetRptBulkOp 0\n");
		}
		else if ((rval = mixNext (mib, (MixNamePtrType) path,
			& pathlen, &v2EOMV)) == (AsnIdType) 0) {
			DEBUG0 ("smpGetRptBulkOp 1\n");
			error = smpErrorNoSuch;
		}
		else if ((rbind = smpBuildBind1 (path,
			(AsnLengthType) pathlen, rval)) == (AsnIdType) 0) {
			DEBUG0 ("smpGetRptBulkOp 2\n");
			error = smpErrorTooBig;
		}		
		else {
			DEBUG0 ("smpGetRptBulkOp 7\n");
			name = asnFree (name);
			rval = asnFree (rval);
			//rbind = asnFree (rbind);
			MRarray[i] = rbind;
	    		i++;
			index++;
		}
		
		if (v2EOMV==1) { //get endOfMibView, stop current component 
			i = i+(maxRepetitions-j);  //skip empty vb
			break;
		}
		if (error != smpErrorNone) { //error occurs
			i = i+(maxRepetitions-j);  //skip empty vb
			break;
		}
	    }; //for (j=0;j<maxRepetitions;j++) {  //execute maxRepetitions getNext of component
	}
	name = asnFree (name);
	seq = asnFree (seq);
	rval = asnFree (rval);
	//append var bind
	for (j=0;j<maxRepetitions;j++) {  //execute maxRepetitions getNext of component
	    for (i=0;i<vbNum;i++) {
	    	if (MRarray[i*maxRepetitions+j]!=0)
		    if (asnAppend (rseq, MRarray[i*maxRepetitions+j]) != asnStatusOk) {
			DEBUG0 ("smpGetRptBulkOp 6\n");
			error = smpErrorTooBig;
			goto free_MRarray;
		    };
	    };
	};
free_MRarray:
	//free rbind array
	for (i=0;i<MR_NUM;i++) {
		if (MRarray[i]!=0) asnFree(MRarray[i]);
	};
	
stop_bulk_get:
	DEBUG1 ("smpGetBulkOp: error %d\n", error);
	DEBUG0 ("smpGetBulkOp: rseq:\n");
	seq = asnFree (seq);
	bind = asnFree (bind);
	DEBUGASN (rseq);
	if  ((reply = smpReply (asn, error, index, rseq)) !=
		(AsnIdType) 0) {
		result = smpSend (smp, aps, reply);
	}
	else {
		result = errBad;
	}
	rseq = asnFree (rseq);
	reply = asnFree (reply);
	return (result);
}

#else		/*	CLIENT	*/

#define		smpSetOp(smp, aps, asn)		(errOk)
#define		smpGetOp(smp, aps, asn)		(errOk)
#define		smpGetBulkOp(smp, aps, asn)	(errOk)
#define		smpNextOp(smp, aps, asn)	(errOk)

#endif		/*	CLIENT	*/

extern AsnNumberType inputVersion;	//SNMP version id

static	SmpStatusType	smpInputEvent (SmpIdType smp, CByteType x)
{
	AsnStatusType		status;
	AsnIdType		asn0;
	ApsIdType		aps;
	SmpStatusType		result;
	SmpRecPtrType		sp;

	sp = smpIdToPtr (smp);
	status = asnDecode (sp->smpRecAsn, x);
	if (status == asnStatusOk) {
		return (errOk);
	}
	else if (status != asnStatusAccept) {
		return (errBad);
	}

	DEBUG0 ("smpInputEvent ");
	aps = apsVerify (sp->smpRecAsn);
	if (aps == (ApsIdType) 0) {
		DEBUG0 ("2\n");
		return (errBad);
	}

	asn0 = apsDecode (aps, sp->smpRecAsn);
	if (asn0 == (AsnIdType) 0) {
		DEBUG0 ("3\n");
		return (errBad);
	}

	DEBUGASN (asn0);
	sp->smpRecAsn = asnFree (sp->smpRecAsn);

	switch ((int) asnTag (asn0)) {

	case 0x00:
		result = smpGetOp (smp, aps, asn0);
		break;

	case 0x01:
		result = smpNextOp (smp, aps, asn0);
		break;

	case 0x02:
		result = smpRspOp (smp, aps, asn0);
		break;

	case 0x03:
		result = smpSetOp (smp, aps, asn0);
		break;

	case 0x04:
		result = smpTrapOp (smp, aps, asn0);
		break;

	case 0x05:  //for v2c
		if (inputVersion<1) 
		    result = errBad;  //v1 not support get-bulk
		else
		    result = smpGetBulkOp (smp, aps, asn0);
		break;

	default:
		result = errBad;
		break;
	}

	asn0 = asnFree (asn0);
	aps = apsFree (aps);
	DEBUG0 ("4\n");
	return (result);
}


SmpStatusType		smpInput (SmpIdType smp, CByteType x)
{
	SmpRecPtrType		sp;

	if (smp == (SmpIdType) 0) {
		return (errBad);
	}

	sp = smpIdToPtr (smp);
	if (sp->smpRecStatus == errOk) {
		if (sp->smpRecAsn == (AsnIdType) 0) {
			sp->smpRecAsn = asnNew ((AsnLanguageType) 0);
		}
		sp->smpRecStatus = smpInputEvent (smp, x);
	}

	return (sp->smpRecStatus);
}

AsnIdType		smpBuildList (SmpBindPtrType list, SmpIndexType count)
{
	AsnIdType		result;
	AsnIdType		bind;
	AsnStatusType		status;

	result = asnSequence (asnClassUniversal, (AsnTagType) 0x10,
		asnTypeSequence);
	if (result == (AsnIdType) 0) {
		return (result);
	}

	for (status = asnStatusOk; (status == asnStatusOk) && (count != 0);
		count--) {
		if ((bind = smpBuildBind0 (list)) == (AsnIdType) 0) {
			status = asnStatusBad;
		}
		else {
			status = asnAppend (result, bind);
			bind = asnFree (bind);
		}
		list++;
	}

	if (status != asnStatusOk) {
		result = asnFree (result);
	}
	return (result);
}

#ifndef		SERVER

static	AsnIdType	smpBuildReq (AsnTagType tag, SmpRequestPtrType req, AsnIdType list)
{
	AsnIdType		result;
	AsnIdType		reqid;

	if ((reqid = asnUnsl (asnClassUniversal, (AsnTagType) 2,
		(CUnslType) req->smpRequestId)) == (AsnIdType) 0) {
		result = (AsnIdType) 0;
	}
	else {
		result = smpBuildMsg (tag, reqid, req->smpRequestError,
			req->smpRequestIndex, list);
		reqid = asnFree (reqid);
	}
	return (result);
}

#else		/*	SERVER	*/

#define		smpBuildReq(tag, req, list)	((AsnIdType) 0)

#endif		/*	SERVER	*/

#ifndef		CLIENT

static	AsnIdType	smpBuildTrap (SmpRequestPtrType req, AsnIdType list)
{
	AsnIdType		result;
	AsnIdType		value;

	if ((result = asnSequence (asnClassContext, (AsnTagType) 4,
		asnTypeSequence)) == (AsnIdType) 0) {
		return ((AsnIdType) 0);
	}
	if ((value = asnObjectId (asnClassUniversal, (AsnTagType) 6,
		(CBytePtrType) req->smpRequestEnterprise,
		(AsnLengthType) req->smpRequestEnterpriseLen)) ==
		(AsnIdType) 0) {
		return (asnFree (result));
	}
	if (asnAppend (result, value) != asnStatusOk) {
		value = asnFree (value);
		return (asnFree (result));
	}
	value = asnFree (value);

	if ((value = asnOctetString (asnClassApplication, (AsnTagType) 0,
		(CBytePtrType) req->smpRequestAgent,
		(AsnLengthType) req->smpRequestAgentLen)) ==
		(AsnIdType) 0) {
		return (asnFree (result));
	}
	if (asnAppend (result, value) != asnStatusOk) {
		value = asnFree (value);
		return (asnFree (result));
	}
	value = asnFree (value);

	if (smpSishKabob (result, asnClassUniversal, (AsnTagType) 2,
		(CUnslType) req->smpRequestGenericTrap) != asnStatusOk) {
		result = asnFree (result);
	}
	else if (smpSishKabob (result, asnClassUniversal, (AsnTagType) 2,
		(CUnslType) req->smpRequestSpecificTrap) != asnStatusOk) {
		result = asnFree (result);
	}
	else if (smpSishKabob (result, asnClassApplication, (AsnTagType) 3,
		(CUnslType) req->smpRequestTimeStamp) != asnStatusOk) {
		result = asnFree (result);
	}
	else if (asnAppend (result, list) != asnStatusOk) {
		result = asnFree (result);
	}

	return (result);
}

static	AsnIdType	smpBuildTrapV2 (SmpRequestPtrType req, AsnIdType list)
{
	AsnIdType		result;
	AsnIdType		value;

	if ((result = asnSequence (asnClassContext, (AsnTagType) 4,
		asnTypeSequence)) == (AsnIdType) 0) {
		return ((AsnIdType) 0);
	}
	if ((value = asnObjectId (asnClassUniversal, (AsnTagType) 6,
		(CBytePtrType) req->smpRequestEnterprise,
		(AsnLengthType) req->smpRequestEnterpriseLen)) ==
		(AsnIdType) 0) {
		return (asnFree (result));
	}
	if (asnAppend (result, value) != asnStatusOk) {
		value = asnFree (value);
		return (asnFree (result));
	}
	value = asnFree (value);

	if ((value = asnOctetString (asnClassApplication, (AsnTagType) 0,
		(CBytePtrType) req->smpRequestAgent,
		(AsnLengthType) req->smpRequestAgentLen)) ==
		(AsnIdType) 0) {
		return (asnFree (result));
	}
	if (asnAppend (result, value) != asnStatusOk) {
		value = asnFree (value);
		return (asnFree (result));
	}
	value = asnFree (value);

	if (smpSishKabob (result, asnClassUniversal, (AsnTagType) 2,
		(CUnslType) req->smpRequestGenericTrap) != asnStatusOk) {
		result = asnFree (result);
	}
	else if (smpSishKabob (result, asnClassUniversal, (AsnTagType) 2,
		(CUnslType) req->smpRequestSpecificTrap) != asnStatusOk) {
		result = asnFree (result);
	}
	else if (smpSishKabob (result, asnClassApplication, (AsnTagType) 3,
		(CUnslType) req->smpRequestTimeStamp) != asnStatusOk) {
		result = asnFree (result);
	}
	else if (asnAppend (result, list) != asnStatusOk) {
		result = asnFree (result);
	}

	return (result);
}

#else		/*	CLIENT	*/

#define		smpBuildTrap(req, list)		((AsnIdType) 0)
#define		smpBuildTrapV2(req, list)		((AsnIdType) 0)

#endif		/*	CLIENT	*/

//hrchen 4/9/04', for MIB value convert to smp bind format, called by trap
SmpStatusType cvtAsnToBind(AsnIdType item, SmpBindPtrType bind)
{

	if ((item==(AsnIdType)0)||(bind==(SmpBindPtrType)0))
	    return errBad;
	
	bind->smpBindValueLen = asnLengthDef(item);
	bind->smpBindValue = asnValueDef(item);   
	switch (asnTypeDef(item)) {
	    case asnTypeInteger:
	    	bind->smpBindKind = smpKindInteger;
	        break;
	    case asnTypeOctetString:
	    	bind->smpBindKind = smpKindOctetString;
	        break;
	    case asnTypeObjectId:
	    	bind->smpBindKind = smpKindObjectId;
	        break;
	    case asnTypeNull:
	    	bind->smpBindKind = smpKindNull;
	        break;
	    default:
	    	bind->smpBindKind = smpKindNone;
	        break;
	};
}

SmpStatusType		smpRequest (SmpIdType smp, SmpRequestPtrType req)
{
	AsnIdType		result;
	AsnIdType		list;
	SmpStatusType		status;

	DEBUG0 ("smpRequest:\n");
	if (smp == (SmpIdType) 0) {
		return (errBad);
	}
	else if (req == (SmpRequestPtrType) 0) {
		return (errBad);
	}
	else if ((list = smpBuildList (req->smpRequestBinds,
		req->smpRequestCount)) == (AsnIdType) 0) {
		return (errBad);
	}
	else {
		switch (req->smpRequestCmd) {
/*hrchen 4/8/04', no use in Agent mode
		case smpCommandGet:
			result = smpBuildReq ((AsnTagType) 0, req, list);
			break;

		case smpCommandNext:
			result = smpBuildReq ((AsnTagType) 1, req, list);
			break;

		case smpCommandSet:
			result = smpBuildReq ((AsnTagType) 3, req, list);
			break;

		case smpCommandRsp:
			result = smpBuildReq ((AsnTagType) 2, req, list);
			break;
*/
		case smpCommandTrap:
			result = smpBuildTrap (req, list);
			break;
		case smpCommandTrapV2:
			result = smpBuildTrapV2 (req, list);
			break;

		default:
			result = (AsnIdType) 0;
			break;
		}
		list = asnFree (list);
		DEBUG1 ("result: %08.08X\n", result);
		if (result == (AsnIdType) 0) {
			return (errBad);
		}
		else {
			status = smpSend (smp,
				req->smpRequestCommunity, result);
			result = asnFree (result);
			return (status);
		}
	}
}

