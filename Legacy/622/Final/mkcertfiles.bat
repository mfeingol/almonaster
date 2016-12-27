setlocal
set server=Smoke

rem Create server cert
makecert.exe -len 2048 -pe -a sha1 -cy end -n "CN=%server%" -sky exchange -sv server.pvk server.cer
cert2spc.exe server.cer server.spc
pvkimprt.exe -pfx server.spc server.pvk

rem import files
rem pvkimprt.exe -import server.spc server.pvk

endlocal