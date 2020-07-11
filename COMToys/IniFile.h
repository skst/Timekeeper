//////////////////
// CInitFile lets you put profile settings in a .INI file instead
// of the registry. To use it, write:
//
//	CIniFile::Use(this, CIniFile::WindowsINI); // put INI file in \WINDOWS dir
//	CIniFile::Use(this, CIniFile::LocalINI);	 // put it in same dir as pgm
//
// where "this" is a ptr to your CWinApp/
// Normally do this in your app's InitInstance fn.
//
class CIniFile {
public:
	enum INI_LOCATION { WinDir, LocalDir };
	static void Use(CWinApp *pApp, const INI_LOCATION where, LPCTSTR szProfileName = NULL);
};
