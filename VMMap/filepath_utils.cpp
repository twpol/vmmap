//------------------------------------------------------------------------------
// http://twpol.dyndns.org/projects/vmmap
// License: Microsoft Public License (Ms-PL).
//------------------------------------------------------------------------------

#include "StdAfx.h"
#include "filepath_utils.h"

std::map<const std::tstring, std::tstring> MapDevicePathToDrivePathCache;

const std::tstring MapDevicePathToDrivePath(const std::tstring& path)
{
	// API: GetLogicalDriveStrings: Windows 2000 Pro/Server.
	// API: QueryDosDevice: Windows 2000 Pro/Server.

	if (MapDevicePathToDrivePathCache.size() == 0) {
		std::tstring drives(27, '\0');
		int drives_length = GetLogicalDriveStrings(drives.size(), &*drives.begin());
		if (drives_length) {
			drives.resize(drives_length);
			std::tstring::size_type start = 0;
			std::tstring::size_type end = drives.find(_T('\0'));
			while (end < drives.size()) {
				std::tstring drive = drives.substr(start, end - start - 1);
				std::tstring device(MAX_PATH, '\0');
				int device_length = QueryDosDevice(drive.c_str(), &*device.begin(), device.size());
				if (device_length) {
					device.resize(device_length - 2);
					device += '\\';
					drive += '\\';
					//std::tcout << "MapDevicePathToDrivePath: " << device << " --> " << drive << std::endl;
					MapDevicePathToDrivePathCache[device] = drive;
				} else {
					std::tcerr << "QueryDosDevice(" << drive << ") failed: " << std::hex << std::setw(8) << std::setfill(_T('0')) << GetLastError() << std::endl;
				}
				start = end + 1;
				end = drives.find(_T('\0'), start);
			}
		} else {
			std::tcerr << "GetLogicalDriveStrings failed: " << std::hex << std::setw(8) << std::setfill(_T('0')) << GetLastError() << std::endl;
		}
	}

	for (std::map<const std::tstring, std::tstring>::iterator map = MapDevicePathToDrivePathCache.begin(); map != MapDevicePathToDrivePathCache.end(); map++) {
		if (path.compare(0, (*map).first.size(), (*map).first) == 0) {
			return (*map).second + path.substr((*map).first.size());
		}
	}

	return path;
}