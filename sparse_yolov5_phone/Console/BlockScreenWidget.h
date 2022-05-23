#pragma once

#include <QWidget>
#include "ui_BlockScreenWidget.h"

class BlockScreenWidget : public QWidget
{
	Q_OBJECT

public:
	BlockScreenWidget(int nType, QWidget *parent = Q_NULLPTR);
	~BlockScreenWidget();

protected:
	virtual void resizeEvent(QResizeEvent* event);
	virtual void closeEvent(QCloseEvent* event);

private:
	Ui::BlockScreenWidget ui;
	QPixmap m_pixBlockScreen;
};
