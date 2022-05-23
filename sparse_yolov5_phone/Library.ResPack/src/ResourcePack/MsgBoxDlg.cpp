#include "stdafx.h"
#include <ResourcePack/MsgBoxDlg.h>
#include <ResourcePack/QGraphicUtils.h>
#include "ui_MsgBoxDlg.h"

MsgBoxDlg::MsgBoxDlg(QWidget* host, QWidget* parent)
	: QDialog(parent)
{
	ui = new Ui::MsgBoxDlg();
	ui->setupUi(this);

	//this->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
	this->setWindowFlags(Qt::FramelessWindowHint);
	this->setAttribute(Qt::WA_TranslucentBackground, true);

	QString qss_dialog = GET_STYLESHEET(Module_Common::QSS_MESSAGEBOX);
	this->setStyleSheet(qss_dialog);

	setWindowIcon(QPixmap(Resource_Image::GetImagePath(Module_Common::ICON_WINDOW)));

	m_strTitle = Resource_Lang::GetText(Module_Common::PRODUCT_NAME);
	setWindowTitle(m_strTitle);
	ui->m_lbQuestion->setText(m_strTitle);

	connect(ui->m_btnNo, &QPushButton::clicked, this, &QDialog::reject);
	connect(ui->m_btnYes, &QPushButton::clicked, this, &MsgBoxDlg::accept);

	this->setModal(true);

	QString qstrYes = Resource_Lang::GetText(Module_Common::MSGBOX_BUTTON_ACCEPT);
	QString qstrNo = Resource_Lang::GetText(Module_Common::MSGBOX_BUTTON_CANCEL);

	ui->m_btnYes->setText(qstrYes);
	ui->m_btnNo->setText(qstrNo);

	m_orgRectYes = ui->m_btnYes->geometry();
	m_orgRectNo = ui->m_btnNo->geometry();

	m_bMouseLeftPressed = false;

	installEventFilter(this);

	QTimer::singleShot(0, [=]() {
		move(host->geometry().center() - rect().center());
		});
}

MsgBoxDlg::MsgBoxDlg(QWidget* host, const QString& strDescription, const QString& strTitle, bool bYesNo, const QString& strBtnYes, const QString& strBtnNo)
	: MsgBoxDlg(host, Q_NULLPTR)
{
	this->setButtons(bYesNo);
	this->setText(strTitle, strDescription);

	if (bYesNo)
	{
		if (!strBtnNo.isEmpty())
		{
			this->setButtonText(eBUTTONS::eBTN_NO, strBtnNo);
		}
	}
	if (!strBtnYes.isEmpty())
	{
		this->setButtonText(eBUTTONS::eBTN_YES, strBtnYes);
	}
}

MsgBoxDlg::~MsgBoxDlg()
{
	delete ui;
}

bool MsgBoxDlg::nativeEvent(const QByteArray& eventType, void* message, long* result)
{
	//MSG* msg = static_cast<MSG*>(message);


	return QDialog::nativeEvent(eventType, message, result);
}

void MsgBoxDlg::setText(const QString& strTitle, const QString& strDescription)
{
	if (false == strTitle.isEmpty())
		ui->m_lbQuestion->setText(strTitle);

	ui->m_lbQuestion2->setText(strDescription);
}

void MsgBoxDlg::setButtonText(eBUTTONS button, const QString& strText)
{
	if (button == eBTN_YES)
	{
		ui->m_btnYes->setText(strText);
	}
	else if (button == eBTN_NO)
	{
		ui->m_btnNo->setText(strText);
	}
}

void MsgBoxDlg::setButtons(bool bYesOrNo)
{
	if (!bYesOrNo)
	{
		ui->m_btnNo->setVisible(false);
		ui->m_btnYes->setGeometry(m_orgRectNo);
		ui->m_btnYes->setVisible(true);
	}
	else
	{
		ui->m_btnYes->setGeometry(m_orgRectYes);
		ui->m_btnNo->setGeometry(m_orgRectNo);
		ui->m_btnYes->setVisible(true);
		ui->m_btnNo->setVisible(true);
	}
}

bool MsgBoxDlg::eventFilter(QObject* watched, QEvent* event)
{
	if (event->type() == QEvent::Paint)
	{
		QGraphicUtils::DrawDialogShadow(this, 5);
	}
	return QDialog::eventFilter(watched, event);
}


void MsgBoxDlg::mousePressEvent(QMouseEvent* event)
{
	if (Qt::MouseButton::LeftButton == event->button())
	{
		if (!m_bMouseLeftPressed)
		{
			m_bMouseLeftPressed = true;
			m_ptMouseLeftPressed = event->pos();
		}
	}
	return QDialog::mouseReleaseEvent(event);
}

void MsgBoxDlg::mouseReleaseEvent(QMouseEvent* event)
{
	if (Qt::MouseButton::LeftButton == event->button())
	{
		if (m_bMouseLeftPressed)
		{
			m_bMouseLeftPressed = false;
		}
	}
	return QDialog::mouseReleaseEvent(event);
}

void MsgBoxDlg::mouseMoveEvent(QMouseEvent* event)
{
	if (m_bMouseLeftPressed)// && Qt::MouseButton::LeftButton == event->button())
	{
		QPoint newPos = this->pos() + event->pos() - m_ptMouseLeftPressed;
		this->move(newPos);
	}
	return QDialog::mouseMoveEvent(event);
}

void MsgBoxDlg::keyPressEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key_Escape)
	{
		reject();
		return;
	}

	QDialog::keyPressEvent(event);
}
