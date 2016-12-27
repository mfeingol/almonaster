REM E = almonaster@myrealbox.com, CN = prometheus.almonaster.net, OU = Almonaster, L = Bellevue, WA, C = US

openssl genrsa -out server.key 2048
openssl req -new -x509 -key server.key -out cert.pem -days 1825

