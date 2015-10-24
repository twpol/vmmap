//------------------------------------------------------------------------------
// http://twpol.dyndns.org/projects/vmmap
// License: New BSD License (BSD).
//------------------------------------------------------------------------------

#pragma once

struct tstring_caseless_compare_t
{
	bool operator() (const std::tstring& l, const std::tstring& r) const
	{
#ifdef UNICODE
		return _wcsicmp(l.c_str(), r.c_str()) < 0;
#else
		return _stricmp(l.c_str(), r.c_str()) < 0;
#endif
	}
};

const std::tstring format_size(const unsigned long long size);
const std::tstring format_number(const unsigned long long number);
const std::tstring format_process_memory_data_type(const int type);
const std::tstring format_process_memory_block_type(const int type);
const std::tstring format_process_memory_group_type(const int type);
