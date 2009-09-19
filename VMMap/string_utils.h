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

std::tstring format_size(unsigned long long size);
std::tstring format_number(unsigned long long number);
std::tstring format_process_memory_data_type(int type);
std::tstring format_process_memory_block_type(int type);
std::tstring format_process_memory_group_type(int type);
