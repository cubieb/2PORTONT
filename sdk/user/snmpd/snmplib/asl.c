
#include	"ctypes.h"
#include	"debug.h"
#include	"asn.h"
#include	"asl.h"
#include	"asldefs.h"

#define		aslNodeNil	((AslIdType) 0)

#define		aslNode(n)	(aslPtrToId (& aslNodes [ (n) ]))

#define		aslNodeDef(t,s,n,v)	\
		{ \
		(t), \
		(CUnswType) (s), \
		(AsnLengthType) (n), \
		(v) \
		}
static	AslNodeType	aslNodes []	=	{

/*	0	*/
		aslNodeDef (asnTypeNone, "\60", 20, aslNode (1)),
/*	1	*/
		aslNodeDef (asnTypeSequence, aslNode (2), 20, aslNodeNil),
/*	2	*/
		aslNodeDef (asnTypeNone, "\2", 18, aslNode (3)),
/*	3	*/
		aslNodeDef (asnTypeInteger, aslNodeNil, 1, aslNode (4)),
/*	4	*/
		aslNodeDef (asnTypeNone, "\4", 15, aslNode (5)),
/*	5	*/
		aslNodeDef (asnTypeOctetString, aslNodeNil, 0, aslNode (6)),
/*	6	*/  //for v2c
		aslNodeDef (asnTypeNone, "\240\241\242\243\244\245\246\247", 13,
			aslNode (7)),
/*	7	*/  //0 get
		aslNodeDef (asnTypeSequence, aslNode (15), 12, aslNodeNil),//12), 12, aslNodeNil),
/*	8	*/  //1 get-next
		aslNodeDef (asnTypeSequence, aslNode (15), 12, aslNodeNil),//12), 12, aslNodeNil),
/*	9	*/  //2 responce
		aslNodeDef (asnTypeSequence, aslNode (15), 12, aslNodeNil),//12), 12, aslNodeNil),
/*	10	*/  //3 set
		aslNodeDef (asnTypeSequence, aslNode (15), 12, aslNodeNil),//12), 12, aslNodeNil),
/*	11	*/  //4 trap
		aslNodeDef (asnTypeSequence, aslNode (38), 16, aslNodeNil),//35), 16, aslNodeNil),
/*	12	*/  //5 set
		aslNodeDef (asnTypeSequence, aslNode (15), 12, aslNodeNil),//12), 12, aslNodeNil),
/*	13	*/  //6 inform
		aslNodeDef (asnTypeSequence, aslNode (15), 12, aslNodeNil),//12), 12, aslNodeNil),
/*	14	*/  //7 trap v2
		aslNodeDef (asnTypeSequence, aslNode (38), 16, aslNodeNil),//35), 12, aslNodeNil),
		
/*	12/15	*/
		aslNodeDef (asnTypeNone, "\2", 11, aslNode (16)),//13)),
/*	13/16	*/
		aslNodeDef (asnTypeInteger, aslNodeNil, 1, aslNode (17)),//14)),
/*	14/17	*/
		aslNodeDef (asnTypeNone, "\2", 8, aslNode (18)),//15)),
/*	15/18	*/
		aslNodeDef (asnTypeInteger, aslNodeNil, 1, aslNode (19)),//16)),
/*	16/19	*/
		aslNodeDef (asnTypeNone, "\2", 5, aslNode (20)),//17)),
/*	17/20	*/
		aslNodeDef (asnTypeInteger, aslNodeNil, 1, aslNode (21)),//18)),
/*	18/21	*/
		aslNodeDef (asnTypeNone, "\60", 2, aslNode (22)),//19)),
/*	19/22	*/
		aslNodeDef (asnTypeSequenceOf, aslNode (23), 0, aslNodeNil),//20), 0, aslNodeNil),
/*	20/23	*/
		aslNodeDef (asnTypeNone, "\60", 0, aslNode (24)),//21)),
/*	21/24	*/
		aslNodeDef (asnTypeSequence, aslNode (25), 4, aslNode (23)),//22), 4, aslNode (20)),
/*	22/25	*/
		aslNodeDef (asnTypeNone, "\6", 4, aslNode (26)),//23)),
/*	23/26	*/
		aslNodeDef (asnTypeObjectId, aslNodeNil, 0, aslNode (27)),//24)),
/*	24/27	*/
		aslNodeDef (asnTypeNone, "\2\4\5\6\100\101\102\103\104\44",
			2, aslNode (28)),//25)),
/*	25/28	*/
		aslNodeDef (asnTypeInteger, aslNodeNil, 1, aslNodeNil),
/*	26/29	*/
		aslNodeDef (asnTypeOctetString, aslNodeNil, 0, aslNodeNil),
/*	27/30	*/
		aslNodeDef (asnTypeNull, aslNodeNil, 0, aslNodeNil),
/*	28/31	*/
		aslNodeDef (asnTypeObjectId, aslNodeNil, 0, aslNodeNil),
/*	29/32	*/
		aslNodeDef (asnTypeOctetString, aslNodeNil, 4, aslNodeNil),
/*	30/33	*/
		aslNodeDef (asnTypeInteger, aslNodeNil, 1, aslNodeNil),
/*	31/34	*/
		aslNodeDef (asnTypeInteger, aslNodeNil, 1, aslNodeNil),
/*	32/35	*/
		aslNodeDef (asnTypeInteger, aslNodeNil, 0, aslNodeNil),
/*	33/36	*/
		aslNodeDef (asnTypeOctetString, aslNodeNil, 0, aslNodeNil),
/*	34/37	*/
		aslNodeDef (asnTypeSequenceOf, aslNode (48), 0, aslNodeNil),//45), 0, aslNodeNil),
/*	35/38	*/
		aslNodeDef (asnTypeNone, "\6", 14, aslNode (39)),//36)),
/*	36/39	*/
		aslNodeDef (asnTypeObjectId, aslNodeNil, 0, aslNode (40)),//37)),
/*	37/40	*/
		aslNodeDef (asnTypeNone, "\100", 12, aslNode (41)),//38)),
/*	38/41	*/
		aslNodeDef (asnTypeOctetString, aslNodeNil, 4, aslNode (42)),//39)),
/*	39/42	*/
		aslNodeDef (asnTypeNone, "\2", 10, aslNode (43)),//40)),
/*	40/43	*/
		aslNodeDef (asnTypeInteger, aslNodeNil, 1, aslNode (44)),//41)),
/*	41/44	*/
		aslNodeDef (asnTypeNone, "\2", 7, aslNode (45)),//42)),
/*	42/45	*/
		aslNodeDef (asnTypeInteger, aslNodeNil, 1, aslNode (46)),//43)),
/*	43/46	*/
		aslNodeDef (asnTypeNone, "\103", 4, aslNode (47)),//44)),
/*	44/47	*/
		aslNodeDef (asnTypeInteger, aslNodeNil, 1, aslNode (21)),//18)),
/*	45/48	*/
		aslNodeDef (asnTypeNone, "\4\44", 0, aslNode (49)),//46)),
/*	46/49	*/
		aslNodeDef (asnTypeOctetString, aslNodeNil, 0, aslNodeNil),
/*	47/50	*/
		aslNodeDef (asnTypeSequenceOf, aslNode (48), 0, aslNodeNil),//45), 0, aslNodeNil),

		};

