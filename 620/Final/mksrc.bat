@setlocal
@set devdir=.

@set DEFOPT=-ST8!n .zip:.jpg:.mp3

zip %DEFOPT% -r -D -X -S alajar-src.zip makefile %devdir%\Admin %devdir%\Alajar %devdir%\AlajarDll %devdir%\AlajarSvc %devdir%\Almonaster %devdir%\AlmonasterHook %devdir%\Asfpp %devdir%\Database %devdir%\OldDatabase %devdir%\DBConv %devdir%\Osal %devdir%\Include -x *.pdb -x *.idb -x *.obj -x *.pch -x *.lib -x *.ilk -x *.exp -x *.dll -x *.exe -x *.plg -x *.opt -x *.ncb -x *.report -x *.stat -x *.dat -x *.log -x *.html -x *.zip -x *.gif -x *.jpg -x *.conf -x *.counters -x *.bmp -x deleteme* -x *.ico -x %devdir%\Alajar\www_root\Almonaster\Resource\* -x *buildlog.htm -x *.enc -x deleteme*.txt -x *.scc -x *.vspscc -x *.suo

gpg --openpgp --output alajar-src.sig --local-user 3F49A1DD --detach-sign alajar-src.zip
gpg --openpgp --verify alajar-src.sig alajar-src.zip

zip %DEFOPT% -D alajar-src-%1.zip alajar-src.zip alajar-src.sig
del alajar-src.sig
del alajar-src.zip

@endlocal