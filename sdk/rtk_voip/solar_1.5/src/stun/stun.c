#include <errno.h>
#include <assert.h>

#ifdef WIN32
#include <winsock2.h>
#include <stdlib.h>
#include <io.h>
#include <time.h>
#else

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h> 
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <net/if.h>

#endif


#if defined(__sparc__) || defined(WIN32)
#define NOSSL
#endif
#define NOSSL

#include "stun_udp.h"
#include "stun.h"
#include "stun_api.h"


#define false 0
#define true 1

extern int EXOSIP_FD;
extern int EXOSIP_SipPort;

int STUN_PACKETS_RESEND;

static void
computeHmac(char* hmac, const char* input, int length, const char* key, int keySize);

#define result (*presult)
static int 
stunParseAtrAddress( char* body, unsigned int hdrLen,  StunAtrAddress4 *presult )
{
   if ( hdrLen != 8 )
   {
      stun_message( "hdrLen wrong for Address" );
      return false;
   }
   result.pad = *body++;
   result.family = *body++;
   if (result.family == IPv4Family)
   {
      UInt16 nport;
      UInt32 naddr;

      memcpy(&nport, body, 2); body+=2;
      result.ipv4.port = ntohs(nport);
		
      memcpy(&naddr, body, 4); body+=4;
      result.ipv4.addr = ntohl(naddr);
      return true;
   }
   else if (result.family == IPv6Family)
   {
      stun_message( "ipv6 not supported");
   }
   else
   {
      stun_message( "bad address family: %d", result.family );
   }
	
   return false;
}

static int 
stunParseAtrChangeRequest( char* body, unsigned int hdrLen,  StunAtrChangeRequest *presult )
{
   if ( hdrLen != 4 )
   {
      stun_message( "hdr length = %d expecting %d", hdrLen, sizeof(result) );
		
      stun_message( "Incorrect size for ChangeRequest");
      return false;
   }
   else
   {
      memcpy(&result.value, body, 4);
      result.value = ntohl(result.value);
      return true;
   }
}

static int 
stunParseAtrError( char* body, unsigned int hdrLen,  StunAtrError *presult )
{
   if ( hdrLen >= sizeof(result) )
   {
      stun_message( "head on Error too large");
      return false;
   }
   else
   {
      memcpy(&result.pad, body, 2); body+=2;
      result.pad = ntohs(result.pad);
      result.errorClass = *body++;
      result.number = *body++;
		
      result.sizeReason = hdrLen - 4;
      memcpy(&result.reason, body, result.sizeReason);
      result.reason[result.sizeReason] = 0;
      return true;
   }
}

static int 
stunParseAtrUnknown( char* body, unsigned int hdrLen,  StunAtrUnknown *presult )
{
   if ( hdrLen >= sizeof(result) )
   {
      return false;
   }
   else
   {
      int i;
      if (hdrLen % 4 != 0) return false;
      result.numAttributes = hdrLen / 4;
      for (i=0; i<result.numAttributes; i++)
      {
         memcpy(&result.attrType[i], body, 2); body+=2;
         result.attrType[i] = ntohs(result.attrType[i]);
      }
      return true;
   }
}


static int 
stunParseAtrString( char* body, unsigned int hdrLen,  StunAtrString *presult )
{
   if ( hdrLen >= STUN_MAX_STRING )
   {
      stun_message( "String is too large");
      return false;
   }
   else
   {
      if (hdrLen % 4 != 0)
      {
         stun_message( "Bad length string %d", hdrLen );
         return false;
      }
		
      result.sizeValue = hdrLen;
      memcpy(&result.value, body, hdrLen);
      result.value[hdrLen] = 0;
      return true;
   }
}


static int 
stunParseAtrIntegrity( char* body, unsigned int hdrLen,  StunAtrIntegrity *presult )
{
   if ( hdrLen != 20)
   {
      stun_message( "MessageIntegrity must be 20 bytes");
      return false;
   }
   else
   {
      memcpy(&result.hash, body, hdrLen);
      return true;
   }
}


#define msg (*pmsg)
int
stunParseMessage( char* buf, unsigned int bufLen, StunMessage *pmsg, int verbose)
{
   char abuf[64];
   char* body = buf + sizeof(StunMsgHdr);
   unsigned int size;

   if (verbose) stun_message( "Received stun message: %d bytes", bufLen );
   memset(&msg, 0, sizeof(msg));
	
   if (sizeof(StunMsgHdr) > bufLen)
   {
      stun_message( "Bad message");
      return false;
   }
	
   memcpy(&msg.msgHdr, buf, sizeof(StunMsgHdr));
   msg.msgHdr.msgType = ntohs(msg.msgHdr.msgType);
   msg.msgHdr.msgLength = ntohs(msg.msgHdr.msgLength);
	
   if (msg.msgHdr.msgLength + sizeof(StunMsgHdr) != bufLen)
   {
      stun_message( "Message header length doesn't match message size: %d - %d", msg.msgHdr.msgLength, bufLen );
      return false;
   }
	
   size = msg.msgHdr.msgLength;
	
   //stun_message( "bytes after header = " << size );
	
   while ( size > 0 )
   {
      // !jf! should check that there are enough bytes left in the buffer
		
      //StunAtrHdr* attr = reinterpret_cast<StunAtrHdr*>(body);
	  StunAtrHdr* attr = (StunAtrHdr*)(body);
		
      unsigned int attrLen = ntohs(attr->length);
      int atrType = ntohs(attr->type);
		
      //if (verbose) stun_message( "Found attribute type=" << AttrNames[atrType] << " length=" << attrLen );
      if ( attrLen+4 > size ) 
      {
         stun_message( "claims attribute is larger than size of message (attribute type=%d)", atrType );
         return false;
      }
		
      body += 4; // skip the length and type in attribute header
      size -= 4;
		
      switch ( atrType )
      {
         case MappedAddress:
            msg.hasMappedAddress = true;
            if ( stunParseAtrAddress(  body,  attrLen,  &msg.mappedAddress )== false )
            {
               stun_message( "problem parsing MappedAddress");
               return false;
            }
            else
            {
               if (verbose) stun_message( "MappedAddress = %s", ipv4tostr(abuf, msg.mappedAddress.ipv4) );
            }
					
            break;  

         case ResponseAddress:
            msg.hasResponseAddress = true;
            if ( stunParseAtrAddress(  body,  attrLen,  &msg.responseAddress )== false )
            {
               stun_message( "problem parsing ResponseAddress");
               return false;
            }
            else
            {
               if (verbose) stun_message( "ResponseAddress = %s", ipv4tostr(abuf, msg.responseAddress.ipv4) );
            }
            break;  
				
         case ChangeRequest:
            msg.hasChangeRequest = true;
            if (stunParseAtrChangeRequest( body, attrLen, &msg.changeRequest) == false)
            {
               stun_message( "problem parsing ChangeRequest");
               return false;
            }
            else
            {
               if (verbose) stun_message( "ChangeRequest = %d", msg.changeRequest.value );
            }
            break;
				
         case SourceAddress:
            msg.hasSourceAddress = true;
            if ( stunParseAtrAddress(  body,  attrLen,  &msg.sourceAddress )== false )
            {
               stun_message( "problem parsing SourceAddress");
               return false;
            }
            else
            {
               if (verbose) stun_message( "SourceAddress = %s", ipv4tostr(abuf, msg.sourceAddress.ipv4) );
            }
            break;  
				
         case ChangedAddress:
            msg.hasChangedAddress = true;
            if ( stunParseAtrAddress(  body,  attrLen,  &msg.changedAddress )== false )
            {
               stun_message( "problem parsing ChangedAddress");
               return false;
            }
            else
            {
               if (verbose) stun_message( "ChangedAddress = %s", ipv4tostr(abuf, msg.changedAddress.ipv4) );
            }
            break;  
				
         case Username: 
            msg.hasUsername = true;
            if (stunParseAtrString( body, attrLen, &msg.username) == false)
            {
               stun_message( "problem parsing Username");
               return false;
            }
            else
            {
               if (verbose) stun_message( "Username = %s", msg.username.value );
            }
					
            break;
				
         case Password: 
            msg.hasPassword = true;
            if (stunParseAtrString( body, attrLen, &msg.password) == false)
            {
               stun_message( "problem parsing Password");
               return false;
            }
            else
            {
               if (verbose) stun_message( "Password = %s", msg.password.value );
            }
            break;
				
         case MessageIntegrity:
            msg.hasMessageIntegrity = true;
            if (stunParseAtrIntegrity( body, attrLen, &msg.messageIntegrity) == false)
            {
               stun_message( "problem parsing MessageIntegrity");
               return false;
            }
            else
            {
               //if (verbose) stun_message( "MessageIntegrity = " << msg.messageIntegrity.hash );
            }
					
            // read the current HMAC
            // look up the password given the user of given the transaction id 
            // compute the HMAC on the buffer
            // decide if they match or not
            break;
				
         case ErrorCode:
            msg.hasErrorCode = true;
            if (stunParseAtrError(body, attrLen, &msg.errorCode) == false)
            {
               stun_message( "problem parsing ErrorCode");
               return false;
            }
            else
            {
               if (verbose) stun_message( "ErrorCode = %d %d %s", msg.errorCode.errorClass,
                                     msg.errorCode.number,
                                     msg.errorCode.reason );
            }
					
            break;
				
         case UnknownAttribute:
            msg.hasUnknownAttributes = true;
            if (stunParseAtrUnknown(body, attrLen, &msg.unknownAttributes) == false)
            {
               stun_message( "problem parsing UnknownAttribute");
               return false;
            }
            break;
				
         case ReflectedFrom:
            msg.hasReflectedFrom = true;
            if ( stunParseAtrAddress(  body,  attrLen,  &msg.reflectedFrom ) == false )
            {
               stun_message( "problem parsing ReflectedFrom");
               return false;
            }
            break;  
				
         case XorMappedAddress:
            msg.hasXorMappedAddress = true;
            if ( stunParseAtrAddress(  body,  attrLen,  &msg.xorMappedAddress ) == false )
            {
               stun_message( "problem parsing XorMappedAddress");
               return false;
            }
            else
            {
               if (verbose) stun_message( "XorMappedAddress = %s", ipv4tostr(abuf, msg.mappedAddress.ipv4) );
            }
            break;  

         case XorOnly:
            msg.xorOnly = true;
            if (verbose) 
            {
               stun_message( "xorOnly = true");
            }
            break;  
				
         case ServerName: 
            msg.hasServerName = true;
            if (stunParseAtrString( body, attrLen, &msg.serverName) == false)
            {
               stun_message( "problem parsing ServerName");
               return false;
            }
            else
            {
               if (verbose) stun_message( "ServerName = %s", msg.serverName.value );
            }
            break;
				
         case SecondaryAddress:
            msg.hasSecondaryAddress = true;
            if ( stunParseAtrAddress(  body,  attrLen,  &msg.secondaryAddress ) == false )
            {
               stun_message( "problem parsing secondaryAddress");
               return false;
            }
            else
            {
               if (verbose) stun_message( "SecondaryAddress = %s", ipv4tostr(abuf, msg.secondaryAddress.ipv4) );
            }
            break;  
					
         default:
            if (verbose) stun_message( "Unknown attribute: %d", atrType );
            if ( atrType <= 0x7FFF ) 
            {
               return false;
            }
      }
		
      body += attrLen;
      size -= attrLen;
   }
    
   return true;
}
#undef msg


