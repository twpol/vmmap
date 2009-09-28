//------------------------------------------------------------------------------
// http://twpol.dyndns.org/projects/vmmap
// License: Microsoft Public License (Ms-PL).
//------------------------------------------------------------------------------

#include "StdAfx.h"
#include "commandline_options.h"
#include "string_utils.h"

commandline_options::commandline_options(const std::tstring settings)
{
	// API: GetCommandLine: Windows 2000 Pro/Server.
	const TCHAR* commandline_ptr = GetCommandLine();
	std::tstring commandline(commandline_ptr, tstrlen(commandline_ptr));

	std::map<std::tstring, std::pair<std::tstring, bool>, tstring_caseless_compare_t> options;
	std::tstring optionName(_T(""));
	bool optionHasValue = false;
	bool optionInName = true;
	std::tstring optionBuffer(_T(""));
	std::tstring settings_safe(settings);
	settings_safe += _T(",");
	for (std::tstring::const_iterator ch = settings_safe.begin(); ch != settings_safe.end(); ch++) {
		//std::tcout << "CHAR: " << *ch << "  [" << optionName << "][" << optionHasValue << "][" << optionInName << "][" << optionBuffer << "]" << std::endl;
		switch (*ch) {
			case ':':
				optionHasValue = true;
				break;
			case ' ':
				if (optionInName) optionBuffer = optionName;
				{
					std::pair<std::tstring, bool> data(optionName, optionHasValue);
					//std::tcout << "SETTING: " << optionBuffer << " -> " << data.first << " (" << data.second << ")" << std::endl;
					options[optionBuffer] = data;
				}
				optionInName = false;
				optionBuffer = _T("");
				break;
			case ',':
				if (optionInName) optionBuffer = optionName;
				{
					std::pair<std::tstring, bool> data(optionName, optionHasValue);
					//std::tcout << "SETTING: " << optionBuffer << " -> " << data.first << " (" << data.second << ")" << std::endl;
					options[optionBuffer] = data;
				}
				optionName = _T("");
				optionHasValue = false;
				optionInName = true;
				optionBuffer = _T("");
				break;
			default:
				if (optionInName) {
					optionName += *ch;
				} else {
					optionBuffer += *ch;
				}
				break;
		}
	}

	std::list<std::tstring> arguments;
	bool optionInQuotes = false;
	optionBuffer = _T("");
	for (std::tstring::iterator ch = commandline.begin(); ch != commandline.end(); ch++) {
		//std::tcout << "CHAR: " << *ch << "  [" << optionBuffer << "][" << optionInQuotes << "]" << std::endl;
		switch (*ch) {
			case '"':
				optionInQuotes = !optionInQuotes;
				break;
			case '\\': {
				std::tstring::iterator old_ch = ch;
				int escape_count = 1;
				std::tstring escape_buffer;
				while ((++ch != commandline.end()) && (*ch == '\\')) {
					escape_count++;
					if (escape_count % 2 == 0) escape_buffer += '\\';
				}
				if ((ch == commandline.end()) || (*ch != '"')) {
					optionBuffer += commandline.substr(old_ch - commandline.begin(), ch - old_ch + 1);
				} else {
					optionBuffer += escape_buffer;
					if (escape_count % 2 == 0) ch--;
					else optionBuffer += *ch;
				}
				break;
			}
			case ' ':
			case '\t':
				if (!optionInQuotes) {
					if (optionBuffer.size()) {
						//std::tcout << "OPTION: " << optionBuffer << std::endl;
						arguments.push_back(optionBuffer);
						optionBuffer = _T("");
					}
					break;
				}
			default:
				optionBuffer += *ch;
				break;
		}
	}
	if (optionBuffer.size()) {
		//std::tcout << "OPTION: " << optionBuffer << std::endl;
		arguments.push_back(optionBuffer);
	}

	//for (std::map<std::tstring, std::pair<std::tstring, bool>, tstring_caseless_compare_t>::iterator opt = options.begin(); opt != options.end(); opt++) {
	//	_arguments[*opt];
	//}

	std::tstring lastFlag(_T(""));
	for (std::list<std::tstring>::iterator arg = arguments.begin(); arg != arguments.end(); arg++) {
		if (((*arg)[0] == '/') || ((*arg)[0] == '-')) {
			lastFlag = _T("");
			std::tstring flag = (*arg).substr(1);
			std::map<std::tstring, std::pair<std::tstring, bool>, tstring_caseless_compare_t>::iterator opt = options.find(flag);
			if (opt == options.end()) {
				std::tcerr << "Unknown option: " << flag << std::endl;
				continue;
			}
			if ((*opt).second.second) {
				lastFlag = (*opt).second.first;
			} else {
				_arguments[(*opt).second.first].push_back(_T(""));
			}
		} else if (lastFlag.size()) {
			_arguments[lastFlag].push_back(*arg);
		} else {
			_files.push_back(*arg);
		}
	}
}

const bool commandline_options::has(const std::tstring option)
{
	std::map<const std::tstring, std::list<const std::tstring> >::const_iterator item = _arguments.find(option);
	return (item != _arguments.end());
}

const std::tstring& commandline_options::get(const std::tstring option)
{
	std::map<const std::tstring, std::list<const std::tstring> >::const_iterator item = _arguments.find(option);
	return *item->second.begin();
}

const std::list<const std::tstring>& commandline_options::gets(const std::tstring option)
{
	std::map<const std::tstring, std::list<const std::tstring> >::const_iterator item = _arguments.find(option);
	return item->second;
}
