//------------------------------------------------------------------------------
// http://twpol.dyndns.org/projects/vmmap
// License: Microsoft Public License (Ms-PL).
//------------------------------------------------------------------------------

#include "StdAfx.h"
#include "process_memory.h"
#include "filepath_utils.h"

process_memory::process_memory(const process& process) : _process(process)
{
	// API: CreateToolhelp32Snapshot: Windows 2000 Pro/Server.
	// API: OpenProcess: Windows 2000 Pro/Server.
	// API: VirtualQueryEx: Windows 2000 Pro/Server.
	// API: GetMappedFileName: Windows 2000 Pro/Server.

	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, _process.process_id());
	if (NULL == hProcess) {
		std::tcerr << _process.process_id() << ": OpenProcess failed: " << std::hex << std::setw(8) << std::setfill(_T('0')) << GetLastError() << std::endl;
		return;
	}

	// Get list of images.
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32 | TH32CS_SNAPTHREAD | TH32CS_SNAPHEAPLIST, _process.process_id());
	if (INVALID_HANDLE_VALUE != hSnapshot) {
		MODULEENTRY32 module = { sizeof(module) };
		if (Module32First(hSnapshot, &module)) {
			do {
				_modules.push_back(process_module((unsigned long long)module.modBaseAddr, (unsigned long long)module.modBaseSize, module.szExePath));
			} while (Module32Next(hSnapshot, &module));
		} else {
			std::tcerr << _process.process_id() << ": Module32First failed: " << std::hex << std::setw(8) << std::setfill(_T('0')) << GetLastError() << std::endl;
		}

		HEAPLIST32 heap = { sizeof(heap) };
		unsigned long heap_index = 0;
		if (Heap32ListFirst(hSnapshot, &heap)) {
			do {
				_heaps.push_back(process_heap(heap_index++, (unsigned long long)heap.th32HeapID, heap.dwFlags & HF32_DEFAULT));
				
				//HEAPENTRY32 heap_block = { sizeof(HEAPENTRY32) };
				//if (Heap32First(&heap_block, heap.th32ProcessID, heap.th32HeapID)) {
				//	do {
				//		std::tcout << "   BLOCK(" << std::hex << std::setfill(_T('0')) << std::setw(16) << heap_block.dwAddress << "-" << std::setw(16) << (heap_block.dwAddress + heap_block.dwBlockSize) << ") " << std::setfill(_T(' ')) << (heap_block.dwFlags & LF32_FIXED ? "FIXED" : "     ") << " " << (heap_block.dwFlags & LF32_FREE ? "FREE" : "    ") << " " << (heap_block.dwFlags & LF32_MOVEABLE ? "MOVEABLE" : "        ") << " lock=" << heap_block.dwLockCount << ", resvd=" << heap_block.dwResvd << ", handle=" << heap_block.hHandle; // << std::endl;
				//		if (1) {
				//			byte* buffer_base = (byte*)heap_block.dwAddress;
				//			SIZE_T buffer_length = heap_block.dwBlockSize < 0x1000 ? heap_block.dwBlockSize : 0x1000;
				//			byte* buffer = new byte[buffer_length];
				//			ReadProcessMemory(hProcess, buffer_base, buffer, buffer_length, &buffer_length);
				//			if (buffer_length > 0) {
				//				std::tcout << std::hex << std::setfill(_T('0'));
				//				for (int i = 0; i < buffer_length / sizeof(buffer[0]); i++) {
				//					if (i % 60 == 0) std::tcout << std::endl << "     " << (buffer_base + i) << ":";
				//					if (i % 4 == 0) std::tcout << " ";
				//					std::tcout << std::setw(2 * sizeof(buffer[0])) << buffer[i];
				//				}
				//				std::tcout << std::setfill(_T(' ')) << std::endl;
				//			} else {
				//				std::tcerr << _process.process_id() << ": ReadProcessMemory failed: " << buffer_length << " bytes, " << std::hex << std::setw(8) << std::setfill(_T('0')) << GetLastError() << std::endl;
				//			}
				//		}
				//	} while (Heap32Next(&heap_block));
				//} else {
				//	std::tcerr << _process.process_id() << ": Heap32First failed: " << std::hex << std::setw(8) << std::setfill(_T('0')) << GetLastError() << std::endl;
				//}
			} while (Heap32ListNext(hSnapshot, &heap));
		} else {
			std::tcerr << _process.process_id() << ": Heap32ListFirst failed: " << std::hex << std::setw(8) << std::setfill(_T('0')) << GetLastError() << std::endl;
		}

		THREADENTRY32 thread = { sizeof(thread) };
		if (Thread32First(hSnapshot, &thread)) {
			do {
				if (thread.th32OwnerProcessID == _process.process_id()) {
					//std::tcout << "  THREAD(" << std::dec << thread.th32OwnerProcessID << "/" << thread.th32ThreadID << "): flags=" << std::hex << thread.dwFlags << ", usage=" << std::dec << thread.cntUsage << std::endl;
					HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION | THREAD_GET_CONTEXT | THREAD_SUSPEND_RESUME, false, thread.th32ThreadID);
					if (NULL != hThread) {
						if (GetCurrentThreadId() != thread.th32ThreadID) {
							SuspendThread(hThread);
						}
#ifdef _WIN64
						CONTEXT context_64bit = {}; context_64bit.ContextFlags = CONTEXT_CONTROL;
						if (GetThreadContext(hThread, &context_64bit)) {
							_stacks.push_back(process_stack(thread.th32ThreadID, context_64bit.Rsp, PST_64BIT));
							//std::tcout << "    STACK(" << std::hex << context_64bit.Rsp << ") [64bit]" << std::endl;
						} else if (GetLastError() != ERROR_INVALID_PARAMETER) {
							std::tcerr << _process.process_id() << ": GetThreadContext(64bit) failed: " << std::hex << std::setw(8) << std::setfill(_T('0')) << GetLastError() << std::endl;
						}
						WOW64_CONTEXT context_32bit = { WOW64_CONTEXT_ALL };
						if (Wow64GetThreadContext(hThread, &context_32bit)) {
#elif _WIN32
						CONTEXT context_32bit = { CONTEXT_CONTROL };
						if (GetThreadContext(hThread, &context_32bit)) {
#endif
							_stacks.push_back(process_stack(thread.th32ThreadID, context_32bit.Esp, PST_32BIT));
							//std::tcout << "    STACK(" << std::hex << context_32bit.Esp << ") [32bit]" << std::endl;
						} else if (GetLastError() != ERROR_INVALID_PARAMETER) {
							std::tcerr << _process.process_id() << ": GetThreadContext(32bit) failed: " << std::hex << std::setw(8) << std::setfill(_T('0')) << GetLastError() << std::endl;
						}
						if (GetCurrentThreadId() != thread.th32ThreadID) {
							ResumeThread(hThread);
						}
					} else {
						std::tcerr << _process.process_id() << ": OpenThread failed: " << std::hex << std::setw(8) << std::setfill(_T('0')) << GetLastError() << std::endl;
					}
				}
			} while (Thread32Next(hSnapshot, &thread));
		} else {
			std::tcerr << _process.process_id() << ": Thread32First failed: " << std::hex << std::setw(8) << std::setfill(_T('0')) << GetLastError() << std::endl;
		}

		CloseHandle(hSnapshot);
	}
	_modules.sort(std::less<unsigned long long>());
	_stacks.sort(std::less<unsigned long long>());
	_heaps.sort(std::less<unsigned long long>());

	// Collect virtual memory allocations...
	{
		// Windows 32bit limit: 0xFFFFFFFF.
		// windows 64bit limit: 0x7FFFFFFFFFF.
		MEMORY_BASIC_INFORMATION info = { 0 };
		for (unsigned long long address = 0; address < 0x80000000000; address += info.RegionSize) {
			size_t info_size = VirtualQueryEx(hProcess, (void*)address, &info, sizeof(info));
			if (info_size == 0) break;
			if (info_size != sizeof(info)) {
				std::tcerr << "VirtualQueryEx returned unexpected info_size (" << std::hex << info_size << ", expected " << std::hex << sizeof(info) << ")" << std::endl;
			}
			if (info.BaseAddress != (void*)address) {
				std::tcerr << "VirtualQueryEx returned unexpected BaseAddress (" << std::hex << info.BaseAddress << ", expected " << std::hex << (void*)address << ")" << std::endl;
			}

			unsigned long long allocation_base = (unsigned long long)info.AllocationBase;
			if (info.State == MEM_FREE) allocation_base = (unsigned long long)info.BaseAddress;

			// Set up the memory group (== allocation) with details and type.
			process_memory_group& memory_group = _groups[allocation_base];

			if (memory_group.type() == PMGT__LAST) {
				memory_group = _groups[allocation_base] = process_memory_group(*this, hProcess, &info);
			}
			
			// Set up the memory block with current state.
			memory_group.add_block(process_memory_block(*this, hProcess, memory_group, &info));
		}
	}

	// Collect working set pages...
	{
		//std::tcout << std::endl;
		PERFORMACE_INFORMATION performance_info = { 0 };
		if (GetPerformanceInfo(&performance_info, sizeof(performance_info))) {
			PROCESS_MEMORY_COUNTERS memory_counters = { 0 };
			if (GetProcessMemoryInfo(hProcess, &memory_counters, sizeof(memory_counters))) {
				unsigned long working_set_page_count = memory_counters.WorkingSetSize / performance_info.PageSize;
				//std::tcout << "Working set pages expected = " << std::hex << working_set_page_count << std::endl;

				unsigned long long buffer_length = sizeof(PSAPI_WORKING_SET_INFORMATION) + working_set_page_count * sizeof(PSAPI_WORKING_SET_BLOCK);
				void* buffer = (void*)new byte[buffer_length];
				if (QueryWorkingSet(hProcess, buffer, buffer_length)) {
					PSAPI_WORKING_SET_INFORMATION* ws_info = (PSAPI_WORKING_SET_INFORMATION*)buffer;
					//std::tcout << "Working set pages got      = " << std::hex << ws_info->NumberOfEntries << std::endl;

					PSAPI_WORKING_SET_BLOCK* ws_block = (PSAPI_WORKING_SET_BLOCK*)ws_info->WorkingSetInfo;
					for (unsigned long page = 0; page < ws_info->NumberOfEntries; page++, ws_block++) {
						unsigned long long target_address = ws_block->VirtualPage * performance_info.PageSize;
						std::map<unsigned long long, process_memory_group>::const_iterator it_group = _groups.upper_bound(target_address);
						if (it_group-- != _groups.end()) {
							const process_memory_group& group = (*it_group).second;
							for (std::list<const process_memory_block>::const_iterator it_block = group.block_list().begin(); it_block != group.block_list().end(); it_block++) {
								if (((*it_block).base() <= target_address) && ((*it_block).base() + (*it_block).size() >= target_address)) {
									process_memory_block& block = const_cast<process_memory_block&>(*it_block);
									block.add_ws_page(ws_block, performance_info.PageSize);
									break;
								}
							}
						}
					}
				} else {
					std::tcerr << _process.process_id() << ": QueryWorkingSet failed: " << std::hex << std::setw(8) << std::setfill(_T('0')) << GetLastError() << std::endl;
				}
			} else {
				std::tcerr << _process.process_id() << ": GetProcessMemoryInfo failed: " << std::hex << std::setw(8) << std::setfill(_T('0')) << GetLastError() << std::endl;
			}
		} else {
			std::tcerr << _process.process_id() << ": GetProcessMemoryInfo failed: " << std::hex << std::setw(8) << std::setfill(_T('0')) << GetLastError() << std::endl;
		}
	}

	CloseHandle(hProcess);
}