static char* 
encode16(char* buf, UInt16 data)
{
   UInt16 ndata = htons(data);
   //memcpy(buf, reinterpret_cast<void*>(&ndata), sizeof(UInt16));
   memcpy(buf, &ndata, sizeof(UInt16));
   return buf + sizeof(UInt16);
}

static char* 
encode32(char* buf, UInt32 data)
{
   UInt32 ndata = htonl(data);
   //memcpy(buf, reinterpret_cast<void*>(&ndata), sizeof(UInt32));
   memcpy(buf, &ndata, sizeof(UInt32));
   return buf + sizeof(UInt32);
}


static char* 
encode(char* buf, const char* data, unsigned int length)
{
   memcpy(buf, data, length);
   return buf + length;
}


static char* 
encodeAtrAddress4(char* ptr, UInt16 type, const StunAtrAddress4 atr)
{
   ptr = encode16(ptr, type);
   ptr = encode16(ptr, 8);
   *ptr++ = atr.pad;
   *ptr++ = IPv4Family;
   ptr = encode16(ptr, atr.ipv4.port);
   ptr = encode32(ptr, atr.ipv4.addr);
	
   return ptr;
}

static char* 
encodeAtrChangeRequest(char* ptr, const StunAtrChangeRequest atr)
{
   ptr = encode16(ptr, ChangeRequest);
   ptr = encode16(ptr, 4);
   ptr = encode32(ptr, atr.value);
   return ptr;
}

static char* 
encodeAtrError(char* ptr, const StunAtrError atr)
{
   ptr = encode16(ptr, ErrorCode);
   ptr = encode16(ptr, 6 + atr.sizeReason);
   ptr = encode16(ptr, atr.pad);
   *ptr++ = atr.errorClass;
   *ptr++ = atr.number;
   ptr = encode(ptr, atr.reason, atr.sizeReason);
   return ptr;
}


static char* 
encodeAtrUnknown(char* ptr, const StunAtrUnknown atr)
{
   int i;
   ptr = encode16(ptr, UnknownAttribute);
   ptr = encode16(ptr, 2+2*atr.numAttributes);
   for (i=0; i<atr.numAttributes; i++)
   {
      ptr = encode16(ptr, atr.attrType[i]);
   }
   return ptr;
}


static char* 
encodeXorOnly(char* ptr)
{
   ptr = encode16(ptr, XorOnly );
   return ptr;
}


static char* 
encodeAtrString(char* ptr, UInt16 type, const StunAtrString atr)
{
   assert(atr.sizeValue % 4 == 0);
	
   ptr = encode16(ptr, type);
   ptr = encode16(ptr, atr.sizeValue);
   ptr = encode(ptr, atr.value, atr.sizeValue);
   return ptr;
}


static char* 
encodeAtrIntegrity(char* ptr, const StunAtrIntegrity atr)
{
   ptr = encode16(ptr, MessageIntegrity);
   ptr = encode16(ptr, 20);
   ptr = encode(ptr, atr.hash, sizeof(atr.hash));
   return ptr;
}


unsigned int
stunEncodeMessage( const StunMessage msg, 
                   char* buf, 
                   unsigned int bufLen, 
                   const StunAtrString password, 
                   int verbose)
{
   char* ptr = buf;
   char abuf[64];
   char* lengthp;
   assert(bufLen >= sizeof(StunMsgHdr));
	
   ptr = encode16(ptr, msg.msgHdr.msgType);
   lengthp = ptr;
   ptr = encode16(ptr, 0);
   ptr = encode(ptr, (const char*)(msg.msgHdr.id.octet), sizeof(msg.msgHdr.id));
	
   if (verbose) stun_message( "Encoding stun message: ");
   if (msg.hasMappedAddress)
   {
      if (verbose) stun_message( "Encoding MappedAddress: %s", ipv4tostr(abuf, msg.mappedAddress.ipv4) );
      ptr = encodeAtrAddress4 (ptr, MappedAddress, msg.mappedAddress);
   }
   if (msg.hasResponseAddress)
   {
      if (verbose) stun_message( "Encoding ResponseAddress: %s", ipv4tostr(abuf, msg.responseAddress.ipv4) );
      ptr = encodeAtrAddress4(ptr, ResponseAddress, msg.responseAddress);
   }
   if (msg.hasChangeRequest)
   {
      if (verbose) stun_message( "Encoding ChangeRequest: %d", msg.changeRequest.value );
      ptr = encodeAtrChangeRequest(ptr, msg.changeRequest);
   }
   if (msg.hasSourceAddress)
   {
      if (verbose) stun_message( "Encoding SourceAddress: %s", ipv4tostr(abuf, msg.sourceAddress.ipv4) );
      ptr = encodeAtrAddress4(ptr, SourceAddress, msg.sourceAddress);
   }
   if (msg.hasChangedAddress)
   {
      if (verbose) stun_message( "Encoding ChangedAddress: %s", ipv4tostr(abuf, msg.changedAddress.ipv4) );
      ptr = encodeAtrAddress4(ptr, ChangedAddress, msg.changedAddress);
   }
   if (msg.hasUsername)
   {
      if (verbose) stun_message( "Encoding Username: %s", msg.username.value );
      ptr = encodeAtrString(ptr, Username, msg.username);
   }
   if (msg.hasPassword)
   {
      if (verbose) stun_message( "Encoding Password: %s", msg.password.value );
      ptr = encodeAtrString(ptr, Password, msg.password);
   }
   if (msg.hasErrorCode)
   {
      if (verbose) stun_message( "Encoding ErrorCode: class= %d number=%d reason=%s",
							msg.errorCode.errorClass, 
							msg.errorCode.number,
							msg.errorCode.reason);
		
      ptr = encodeAtrError(ptr, msg.errorCode);
   }
   if (msg.hasUnknownAttributes)
   {
      if (verbose) stun_message( "Encoding UnknownAttribute: ???");
      ptr = encodeAtrUnknown(ptr, msg.unknownAttributes);
   }
   if (msg.hasReflectedFrom)
   {
      if (verbose) stun_message( "Encoding ReflectedFrom: %s", ipv4tostr(abuf, msg.reflectedFrom.ipv4) );
      ptr = encodeAtrAddress4(ptr, ReflectedFrom, msg.reflectedFrom);
   }
   if (msg.hasXorMappedAddress)
   {
      if (verbose) stun_message( "Encoding XorMappedAddress: %s", ipv4tostr(abuf, msg.xorMappedAddress.ipv4) );
      ptr = encodeAtrAddress4 (ptr, XorMappedAddress, msg.xorMappedAddress);
   }
   if (msg.xorOnly)
   {
      if (verbose) stun_message( "Encoding xorOnly: ");
      ptr = encodeXorOnly( ptr );
   }
   if (msg.hasServerName)
   {
      if (verbose) stun_message( "Encoding ServerName: %s", msg.serverName.value );
      ptr = encodeAtrString(ptr, ServerName, msg.serverName);
   }
   if (msg.hasSecondaryAddress)
   {
      if (verbose) stun_message( "Encoding SecondaryAddress: %s", ipv4tostr(abuf, msg.secondaryAddress.ipv4) );
      ptr = encodeAtrAddress4 (ptr, SecondaryAddress, msg.secondaryAddress);
   }

   if (password.sizeValue > 0)
   {
      StunAtrIntegrity integrity;
      if (verbose) stun_message( "HMAC with password: %s", password.value );
		
      computeHmac(integrity.hash, buf, ptr-buf , password.value, password.sizeValue);
      ptr = encodeAtrIntegrity(ptr, integrity);
   }
   if (verbose)      
	  {
         printf("\n");
      }

	
   encode16(lengthp, (ptr - buf - sizeof(StunMsgHdr)));
   return (ptr - buf);
}

