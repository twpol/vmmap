//------------------------------------------------------------------------------
// http://twpol.dyndns.org/projects/vmmap
// License: Microsoft Public License (Ms-PL).
//------------------------------------------------------------------------------

#pragma once

// Converts a kernel-mode path (device path) into a user-mode path (drive
// path). E.g. "\Device\HarddiskVolume1\Windows\explorer.exe" ->
// "C:\Windows\explorer.exe".
const std::tstring MapDevicePathToDrivePath(const std::tstring&);
