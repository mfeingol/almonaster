@setlocal
@set DEFOPT=-ST8!n .zip:.jpg:.mp3

cd release
zip %DEFOPT% -r -D -X -S ..\alajar-win32.zip . -x *.counters -x *.log -x *.dat -x *.report -x *.stat -x *.vcc

gpg --openpgp --output ..\alajar-win32.sig --local-user 3F49A1DD --detach-sign ..\alajar-win32.zip
gpg --openpgp --verify ..\alajar-win32.sig ..\alajar-win32.zip

zip %DEFOPT% -D ..\alajar-win32-%1.zip ..\alajar-win32.zip ..\alajar-win32.sig
del ..\alajar-win32.sig
del ..\alajar-win32.zip

@endlocal