int 
stunRand(void)
{
   static int init=false;
   // return 32 bits of random stuff
   assert( sizeof(int) == 4 );
   if ( !init )
   { 
      UInt64 tick;
	  int seed;
      init = true;
		
		
#if defined(WIN32) 
      volatile unsigned int lowtick=0,hightick=0;
      __asm
         {
            rdtsc 
               mov lowtick, eax
               mov hightick, edx
               }
      tick = hightick;
      tick <<= 32;
      tick |= lowtick;
#elif defined(__GNUC__) && ( defined(__i686__) || defined(__i386__) )
      asm("rdtsc" : "=A" (tick));
#elif defined (__SUNPRO_CC) || defined( __sparc__ )	
      tick = gethrtime();
#elif defined(__MACH__) || 1
      int fd=open("/dev/urandom",O_RDONLY);
      read(fd,&tick,sizeof(tick));
      if (fd != EXOSIP_FD) {
      closesocket(fd);
   	}
#else
#     error Need some way to seed the random number generator 
#endif 
      seed = (tick);
#ifdef WIN32
      srand(seed);
#else
      srandom(seed);
#endif
   }
	
#ifdef WIN32
   assert( RAND_MAX == 0x7fff );
   int r1 = rand();
   int r2 = rand();
	
   int ret = (r1<<16) + r2;
	
   return ret;
#else
   return random(); 
#endif
}


/// return a random number to use as a port 
int
stunRandomPort(void)
{
   int min=0x4000;
   int max=0x7FFF;
	
   int ret = stunRand();
   ret = ret|min;
   ret = ret&max;
	
   return ret;
}


#ifdef NOSSL
static void
computeHmac(char* hmac, const char* input, int length, const char* key, int sizeKey)
{
   strncpy(hmac,"hmac-not-implemented",20);
}
#else
#include <openssl/hmac.h>

static void
computeHmac(char* hmac, const char* input, int length, const char* key, int sizeKey)
{
   unsigned int resultSize=0;
   HMAC(EVP_sha1(), 
        key, sizeKey, 
        reinterpret_cast<const unsigned char*>(input), length, 
        reinterpret_cast<unsigned char*>(hmac), &resultSize);
   assert(resultSize == 20);
}
#endif


static void
toHex(const char* buffer, int bufferSize, char* output) 
{
   static char hexmap[] = "0123456789abcdef";
   int i;
	
   const char* p = buffer;
   char* r = output;
   for (i=0; i < bufferSize; i++)
   {
      unsigned char temp = *p++;
		
      int hi = (temp & 0xf0)>>4;
      int low = (temp & 0xf);
		
      *r++ = hexmap[hi];
      *r++ = hexmap[low];
   }
   *r = 0;
}

void
stunCreateUserName(const StunAddress4 source, StunAtrString* username)
{
   UInt64 time = stunGetSystemTimeSecs();
   UInt64 lotime = time & 0xFFFFFFFF;
   char buffer[1024];
   char hmac[20];
   char key[] = "Jason";
   char hmacHex[41];
   int l;
   time -= (time % 20*60);
   //UInt64 hitime = time >> 32;
   lotime = time & 0xFFFFFFFF;
	
   sprintf(buffer,
           "%08x:%08x:%llu:", 
           (source.addr),
           (stunRand()),
           (lotime));
   assert( strlen(buffer) < 1024 );
	
   assert(strlen(buffer) + 41 < STUN_MAX_STRING);
	
   computeHmac(hmac, buffer, strlen(buffer), key, strlen(key) );
   toHex(hmac, 20, hmacHex );
   hmacHex[40] =0;
	
   strcat(buffer,hmacHex);
	
   l = strlen(buffer);
   assert( l+1 < STUN_MAX_STRING );
   assert( l%4 == 0 );
   
   username->sizeValue = l;
   memcpy(username->value,buffer,l);
   username->value[l]=0;
	
   //if (verbose) stun_message( "computed username=" << username.value );
}

void
stunCreatePassword(const StunAtrString username, StunAtrString* password)
{
   char hmac[20];
   char key[] = "Fluffy";
   //char buffer[STUN_MAX_STRING];
   computeHmac(hmac, username.value, strlen(username.value), key, strlen(key));
   toHex(hmac, 20, password->value);
   password->sizeValue = 40;
   password->value[40]=0;
	
   //stun_message( "password=" << password->value );
}


UInt64
stunGetSystemTimeSecs(void)
{
   UInt64 time=0;
#if defined(WIN32)  
   SYSTEMTIME t;
   // CJ TODO - this probably has bug on wrap around every 24 hours
   GetSystemTime( &t );
   time = (t.wHour*60+t.wMinute)*60+t.wSecond; 
#else
   struct timeval now;
   gettimeofday( &now , NULL );
   //assert( now );
   time = now.tv_sec;
#endif
   return time;
}

#if 0
ostream& operator<< ( ostream& strm, const UInt128& r )
{
   strm << int(r.octet[0]);
   for ( int i=1; i<16; i++ )
   {
      strm << ':' << int(r.octet[i]);
   }
    
   return strm;
}

ostream& 
operator<<( ostream& strm, const StunAddress4& addr)
{
   UInt32 ip = addr.addr;
   strm << ((int)(ip>>24)&0xFF) << ".";
   strm << ((int)(ip>>16)&0xFF) << ".";
   strm << ((int)(ip>> 8)&0xFF) << ".";
   strm << ((int)(ip>> 0)&0xFF) ;
	
   strm << ":" << addr.port;
	
   return strm;
}
#endif


#define ip (*pip)
#define portVal (*pportVal)

// returns true if it scucceeded
int 
stunParseHostName( char* peerName,
               UInt32 *pip,
               UInt16 *pportVal,
               UInt16 defaultPort )
{
   struct in_addr sin_addr;
   char* port = NULL;
   char host[512];
   char *sep;
   int portNum = defaultPort;
    
   strncpy(host,peerName,512);
   host[512-1]='\0';
	
	
   // pull out the port part if present.
   sep = strchr(host,':');
	
   if ( sep == NULL )
   {
      portNum = defaultPort;
   }
   else
   {
      char* endPtr=NULL;

      *sep = '\0';
      port = sep + 1;
      // set port part
		
		
      portNum = strtol(port,&endPtr,10);
		
      if ( endPtr != NULL )
      {
         if ( *endPtr != '\0' )
         {
            portNum = defaultPort;
         }
      }
   }
    
   if ( portNum < 1024 ) return false;
   if ( portNum >= 0xFFFF ) return false;
	
   // figure out the host part 
   struct hostent* h;
	
#ifdef WIN32
   assert( strlen(host) >= 1 );
   if ( isdigit( host[0] ) )
   {
      // assume it is a ip address 
      unsigned long a = inet_addr(host);
      //cerr << "a=0x" << hex << a << dec << endl;
		
      ip = ntohl( a );
   }
   else
   {
      // assume it is a host name 
      h = gethostbyname( host );
		
      if ( h == NULL )
      {
         int err = getErrno();
//         std::cerr << "error was " << err << std::endl;
         assert( err != WSANOTINITIALISED );
			
         ip = ntohl( 0x7F000001L );
			
         return false;
      }
      else
      {
         sin_addr = *(struct in_addr*)h->h_addr;
         ip = ntohl( sin_addr.s_addr );
      }
   }
	
#else
   h = gethostbyname( host );
   if ( h == NULL )
   {
      int err = getErrno();
      stun_message( "error was %d", err);
      ip = ntohl( 0x7F000001L );
      return false;
   }
   else
   {
      sin_addr = *(struct in_addr*)h->h_addr;
      ip = ntohl( sin_addr.s_addr );
   }
#endif
	
   portVal = portNum;
	
   return true;
}
#undef ip
#undef portVal


int
stunParseServerName( char* name, StunAddress4 *paddr)
{
   int ret;
   assert(name);
	
   // TODO - put in DNS SRV stuff.
	
   ret = stunParseHostName( name, &paddr->addr, &paddr->port, 3478); 
   if ( ret != true ) 
   {
       paddr->port=0xFFFF;
   }	
   return ret;
}

