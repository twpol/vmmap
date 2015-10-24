//------------------------------------------------------------------------------
// http://twpol.dyndns.org/projects/vmmap
// License: New BSD License (BSD).
//------------------------------------------------------------------------------

#include "StdAfx.h"
#include "filepath_utils.h"

std::map<std::tstring, std::tstring> MapDevicePathToDrivePathCache;

const std::tstring MapDevicePathToDrivePath(const std::tstring& path)
{
	// NT API Support:
	//   5.0  GetLogicalDriveStrings
	//   5.0  QueryDosDevice

	if (MapDevicePathToDrivePathCache.size() == 0) {
		// Construct the cache of device paths to drive letters (e.g.
		// "\Device\HarddiskVolume1\" -> "C:\", "\Device\CdRom0\" -> "D:\").
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

	// Replace a matching device path with the appropriate drive letter.
	for (std::map<std::tstring, std::tstring>::iterator map = MapDevicePathToDrivePathCache.begin(); map != MapDevicePathToDrivePathCache.end(); map++) {
		if (path.compare(0, (*map).first.size(), (*map).first) == 0) {
			return (*map).second + path.substr((*map).first.size());
		}
	}

	// No match, maybe it doesn't have a device path, or maybe we don't know
	// about that drive (possibly only mounted to a directory).
	return path;
}