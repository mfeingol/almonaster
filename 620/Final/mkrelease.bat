@setlocal
@set DEFOPT=-ST8!n .zip:.jpg:.mp3

cd release
zip %DEFOPT% -r -D -X -S ..\alajar-win32.zip . -x *.counters -x *.log -x *.dat -x *.report -x *.stat -x *.vcc -x *.scc
cd ..

gpg --openpgp --local-user D79C1F6B --print-md md5 alajar-win32.zip > alajar-win32.md5
gpg --openpgp --output alajar-win32.md5.asc --local-user D79C1F6B --clearsign alajar-win32.md5
gpg --openpgp --verify alajar-win32.md5.asc

zip %DEFOPT% -D alajar-win32-%1.zip alajar-win32.zip alajar-win32.md5 alajar-win32.md5.asc

del alajar-win32.zip
del alajar-win32.md5
del alajar-win32.md5.asc

@endlocal