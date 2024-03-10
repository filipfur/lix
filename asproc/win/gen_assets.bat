SET mypath=%~dp0
echo %mypath:~0,-1%

set ASPROC_HOME=%mypath%\..
set ASPROC=%ASPROC_HOME%\bin\asproc

%ASPROC% --version-override "" -s assets/shaders gen/shaders
%ASPROC% --flip-y --convert-to-srgb -i assets/images gen/images
%ASPROC% --convert-to-srgb -o assets/objects gen/objects
%ASPROC% -f assets/fonts gen/fonts