#define response (*presponse)
#if STUN_SERVER
static void
stunCreateErrorResponse(StunMessage *presponse, int cl, int number, const char* msg)
{
   response.msgHdr.msgType = BindErrorResponseMsg;
   response.hasErrorCode = true;
   response.errorCode.errorClass = cl;
   response.errorCode.number = number;
   strcpy(response.errorCode.reason, msg);
}

#if 0
static void
stunCreateSharedSecretErrorResponse(StunMessage& response, int cl, int number, const char* msg)
{
   response.msgHdr.msgType = SharedSecretErrorResponseMsg;
   response.hasErrorCode = true;
   response.errorCode.errorClass = cl;
   response.errorCode.number = number;
   strcpy(response.errorCode.reason, msg);
}
#endif

static void
stunCreateSharedSecretResponse(const StunMessage request, const StunAddress4 source, StunMessage *presponse)
{
   response.msgHdr.msgType = SharedSecretResponseMsg;
   response.msgHdr.id = request.msgHdr.id;
	
   response.hasUsername = true;
   stunCreateUserName( source, &response.username);
	
   response.hasPassword = true;
   stunCreatePassword( response.username, &response.password);
}
#endif  /* STUNSERVER */

#undef response

#if STUN_SERVER
// This funtion takes a single message sent to a stun server, parses
// and constructs an apropriate repsonse - returns true if message is
// valid
int
stunServerProcessMsg( char* buf,
                      unsigned int bufLen,
                      StunAddress4& from, 
                      StunAddress4& secondary,
                      StunAddress4& myAddr,
                      StunAddress4& altAddr, 
                      StunMessage* resp,
                      StunAddress4* destination,
                      StunAtrString* hmacPassword,
                      int* changePort,
                      int* changeIp,
                      int verbose)
{
    
   // set up information for default response 
	
   memset( resp, 0 , sizeof(*resp) );
	
   *changeIp = false;
   *changePort = false;
	
   StunMessage req;
   int ok = stunParseMessage( buf,bufLen, req, verbose);
	
   if (!ok)      // Complete garbage, drop it on the floor
   {
      if (verbose) stun_message( "Request did not parse");
      return false;
   }
   if (verbose) stun_message( "Request parsed ok");
	
   StunAddress4 mapped = req.mappedAddress.ipv4;
   StunAddress4 respondTo = req.responseAddress.ipv4;
   UInt32 flags = req.changeRequest.value;
	
   switch (req.msgHdr.msgType)
   {
      case SharedSecretRequestMsg:
         if(verbose) stun_message( "Received SharedSecretRequestMsg on udp. send error 433.");
         // !cj! - should fix so you know if this came over TLS or UDP
         stunCreateSharedSecretResponse(req, from, *resp);
         //stunCreateSharedSecretErrorResponse(*resp, 4, 33, "this request must be over TLS");
         return true;
			
      case BindRequestMsg:
         if (!req.hasMessageIntegrity)
         {
            if (verbose) stun_message( "BindRequest does not contain MessageIntegrity");
				
            if (0) // !jf! mustAuthenticate
            {
               if(verbose) stun_message( "Received BindRequest with no MessageIntegrity. Sending 401.");
               stunCreateErrorResponse(*resp, 4, 1, "Missing MessageIntegrity");
               return true;
            }
         }
         else
         {
            if (!req.hasUsername)
            {
               if (verbose) stun_message( "No UserName. Send 432.");
               stunCreateErrorResponse(*resp, 4, 32, "No UserName and contains MessageIntegrity");
               return true;
            }
            else
            {
               if (verbose) stun_message( "Validating username: " << req.username.value );
               // !jf! could retrieve associated password from provisioning here
               if (strcmp(req.username.value, "test") == 0)
               {
                  if (0)
                  {
                     // !jf! if the credentials are stale 
                     stunCreateErrorResponse(*resp, 4, 30, "Stale credentials on BindRequest");
                     return true;
                  }
                  else
                  {
                     if (verbose) stun_message( "Validating MessageIntegrity");
                     // need access to shared secret
							
                     unsigned char hmac[20];
#ifndef NOSSL
                     unsigned int hmacSize=20;

                     HMAC(EVP_sha1(), 
                          "1234", 4, 
                          reinterpret_cast<const unsigned char*>(buf), bufLen-20-4, 
                          hmac, &hmacSize);
                     assert(hmacSize == 20);
#endif
							
                     if (memcmp(buf, hmac, 20) != 0)
                     {
                        if (verbose) stun_message( "MessageIntegrity is bad. Sending ");
                        stunCreateErrorResponse(*resp, 4, 3, "Unknown username. Try test with password 1234");
                        return true;
                     }
							
                     // need to compute this later after message is filled in
                     resp->hasMessageIntegrity = true;
                     assert(req.hasUsername);
                     resp->hasUsername = true;
                     resp->username = req.username; // copy username in
                  }
               }
               else
               {
                  if (verbose) stun_message( "Invalid username: " << req.username.value << "Send 430."); 
               }
            }
         }
			
         // TODO !jf! should check for unknown attributes here and send 420 listing the
         // unknown attributes. 
			
         if ( respondTo.port == 0 ) respondTo = from;
         if ( mapped.port == 0 ) mapped = from;
				
         *changeIp   = ( flags & ChangeIpFlag )?true:false;
         *changePort = ( flags & ChangePortFlag )?true:false;
			
         if (verbose)
         {
            stun_message( "Request is valid:");
            stun_message( "\t flags=" << flags );
            stun_message( "\t changeIp=" << *changeIp );
            stun_message( "\t changePort=" << *changePort );
            stun_message( "\t from = " << from );
            stun_message( "\t respond to = " << respondTo );
            stun_message( "\t mapped = " << mapped );
         }
				
         // form the outgoing message
         resp->msgHdr.msgType = BindResponseMsg;
         for ( int i=0; i<16; i++ )
         {
            resp->msgHdr.id.octet[i] = req.msgHdr.id.octet[i];
         }
		
         if ( req.xorOnly == false )
         {
            resp->hasMappedAddress = true;
            resp->mappedAddress.ipv4.port = mapped.port;
            resp->mappedAddress.ipv4.addr = mapped.addr;
         }

         if (1) // do xorMapped address or not 
         {
            resp->hasXorMappedAddress = true;
            UInt16 id16 = req.msgHdr.id.octet[0]<<8 
               | req.msgHdr.id.octet[1];
            UInt32 id32 = req.msgHdr.id.octet[0]<<24 
               | req.msgHdr.id.octet[1]<<16 
               | req.msgHdr.id.octet[2]<<8 
               | req.msgHdr.id.octet[3];
            resp->xorMappedAddress.ipv4.port = mapped.port^id16;
            resp->xorMappedAddress.ipv4.addr = mapped.addr^id32;
         }
         
         resp->hasSourceAddress = true;
         resp->sourceAddress.ipv4.port = (*changePort) ? altAddr.port : myAddr.port;
         resp->sourceAddress.ipv4.addr = (*changeIp)   ? altAddr.addr : myAddr.addr;
			
         resp->hasChangedAddress = true;
         resp->changedAddress.ipv4.port = altAddr.port;
         resp->changedAddress.ipv4.addr = altAddr.addr;
	
         if ( secondary.port != 0 )
         {
            resp->hasSecondaryAddress = true;
            resp->secondaryAddress.ipv4.port = secondary.port;
            resp->secondaryAddress.ipv4.addr = secondary.addr;
         }
         
         if ( req.hasUsername && req.username.sizeValue > 0 ) 
         {
            // copy username in
            resp->hasUsername = true;
            assert( req.username.sizeValue % 4 == 0 );
            assert( req.username.sizeValue < STUN_MAX_STRING );
            memcpy( resp->username.value, req.username.value, req.username.sizeValue );
            resp->username.sizeValue = req.username.sizeValue;
         }
		
         if (1) // add ServerName 
         {
            resp->hasServerName = true;
            const char serverName[] = "Vovida.org " STUN_VERSION; // must pad to mult of 4
            
            assert( sizeof(serverName) < STUN_MAX_STRING );
            //cerr << "sizeof serverName is "  << sizeof(serverName) );
            assert( sizeof(serverName)%4 == 0 );
            memcpy( resp->serverName.value, serverName, sizeof(serverName));
            resp->serverName.sizeValue = sizeof(serverName);
         }
         
         if ( req.hasMessageIntegrity & req.hasUsername )  
         {
            // this creates the password that will be used in the HMAC when then
            // messages is sent
            stunCreatePassword( req.username, hmacPassword );
         }
				
         if (req.hasUsername && (req.username.sizeValue > 64 ) )
         {
            UInt32 source;
            assert( sizeof(int) == sizeof(UInt32) );
					
            sscanf(req.username.value, "%x", &source);
            resp->hasReflectedFrom = true;
            resp->reflectedFrom.ipv4.port = 0;
            resp->reflectedFrom.ipv4.addr = source;
         }
				
         destination->port = respondTo.port;
         destination->addr = respondTo.addr;
			
         return true;
			
      default:
         if (verbose) stun_message( "Unknown or unsupported request ");
         return false;
   }
	
   assert(0);
   return false;
}

