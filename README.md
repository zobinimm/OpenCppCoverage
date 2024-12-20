![](https://github.com/OpenCppCoverage/OpenCppCoverage/workflows/Unit%20tests/badge.svg)
# OpenCppCoverage

OpenCppCoverage is an open source code coverage tool for C++ under Windows.

The main usage is for unit testing coverage, but you can also use it to know the executed lines in a program for debugging purpose.

## Newly added features
Support standard input-output communication between processes
Add --std after starting the exe parameter

## Building OpenCppCoverage
OpenCppCoverage edited this page on Apr 9, 2020 · 5 revisions
This page describes how to build OpenCppCoverage from sources.

## Requirements
Visual Studio 2019 version 16.4.X Desktop development with C++
Visual Studio 2019 modules MFC and ATL support (x86 and x64) and Visual C++ ATL support
Git (git.exe must be in your PATH)
## Get sources
Simply run the command: git clone https://github.com/OpenCppCoverage/OpenCppCoverage.git.

## Install third party libraries
Run the following command at the root of the repository: powershell.exe -executionpolicy bypass -File InstallThirdPartyLibraries.ps1.

## Compilation
Open CppCoverage.sln and build the project OpenCppCoverage.

Remarks:

You need Visual Studio 2019 module C++/CLI support to load the project TestCppCli and to run the test CppCliTest.ManagedUnManagedModule
You need Visual Studio 2013 to run the test CodeCoverageRunnerTest.OptimizedBuild
## Run OpenCppCoverage
msdia140.dll must be available in the $(TargetDir) of the project:

Copy VISUAL_STUDIO_ROOT\DIA SDK\bin\amd64\msdia140.dll to the folders x64\Debug and x64\Release
Copy VISUAL_STUDIO_ROOT\DIA SDK\bin\msdia140.dll to the folders Debug and Release.
To run OpenCppCoverage, right click on the project OpenCppCoverage, select Set as StartUp Project and run the project.

## Features:
- **Visual Studio support**: Support compiler with program database file (.pdb).
- **Non intrusive**: Just run your program with OpenCppCoverage, no need to recompile your application.
- **HTML reporting**
- **Line coverage**.
- **Run as Visual Studio Plugin**: See [here](https://github.com/OpenCppCoverage/OpenCppCoveragePlugin) for more information.
- **Jenkins support**: See [here](https://github.com/OpenCppCoverage/OpenCppCoverage/wiki/Jenkins) for more information.
- **Support optimized build**.
- **Exclude a line based on a regular expression**.
- **Child processes coverage**.
- **Coverage aggregation**: Run several code coverages and merge them into a single report.
 
## Requirements
- Windows Vista or higher.
- Microsoft Visual Studio 2008 or higher all editions **including Express edition**. It should also work with previous version of Visual Studio.

## Download
OpenCppCoverage can be downloaded from [here](../../releases).

## Usage
You can simply run the following command:

```OpenCppCoverage.exe --sources MySourcePath* -- YourProgram.exe arg1 arg2```

For example, *MySourcePath* can be *MyProject*, if your sources are located in *C:\Dev\MyProject*.

See [Getting Started](https://github.com/OpenCppCoverage/OpenCppCoverage/wiki) for more information about the usage.
You can also have a look at [Command-line reference](https://github.com/OpenCppCoverage/OpenCppCoverage/wiki/Command-line-reference).
