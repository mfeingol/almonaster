@setlocal
@set DEFOPT=-ST8!n .zip:.jpg:.mp3

@set PLAT=%1
@set VERSION=622
@set PLATDIR=Drop\%PLAT%\Release\sdk

pushd %PLATDIR%
zip %DEFOPT% -r -D -X -S %temp%\alajar-sdk-%PLAT%.zip .
popd

move %temp%\alajar-sdk-%PLAT%.zip .

gpg --openpgp --local-user 48A9C666 --print-md md5 alajar-sdk-%PLAT%.zip > alajar-sdk-%PLAT%.md5
gpg --openpgp --output alajar-sdk-%PLAT%.md5.asc --local-user D79C1F6B --clearsign alajar-sdk-%PLAT%.md5
gpg --openpgp --verify alajar-sdk-%PLAT%.md5.asc

zip %DEFOPT% -D alajar-sdk-%PLAT%-%VERSION%.zip alajar-sdk-%PLAT%.zip alajar-sdk-%PLAT%.md5 alajar-sdk-%PLAT%.md5.asc

del alajar-sdk-%PLAT%.zip
del alajar-sdk-%PLAT%.md5
del alajar-sdk-%PLAT%.md5.asc

@endlocal