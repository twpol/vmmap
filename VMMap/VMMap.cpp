//------------------------------------------------------------------------------
// http://twpol.dyndns.org/projects/vmmap
// License: New BSD License (BSD).
//------------------------------------------------------------------------------

// VMMap.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include "commandline_options.h"
#include "process_list.h"
#include "process_memory.h"
#include "string_utils.h"

int _tmain(int argc, _TCHAR* argv[])
{
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	commandline_options options(L"h ?,:pid,:im,:d,free,sum,compare");

	if (options.has(L"h")) {
		std::tcout << "Gathers and reports on detailed memory usage by one or more process." << std::endl;
		std::tcout << std::endl;
		std::tcout << "VMMAP [/PID pid|/IM image] [/D level] [/FREE] [/COMPARE] [/SUM]" << std::endl;
		std::tcout << std::endl;
		std::tcout << "  /PID        Report on a specific process. Can be specified multiple times." << std::endl;
		std::tcout << "   pid        Process identifier." << std::endl;
		std::tcout << "  /IM         Report on a specific process. Can be specified multiple times." << std::endl;
		std::tcout << "   image      Process name or '*' for all accessible processes." << std::endl;
		std::tcout << "  /D          Specifies a level of detail for matching processes (defaults to 3)." << std::endl;
		std::tcout << "   level      0  Nothing." << std::endl;
		std::tcout << "              1  Just application name and PID." << std::endl;
		std::tcout << "              2  Adds VM and WS summary graphs." << std::endl;
		std::tcout << "              3  Adds summary table with type breakdown." << std::endl;
		std::tcout << "              4  Adds table of each virtual memory allocation block." << std::endl;
		std::tcout << "              5  Adds details on sub-block allocation statess." << std::endl;
		std::tcout << "  /FREE       Include free and unusable memory ranges." << std::endl;
		std::tcout << "  /COMPARE    Include a comparison, by process name, of all selected processes." << std::endl;
		std::tcout << "  /SUM        Include a summary (detail level 2) of all selected processes." << std::endl;
		std::tcout << std::endl;
		std::tcout << "Access column key:" << std::endl;
		std::tcout << "  r  Read       G  Guard Page" << std::endl;
		std::tcout << "  w  Write      C  No Cache" << std::endl;
		std::tcout << "  x  Execute    W  Write Combine" << std::endl;
		std::tcout << "  c  Copy on Write" << std::endl;
		return 0;
	}

	if (!options.has(L"pid") && !options.has(L"im")) {
		process_list processes;

		std::tcout << "Accessible processes: " << processes.get().size() << std::endl;
		std::tcout << std::endl;
		std::tcout << " " << std::setw(30) << std::left << "Username" << "  " << std::right << std::setw(6) << "PID" << "  " << "Image Name" << std::endl;
		std::tcout << " " << std::setw(78) << std::setfill(L'-') << "" << std::setfill(L' ') << std::endl;
		for (std::list<process>::const_iterator process = processes.get().begin(); process != processes.get().end(); process++) {
			std::tcout << " " << std::setw(30) << std::left << process->username() << "  " << std::right << std::setw(6) << process->process_id() << "  " << process->image_filename() << std::endl;
		}
		return 0;
	}

	std::list<process> processes;
	{
		if (options.has(L"pid")) {
			const std::list<std::tstring> pids = options.gets(L"pid");
			for (std::list<std::tstring>::const_iterator pid = pids.begin(); pid != pids.end(); pid++) {
				if ((*pid).compare(L"self") == 0) {
					processes.push_back(process(GetCurrentProcessId()));
				} else {
					processes.push_back(process(_wtoi((*pid).c_str())));
				}
			}
		}
		if (options.has(L"im")) {
			const std::list<std::tstring> ims = options.gets(L"im");
			std::map<std::tstring, std::list<process>, tstring_caseless_compare_t> process_map;
			process_list process_list;
			for (std::list<process>::const_iterator process = process_list.get().begin(); process != process_list.get().end(); process++) {
				process_map[process->image_filename()].push_back(*process);
			}

			for (std::list<std::tstring>::const_iterator im = ims.begin(); im != ims.end(); im++) {
				if (process_map.find(*im) != process_map.end()) {
					processes.merge(process_map[*im]);
				} else if (im->compare(L"*") == 0) {
					for (std::list<process>::const_iterator process = process_list.get().begin(); process != process_list.get().end(); process++) {
						processes.push_back(*process);
					}
				}
			}
		}
		processes.sort(std::less<DWORD>());
	}

#define SUMMARY_ROW_SIZE (100)
#define SUMMARY_SIZE (2 * SUMMARY_ROW_SIZE)
	{
		int level = 3;
		if (options.has(L"d")) level = _wtoi(options.get(L"d").c_str());

		for (std::list<process>::const_iterator process = processes.begin(); process != processes.end(); process++) {
			if (level < 1) continue;

			std::tcout << std::endl;
			std::tcout << "Process: " << process->image_filename() << std::endl;
			std::tcout << "PID:     " << process->process_id() << std::endl;

			if (level < 2) continue;

			process_memory memory(*process);

			unsigned long long vm_total = memory.data(PMGT_TOTAL, PMDT_COMMITTED);
			unsigned long long ws_total = memory.data(PMGT_TOTAL, PMDT_WS_TOTAL);
			unsigned long long vm_current = 0;
			unsigned long long ws_current = 0;
			std::tstring vm_summary;
			std::tstring ws_summary;
			if (vm_total > 0) {
				for (process_memory_group_type group_type = PMGT_IMAGE; group_type < PMGT_FREE; group_type = (process_memory_group_type)((int)group_type + 1)) {
					vm_current += memory.data(group_type, PMDT_COMMITTED);
					ws_current += memory.data(group_type, PMDT_WS_TOTAL);
					std::tstring vm_temp((int)(SUMMARY_SIZE * vm_current / vm_total) - vm_summary.size(), format_process_memory_group_type(group_type)[0]);
					std::tstring ws_temp((int)(SUMMARY_SIZE * ws_current / vm_total) - ws_summary.size(), format_process_memory_group_type(group_type)[0]);
					vm_summary += vm_temp;
					ws_summary += ws_temp;
				}
			}
			vm_summary.resize(SUMMARY_SIZE, '.');
			ws_summary.resize(SUMMARY_SIZE, '.');

			std::tcout << std::endl;
			std::tcout << std::setw(80) << std::left << "Virtual Memory Summary:" << std::setw(20) << std::right << format_size(vm_total) << std::endl;
			for (int i = 0; i < SUMMARY_SIZE; i += SUMMARY_ROW_SIZE) {
				std::tcout << vm_summary.substr(i, SUMMARY_ROW_SIZE) << std::endl;
			}

			std::tcout << std::endl;
			std::tcout << std::setw(80) << std::left << "Working Set Summary:" << std::setw(20) << std::right << format_size(ws_total) << std::endl;
			for (int i = 0; i < SUMMARY_SIZE; i += SUMMARY_ROW_SIZE) {
				std::tcout << ws_summary.substr(i, SUMMARY_ROW_SIZE) << std::endl;
			}

			if (level < 3) continue;

			std::tcout << std::endl;
			std::tcout << std::left << std::setw(11) << "Type" << "  " << std::right << std::setw(12) << "Size" << "  " << std::setw(12) << "Committed" << "  " << std::setw(12) << "Total WS" << "  " << std::setw(12) << "Private WS" << "  " << std::setw(12) << "Shareable WS" << "  " << std::setw(12) << "Shared WS" << "  " << std::setw(6) << "Blocks" << "  " << std::setw(12) << "Largest" << std::endl;
			std::tcout << std::setw(13 + 14 * 7 + 8 - 2) << std::setfill(L'-') << "" << std::setfill(L' ') << std::endl;

			for (process_memory_group_type group_type = PMGT__FIRST; group_type < PMGT__LAST; group_type = (process_memory_group_type)((int)group_type + 1)) {
				std::tcout << std::setw(11) << std::setfill(L' ') << std::left << format_process_memory_group_type(group_type);
				for (process_memory_data_type type = PMDT_SIZE; type < PMDT__LAST; type = (process_memory_data_type)((int)type + 1)) {
					std::tcout << "  " << std::setw(type == PMDT_BLOCKS ? 6 : 12) << std::setfill(L' ') << std::right << (type == PMDT_BLOCKS ? format_number(memory.data(group_type, type)) : format_size(memory.data(group_type, type)));
				}
				std::tcout << std::endl;
			}

			if (level < 4) continue;

			std::tcout << std::endl;
			std::tcout << std::left << std::setw(16) << "Address" << "    " << std::setw(11) << "Type" << "  " << std::right << std::setw(12) << "Size" << "  " << std::setw(12) << "Committed" << "  " << std::setw(12) << "Total WS" << "  " << std::setw(12) << "Private WS" << "  " << std::setw(12) << "Shareable WS" << "  " << std::setw(12) << "Shared WS" << "  " << std::setw(6) << "Blocks" << "  " << std::left << std::setw(7) << "Access" << "  " << "Details" << std::endl;
			std::tcout << std::setw(20 + 13 + 14 * 6 + 8 + 9 * 2 - 2) << std::setfill(L'-') << "" << std::setfill(L' ') << std::endl;

			for (std::map<unsigned long long, process_memory_group>::const_iterator it_group = memory.groups().begin(); it_group != memory.groups().end(); it_group++) {
				const process_memory_group& group = (*it_group).second;
				if ((group.type() == PMGT_FREE || group.type() == PMGT_UNUSABLE) && !options.has(L"free")) continue;

				std::tcout         << std::setw(16) << std::setfill(L'0') << std::right << std::hex << group.base();
				std::tcout << "  ";
				std::tcout << "  " << std::setw(11) << std::setfill(L' ') << std::left  << format_process_memory_group_type(group.type());
				std::tcout << "  " << std::setw(12) << std::setfill(L' ') << std::right << format_size(group.size());
				std::tcout << "  " << std::setw(12) << std::setfill(L' ') << std::right << format_size(group.committed());
				std::tcout << "  " << std::setw(12) << std::setfill(L' ') << std::right << format_size(group.ws());
				std::tcout << "  " << std::setw(12) << std::setfill(L' ') << std::right << format_size(group.ws_private());
				std::tcout << "  " << std::setw(12) << std::setfill(L' ') << std::right << format_size(group.ws_shareable());
				std::tcout << "  " << std::setw(12) << std::setfill(L' ') << std::right << format_size(group.ws_shared());
				std::tcout << "  " << std::setw(6)  << std::setfill(L' ') << std::right << format_number(group.blocks());
				std::tcout << "  " << std::setw(7)  << std::setfill(L' ') << std::left  << group.protection_str();
				std::tcout << "  "                  << std::setfill(L' ') << std::left  << group.details();
				std::tcout << std::endl;

				if (level >= 5 && group.type() != PMGT_FREE && group.type() != PMGT_UNUSABLE) {
					for (std::list<process_memory_block>::const_iterator it_block = group.block_list().begin(); it_block != group.block_list().end(); it_block++) {
						const process_memory_block& block = *it_block;
						std::tcout << "  " << std::setw(16) << std::setfill(L'0') << std::right << std::hex << block.base();
						std::tcout << "  " << std::setw(11) << std::setfill(L' ') << std::left  << format_process_memory_block_type(block.type());
						std::tcout << "  " << std::setw(12) << std::setfill(L' ') << std::right << format_size(block.size());
						std::tcout << "  " << std::setw(12) << std::setfill(L' ') << std::right << format_size(block.committed());
						std::tcout << "  " << std::setw(12) << std::setfill(L' ') << std::right << format_size(block.ws());
						std::tcout << "  " << std::setw(12) << std::setfill(L' ') << std::right << format_size(block.ws_private());
						std::tcout << "  " << std::setw(12) << std::setfill(L' ') << std::right << format_size(block.ws_shareable());
						std::tcout << "  " << std::setw(12) << std::setfill(L' ') << std::right << format_size(block.ws_shared());
						std::tcout << "  " << std::setw(6)  << std::setfill(L' ') << std::right << format_number(0);
						std::tcout << "  " << std::setw(7)  << std::setfill(L' ') << std::left  << block.protection_str();
						std::tcout << "  "                  << std::setfill(L' ') << std::left  << block.details();
						std::tcout << std::endl;
					}
				}
			}
		}
	}

	if (options.has(L"compare")) {
		std::tcout << std::endl;
		std::tcout << "Process: <comparison>" << std::endl;
		std::tcout << "PID:     <comparison>" << std::endl;

		std::map<std::tstring, std::list<process_memory>, tstring_caseless_compare_t> memories;
		for (std::list<process>::const_iterator process = processes.begin(); process != processes.end(); process++) {
			memories[process->image_filename()].push_back(process_memory(*process));
		}

		std::tcout << std::endl;
		std::tcout << std::left << std::setw(35) << "" << std::right << std::setw(40) << "Physical Memory" << "  " << std::setw(26) << "Virtual Memory" << std::endl;
		std::tcout << std::left << std::setw(33) << "Process" << "  " << std::right << std::setw(12) << "Private" << "  " << std::setw(12) << "Shared" << "  " << std::setw(12) << "Total" << "  " << std::setw(12) << "Private" << "  " << std::setw(12) << "Mapped" << std::endl;
		std::tcout << std::setw(103) << std::setfill(L'-') << "" << std::setfill(L' ') << std::endl;

		for (std::map<std::tstring, std::list<process_memory>, tstring_caseless_compare_t>::const_iterator pms = memories.begin(); pms != memories.end(); pms++) {
			unsigned long long ws_private = 0;
			unsigned long long ws_shared = 0;
			unsigned long long vm_private = 0;
			unsigned long long vm_mapped = 0;

			for (std::list<process_memory>::const_iterator pm = pms->second.begin(); pm != pms->second.end(); pm++) {
				ws_private += pm->data(PMGT_TOTAL, PMDT_WS_TOTAL) - pm->data(PMGT_TOTAL, PMDT_WS_SHARED);
				ws_shared += pm->data(PMGT_TOTAL, PMDT_WS_SHARED);
				vm_private += pm->data(PMGT_PRIVATE, PMDT_COMMITTED) + pm->data(PMGT_HEAP, PMDT_COMMITTED) + pm->data(PMGT_STACK, PMDT_COMMITTED);
				vm_mapped += pm->data(PMGT_SHAREABLE, PMDT_COMMITTED) + pm->data(PMGT_MAPPED_FILE, PMDT_COMMITTED);
			}

			std::tcout << std::left << std::setw(33) << pms->first << "  " << std::right << std::setw(12) << format_size(ws_private) << "  " << std::setw(12) << format_size(ws_shared / pms->second.size()) << "  " << std::setw(12) << format_size(ws_private + ws_shared / pms->second.size()) << "  " << std::setw(12) << format_size(vm_private) << "  " << std::setw(12) << format_size(vm_mapped) << std::endl;
		}
	}

	if (options.has(L"sum")) {
		std::tcout << std::endl;
		std::tcout << "Process: <summary>" << std::endl;
		std::tcout << "PID:     <summary>" << std::endl;

		std::list<process_memory> memories;
		for (std::list<process>::const_iterator process = processes.begin(); process != processes.end(); process++) {
			memories.push_back(process_memory(*process));
		}

		unsigned long long vm_total = 0;
		unsigned long long ws_total = 0;
		for (std::list<process_memory>::const_iterator memory = memories.begin(); memory != memories.end(); memory++) {
			vm_total += memory->data(PMGT_TOTAL, PMDT_COMMITTED);
			ws_total += memory->data(PMGT_TOTAL, PMDT_WS_TOTAL);
		}

		unsigned long long vm_current = 0;
		unsigned long long ws_current = 0;
		std::tstring vm_summary;
		std::tstring ws_summary;
		if (vm_total > 0) {
			for (process_memory_group_type group_type = PMGT_IMAGE; group_type < PMGT_FREE; group_type = (process_memory_group_type)((int)group_type + 1)) {
				for (std::list<process_memory>::const_iterator memory = memories.begin(); memory != memories.end(); memory++) {
					vm_current += memory->data(group_type, PMDT_COMMITTED);
					ws_current += memory->data(group_type, PMDT_WS_TOTAL);
				}
				std::tstring vm_temp((int)(SUMMARY_SIZE * vm_current / vm_total) - vm_summary.size(), format_process_memory_group_type(group_type)[0]);
				std::tstring ws_temp((int)(SUMMARY_SIZE * ws_current / vm_total) - ws_summary.size(), format_process_memory_group_type(group_type)[0]);
				vm_summary += vm_temp;
				ws_summary += ws_temp;
			}
		}

		vm_summary.resize(SUMMARY_SIZE, '.');
		ws_summary.resize(SUMMARY_SIZE, '.');

		std::tcout << std::endl;
		std::tcout << std::setw(80) << std::left << "Virtual Memory Summary:" << std::setw(20) << std::right << format_size(vm_total) << std::endl;
		for (int i = 0; i < SUMMARY_SIZE; i += SUMMARY_ROW_SIZE) {
			std::tcout << vm_summary.substr(i, SUMMARY_ROW_SIZE) << std::endl;
		}

		std::tcout << std::endl;
		std::tcout << std::setw(80) << std::left << "Working Set Summary:" << std::setw(20) << std::right << format_size(ws_total) << std::endl;
		for (int i = 0; i < SUMMARY_SIZE; i += SUMMARY_ROW_SIZE) {
			std::tcout << ws_summary.substr(i, SUMMARY_ROW_SIZE) << std::endl;
		}

		std::tcout << std::endl;
		std::tcout << std::left << std::setw(11) << "Type" << "  " << std::right << std::setw(12) << "Size" << "  " << std::setw(12) << "Committed" << "  " << std::setw(12) << "Total WS" << "  " << std::setw(12) << "Private WS" << "  " << std::setw(12) << "Shareable WS" << "  " << std::setw(12) << "Shared WS" << "  " << std::setw(6) << "Blocks" << "  " << std::setw(12) << "Largest" << std::endl;
		std::tcout << std::setw(13 + 14 * 7 + 8 - 2) << std::setfill(L'-') << "" << std::setfill(L' ') << std::endl;

		for (process_memory_group_type group_type = PMGT__FIRST; group_type < PMGT__LAST; group_type = (process_memory_group_type)((int)group_type + 1)) {
			std::tcout << std::setw(11) << std::setfill(L' ') << std::left << format_process_memory_group_type(group_type);
			for (process_memory_data_type type = PMDT_SIZE; type < PMDT__LAST; type = (process_memory_data_type)((int)type + 1)) {
				unsigned long long sum = 0;
				for (std::list<process_memory>::const_iterator memory = memories.begin(); memory != memories.end(); memory++) {
					sum += memory->data(group_type, type);
				}
				std::tcout << "  " << std::setw(type == PMDT_BLOCKS ? 6 : 12) << std::setfill(L' ') << std::right << (type == PMDT_BLOCKS ? format_number(sum) : format_size(sum));
			}
			std::tcout << std::endl;
		}
	}

	return 0;
}