process_memory::~process_memory(void)
{
}

unsigned long long process_memory::data(process_memory_group_type group_type, process_memory_data_type type) const
{
	if ((group_type == PMGT_TOTAL) && (type == PMDT_BLOCKS)) return 0;

	unsigned long long total = 0;
	for (std::map<unsigned long long, process_memory_group>::const_iterator group = _groups.begin(); group != _groups.end(); group++) {
		if ((group->second.type() == group_type) || ((group_type == PMGT_TOTAL) && (group->second.type() != PMGT_FREE))) {
			total += group->second.data(type);
		}
	}
	return total;
}

process_memory_group::process_memory_group(void) : _type(PMGT__LAST), _details(_T(""))
{
}

process_memory_group::process_memory_group(const process_memory& memory, const HANDLE hProcess, PMEMORY_BASIC_INFORMATION info)
{
	if (info->State == MEM_FREE) {
		_type = PMGT_FREE;
	} else {
		std::tstring filepath;
		if ((info->Type == MEM_MAPPED) || (info->Type == MEM_IMAGE)) {
			filepath.assign(MAX_PATH, '\0');
			int filepath_length = GetMappedFileName(hProcess, info->BaseAddress, &*filepath.begin(), filepath.size());
			filepath.resize(filepath_length);
			if (filepath.size()) {
				filepath = MapDevicePathToDrivePath(filepath);
			}
		}

		if (info->Type == MEM_IMAGE) {
			_type = PMGT_IMAGE;
			_details = filepath;
		} else if (info->Type == MEM_MAPPED) {
			if (filepath.size()) {
				_type = PMGT_MAPPED_FILE;
				_details = filepath;
			} else {
				_type = PMGT_SHAREABLE;
			}
		} else if (info->Type == MEM_PRIVATE) {
			_type = PMGT_PRIVATE;
		} else {
			std::tcerr << "VirtualQueryEx returned unexpected info.Type: " << info->Type << std::endl;
		}

		if ((_type == PMGT_SHAREABLE) || (_type == PMGT_PRIVATE)) {
			// Shareable and private memory may be a process heap.
			for (std::list<const process_heap>::const_iterator heap = memory.heaps().begin(); heap != memory.heaps().end(); heap++) {
				if (((unsigned long long)info->BaseAddress <= heap->base()) && ((unsigned long long)info->BaseAddress + (unsigned long long)info->RegionSize >= heap->base())) {
					std::tstring heap_details(_T("Heap ID:           "));
					_itow_s(heap->id(), &heap_details[9], 10, 10);
					heap_details.resize(heap_details.find(_T('\0')));
					if (heap->default()) {
						heap_details += _T(" (Default)");
					}
					_type = PMGT_HEAP;
					_details = heap_details;
					break;
				}
			}
		}
	}
}

