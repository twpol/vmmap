//------------------------------------------------------------------------------
// http://twpol.dyndns.org/projects/vmmap
// License: New BSD License (BSD).
//------------------------------------------------------------------------------

#include "StdAfx.h"
#include "commandline_options.h"
#include "string_utils.h"

commandline_options::commandline_options(const std::tstring settings)
{
	// NT API Support:
	//   5.0  GetCommandLine

	// Alias -> (Canonical Name, Has Value)
	std::map<std::tstring, std::pair<std::tstring, bool>, tstring_caseless_compare_t> options;
	std::tstring optionName(_T(""));
	std::tstring optionAlias(_T(""));
	bool inOptionName = true;
	bool optionHasValue = false;
	// Add the extra comma locally to force the code to process the last item.
	std::tstring settings_safe(settings);
	settings_safe += _T(",");
	for (std::tstring::const_iterator ch = settings_safe.begin(); ch != settings_safe.end(); ch++) {
		switch (*ch) {
			case ':':
				optionHasValue = true;
				break;
			case ' ':
				if (inOptionName) {
					optionAlias = optionName;
				}
				{
					std::pair<std::tstring, bool> data(optionName, optionHasValue);
					options[optionAlias] = data;
				}
				inOptionName = false;
				optionAlias = _T("");
				break;
			case ',':
				if (inOptionName) {
					optionAlias = optionName;
				}
				{
					std::pair<std::tstring, bool> data(optionName, optionHasValue);
					options[optionAlias] = data;
				}
				optionName = _T("");
				optionHasValue = false;
				inOptionName = true;
				optionAlias = _T("");
				break;
			default:
				if (inOptionName) {
					optionName += *ch;
				} else {
					optionAlias += *ch;
				}
				break;
		}
	}

	// Get the process' command line. We'll do our own parsing of it, thanks.
	const TCHAR* commandline_ptr = GetCommandLine();
	std::tstring commandline(commandline_ptr, tstrlen(commandline_ptr));

	// List of whitespace-separated arguments from the command line.
	std::list<std::tstring> arguments;
	bool inArgumentQuotes = false;
	optionName = _T("");
	for (std::tstring::iterator ch = commandline.begin(); ch != commandline.end(); ch++) {
		switch (*ch) {
			case '"':
				inArgumentQuotes = !inArgumentQuotes;
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
					optionName += commandline.substr(old_ch - commandline.begin(), ch - old_ch + 1);
				} else {
					optionName += escape_buffer;
					if (escape_count % 2 == 0) {
						ch--;
					} else {
						optionName += *ch;
					}
				}
				break;
			}
			case ' ':
			case '\t':
				if (!inArgumentQuotes) {
					if (optionName.size()) {
						arguments.push_back(optionName);
						optionName = _T("");
					}
					break;
				}
			default:
				optionName += *ch;
				break;
		}
	}
	if (optionName.size()) {
		arguments.push_back(optionName);
	}

	// Actually process the arguments and store the values as needed. It
	// stores "" for arguments that don't accept values.
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

const bool commandline_options::has(const std::tstring option) const
{
	const std::map<std::tstring, std::list<std::tstring> >::const_iterator item = _arguments.find(option);
	return (item != _arguments.end());
}

const std::tstring& commandline_options::get(const std::tstring option) const
{
	const std::map<std::tstring, std::list<std::tstring> >::const_iterator item = _arguments.find(option);
	return *item->second.begin();
}

const std::list<std::tstring>& commandline_options::gets(const std::tstring option) const
{
	const std::map<std::tstring, std::list<std::tstring> >::const_iterator item = _arguments.find(option);
	return item->second;
}
