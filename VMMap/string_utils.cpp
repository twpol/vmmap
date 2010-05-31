//------------------------------------------------------------------------------
// http://twpol.dyndns.org/projects/vmmap
// License: New BSD License (BSD).
//------------------------------------------------------------------------------

#include "StdAfx.h"
#include "process_memory.h"

// Output formatting for sizes ("#,##0 K").
std::tstring format_size(unsigned long long size)
{
	if (size == 0) {
		return _T("");
	}
	
	std::tstring text(128, '\0');
	_ui64tow_s(size / 1024, &*text.begin(), text.size() - 1, 10);
	text.resize(text.find(_T('\0')));
	text += _T(" K");
	return text;
}

// Output formatting for numbers ("#,##0").
std::tstring format_number(unsigned long long size)
{
	if (size == 0) {
		return _T("");
	}
	
	std::tstring text(128, '\0');
	_ui64tow_s(size, &*text.begin(), text.size() - 1, 10);
	text.resize(text.find(_T('\0')));
	return text;
}

// Output labels for memory data types.
std::tstring format_process_memory_data_type(int type)
{
	if (type == PMDT_BASE)         return _T("Address");
	if (type == PMDT_SIZE)         return _T("Size");
	if (type == PMDT_COMMITTED)    return _T("Committed");
	if (type == PMDT_WS_TOTAL)     return _T("WS Total");
	if (type == PMDT_WS_PRIVATE)   return _T("WS Private");
	if (type == PMDT_WS_SHAREABLE) return _T("WS Shareable");
	if (type == PMDT_WS_SHARED)    return _T("WS Shared");
	if (type == PMDT_BLOCKS)       return _T("Blocks");
	return _T("?");
}

// Output labels for memory block types.
std::tstring format_process_memory_block_type(int type)
{
	if (type == PMBT_FREE)      return _T("Free");
	if (type == PMBT_RESERVED)  return _T("Reserved");
	if (type == PMBT_COMMITTED) return _T("Committed");
	return _T("?");
}

// Output labels for memory group types.
std::tstring format_process_memory_group_type(int type)
{
	if (type == PMGT_TOTAL)       return _T("Total");
	if (type == PMGT_IMAGE)       return _T("Image");
	if (type == PMGT_PRIVATE)     return _T("Private");
	if (type == PMGT_SHAREABLE)   return _T("Shareable");
	if (type == PMGT_MAPPED_FILE) return _T("Mapped File");
	if (type == PMGT_HEAP)        return _T("Heap");
	if (type == PMGT_STACK)       return _T("Stack");
	if (type == PMGT_FREE)        return _T("Free");
	return _T("?");
}