process_memory_group::~process_memory_group(void)
{
}

unsigned long long process_memory_group::data(process_memory_data_type type) const
{
	if ((type == PMDT_BASE) && _blocks.size()) return _blocks.begin()->base();
	if (type == PMDT_BASE) return 0;
	if (type == PMDT_BLOCKS) return _blocks.size();

	unsigned long long total = 0;
	for (std::list<const process_memory_block>::const_iterator block = _blocks.begin(); block != _blocks.end(); block++) {
		total += (*block).data(type);
	}
	return total;
}

process_memory_protection process_memory_group::protection(void) const
{
	process_memory_protection rv = PMP_NONE;
	for (std::list<const process_memory_block>::const_iterator block = _blocks.begin(); block != _blocks.end(); block++) {
		if ((*block).type() == PMBT_COMMITTED) {
			rv |= (*block).protection();
		}
	}
	return rv;
}

std::tstring process_memory_group::protection_str(void) const
{
	process_memory_protection p = protection();
	std::tstring rv;
	if (p & PMP_READ)          rv += _T("r"); else rv+= _T("-");
	if (p & PMP_WRITE)         rv += _T("w"); else rv+= _T("-");
	if (p & PMP_EXECUTE)       rv += _T("x"); else rv+= _T("-");
	if (p & PMP_COPY)          rv += _T("c"); else rv+= _T("-");
	if (p & PMP_GUARD)         rv += _T("G"); else rv+= _T("-");
	if (p & PMP_NO_CACHE)      rv += _T("C"); else rv+= _T("-");
	if (p & PMP_WRITE_COMBINE) rv += _T("W"); else rv+= _T("-");
	return rv;
}

