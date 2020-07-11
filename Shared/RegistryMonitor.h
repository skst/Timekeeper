#pragma once

#include <afxmt.h>

class RegistryMonitor
{
protected:
	HKEY _hKeyMonitor;	// The key to monitor
	CEvent _evtNotify;	// Signalled when registry key changes

public:
	RegistryMonitor();
	virtual ~RegistryMonitor();

	void Monitor(HKEY hAppKey, LPCTSTR szKeyName);
	bool IsModified() const;

protected:
	void Close();
};