int
stunInitServer(StunServerInfo& info, const StunAddress4& myAddr,
               const StunAddress4& altAddr, int startMediaPort, bool verbose )
{
   assert( myAddr.port != 0 );
   assert( altAddr.port!= 0 );
   assert( myAddr.addr  != 0 );
   //assert( altAddr.addr != 0 );
	
   info.myAddr = myAddr;
   info.altAddr = altAddr;
	
   info.myFd = INVALID_SOCKET;
   info.altPortFd = INVALID_SOCKET;
   info.altIpFd = INVALID_SOCKET;
   info.altIpPortFd = INVALID_SOCKET;

   memset(info.relays, 0, sizeof(info.relays));
   if (startMediaPort > 0)
   {
      info.relay = true;

      for (int i=0; i<MAX_MEDIA_RELAYS; ++i)
      {
         StunMediaRelay* relay = &info.relays[i];
         relay->relayPort = startMediaPort+i;
         relay->fd = 0;
         relay->expireTime = 0;
      }
   }
   else
   {
      info.relay = false;
   }
   
   if ((info.myFd = openPort(myAddr.port, myAddr.addr,verbose)) == INVALID_SOCKET)
   {
      stun_message( "Can't open " << myAddr );
      stunStopServer(info);

      return false;
   }
   //if (verbose) stun_message( "Opened " << myAddr.addr << ":" << myAddr.port << " --> " << info.myFd );

   if ((info.altPortFd = openPort(altAddr.port,myAddr.addr,verbose)) == INVALID_SOCKET)
   {
      stun_message( "Can't open " << myAddr );
      stunStopServer(info);
      return false;
   }
   //if (verbose) stun_message( "Opened " << myAddr.addr << ":" << altAddr.port << " --> " << info.altPortFd );
   
   
   info.altIpFd = INVALID_SOCKET;
   if (  altAddr.addr != 0 )
   {
      if ((info.altIpFd = openPort( myAddr.port, altAddr.addr,verbose)) == INVALID_SOCKET)
      {
         stun_message( "Can't open " << altAddr );
         stunStopServer(info);
         return false;
      }
      //if (verbose) stun_message( "Opened " << altAddr.addr << ":" << myAddr.port << " --> " << info.altIpFd );;
   }
   
   info.altIpPortFd = INVALID_SOCKET;
   if (  altAddr.addr != 0 )
   {  if ((info.altIpPortFd = openPort(altAddr.port, altAddr.addr,verbose)) == INVALID_SOCKET)
      {
         stun_message( "Can't open " << altAddr );
         stunStopServer(info);
         return false;
      }
      //if (verbose) stun_message( "Opened " << altAddr.addr << ":" << altAddr.port << " --> " << info.altIpPortFd );;
   }
   
   return true;
}

void
stunStopServer(StunServerInfo& info)
{
   if (info.myFd > 0) closesocket(info.myFd);
   if (info.altPortFd > 0) closesocket(info.altPortFd);
   if (info.altIpFd > 0) closesocket(info.altIpFd);
   if (info.altIpPortFd > 0) closesocket(info.altIpPortFd);
   
   if (info.relay)
   {
      for (int i=0; i<MAX_MEDIA_RELAYS; ++i)
      {
         StunMediaRelay* relay = &info.relays[i];
         if (relay->fd)
         {
            closesocket(relay->fd);
            relay->fd = 0;
         }
      }
   }
}

bool
stunServerProcess(StunServerInfo& info, bool verbose)
{
   char msg[STUN_MAX_MESSAGE_SIZE];
   int msgLen = sizeof(msg);
   	
   bool ok = false;
   bool recvAltIp =false;
   bool recvAltPort = false;
	
   fd_set fdSet; 
   Socket maxFd=0;

   FD_ZERO(&fdSet); 
   FD_SET(info.myFd,&fdSet); 
   if ( info.myFd >= maxFd ) maxFd=info.myFd+1;
   FD_SET(info.altPortFd,&fdSet); 
   if ( info.altPortFd >= maxFd ) maxFd=info.altPortFd+1;

   if ( info.altIpFd != INVALID_SOCKET )
   {
      FD_SET(info.altIpFd,&fdSet);
      if (info.altIpFd>=maxFd) maxFd=info.altIpFd+1;
   }
   if ( info.altIpPortFd != INVALID_SOCKET )
   {
      FD_SET(info.altIpPortFd,&fdSet);
      if (info.altIpPortFd>=maxFd) maxFd=info.altIpPortFd+1;
   }

   if (info.relay)
   {
      for (int i=0; i<MAX_MEDIA_RELAYS; ++i)
      {
         StunMediaRelay* relay = &info.relays[i];
         if (relay->fd)
         {
            FD_SET(relay->fd, &fdSet);
            if (relay->fd >= maxFd) 
			{
				maxFd=relay->fd+1;
			}
         }
      }
   }
   
   if ( info.altIpFd != INVALID_SOCKET )
   {
      FD_SET(info.altIpFd,&fdSet);
      if (info.altIpFd>=maxFd) maxFd=info.altIpFd+1;
   }
   if ( info.altIpPortFd != INVALID_SOCKET )
   {
      FD_SET(info.altIpPortFd,&fdSet);
      if (info.altIpPortFd>=maxFd) maxFd=info.altIpPortFd+1;
   }
   
   struct timeval tv;
   tv.tv_sec = 0;
   tv.tv_usec = 1000;
	
   int e = select( maxFd, &fdSet, NULL,NULL, &tv );
   if (e < 0)
   {
      int err = getErrno();
      stun_message( "Error on select: " << strerror(err) );
   }
   else if (e >= 0)
   {
      StunAddress4 from;

      // do the media relaying
      if (info.relay)
      {
         time_t now = time(0);
         for (int i=0; i<MAX_MEDIA_RELAYS; ++i)
         {
            StunMediaRelay* relay = &info.relays[i];
            if (relay->fd)
            {
               if (FD_ISSET(relay->fd, &fdSet))
               {
                  char msg[MAX_RTP_MSG_SIZE];
                  int msgLen = sizeof(msg);
                  
                  StunAddress4 rtpFrom;
                  ok = getMessage( relay->fd, msg, &msgLen, &rtpFrom.addr, &rtpFrom.port ,verbose);
                  if (ok)
                  {
                     sendMessage(info.myFd, msg, msgLen, relay->destination.addr, relay->destination.port, verbose);
                     relay->expireTime = now + MEDIA_RELAY_TIMEOUT;
                     if ( verbose ) stun_message( "Relay packet on " 
                                         << relay->fd 
                                         << " from " << rtpFrom 
                                         << " -> " << relay->destination 
                                         );
                  }
               }
               else if (now > relay->expireTime)
               {
                  closesocket(relay->fd);
                  relay->fd = 0;
               }
            }
         }
      }
      
     
      if (FD_ISSET(info.myFd,&fdSet))
      {
         if (verbose) stun_message( "received on A1:P1");
         recvAltIp = false;
         recvAltPort = false;
         ok = getMessage( info.myFd, msg, &msgLen, &from.addr, &from.port,verbose );
      }
      else if (FD_ISSET(info.altPortFd, &fdSet))
      {
         if (verbose) stun_message( "received on A1:P2");
         recvAltIp = false;
         recvAltPort = true;
         ok = getMessage( info.altPortFd, msg, &msgLen, &from.addr, &from.port,verbose );
      }
      else if ( (info.altIpFd!=INVALID_SOCKET) && FD_ISSET(info.altIpFd,&fdSet))
      {
         if (verbose) stun_message( "received on A2:P1");
         recvAltIp = true;
         recvAltPort = false;
         ok = getMessage( info.altIpFd, msg, &msgLen, &from.addr, &from.port ,verbose);
      }
      else if ( (info.altIpPortFd!=INVALID_SOCKET) && FD_ISSET(info.altIpPortFd, &fdSet))
      {
         if (verbose) stun_message( "received on A2:P2");
         recvAltIp = true;
         recvAltPort = true;
         ok = getMessage( info.altIpPortFd, msg, &msgLen, &from.addr, &from.port,verbose );
      }
      else
      {
         return true;
      }

      int relayPort = 0;
      if (info.relay)
      {
         for (int i=0; i<MAX_MEDIA_RELAYS; ++i)
         {
            StunMediaRelay* relay = &info.relays[i];
            if (relay->destination.addr == from.addr && 
                relay->destination.port == from.port)
            {
               relayPort = relay->relayPort;
               relay->expireTime = time(0) + MEDIA_RELAY_TIMEOUT;
               break;
            }
         }

         if (relayPort == 0)
         {
            for (int i=0; i<MAX_MEDIA_RELAYS; ++i)
            {
               StunMediaRelay* relay = &info.relays[i];
               if (relay->fd == 0)
               {
                  if ( verbose ) stun_message( "Open relay port " << relay->relayPort );
                  
                  relay->fd = openPort(relay->relayPort, info.myAddr.addr, verbose);
                  relay->destination.addr = from.addr;
                  relay->destination.port = from.port;
                  relay->expireTime = time(0) + MEDIA_RELAY_TIMEOUT;
                  relayPort = relay->relayPort;
                  break;
               }
            }
         }
      }
         
      if ( !ok ) 
      {
         if ( verbose ) stun_message( "Get message did not return a valid message" <<endl;
         return true;
      }
		
      if ( verbose ) stun_message( "Got a request (len=" << msgLen << ") from " << from );
		
      if ( msgLen <= 0 )
      {
         return true;
      }
		
      bool changePort = false;
      bool changeIp = false;
		
      StunMessage resp;
      StunAddress4 dest;
      StunAtrString hmacPassword;  
      hmacPassword.sizeValue = 0;

      StunAddress4 secondary;
      secondary.port = 0;
      secondary.addr = 0;
               
      if (info.relay && relayPort)
      {
         secondary = from;
         
         from.addr = info.myAddr.addr;
         from.port = relayPort;
      }
      
      ok = stunServerProcessMsg( msg, msgLen, from, secondary,
                                 recvAltIp ? info.altAddr : info.myAddr,
                                 recvAltIp ? info.myAddr : info.altAddr, 
                                 &resp,
                                 &dest,
                                 &hmacPassword,
                                 &changePort,
                                 &changeIp,
                                 verbose );
		
      if ( !ok )
      {
         if ( verbose ) stun_message( "Failed to parse message");
         return true;
      }
		
      char buf[STUN_MAX_MESSAGE_SIZE];
      int len = sizeof(buf);
      		
      len = stunEncodeMessage( resp, buf, len, hmacPassword,verbose );
		
      if ( dest.addr == 0 )  ok=false;
      if ( dest.port == 0 ) ok=false;
		
      if ( ok )
      {
         assert( dest.addr != 0 );
         assert( dest.port != 0 );
			
         Socket sendFd;
			
         bool sendAltIp   = recvAltIp;   // send on the received IP address 
         bool sendAltPort = recvAltPort; // send on the received port
			
         if ( changeIp )   sendAltIp   = !sendAltIp;   // if need to change IP, then flip logic 
         if ( changePort ) sendAltPort = !sendAltPort; // if need to change port, then flip logic 
			
         if ( !sendAltPort )
         {
            if ( !sendAltIp )
            {
               sendFd = info.myFd;
            }
            else
            {
               sendFd = info.altIpFd;
            }
         }
         else
         {
            if ( !sendAltIp )
            {
               sendFd = info.altPortFd;
            }
            else
            {
               sendFd = info.altIpPortFd;
            }
         }
	
         if ( sendFd != INVALID_SOCKET )
         {
            sendMessage( sendFd, buf, len, dest.addr, dest.port, verbose );
         }
      }
   }
	
   return true;
}
#endif  /* STUNSERVER */

int 
stunFindLocalInterfaces(UInt32* addresses,int maxRet)
{
#if defined(WIN32) || defined(__sparc__)
   return 0;
#else
   struct ifconf ifc;
   char *ptr;
   int e, tl, count;
   int s = socket( AF_INET, SOCK_DGRAM, 0 );
   int len = 100 * sizeof(struct ifreq);
	
   char buf[ len ];
	
   ifc.ifc_len = len;
   ifc.ifc_buf = buf;
	
   e = ioctl(s,SIOCGIFCONF,&ifc);
   ptr = buf;
   tl = ifc.ifc_len;
   count=0;
	
   while ( (tl > 0) && ( count < maxRet) )
   {
      int si;
      UInt32 ai;
      struct sockaddr a;
      struct sockaddr_in* addr;
      struct ifreq ifr2;
      struct ifreq* ifr = (struct ifreq *)ptr;
		
      si = sizeof(ifr->ifr_name) + sizeof(struct sockaddr);
      tl -= si;
      ptr += si;
      //char* name = ifr->ifr_ifrn.ifrn_name;
      //cerr << "name = " << name );
		
      ifr2 = *ifr;
		
      e = ioctl(s,SIOCGIFADDR,&ifr2);
      if ( e == -1 )
      {
         break;
      }
		
      //cerr << "ioctl addr e = " << e );
		
      a = ifr2.ifr_addr;
      addr = (struct sockaddr_in*) &a;
		
      ai = ntohl( addr->sin_addr.s_addr );
      if (((ai>>24)&0xFF) != 127)
      {
         addresses[count++] = ai;
      }
		
#if 0
      cerr << "Detected interface "
           << int((ai>>24)&0xFF) << "." 
           << int((ai>>16)&0xFF) << "." 
           << int((ai>> 8)&0xFF) << "." 
           << int((ai    )&0xFF) );
#endif
   }
	
  if (s != EXOSIP_FD) {
   closesocket(s);
  }
	
   return count;
#endif
}


