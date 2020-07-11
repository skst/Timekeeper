#include "StdAfx.h"
#include "RegistryMonitor.h"

RegistryMonitor::RegistryMonitor()
{
	_hKeyMonitor = nullptr;
}


RegistryMonitor::~RegistryMonitor()
{
	Close();
}

void RegistryMonitor::Close()
{
	if (_hKeyMonitor != nullptr)
		::RegCloseKey(_hKeyMonitor);
}


void RegistryMonitor::Monitor(HKEY hkeyRoot, LPCTSTR szKeyName)
{
	Close();

	/*
		monitor the registry key for changes
	*/
	VERIFY(::RegOpenKeyEx(hkeyRoot, szKeyName, NULL, KEY_NOTIFY, &_hKeyMonitor) == ERROR_SUCCESS);
	VERIFY(::RegNotifyChangeKeyValue(_hKeyMonitor, FALSE /*watch subtree*/, REG_NOTIFY_CHANGE_LAST_SET, _evtNotify, TRUE /*asynch*/) == ERROR_SUCCESS);
}


bool RegistryMonitor::IsModified() const
{
	if (_hKeyMonitor == nullptr)
		return false;

	// if there's a change in the monitored registry key, re-load the format
	if (::WaitForSingleObject(_evtNotify, 0) == WAIT_OBJECT_0)
	{
		// monitor the registry key again
		VERIFY(::RegNotifyChangeKeyValue(_hKeyMonitor, FALSE /*watch subtree*/, REG_NOTIFY_CHANGE_LAST_SET, _evtNotify, TRUE /*asynch*/) == ERROR_SUCCESS);
		return true;
	}

	return false;
}
