REM E = root@almonaster.net, CN = falcon.almonaster.net, OU = Almonaster, L = Redmond, WA, C = US

openssl genrsa -out server.key 2048
openssl req -new -x509 -key server.key -out cert.pem -days 1825

