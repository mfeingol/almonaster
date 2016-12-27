@setlocal

@set DEFOPT=-ST8!n .zip:.jpg:.mp3

zip %DEFOPT% -r -D -X -S alajar-sdk-win32.zip sdk\* -x sdk\database.lib

gpg --openpgp --local-user D79C1F6B --print-md md5 alajar-sdk-win32.zip > alajar-sdk-win32.md5
gpg --openpgp --output alajar-sdk-win32.md5.asc --local-user D79C1F6B --clearsign alajar-sdk-win32.md5
gpg --openpgp --verify alajar-sdk-win32.md5.asc

zip %DEFOPT% -D alajar-sdk-win32-%1.zip alajar-sdk-win32.zip alajar-sdk-win32.md5 alajar-sdk-win32.md5.asc

del alajar-sdk-win32.zip
del alajar-sdk-win32.md5
del alajar-sdk-win32.md5.asc

@endlocal