void
stunBuildReqSimple( StunMessage* msg,
                    const StunAtrString username,
                    int changePort, int changeIp, unsigned int id )
{
   int i;
   assert( msg );
   memset( msg , 0 , sizeof(*msg) );
	
   msg->msgHdr.msgType = BindRequestMsg;
	
   for (i=0; i<16; i=i+4 )
   {
      int r;
      assert(i+3<16);
      r = stunRand();
      msg->msgHdr.id.octet[i+0]= r>>0;
      msg->msgHdr.id.octet[i+1]= r>>8;
      msg->msgHdr.id.octet[i+2]= r>>16;
      msg->msgHdr.id.octet[i+3]= r>>24;
   }
	
   if ( id != 0 )
   {
      msg->msgHdr.id.octet[0] = id; 
   }
	
   msg->hasChangeRequest = true;
   msg->changeRequest.value =(changeIp?ChangeIpFlag:0) | 
      (changePort?ChangePortFlag:0);
	
   if ( username.sizeValue > 0 )
   {
      msg->hasUsername = true;
      msg->username = username;
   }
}


#define dest (*pdest)
#ifdef NEW_STUN
int
stunSendTest( Socket myFd, StunAddress4 *pdest, 
              const StunAtrString username, const StunAtrString password, 
              int testNum, int verbose )
#else
static
int
stunSendTest( Socket myFd, StunAddress4 *pdest, 
              const StunAtrString username, const StunAtrString password, 
              int testNum, int verbose )
#endif
{ 
   char abuf[64];
   StunMessage req;
   char buf[STUN_MAX_MESSAGE_SIZE];
   int len = STUN_MAX_MESSAGE_SIZE;

   int changePort=false;
   int changeIP=false;
   int discard=false;

   int ret;

   assert( dest.addr != 0 );
   assert( dest.port != 0 );
		
   switch (testNum)
   {
      case 1:
      case 10:
      case 11:
         break;
      case 2:
         //changePort=true;
         changeIP=true;
         break;
      case 3:
         changePort=true;
         break;
      case 4:
         changeIP=true;
         break;
      case 5:
         discard=true;
         break;
      default:
         stun_message( "Test %d is unkown\n", testNum);
         assert(0);
   }
	
   memset(&req, 0, sizeof(StunMessage));
	
   stunBuildReqSimple( &req, username, 
                       changePort , changeIP , 
                       testNum );
	
   len = stunEncodeMessage( req, buf, len, password,verbose );
	
   if ( verbose )
   {
      stun_message( "About to send msg of len %d to %s", len, ipv4tostr(abuf, dest) );
   }
	
   ret = sendMessage( myFd, buf, len, dest.addr, dest.port, verbose );
	
   // add some delay so the packets don't get sent too quickly 
#ifdef WIN32 // !cj! TODO - should fix this up in windows
		 clock_t now = clock();
		 assert( CLOCKS_PER_SEC == 1000 );
		 while ( clock() <= now+10 ) { };
#else
		 usleep(10*1000);
#endif

	return ret;
}
#undef dest


void 
stunGetUserNameAndPassword(  const StunAddress4 dest, 
                             StunAtrString* username,
                             StunAtrString* password)
{ 
   // !cj! This is totally bogus - need to make TLS connection to dest and get a
   // username and password to use 
   stunCreateUserName(dest, username);
   stunCreatePassword(*username, password);
}

#define dest (*pdest)

#if 0
void 
stunTest( StunAddress4& dest, int testNum, bool verbose, StunAddress4* sAddr )
{ 
   assert( dest.addr != 0 );
   assert( dest.port != 0 );
	
   int port = stunRandomPort();
   UInt32 interfaceIp=0;
   if (sAddr)
   {
      interfaceIp = sAddr->addr;
      if ( sAddr->port != 0 )
      {
        port = sAddr->port;
      }
   }
   Socket myFd = openPort(port,interfaceIp,verbose);
	
   StunAtrString username;
   StunAtrString password;
	
   username.sizeValue = 0;
   password.sizeValue = 0;
	
#ifdef USE_TLS
   stunGetUserNameAndPassword( dest, username, password );
#endif
	
   stunSendTest( myFd, dest, username, password, testNum, verbose );
    
   char msg[STUN_MAX_MESSAGE_SIZE];
   int msgLen = STUN_MAX_MESSAGE_SIZE;
	
   StunAddress4 from;
   getMessage( myFd,
               msg,
               &msgLen,
               &from.addr,
               &from.port,verbose );
	
   StunMessage resp;
   memset(&resp, 0, sizeof(StunMessage));
	
   if ( verbose ) stun_message( "Got a response");
   bool ok = stunParseMessage( msg,msgLen, resp,verbose );
	
   if ( verbose )
   {
      stun_message( "\t ok=" << ok );
      stun_message( "\t id=" << resp.msgHdr.id );
      stun_message( "\t mappedAddr=" << resp.mappedAddress.ipv4 );
      stun_message( "\t changedAddr=" << resp.changedAddress.ipv4 );
      clog );
   }
	
   if (sAddr)
   {
      sAddr->port = resp.mappedAddress.ipv4.port;
      sAddr->addr = resp.mappedAddress.ipv4.addr;
   }
}
#endif