void process_memory_group::add_block(const process_memory_block& block)
{
	_blocks.push_back(block);
}

process_memory_block::process_memory_block(const process_memory& memory, const HANDLE hProcess, process_memory_group& group, const PMEMORY_BASIC_INFORMATION info) : _protection(PMP_NONE)
{
	UNREFERENCED_PARAMETER(hProcess);

	for (int i = PMDT__FIRST; i < PMDT__LAST; i++) _data[i] = 0;
	_data[PMDT_BASE] = (unsigned long long)info->BaseAddress;
	_data[PMDT_SIZE] = (unsigned long long)info->RegionSize;
	_type = info->State == MEM_COMMIT ? PMBT_COMMITTED : info->State == MEM_RESERVE ? PMBT_RESERVED : PMBT_FREE;
	switch (info->Protect & 0x00FF) {
		case PAGE_NOACCESS:
			_protection = PMP_NONE;
			break;
		case PAGE_READONLY:
			_protection = PMP_READ;
			break;
		case PAGE_READWRITE:
			_protection = PMP_READ | PMP_WRITE;
			break;
		case PAGE_WRITECOPY:
			_protection = PMP_READ | PMP_WRITE | PMP_COPY;
			break;
		case PAGE_EXECUTE:
			_protection = PMP_EXECUTE;
			break;
		case PAGE_EXECUTE_READ:
			_protection = PMP_EXECUTE | PMP_READ;
			break;
		case PAGE_EXECUTE_READWRITE:
			_protection = PMP_EXECUTE | PMP_READ | PMP_WRITE;
			break;
		case PAGE_EXECUTE_WRITECOPY:
			_protection = PMP_EXECUTE | PMP_READ | PMP_COPY;
			break;
	}
	if (info->Protect & PAGE_GUARD)        _protection |= PMP_GUARD;
	if (info->Protect & PAGE_NOCACHE)      _protection |= PMP_NO_CACHE;
	if (info->Protect & PAGE_WRITECOMBINE) _protection |= PMP_WRITE_COMBINE;

	if ((_type != PMBT_FREE) && (group.type() == PMGT_PRIVATE)) {
		// Private memory may be a stack.
		for (std::list<const process_stack>::const_iterator stack = memory.stacks().begin(); stack != memory.stacks().end(); stack++) {
			if (((unsigned long long)info->BaseAddress <= stack->base()) && ((unsigned long long)info->BaseAddress + (unsigned long long)info->RegionSize >= stack->base())) {
				std::tstring stack_details(_T("Thread ID:           "));
				_itow_s(stack->id(), &stack_details[11], 10, 10);
				stack_details.resize(stack_details.find(_T('\0')));
#ifdef _WIN64
				if (stack->type() == PST_32BIT) {
					stack_details += _T(" (Wow64)");
				}
#endif
				group.type(PMGT_STACK);
				group.details(stack_details);
				break;
			}
		}
	}
}

