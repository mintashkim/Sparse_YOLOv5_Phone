#include "stdafx.h"
#include "BlockScreenWidget.h"
#include "ConsoleMain.h"

#include "wpp.h"
#include "BlockScreenWidget.tmh"

BlockScreenWidget::BlockScreenWidget(int nType, QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	QString strImagePath;
	if (1 == nType)
		strImagePath = Resource_Image::GetImagePath(Module_Console::PHONE_PRESENCE);
	else
		strImagePath = Resource_Image::GetImagePath(Module_Console::PERSON_ABSENCE);
	m_pixBlockScreen = QPixmap::fromImage(QImage(strImagePath));

	ui.labelImage->setPixmap(m_pixBlockScreen);

	setWindowFlags(Qt::WindowStaysOnTopHint | Qt::SubWindow);	// #2884
}

BlockScreenWidget::~BlockScreenWidget()
{
}

void BlockScreenWidget::resizeEvent(QResizeEvent* event)
{
	QPixmap pixScaled = m_pixBlockScreen.scaled(
		ui.labelImage->width(),
		ui.labelImage->height(),
		Qt::KeepAspectRatio);
	ui.labelImage->resize(pixScaled.size());
}

void BlockScreenWidget::closeEvent(QCloseEvent* event)
{
	if (Qt::RightButton & qApp->mouseButtons())
		event->accept();
	else
		event->ignore();
}
