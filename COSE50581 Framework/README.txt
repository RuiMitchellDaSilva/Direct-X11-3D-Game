If you are running on Windows 7, this programj will not run and will come up with this message :

"The procedure entry point CreateFile2 could not be located in the dynamic link library KERNEL32.dll"

This is due to that Visual Studio 2013 assumes you are running the application in Windows 8, and so all calls to the operating system
will be calibrated to Windows 8 instead of Windows 7.

So here's the solution:

1). Open the project in Visual Studio 2013
2). Right-click the project name within the Solution Explorer on the right-hand side (where you can see all your source and header files,
"DX11 Framework" is the name in this current project).
3). Go into properties.
4). Configuration Properties drop-down, C/C++ drop-down, Perprocessor.
5). On "Preprocessor Definitions", add the following line of code after the statement "WIN32;" : _WIN32_WINNT=0x0601;

The program should now run.


DDS File Format
A8B8G8R8 (MipMap or not)
