@setlocal
@set devdir=.

@set DEFOPT=-ST9!n .zip:.jpg:.mp3

zip %DEFOPT% -r -D alajar-src.zip %devdir%\Admin %devdir%\Alajar %devdir%\AlajarDll %devdir%\AlajarSvc %devdir%\Almonaster %devdir%\AlmonasterHook %devdir%\Asfpp %devdir%\Database %devdir%\Database3 %devdir%\DBConv %devdir%\Osal %devdir%\Include -x *.pdb -x *.idb -x *.obj -x *.pch -x *.lib -x *.ilk -x *.exp -x *.dll -x *.exe -x *.plg -x *.opt -x *.ncb -x *.report -x *.stat -x *.dat -x *.log -x *.html -x *.zip -x *.gif -x *.jpg -x *.conf -x *.counters -x *.bmp -x deleteme* -x *.ico -x %devdir%\Alajar\www_root\Almonaster\Resource\AlienUploads\*

@endlocal