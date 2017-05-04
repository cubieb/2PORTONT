
EXAMPLES

Examples are included in the 'samples' directory. The examples are:

calc:				Simple calculator client and server
ck:				HTTP cookie client and server
components/c, components/cpp:	Multiple clients/services in one executable
dime:				DIME attachment client and server
events:				Asynchronous one-way messaging
factory:			Remote object factory and simple ORB
factorytest:			Test client for remote object factory
googleapi:			Google Web API client
listing:			XMethod service listing client
localtime:			Get localtime client
lu:				Linear solver client and server
magic:				Magic Squares client and server
mtom:				MTOM attachment client and server
oneliners:			Several one-liner clients and services
polytest:			Polymorphic object exchange
quote:				Get delayed stock quote
quote2:				Get delayed stock quote (SOAP async messages)
quote3:				Get delayed stock quote (with Stock class)
quote4:				Get delayed stock quote (using XML DOM)
quotex:				Combined client/server example in one
router:				Router of HTTP messages over TCP/IP
rss:				RSS 0.91, 0.92, and 2.0 server
ssl:				HTTPS/SSL example (requires OpenSSL)
udp:				SOAP-over-UDP example client and server
varparam:			Variable polymorphic parameters client/server
webserver:			Stand-alone Web server with a lot of features
wsse:				WS-Security demo
xml-rpc:			XML-RPC support

To build the example services and clients, type "make" in the 'samples'
directory (Sun Solaris users should type "make -f MakefileSolaris").

DISCLAIMER

WE TRY OUR BEST TO PROVIDE YOU WITH "REAL-WORLD" EXAMPLES BUT WE CANNOT
GUARANTEE THAT ALL CLIENT EXAMPLES CAN CONNECT TO THIRD PARTY WEB SERVICES
WHEN THESE SERVICES ARE DOWN OR HAVE BEEN REMOVED.

MS WINDOWS

For Windows users, the archive includes 'magic_VC' and 'quote_VC' Visual
Studio projects in the 'samples' directory.

SSL

To try the SSL-secure SOAP server, install OpenSSL and change the occurrences
of "linprog2.cs.fsu.edu" in sslclient.c and sslserver.c to the machine name
(or machine IP) you are using. Example .pem files are included but you need to
create your own .pem files (see OpenSSL documentation).

The sslclient and sslserver codes can then be build as follows:

soapcpp2 -c ssl.h
gcc -DWITH_OPENSSL -o sslclient sslclient.c stdsoap2.c soapC.c soapClient.c -lssl -lcrypto
gcc -DWITH_OPENSSL -o sslserver sslserver.c stdsoap2.c soapC.c soapServer.c -lssl -lcrypto -lpthread

COPYRIGHT AND LICENSE

The gSOAP examples in the 'samples' directory are distributed under the GPL.
See the gpl.txt file for details.

A commercial license is available from Genivia, Inc. Please contact:
contact@genivia.com

gSOAP XML Web services tools
Copyright (C) 2001-2004, Robert van Engelen, Genivia, Inc. All Rights Reserved.

--------------------------------------------------------------------------------
This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place, Suite 330, Boston, MA 02111-1307 USA

Author contact information:
engelen@genivia.com / engelen@acm.org
--------------------------------------------------------------------------------

