// OpenCppCoverage is an open source code coverage for C++.
// Copyright (C) 2014 OpenCppCoverage
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "stdafx.h"
#include "Process.hpp"

#include <Windows.h>
#include <boost/optional.hpp>

#include "Tools/Log.hpp"
#include "Tools/Tool.hpp"

#include "StartInfo.hpp"
#include "CppCoverageException.hpp"
#include <thread>
#include <iostream>

namespace CppCoverage
{
	namespace
	{
		//---------------------------------------------------------------------
		boost::optional<std::vector<wchar_t>> 
			CreateCommandLine(const std::vector<std::wstring>& arguments)
		{
			boost::optional<std::vector<wchar_t>> commandLine;

			if (!arguments.empty())
			{				
				std::vector<wchar_t> buffer;
				for (const auto& argument : arguments)
				{
					buffer.push_back(L'\"');
					buffer.insert(buffer.end(), argument.begin(), argument.end());
					buffer.push_back(L'\"');
					buffer.push_back(L' ');
				}
					
				buffer.push_back(L'\0');
				return buffer;
			}

			return commandLine;
		}		
	}

	const std::wstring Process::CannotFindPathMessage = L"Cannot find path: ";
	const std::wstring Process::CheckIfValidExecutableMessage =
	    L"Cannot run process, check if it is a valid executable:";

	//-------------------------------------------------------------------------
	Process::Process(const StartInfo& startInfo)
		: startInfo_(startInfo)
	{		
	}

	//-------------------------------------------------------------------------
	Process::~Process()
	{
		if (processInformation_)
		{			
			auto hProcess = processInformation_->hProcess;
			if (hProcess && !CloseHandle(hProcess))
				LOG_ERROR << "Cannot close process handle";

			auto hThread = processInformation_->hThread;
			if (hThread && !CloseHandle(hThread))
				LOG_ERROR << "Cannot close thread handle";
		}
	}

	//-------------------------------------------------------------------------
	void Process::Start(DWORD creationFlags)
	{
		if (processInformation_)
			THROW(L"Process already started");

		STARTUPINFO lpStartupInfo;

		ZeroMemory(&lpStartupInfo, sizeof(lpStartupInfo));
		lpStartupInfo.cb = sizeof(STARTUPINFO);

		HANDLE hStdInRead = nullptr, hStdInWrite = nullptr;
		HANDLE hStdOutRead = nullptr, hStdOutWrite = nullptr;

		// Check if standard I/O redirection is enabled
		if (startInfo_.GetEnableStd()) {
			SECURITY_ATTRIBUTES sa;
			sa.nLength = sizeof(SECURITY_ATTRIBUTES);
			sa.bInheritHandle = TRUE;
			sa.lpSecurityDescriptor = nullptr;

			if (!CreatePipe(&hStdInRead, &hStdInWrite, &sa, 0)) {
				throw std::runtime_error("Failed to create stdin pipe");
			}

			if (!SetHandleInformation(hStdInWrite, HANDLE_FLAG_INHERIT, 0)) {
				throw std::runtime_error("Failed to set handle information for stdin");
			}

			if (!CreatePipe(&hStdOutRead, &hStdOutWrite, &sa, 0)) {
				throw std::runtime_error("Failed to create stdout pipe");
			}

			if (!SetHandleInformation(hStdOutRead, HANDLE_FLAG_INHERIT, 0)) {
				throw std::runtime_error("Failed to set handle information for stdout");
			}

			lpStartupInfo.hStdInput = hStdInRead;
			lpStartupInfo.hStdOutput = hStdOutWrite;
			lpStartupInfo.hStdError = hStdOutWrite;
			lpStartupInfo.dwFlags |= STARTF_USESTDHANDLES;
		}

		const auto* workindDirectory = startInfo_.GetWorkingDirectory();
		auto optionalCommandLine = CreateCommandLine(startInfo_.GetArguments());
		auto commandLine = (optionalCommandLine) ? &(*optionalCommandLine)[0] : nullptr;

		processInformation_ = PROCESS_INFORMATION{};
		if (!CreateProcess(
			nullptr,
			commandLine,
			nullptr,
			nullptr,
			startInfo_.GetEnableStd(),
			creationFlags,
			nullptr,
			(workindDirectory) ? workindDirectory->c_str() : nullptr,
			&lpStartupInfo,
			&processInformation_.get()
			))
		{
			std::wostringstream ostr;

			if (!Tools::FileExists(startInfo_.GetPath()))
				ostr << CannotFindPathMessage + startInfo_.GetPath().wstring();
			else
			{
				ostr
				    << CheckIfValidExecutableMessage
				    << std::endl;

#ifndef _WIN64
				ostr << L"\n*** This version support only 32 bits executable "
				        L"***.\n\n";
#endif
				ostr << startInfo_
				     << CppCoverage::GetErrorMessage(GetLastError());
			}
			// Close handles if opened
			if (hStdInRead) CloseHandle(hStdInRead);
			if (hStdInWrite) CloseHandle(hStdInWrite);
			if (hStdOutRead) CloseHandle(hStdOutRead);
			if (hStdOutWrite) CloseHandle(hStdOutWrite);

			throw std::runtime_error(Tools::ToLocalString(ostr.str()));
		}

		// Close unnecessary pipe ends in the parent process
		if (startInfo_.GetEnableStd()) {
			CloseHandle(hStdInRead);
			CloseHandle(hStdOutWrite);

			// Thread to read child process output and send to parent stdout
			std::thread([hStdOutRead]() {
				char buffer[4096];
				DWORD bytesRead;

				while (true) {
					if (!ReadFile(hStdOutRead, buffer, sizeof(buffer) - 1, &bytesRead, nullptr) || bytesRead == 0)
						break;

					buffer[bytesRead] = '\0';
					if (!WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), buffer, bytesRead, nullptr, nullptr))
						break;
				}

				CloseHandle(hStdOutRead);
			}).detach();

			std::thread([hStdInWrite]() {
				char buffer[4096];

				while (true) {
					std::cin.getline(buffer, sizeof(buffer));
					DWORD bytesToWrite = static_cast<DWORD>(std::strlen(buffer));

					if (bytesToWrite == 0)
						continue;

					buffer[bytesToWrite] = '\n';

					DWORD bytesWritten;
					if (!WriteFile(hStdInWrite, buffer, bytesToWrite + 1, &bytesWritten, nullptr))
						break;
				}

				CloseHandle(hStdInWrite);
			}).detach();
		}
	}
}
