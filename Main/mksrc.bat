@setlocal
@set devdir=.

@set DEFOPT=-ST8!n .zip:.jpg:.mp3
@set VERSION=622

zip %DEFOPT% -r -D -X -S alajar-src.zip makefile %devdir%\Admin %devdir%\Alajar %devdir%\AlajarDll %devdir%\AlajarSvc %devdir%\Almonaster %devdir%\AlmonasterHook %devdir%\Asfpp %devdir%\Database %devdir%\DBConv %devdir%\Osal %devdir%\SDK -x *.pdb -x *.idb -x *.obj -x *.pch -x *.lib -x *.ilk -x *.exp -x *.dll -x *.exe -x *.plg -x *.opt -x *.ncb -x *.report -x *.stat -x *.dat -x *.log -x *.html -x *.zip -x *.gif -x *.jpg -x *.conf -x *.counters -x *.bmp -x deleteme* -x *.ico -x %devdir%\Alajar\www_root\Almonaster\Resource\* -x *buildlog.htm -x *.enc -x deleteme*.txt -x *.scc -x *.vspscc -x *.vssscc -x *.suo -x *.scc -x *.ipch -x *.vspscc -x *.tlog -x *.res -x *.manifest -x *.opensdf -x *.sdf -x *user -x *manifest.rc -x *lastbuildstate -x *.dep -x *.old -x *.cer -x *.pfx -x UpgradeLog*

gpg --openpgp --local-user 48A9C666 --print-md md5 alajar-src.zip > alajar-src.md5
gpg --openpgp --output alajar-src.md5.asc --local-user D79C1F6B --clearsign alajar-src.md5
gpg --openpgp --verify alajar-src.md5.asc

zip %DEFOPT% -D alajar-src-%VERSION%.zip alajar-src.zip alajar-src.md5 alajar-src.md5.asc

del alajar-src.zip
del alajar-src.md5
del alajar-src.md5.asc

@endlocal