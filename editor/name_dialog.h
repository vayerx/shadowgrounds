#ifndef NAME_DIALOG_H
#define NAME_DIALOG_H

struct NameDialogData;
class NameDialog {
	boost::scoped_ptr<NameDialogData> m;
public:
	NameDialog();
	~NameDialog();
	bool doModal(HWND parent, int resourceID);
	const std::string& getName();
	void setName(const std::string& name);
};


#endif
