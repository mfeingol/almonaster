@setlocal
@set DEFOPT=-ST8!n .zip:.jpg:.mp3

@set PLAT=%1
@set VERSION=622
@set PLATDIR=Drop\%PLAT%\Release\bin

pushd %PLATDIR%
zip %DEFOPT% -r -D -X -S %temp%\alajar-%PLAT%.zip . -x *.counters -x *.log -x *.dat -x *.report -x *.stat -x *.vcc -x *.scc
popd

move %temp%\alajar-%PLAT%.zip .

gpg --openpgp --local-user 48A9C666 --print-md md5 alajar-%PLAT%.zip > alajar-%PLAT%.md5
gpg --openpgp --output alajar-%PLAT%.md5.asc --local-user D79C1F6B --clearsign alajar-%PLAT%.md5
gpg --openpgp --verify alajar-%PLAT%.md5.asc

zip %DEFOPT% -D alajar-%PLAT%-%VERSION%.zip alajar-%PLAT%.zip alajar-%PLAT%.md5 alajar-%PLAT%.md5.asc

del alajar-%PLAT%.zip
del alajar-%PLAT%.md5
del alajar-%PLAT%.md5.asc

@endlocal