CVoidType		aslInit (void)
{
}

AslIdType  		aslChoice (AslIdType n, CByteType x)
{
        CIntfType               i;
        CBytePtrType            cp;

	DEBUG0 ("aslChoice ");
	cp = (CBytePtrType) ((aslIdToPtr(n))->aslNodeStuff);
	if (x == 0) {
		DEBUG0 ("0\n");
        	return ((*cp != x) ? (AslIdType) 0 :
			aslPtrToId ((aslIdToPtr
			((aslIdToPtr (n))->aslNodeNext))));
	}
	else {
		DEBUG0 ("1\n");
		i = 0;
		while ((*cp) && (*cp != x)) {
			cp++;
			i++;
		}
        	return ((*cp) ? aslPtrToId ((aslIdToPtr
			((aslIdToPtr (n))->aslNodeNext)) + i) : (AslIdType) 0);
	}
}

AslIdType  		aslAny (AslIdType n, CByteType x)
{
	AslNodePtrType		np;


	np = aslIdToPtr (n);
        if (x < 9) {
                np += (int) x;
        }
        else if (x == 0x24) {
                np += 10;
        }
        else if (x == 0x25) {
                np += 11;
        }
        else if (x > 0x20) {
                np += 12;
        }
        else {
                np += 4;
        }

	return (aslPtrToId (np));
}

AslIdType	aslLanguage (AsnLanguageType language)
{
	language = language;
	return (aslNode (0));
}

#ifndef		INLINE

AslIdType	aslSon (AslIdType n)
{
	return (aslSonDef (n));
}


AslIdType	aslNext (AslIdType n)
{
	return (aslNextDef (n));
}


AsnLengthType	aslMinLen (AslIdType n)
{
	return (aslMinLenDef (n));
}


AsnTypeType	aslKind (AslIdType n)
{
	return (aslKindDef (n));
}

#endif		/*	INLINE	*/

