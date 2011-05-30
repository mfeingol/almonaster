@setlocal
@set devdir=.

@set DEFOPT=-ST9!n .zip:.jpg:.mp3

cd release
zip %DEFOPT% -r -D %devdir%\..\alajar-win32.zip . -x *.counters -x *.log -x *.dat -x *.report -x *.stat
cd ..

@endlocal