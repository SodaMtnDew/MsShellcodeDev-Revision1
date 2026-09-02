# MsShellcodeDev - Revision 1

The toolset that could help turning C programmers to Shellcode developers.

# Requirements - To Build the Solution
Main part: MsShellcodeDev -- Visual Studio 2026 is the recommended IDE to compile the solution, however, as sln provided, techinically using Visual Studio 2013, 2015, 2017, 2019 & 2022 to build this project is possible, just remember to modify some important settings such as platform toolset and others. Hardware & software requirements are the same as used Visual Studio Version. (For those want to develop offline under a not-so-fat IDE, Visual Studio 2015 might be your best choice.)
As v140xp and v141xp removed from Visual Studio 2026, project "ShellcodeTestLoader" is no more a part of the main solution.

(For Demonstration Video: Click on the Image & Download the MP4 to play)

Demonstration Video showing project "VcxprojToShellcodeCompiler" (the core part) & "ShellcodeTestLoader" (the tester) of MsShellcodeDev built using Visual Studio 2026<br />
[![Watch the video](https://github.com/SodaMtnDew/MsShellcodeDev/blob/main/images/MsShellcodeDev-Built@VS2026.png)](https://github.com/SodaMtnDew/MsShellcodeDev/blob/main/videos/MsShellcodeDev-Built@VS2026.mp4)

Additional tool part 1: ShellcodeTestLoader -- To provide both x86 & x64 shellcode loader exe compatible with Windows XP platform, v140xp or v141xp toolsets (though deprecated) should be installed in addtion to the C Desktop Development Core Components. If using Visual Studio 2017, 2019 & 2022, please keep that in mind; if using Visual Studio 2013 & 2015, both IDEs are already XP compatible.

Demonstration Video showing project "ShellcodeTestLoader" built using Visual Studio 2022
[![Watch the video](https://github.com/SodaMtnDew/MsShellcodeDev/blob/main/images/ShellcodeTestLoader-Built@VS2022.png)](https://github.com/SodaMtnDew/MsShellcodeDev/blob/main/videos/ShellcodeTestLoader-Built@VS2022.mp4)

Additional tool part 2: ShellcodeTestLoader.x86 -- The "ShellcodeTestLoader" has another copy of sln that should be built using Visual Studio 2008 Express with SP1 since windows 2000 compatibility is considered. VS2008 is the last version supporting Windows 2000. Although it's also the first or second version supporting x64, the free "Visual Studio 2008 Express" version did not have a built-in x64 compiler (but not a problem since Windows 2000 did not have x64 version). The compiled ShellcodeTestLoader could be used to load & test x86 shellcode from Windows 2000 to Windows 11 platforms.

Demonstration Video showing project "ShellcodeTestLoader.x86" built using Visual Studio 2008 Express<br />
[![Watch the video](https://github.com/SodaMtnDew/MsShellcodeDev/blob/main/images/ShellcodeTestLoader.x86-Built.png)](https://github.com/SodaMtnDew/MsShellcodeDev/blob/main/videos/ShellcodeTestLoader.x86-Built@VS2008Express.mp4)

# Requirements - To Build the Shellcode
To execute "Vcxproj to Shellcode Compiler" to convert & compile the vcxproj to shellcode, Windows 7 & Visual Studio 2013 (or higher version OSes/IDEs) are required, so the Software/Hardwre requirements just same as Win 7 & VS 2013. The converted project should be developed using pure C & win32/win64 APIs without class & global variable definitions.

Demonstration Video showing the result of executing "VcxprojToShellcodeCompiler" and load "ShellcodeSample" to generate x86 & x64 shellcode files using Visual Studio 2026<br />
[![Watch the video](https://github.com/SodaMtnDew/MsShellcodeDev/blob/main/images/Compiler@VS2026.png)](https://github.com/SodaMtnDew/MsShellcodeDev/blob/main/videos/GenerateShellcode@VS2026.mp4)

Demonstration Video showing the result of executing "VcxprojToShellcodeCompiler" and load "ShellcodeSample" to generate x86 & x64 shellcode files using Visual Studio 2022<br />
[![Watch the video](https://github.com/SodaMtnDew/MsShellcodeDev/blob/main/images/Compiler@VS2022.png)](https://github.com/SodaMtnDew/MsShellcodeDev/blob/main/videos/GenerateShellcode@VS2022.mp4)

Demonstration Video showing the result of executing "VcxprojToShellcodeCompiler" and load "ShellcodeSample" to generate x86 & x64 shellcode files using Visual Studio 2019<br />
[![Watch the video](https://github.com/SodaMtnDew/MsShellcodeDev/blob/main/images/Compiler@VS2019.png)](https://github.com/SodaMtnDew/MsShellcodeDev/blob/main/videos/GenerateShellcode@VS2019.mp4)

Demonstration Video showing the result of executing "VcxprojToShellcodeCompiler" and load "ShellcodeSample" to generate x86 & x64 shellcode files using Visual Studio 2017<br />
[![Watch the video](https://github.com/SodaMtnDew/MsShellcodeDev/blob/main/images/Compiler@VS2017.png)](https://github.com/SodaMtnDew/MsShellcodeDev/blob/main/videos/GenerateShellcode@VS2017.mp4)

Demonstration Video showing the result of executing "VcxprojToShellcodeCompiler" and load "ShellcodeSample" to generate x86 & x64 shellcode files using Visual Studio 2015<br />
[![Watch the video](https://github.com/SodaMtnDew/MsShellcodeDev/blob/main/images/Compiler@VS2015.png)](https://github.com/SodaMtnDew/MsShellcodeDev/blob/main/videos/GenerateShellcode@VS2015.mp4)

Demonstration Video showing the result of executing "VcxprojToShellcodeCompiler" and load "ShellcodeSample" to generate x86 & x64 shellcode files using Visual Studio 2013<br />
[![Watch the video](https://github.com/SodaMtnDew/MsShellcodeDev/blob/main/images/Compiler@VS2013.png)](https://github.com/SodaMtnDew/MsShellcodeDev/blob/main/videos/GenerateShellcode@VS2013.mp4)

# Usage
Both "ShellcodePrototype" & "ShellcodeSample" are programs could be compiled & executed, the outcome are "run calc.exe & pop MessageBox" and "get system info & pop MessageBox;" "Hash of Func & LibName Calculator" could be used to pick existing dll file & export hash values of DLL file name (including file extension, case-insensitive) & API names (case-sensitive) then save the outcome into assigned csv file path; "ShellcodeTestLoader" is the tool to test extracted binary shellcode files using VirtualAlloc API, just be careful that x86/x64 build of loader could only test corresponding shellcode; "Vcxproj to Shellcode Compiler" is GUI application, since the IDE version & "vcvars32.bat" path could be automatically detected, just pick the vcxproj & assign the output name (or not, in this case the name of the converted project will be used) & click on the "Compile" button, the outcome will be there, 2 binary files with "successfully generated" message (as rusult shown in demonstration video) or "one of many different error messages." The folder "PicProj" will not be deleted for checking the real error messages generated by VC compiler of MsDev, just open terminal, move to the PicProj folder & execute "VcxBuild.bat" to see what's wrong. Please be noticed that the "ShellcodeSample" might have newer versions uploaded, please keep following this repository.

For the project to be converted & compiled, developers could type the codes & debug the program in MS IDE. After making sure it works and no class, global variable & any C++ feature that does not supported in "MsShellcodeDev" in all source & header files of your project, it is ready to be converted & compiled into the shellcode format. By adding a special fomratted comment "\/\*\/DLLName.dll\[#Ordinal\]\/\*\/" (#Ordinal is optional, only used when using ordinal instead of hash of api name to get function address) before the 1st api appearance in the function block with no space or any other special character between the comment & the function name (you could add before all appearances if you prefer), you could convert & compile your project into x86/x64 shellcode files after running "VcxprojToShellcodeCompiler.exe" to process your project, or get error messages if failed. In the latter case, please use terminal or powershell, change the working directory to the temporary folder "PicPorj" and execute "VcxBuild.bat" to see what's wrong. Taking a deep look into "ShellcodeSample.vcxproj" in "MsShellcodeDev" to see how a project should be prepared to be converted & compiled.

# Pros Compared to Other Solutions
1. Auto Converting, Compiling & Extracting: No need to assign string variable as local char array manually; using dword to save local array variables could help preventing SIMD instruction extension from being used (in the case, array data will be put into data section); no need to calculate api hashes for converter will do it for you.
2. Supporting Getting API by Ordinal: Some undocumented APIs were provided very early (NT 5.0 or even 4.0) but in older version of DLLs, exported by ordinal (Those ordinals are unchanged bacause of backward compatibility). If want to develop the shellcode compitible for all NT-based platform, try to find those APIs & use them well (You could Google "Geoff Chappel" for the references he provided)
3. Supporting API Forwarding: Using "dumpbin /exports" on DLLs of different versions of Windows, API forwarding could be easily found on many legacy API/DLL pairs. For those using import table in EXE or using "GetProcAddress" API to get function address, that's not a problem; for those enumerating export table from DLLs to find function name hash that match given value to get API address, it will cause error. MsShellcodeDev could detect if API forwarding happened in loaded DLL and get the real function address from the forwarded DLL. (FYI, even in kernel32.dll & user32.dll, those frequently used dll, API forwarding was implemented on Windows 11 25H2, & I'm not trying other versions) Using "ShellcodeSample" of "MsShellcodeDev," since the source code included APIs from netapi32.dll that forwarded to other DLLs & APIs from shlwapi.dll that exported using ordinals, Feature 2 & 3 could be tested using one sample.

# PoC Provided
PoC Part 0: The start of this project -- My patched poison ivy RAT executed on windows 11 & C2 on Windows Server 2025.

PoC Part 1: GetSysInfo, as Part of MsShellcodeDev Soluion - ShellcodeSample.vcxproj.

PoC Part 2: Poc_Reconnaissance -- The "ReconCommand" is the C2 that received SysInfo (including icon images) sent from targets, Please use "VcxprojToShellcodeCompiler" to compile "ReconAgent" as both x86/x64 versions of shellcode (Remember to modify the C2 host name in source code). If "ReconCommand.ini" not provided or "ListenPort" not set, the default is 8080.

PoC Part 3: Poc_Licensing@USB -- Shellcode could also be used for non-malicious purpose, and this is one example. In Poc_Licensing@USB, "GenerateLicenseAtUSB" could generate file "License.ini" and save it in the root directory of the selected thumb drive. The key in the License file is generated using HMAC_SHA512 (Key length=128 bytes, 2nd half is generated using SHA512 output of iSerialNumber of "that" thumb drive) and Username given in the same license file. Use generated "CheckLicense" shellcose could test if the license valid. Since iSerialNumber is the key factor, the License.ini generated & tested valid on thumb drive 1 will not be valid if just copied to thumb drive 2.

P.S. For C2 in PoC, Remember to Set Firewall Rules for C2 Host ...

# List of Demonstration
1. Executing Patched Poison Ivy 2.3.2 Shellcode on Windows 11 26H1, C2 Listening on Port 3460 at Windows Server 2025 Standard Version, Just to Mention How MsShellcodeDev Emerged as an Open Source Project (From What I Learned in Patching Poison Ivy RAT)<br />
   [![Watch the video](https://github.com/SodaMtnDew/MsShellcodeDev/blob/main/images/PatchedPI232@Win11.png)](https://github.com/SodaMtnDew/MsShellcodeDev/blob/main/videos/PatchedPI232@Win11.mp4)
2. Compiling ShellcodeSample, Output as GetSysInfo & Executing Both x86/x64 Versions on Windows 2000 Professional, Windows XP, etc. (Including Windows 11)<br />
   [![Watch the video](https://github.com/SodaMtnDew/MsShellcodeDev/blob/main/images/GetSysInfo@Windows.png)](https://github.com/SodaMtnDew/MsShellcodeDev/blob/main/videos/GetSysInfo@Windows.mp4)
3. Building PoC_Reconnaissance, Compiling ReconAgent as Shellcode & Executing It on Windows XP, Windows 7, Windows 10 CMGE, Windows 11 26H1 zhTW & Windows 11 26H1 enUS, With Different Office Version & PDF Viewer Installed<br />
   [![Watch the video](https://github.com/SodaMtnDew/MsShellcodeDev/blob/main/images/ReconAgents2ReconCommand.png)](https://github.com/SodaMtnDew/MsShellcodeDev/blob/main/videos/ReconAgents2ReconCommand.mp4)
4. Building PoC_Licensing@USB, Compiling CheckLicense as Shellcode & Executing It on Windows 11 26H1 enUS, With Same License.ini File on Different USB Thumbdrive<br />
   [![Watch the video](https://github.com/SodaMtnDew/MsShellcodeDev/blob/main/images/CheckLicense@USB.png)](https://github.com/SodaMtnDew/MsShellcodeDev/blob/main/videos/CheckLicense@USB.mp4)
5. Popup MessageBox (Screenshot Only) Displayed Afetr Executing GetSysInfo Shellcode File on Windows 11 Pro on Arm64<br />
   ![image](https://github.com/SodaMtnDew/MsShellcodeDev/blob/main/images/SysInfo.Win11.Arm64.Pro.png)
