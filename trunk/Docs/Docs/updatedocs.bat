cd Config
doxygen html_update.cfg
cd ..
cd temp\html
copy ..\..\copyifdiff.bat
for %%d in (*.*) do copyifdiff %%d
del *.* /Q


