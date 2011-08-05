#include <windows.h>
#include <windowsx.h>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <vector>
#include <string>
#include <commctrl.h>
#include "name_dialog.h"
#include "resource/resource.h"

struct NameDialogData {
	
	std::string mString;
	
	NameDialogData() {

	}

	~NameDialogData() {

	}
	
	static BOOL CALLBACK dlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

		static std::string* pString = NULL;

		switch(msg) {
		case WM_INITDIALOG:
			{
				pString = (std::string*)lParam;
				SetDlgItemText(hwnd, IDC_NAME_EDIT, pString->c_str());
				return TRUE;
			} break;		
		case WM_COMMAND:
			{
				if(LOWORD(wParam)==IDOK) {
					char buffer[256];
					memset(buffer, 0, sizeof(buffer));
					GetDlgItemText(hwnd, IDC_NAME_EDIT, buffer, 256);
					*pString = buffer;
					EndDialog(hwnd, IDOK);
					return TRUE;
				}
				if(LOWORD(wParam)==IDCANCEL) {
					EndDialog(hwnd, IDCANCEL);
					return TRUE;
				}

			} break;
		}

		return FALSE;
	}

	bool doModal(HWND parent, int resourceID) {

		if(IDOK==DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(resourceID), parent, dlgProc, 
			(LPARAM)&mString))
			return true;

		return false;
	}



};


NameDialog::NameDialog() {
	boost::scoped_ptr<NameDialogData> temp(new NameDialogData());
	m.swap(temp);
}

NameDialog::~NameDialog() {
	
}



bool NameDialog::doModal(HWND parent, int resourceID) {
	return m->doModal(parent, resourceID);
}
	
const std::string& NameDialog::getName() {
	return m->mString;
}
	
void NameDialog::setName(const std::string& name) {
	m->mString = name;
}