#if 0
NatType
stunNatType( StunAddress4& dest, 
             bool verbose,
             bool* preservePort, // if set, is return for if NAT preservers ports or not
             bool* hairpin,  // if set, is the return for if NAT will hairpin packets
             int port, // port to use for the test, 0 to choose random port
             StunAddress4* sAddr // NIC to use 
   )
{ 
   assert( dest.addr != 0 );
   assert( dest.port != 0 );
	
   if ( hairpin ) 
   {
      *hairpin = false;
   }
	
   if ( port == 0 )
   {
      port = stunRandomPort();
   }
   UInt32 interfaceIp=0;
   if (sAddr)
   {
      interfaceIp = sAddr->addr;
   }
   Socket myFd1 = openPort(port,interfaceIp,verbose);
   Socket myFd2 = openPort(port+1,interfaceIp,verbose);

   if ( ( myFd1 == INVALID_SOCKET) || ( myFd2 == INVALID_SOCKET) )
   {
        cerr << "Some problem opening port/interface to send on");
       return StunTypeFailure; 
   }

   assert( myFd1 != INVALID_SOCKET );
   assert( myFd2 != INVALID_SOCKET );
    
   bool respTestI=false;
   bool isNat=true;
   StunAddress4 testIchangedAddr;
   StunAddress4 testImappedAddr;
   bool respTestI2=false; 
   bool mappedIpSame = true;
   StunAddress4 testI2mappedAddr;
   StunAddress4 testI2dest=dest;
   bool respTestII=false;
   bool respTestIII=false;

   bool respTestHairpin=false;
   bool respTestPreservePort=false;
	
   memset(&testImappedAddr,0,sizeof(testImappedAddr));
	
   StunAtrString username;
   StunAtrString password;
	
   username.sizeValue = 0;
   password.sizeValue = 0;
	
#ifdef USE_TLS 
   stunGetUserNameAndPassword( dest, username, password );
#endif
	
   int count=0;
   while ( count < 7 )
   {
      struct timeval tv;
      fd_set fdSet; 
#ifdef WIN32
      unsigned int fdSetSize;
#else
      int fdSetSize;
#endif
      FD_ZERO(&fdSet); fdSetSize=0;
      FD_SET(myFd1,&fdSet); fdSetSize = (myFd1+1>fdSetSize) ? myFd1+1 : fdSetSize;
      FD_SET(myFd2,&fdSet); fdSetSize = (myFd2+1>fdSetSize) ? myFd2+1 : fdSetSize;
      tv.tv_sec=0;
      tv.tv_usec=150*1000; // 150 ms 
      if ( count == 0 ) tv.tv_usec=0;
		
      int  err = select(fdSetSize, &fdSet, NULL, NULL, &tv);
      int e = getErrno();
      if ( err == SOCKET_ERROR )
      {
         // error occured
         cerr << "Error " << e << " " << strerror(e) << " in select");
        return StunTypeFailure; 
     }
      else if ( err == 0 )
      {
         // timeout occured 
         count++;
			
         if ( !respTestI ) 
         {
            stunSendTest( myFd1, dest, username, password, 1 ,verbose );
         }         
			
         if ( (!respTestI2) && respTestI ) 
         {
            // check the address to send to if valid 
            if (  ( testI2dest.addr != 0 ) &&
                  ( testI2dest.port != 0 ) )
            {
               stunSendTest( myFd1, testI2dest, username, password, 10  ,verbose);
            }
         }
			
         if ( !respTestII )
         {
            stunSendTest( myFd2, dest, username, password, 2 ,verbose );
         }
			
         if ( !respTestIII )
         {
            stunSendTest( myFd2, dest, username, password, 3 ,verbose );
         }
			
         if ( respTestI && (!respTestHairpin) )
         {
            if (  ( testImappedAddr.addr != 0 ) &&
                  ( testImappedAddr.port != 0 ) )
            {
               stunSendTest( myFd1, testImappedAddr, username, password, 11 ,verbose );
            }
         }
      }
      else
      {
         //if (verbose) stun_message( "-----------------------------------------");
         assert( err>0 );
         // data is avialbe on some fd 
			
         for ( int i=0; i<2; i++)
         {
            Socket myFd;
            if ( i==0 ) 
            {
               myFd=myFd1;
            }
            else
            {
               myFd=myFd2;
            }
				
            if ( myFd!=INVALID_SOCKET ) 
            {					
               if ( FD_ISSET(myFd,&fdSet) )
               {
                  char msg[STUN_MAX_MESSAGE_SIZE];
                  int msgLen = sizeof(msg);
                  						
                  StunAddress4 from;
						
                  getMessage( myFd,
                              msg,
                              &msgLen,
                              &from.addr,
                              &from.port,verbose );
						
                  StunMessage resp;
                  memset(&resp, 0, sizeof(StunMessage));
						
                  stunParseMessage( msg,msgLen, resp,verbose );
						
                  if ( verbose )
                  {
                     stun_message( "Received message of type " << resp.msgHdr.msgType 
                          << "  id=" << (int)(resp.msgHdr.id.octet[0]) );
                  }
						
                  switch( resp.msgHdr.id.octet[0] )
                  {
                     case 1:
                     {
                        if ( !respTestI )
                        {
									
                           testIchangedAddr.addr = resp.changedAddress.ipv4.addr;
                           testIchangedAddr.port = resp.changedAddress.ipv4.port;
                           testImappedAddr.addr = resp.mappedAddress.ipv4.addr;
                           testImappedAddr.port = resp.mappedAddress.ipv4.port;
			
                           respTestPreservePort = ( testImappedAddr.port == port ); 
                           if ( preservePort )
                           {
                              *preservePort = respTestPreservePort;
                           }								
									
                           testI2dest.addr = resp.changedAddress.ipv4.addr;
									
                           if (sAddr)
                           {
                              sAddr->port = testImappedAddr.port;
                              sAddr->addr = testImappedAddr.addr;
                           }
									
                           count = 0;
                        }		
                        respTestI=true;
                     }
                     break;
                     case 2:
                     {  
                        respTestII=true;
                     }
                     break;
                     case 3:
                     {
                        respTestIII=true;
                     }
                     break;
                     case 10:
                     {
                        if ( !respTestI2 )
                        {
                           testI2mappedAddr.addr = resp.mappedAddress.ipv4.addr;
                           testI2mappedAddr.port = resp.mappedAddress.ipv4.port;
								
                           mappedIpSame = false;
                           if ( (testI2mappedAddr.addr  == testImappedAddr.addr ) &&
                                (testI2mappedAddr.port == testImappedAddr.port ))
                           { 
                              mappedIpSame = true;
                           }
								
							
                        }
                        respTestI2=true;
                     }
                     break;
                     case 11:
                     {
							
                        if ( hairpin ) 
                        {
                           *hairpin = true;
                        }
                        respTestHairpin = true;
                     }
                     break;
                  }
               }
            }
         }
      }
   }
	
   // see if we can bind to this address 
   //cerr << "try binding to " << testImappedAddr );
   Socket s = openPort( 0/*use ephemeral*/, testImappedAddr.addr, false );
   if ( s != INVALID_SOCKET )
   {
      closesocket(s);
      isNat = false;
      //cerr << "binding worked");
   }
   else
   {
      isNat = true;
      //cerr << "binding failed");
   }
	
   if (verbose)
   {
      stun_message( "test I = " << respTestI );
      stun_message( "test II = " << respTestII );
      stun_message( "test III = " << respTestIII );
      stun_message( "test I(2) = " << respTestI2 );
      stun_message( "is nat  = " << isNat <<endl;
      stun_message( "mapped IP same = " << mappedIpSame );
      stun_message( "hairpin = " << respTestHairpin );
      stun_message( "preserver port = " << respTestPreservePort );
   }
	
#if 0
   // implement logic flow chart from draft RFC
   if ( respTestI )
   {
      if ( isNat )
      {
         if (respTestII)
         {
            return StunTypeConeNat;
         }
         else
         {
            if ( mappedIpSame )
            {
               if ( respTestIII )
               {
                  return StunTypeRestrictedNat;
               }
               else
               {
                  return StunTypePortRestrictedNat;
               }
            }
            else
            {
               return StunTypeSymNat;
            }
         }
      }
      else
      {
         if (respTestII)
         {
            return StunTypeOpen;
         }
         else
         {
            return StunTypeSymFirewall;
         }
      }
   }
   else
   {
      return StunTypeBlocked;
   }
#else
   if ( respTestI ) // not blocked 
   {
      if ( isNat )
      {
         if ( mappedIpSame )
         {
            if (respTestII)
            {
               return StunTypeIndependentFilter;
            }
            else
            {
               if ( respTestIII )
               {
                  return StunTypeDependentFilter;
               }
               else
               {
                  return StunTypePortDependedFilter;
               }
            }
         }
         else // mappedIp is not same 
         {
            return StunTypeDependentMapping;
         }
      }
      else  // isNat is false
      {
         if (respTestII)
         {
            return StunTypeOpen;
         }
         else
         {
            return StunTypeFirewall;
         }
      }
   }
   else
   {
      return StunTypeBlocked;
   }
#endif
	
   return StunTypeUnknown;
}
#endif

