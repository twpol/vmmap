//------------------------------------------------------------------------------
// http://twpol.dyndns.org/projects/vmmap
// License: New BSD License (BSD).
//------------------------------------------------------------------------------

#include "StdAfx.h"
#include "process.h"

process::process(void) : _process_id(0)
{
}

process::process(DWORD pid) : _process_id(pid)
{
	// NT API Support:
	//   5.0  GetModuleFileNameEx
	//   5.1  GetProcessImageFileName
	//   5.0  GetProcessTimes
	//   5.0  GetTokenInformation
	//   5.0  LookupAccountSid
	//   5.0  OpenProcess
	//   5.0  OpenProcessToken
	//   6.0  QueryFullProcessImageName
#if _WIN32_WINNT < 0x0600
	//HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
#else
	//HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
#endif
	HANDLE hProcess = OpenProcess(MAXIMUM_ALLOWED, FALSE, pid);
	if (NULL != hProcess) {
		FILETIME ctime = { 0, 0 };
		FILETIME etime = { 0, 0 };
		FILETIME ktime = { 0, 0 };
		FILETIME utime = { 0, 0 };
		if (GetProcessTimes(hProcess, &ctime, &etime, &ktime, &utime)) {
			_creation_time = ctime;
		} else {
			std::tcerr << std::dec << pid << ": GetProcessTimes failed: " << std::hex << std::setw(8) << std::setfill(_T('0')) << GetLastError() << std::endl;
		}

#if _WIN32_WINNT < 0x0600
		std::tstring image(MAX_PATH, '\0');
		// This needs PROCESS_VM_READ.
		DWORD image_length = GetModuleFileNameEx(hProcess, NULL, &image[0], image.size());
		if (image_length > 0) {
			image.resize(image_length);
		} else {
			std::tcerr << std::dec << pid << ": GetModuleFileNameEx failed: " << std::hex << std::setw(8) << std::setfill(_T('0')) << GetLastError() << std::endl;
		}
#else
		std::tstring image(MAX_PATH, '\0');
		DWORD image_length = image.size();
		// This needs PROCESS_QUERY_LIMITED_INFORMATION.
		if (QueryFullProcessImageName(hProcess, 0, &image[0], &image_length)) {
			image.resize(image_length);
		} else {
			std::tcerr << std::dec << pid << ": QueryFullProcessImageName failed: " << std::hex << std::setw(8) << std::setfill(_T('0')) << GetLastError() << std::endl;
		}
#endif

		_image_filepath.assign(image);
		std::tstring::size_type last_slash = _image_filepath.rfind('\\'); 
		if (last_slash != std::tstring::npos) {
			_image_filename = _image_filepath.substr(++last_slash, _image_filepath.size());
		}

		HANDLE hProcessToken;
		if (OpenProcessToken(hProcess, TOKEN_QUERY, &hProcessToken)) {
			DWORD data_length = 0;
			if (!GetTokenInformation(hProcessToken, TokenUser, NULL, 0, &data_length) && (GetLastError() == ERROR_INSUFFICIENT_BUFFER)) {
				void* data = new byte[data_length];
				if (GetTokenInformation(hProcessToken, TokenUser, data, data_length, &data_length)) {
					TOKEN_USER* user = static_cast<TOKEN_USER*>(data);
					std::tstring name(MAX_NAME, '\0');
					DWORD name_length = name.size();
					std::tstring domain(MAX_NAME, '\0');
					DWORD domain_length = domain.size();
					SID_NAME_USE type;
					if (LookupAccountSid(NULL, user->User.Sid, &name[0], &name_length, &domain[0], &domain_length, &type)) {
						name.resize(name_length);
						domain.resize(domain_length);
						_username = _T("");
						if (domain.size()) {
							_username += domain;
							_username += _T("\\");
						}
						_username += name;
					} else {
						std::tcerr << std::dec << pid << ": LookupAccountSid failed: " << std::hex << std::setw(8) << std::setfill(_T('0')) << GetLastError() << std::endl;
					}
				} else {
					std::tcerr << std::dec << pid << ": GetTokenInformation(2) failed: " << std::hex << std::setw(8) << std::setfill(_T('0')) << GetLastError() << std::endl;
				}
				delete data;
			} else {
				std::tcerr << std::dec << pid << ": GetTokenInformation failed: " << std::hex << std::setw(8) << std::setfill(_T('0')) << GetLastError() << std::endl;
			}
			CloseHandle(hProcessToken);
		} else {
			std::tcerr << std::dec << pid << ": OpenProcessToken failed: " << std::hex << std::setw(8) << std::setfill(_T('0')) << GetLastError() << std::endl;
		}

		CloseHandle(hProcess);
	}
}

process::~process(void)
{
}
