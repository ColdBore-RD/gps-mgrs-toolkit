# Host test

Runs the `MGRS` library on a PC against fixed reference strings, so a math change can be checked
without flashing a board. `shim/Arduino.h` is an empty stand-in for the Arduino header.

```bash
# g++ / clang
g++ -std=c++11 -Itests/host/shim -Ifirmware/MGRS/src tests/host/host_test.cpp firmware/MGRS/src/MGRS.cpp firmware/MGRS/src/UTM.cpp -o host_test && ./host_test
```

```powershell
# MSVC (Developer PowerShell)
cl /nologo /EHsc /Itests\host\shim /Ifirmware\MGRS\src tests\host\host_test.cpp firmware\MGRS\src\MGRS.cpp firmware\MGRS\src\UTM.cpp /Fe:host_test.exe; .\host_test.exe
```

Exit code 0 and `ALL PASS` means every reference string matched.