#ifndef NEW_STUN

Socket
stunOpenSocket( StunAddress4 *pdest, StunAddress4* mapAddr, 
                int port, StunAddress4* srcAddr, 
                int qos, int verbose )
{
unsigned int interfaceIp = 0;
Socket myFd;
char msg[STUN_MAX_MESSAGE_SIZE];
int msgLen = sizeof(msg);
StunAtrString username;
StunAtrString password;
StunAddress4 from;
StunMessage resp;
int ok;
StunAddress4 mappedAddr;
StunAddress4 changedAddr;
struct timeval timeout;
int run=true;
fd_set fdset;
int err;


	assert( dest.addr != 0 );
	assert( dest.port != 0 );
	assert( mapAddr );
 

	if ( port == 0 ) {
		port = stunRandomPort();
	}

	if ( srcAddr ) {
		interfaceIp = srcAddr->addr;
	}
   
	if (port == EXOSIP_SipPort) {
		myFd = EXOSIP_FD;
	}
	else {
		myFd = openPort(port,interfaceIp,verbose);
		if (myFd == INVALID_SOCKET) {
			return myFd;
		}

#ifdef SUPPORT_DSCP
{
		int priority;
		extern int qos2tos(unsigned char index);

		// add voip priority (for vlan tag in the future)
		priority = 7;
		setsockopt(myFd, SOL_SOCKET, SO_PRIORITY, &priority, sizeof(priority));
		// add voip tos
		priority = qos2tos(g_pVoIPCfg->sipDscp);
		setsockopt(myFd, IPPROTO_IP, IP_TOS, &priority, sizeof(priority)); 
}
#endif
	} 
   
	username.sizeValue = 0;
	password.sizeValue = 0;
	
#ifdef USE_TLS
	stunGetUserNameAndPassword( dest, username, password );
#endif
	//fprintf(stderr, "stunSendTest begin.....\n");	
	stunSendTest(myFd, &dest, username, password, 1, 0/*false*/ );

	STUN_PACKETS_RESEND = 0;
	while (run)	 {
		FD_ZERO(&fdset);
		FD_SET(myFd, &fdset);

		// set timeout
		timeout.tv_sec= 1;
		timeout.tv_usec = 0;

		// wait event by select
		err = select(myFd + 1, &fdset, NULL, NULL, &timeout);
		//printf("wait event by select.....port = %d.\n", port);
		if (STUN_PACKETS_RESEND == 3)
		{
			printf("STOP resend STUN packets!.....%d.\n", port);	
			if (myFd != EXOSIP_FD) {
			closesocket(myFd);
      			}
			run=false;
			return -1;
		}
		else if (err  == 0) // timeout
		{
			printf("stunSendTest again.....no. %d, for port:%d.\n", STUN_PACKETS_RESEND, port);	
			stunSendTest(myFd, &dest, username, password, 1, 0/*false*/ );
			STUN_PACKETS_RESEND++;
		}
		else
		{
			if (FD_ISSET(myFd, &fdset)) {
				//fprintf(stderr, "getMessage begin.....\n");	
				if (getMessage( myFd, msg, &msgLen, &from.addr, &from.port,verbose ) == true) 
					run=false;
				else {
					msgLen = sizeof(msg);
					fprintf(stderr, "getMessage failed.....\n");	
				}
			}
		}
	}
	//fprintf(stderr, "getMessage End.....\n");	


	memset(&resp, 0, sizeof(StunMessage));
	
	ok = stunParseMessage( msg, msgLen, &resp,verbose );
	if (!ok) {
		if (myFd != EXOSIP_FD) {
			closesocket(myFd);
			return -1;
		}
	}
	
	mappedAddr = resp.mappedAddress.ipv4;
	changedAddr = resp.changedAddress.ipv4;
	
	//stun_message( "--- stunOpenSocket --- ");
	//stun_message( "\treq  id=" << req.id );
	//stun_message( "\tresp id=" << id );
	//stun_message( "\tmappedAddr=" << mappedAddr );
	
	*mapAddr = mappedAddr;
	
	return myFd;
}

#endif

#if 0
bool
stunOpenSocketPair( StunAddress4& dest, StunAddress4* mapAddr, 
                    int* fd1, int* fd2, 
                    int port, StunAddress4* srcAddr, 
                    bool verbose )
{
   assert( dest.addr!= 0 );
   assert( dest.port != 0 );
   assert( mapAddr );
   
   const int NUM=3;
	
   if ( port == 0 )
   {
      port = stunRandomPort();
   }
	
   *fd1=-1;
   *fd2=-1;
	
   char msg[STUN_MAX_MESSAGE_SIZE];
   int msgLen =sizeof(msg);
	
   StunAddress4 from;
   int fd[NUM];
   int i;
	
   unsigned int interfaceIp = 0;
   if ( srcAddr )
   {
      interfaceIp = srcAddr->addr;
   }

   for( i=0; i<NUM; i++)
   {
      fd[i] = openPort( (port == 0) ? 0 : (port + i), 
                        interfaceIp, verbose);
      if (fd[i] < 0) 
      {
         while (i > 0)
         {
            closesocket(fd[--i]);
         }
         return false;
      }
   }
	
   StunAtrString username;
   StunAtrString password;
	
   username.sizeValue = 0;
   password.sizeValue = 0;
	
#ifdef USE_TLS
   stunGetUserNameAndPassword( dest, username, password );
#endif
	
   for( i=0; i<NUM; i++)
   {
      stunSendTest(fd[i], dest, username, password, 1/*testNum*/, verbose );
   }
	
   StunAddress4 mappedAddr[NUM];
   for( i=0; i<NUM; i++)
   {
      msgLen = sizeof(msg)/sizeof(*msg);
      getMessage( fd[i],
                  msg,
                  &msgLen,
                  &from.addr,
                  &from.port ,verbose);
		
      StunMessage resp;
      memset(&resp, 0, sizeof(StunMessage));
		
      bool ok = stunParseMessage( msg, msgLen, resp, verbose );
      if (!ok) 
      {
         return false;
      }
		
      mappedAddr[i] = resp.mappedAddress.ipv4;
      StunAddress4 changedAddr = resp.changedAddress.ipv4;
   }
	
   if (verbose)
   {               
      stun_message( "--- stunOpenSocketPair --- ");
      for( i=0; i<NUM; i++)
      {
         stun_message( "\t mappedAddr=" << mappedAddr[i] );
      }
   }
	
   if ( mappedAddr[0].port %2 == 0 )
   {
      if (  mappedAddr[0].port+1 ==  mappedAddr[1].port )
      {
         *mapAddr = mappedAddr[0];
         *fd1 = fd[0];
         *fd2 = fd[1];
         closesocket( fd[2] );
         return true;
      }
   }
   else
   {
      if (( mappedAddr[1].port %2 == 0 )
          && (  mappedAddr[1].port+1 ==  mappedAddr[2].port ))
      {
         *mapAddr = mappedAddr[1];
         *fd1 = fd[1];
         *fd2 = fd[2];
         closesocket( fd[0] );
         return true;
      }
   }

   // something failed, close all and return error
   for( i=0; i<NUM; i++)
   {
      closesocket( fd[i] );
   }
	
   return false;
}
#endif

char *ipv4tostr(char *buf, const StunAddress4 addr)
{
      UInt32 ip = addr.addr;

      sprintf(buf, "%d.%d.%d.%d", (ip >> 24) & 255,  (ip >> 16) & 255, (ip >> 8) & 255, ip & 255);

      return buf;
}

/* ====================================================================
 * The Vovida Software License, Version 1.0 
 * 
 * Copyright (c) 2000 Vovida Networks, Inc.  All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 
 * 3. The names "VOCAL", "Vovida Open Communication Application Library",
 *    and "Vovida Open Communication Application Library (VOCAL)" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written
 *    permission, please contact vocal@vovida.org.
 *
 * 4. Products derived from this software may not be called "VOCAL", nor
 *    may "VOCAL" appear in their name, without prior written
 *    permission of Vovida Networks, Inc.
 * 
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE AND
 * NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL VOVIDA
 * NETWORKS, INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT DAMAGES
 * IN EXCESS OF $1,000, NOR FOR ANY INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 * 
 * ====================================================================
 * 
 * This software consists of voluntary contributions made by Vovida
 * Networks, Inc. and many individuals on behalf of Vovida Networks,
 * Inc.  For more information on Vovida Networks, Inc., please see
 * <http://www.vovida.org/>.
 *
 */

/*
Local Variables:
mode:c++
c-file-style:"ellemtel"
c-file-offsets:((case-label . +))
indent-tabs-mode:nil
End:
*/