process_memory_block::~process_memory_block(void)
{
}

unsigned long long process_memory_block::data(process_memory_data_type type) const
{
	if (type == PMDT_COMMITTED) return (_type == PMBT_COMMITTED ? _data[PMDT_SIZE] : 0);
	if (type == PMDT_WS_TOTAL) return _data[PMDT_WS_PRIVATE] + _data[PMDT_WS_SHAREABLE];
	return _data[type];
}

std::tstring process_memory_block::protection_str(void) const
{
	if (_type == PMBT_COMMITTED) {
		process_memory_protection p = protection();
		std::tstring rv;
		if (p & PMP_READ)          rv += _T("r"); else rv+= _T("-");
		if (p & PMP_WRITE)         rv += _T("w"); else rv+= _T("-");
		if (p & PMP_EXECUTE)       rv += _T("x"); else rv+= _T("-");
		if (p & PMP_COPY)          rv += _T("c"); else rv+= _T("-");
		if (p & PMP_GUARD)         rv += _T("G"); else rv+= _T("-");
		if (p & PMP_NO_CACHE)      rv += _T("C"); else rv+= _T("-");
		if (p & PMP_WRITE_COMBINE) rv += _T("W"); else rv+= _T("-");
		return rv;
	}
	return _T("");
}

void process_memory_block::add_ws_page(PSAPI_WORKING_SET_BLOCK* ws_block, unsigned long page_size)
{
	if (ws_block->Shared) {
		_data[PMDT_WS_SHAREABLE] += page_size;
		if (ws_block->ShareCount > 1) {
			_data[PMDT_WS_SHARED] += page_size;
		}
	} else {
		_data[PMDT_WS_PRIVATE] += page_size;
	}
}
