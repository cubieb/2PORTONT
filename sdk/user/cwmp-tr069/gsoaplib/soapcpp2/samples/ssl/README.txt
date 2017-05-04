How to generate self-signed root certificate and client.pem/server.pem

Create HOME/CA directory and copy openssl.cnf, root.sh, and cert.sh to this dir.

Change dir to HOME/CA

To generate the root CA (root.pem and cacert.pem):

./root.sh

Save the root.pem and the passphrase in a safe place (don't distribute them!).
You need these to create certificates.

The root.pem and cacert.pem are valid for three years. Don't repeat this step
until the certificate expires.

To generate the client.pem key file (enter "password" and "localhost" and use
the root CA passphrase when prompted to sign the client certificate):

./cert.sh client

To generate the server.pem key file (enter "password" and "localhost" and use
the root CA passphrase when prompted to sign the server certificate):

./cert.sh server

The client.pem and server.pem certificates are valid for one year.

Required files in HOME/CA directory:

openssl.cnf
root.sh
cert.sh

Files generated:

cacert.pem	root's certificate for distribution
root.pem	root CA (to sign client/server key files, do not distribute)
rootkey.pem	private key
rootreq.pem	sign request
root.srl	serial number

client.pem	client key file
clientkey.pem	private key
clientreq.pem	sign request

server.pem	server key file
serverkey.pem	private key
serverreq.pem	sign request

