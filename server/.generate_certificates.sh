mkdir $INSTALLDIR/certificates && cd $INSTALLDIR/certificates/ && openssl req -x509 -newkey rsa:4096 -keyout CA-key.pem -out CA-cert.pem -days 365
