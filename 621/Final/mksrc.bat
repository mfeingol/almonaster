@setlocal
@set devdir=.

@set DEFOPT=-ST8!n .zip:.jpg:.mp3

zip %DEFOPT% -r -D -X -S alajar-src.zip makefile %devdir%\Admin %devdir%\Alajar %devdir%\AlajarDll %devdir%\AlajarSvc %devdir%\Almonaster %devdir%\AlmonasterHook %devdir%\Asfpp %devdir%\Database %devdir%\DBConv %devdir%\Osal %devdir%\Include -x *.pdb -x *.idb -x *.obj -x *.pch -x *.lib -x *.ilk -x *.exp -x *.dll -x *.exe -x *.plg -x *.opt -x *.ncb -x *.report -x *.stat -x *.dat -x *.log -x *.html -x *.zip -x *.gif -x *.jpg -x *.conf -x *.counters -x *.bmp -x deleteme* -x *.ico -x %devdir%\Alajar\www_root\Almonaster\Resource\* -x *buildlog.htm -x *.enc -x deleteme*.txt -x *.scc -x *.vspscc -x *.vssscc -x *.suo -x *.manifest -x *.res -x *.dep -x *.user -x *.old -x *.cer -x *.pfx -x *.pvk -x *.key -x *.pem

gpg --openpgp --local-user D79C1F6B --print-md md5 alajar-src.zip > alajar-src.md5
gpg --openpgp --output alajar-src.md5.asc --local-user D79C1F6B --clearsign alajar-src.md5
gpg --openpgp --verify alajar-src.md5.asc

zip %DEFOPT% -D alajar-src-%1.zip alajar-src.zip alajar-src.md5 alajar-src.md5.asc

del alajar-src.zip
del alajar-src.md5
del alajar-src.md5.asc

@endlocal