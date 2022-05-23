#pragma once
#include <ShellAPI.h>

namespace Library
{
namespace API
{

class ShellAPI : public Object
{
public:
	// Shell Icon
	static HICON GetIcon(int nIndex, int nType = SHIL_EXTRALARGE);
	static HICON GetFileIcon(string& strPath, int nType = SHIL_EXTRALARGE);
	static HICON GetFileIcon(string& strPath, int nIndex, int nType);

	// Window
	static void CloseRemovableExplorerWindow();
	static void BringWindowToTopLevel(HWND hWnd);
	static HWND FindMyTopMostWindow();

	static HWND FindTrayToolbarWindow();
	static void ClearDeadTrayIcon();
	static void ClearDeadSystemTrayIcon();

	static void CheckIdleTimeout();

	// Shell Folder #1753
	static void RegisterShellFolder(
		const string& strClsid,
		const string& strName,
		const string& strFolderPath,
		const string& strIconPath,
		bool fWow64);
	static void UnregisterShellFolder(
		const string& strClsid,
		bool fWow64);
	static void ShowShellFolderOnExplorer(
		const string& strClsid,
		const string& strName,
		RegistryKey* pHKLMKey,
		bool fImport,
		bool fWow64);
	static void HideShellFolderOnExplorer(
		const string& strClsid,
		RegistryKey* pHKLMKey,
		bool fWow64);

private:
	static void ClearDeadSystemTrayIcon(HWND hWndTray);
	static bool IsProcessExists(HANDLE hSnapshot, DWORD pid);
};

}
}