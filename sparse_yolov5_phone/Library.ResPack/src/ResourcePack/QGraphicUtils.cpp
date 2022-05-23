#include "stdafx.h"
#include <ResourcePack/QGraphicUtils.h>

void QGraphicUtils::DrawDialogShadow(QPaintDevice* pTarget, uint round_radius)
{
	const int DialogShadowColors[20] = { 0, 0, 0, 0, 13, 9, 13, 15, 16, 16, 14, 17, 15, 16, 17, 18, 17, 19, 20, 18 };
	const int DialogShadowAlphas[20] = { 1, 3, 7, 13, 20, 28, 38, 50, 63, 78, 93, 107, 117, 127, 135, 142, 147, 151, 153, 153 };
	const int DialogShadowWidth = 10;
	const int DialogShadowRadius = 20;


	QPainter painter(pTarget);
	painter.setRenderHint(QPainter::Antialiasing, true);
	int shadowWidth = DialogShadowWidth;
	int shadowRadius = round_radius;
	int target_width = pTarget->width();
	int target_height = pTarget->height();

	for (int i = 0; i < shadowWidth; i++)
	{
		QPainterPath path;
		path.setFillRule(Qt::FillRule::WindingFill);
		int pos_x = shadowWidth - i;
		int pos_y = pos_x;
		int width = target_width - ((shadowWidth - i) * 2);
		int height = target_height - ((shadowWidth - i) * 2);
		int radius = shadowRadius + i;

		path.addRoundedRect(pos_x, pos_y, width, height, radius, radius);
		int index = (shadowWidth - 1) - i;
		int color = DialogShadowColors[index];
		int alpha = DialogShadowAlphas[index];
		painter.setPen(QColor(color, color, color, alpha));
		painter.drawPath(path);
	}
}
