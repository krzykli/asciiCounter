
IF NOT EXIST bin mkdir bin
pushd bin
REM cl /O2 /Zi /EHsc /DNOMINMAX /DTRACY_ENABLE /Fe:letterCounter.exe ../../../libs/tracy/TracyClient.cpp ../main.cpp user32.lib AdvAPI32.lib /I../../../libs /link -subsystem:windows
cl /Od /Zi /EHsc /DNOMINMAX /Fe:letterCounter.exe ../../../libs/tracy/TracyClient.cpp ../main.cpp user32.lib AdvAPI32.lib /I../../../libs /link -subsystem:windows 
