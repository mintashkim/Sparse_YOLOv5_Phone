#pragma once
#include <ResourcePack/ResourcePack.h>
#include <QDialog>

namespace Ui { class MsgBoxDlg; };

class RESOURCEPACK_EXPORT MsgBoxDlg : public QDialog
{
	Q_OBJECT

public:

	enum eBUTTONS { eBTN_YES, eBTN_NO, eBTN_MAX };

	MsgBoxDlg(QWidget* host, QWidget* parent = Q_NULLPTR);
	MsgBoxDlg(QWidget* host, const QString& strDescription, const QString& strTitle = "",
		bool bYesNo = false, const QString& strBtnYes = "", const QString& strBtnNo = "");
	~MsgBoxDlg();

	void setText(const QString& strTitle, const QString& strDescription);
	void setButtonText(eBUTTONS button, const QString& strText);
	void setButtons(bool bYesOrNo = true);

protected:
	virtual bool nativeEvent(const QByteArray& eventType, void* message, long* result);
	virtual bool eventFilter(QObject* watched, QEvent* event);

	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event);

	virtual void keyPressEvent(QKeyEvent* event);

private:
	Ui::MsgBoxDlg* ui;

	QString m_strTitle;

	QRect m_orgRectYes;
	QRect m_orgRectNo;

	bool m_bMouseLeftPressed;
	QPoint m_ptMouseLeftPressed;

};
