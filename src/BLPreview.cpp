/**
 * This file is part of Balkinizer v3.
 *
 * Balkinizer v3 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Balkinizer v3 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Balkinizer v3.  If not, see <http://www.gnu.org/licenses/>.
**/

#include "BLPreview.h"

BLPreview::BLPreview(BLFile *video) : QLabel(){
	importer = new QProcess;
	importer->setProcessChannelMode(QProcess::MergedChannels);

	QString importCommand = QString("ffmpeg -hide_banner -v quiet -i \"%1\" -vf thumbnail -frames:v 1 -f rawvideo -c:v rawvideo -pix_fmt rgb24 -")
								.arg(video->file()->absoluteFilePath());

	connect(importer, static_cast<void(QProcess::*)(int)>(&QProcess::finished), this, [=](){
		qint64 fBytes = video->file()->width() * video->file()->height() * 3;
		uchar *ucData = new uchar[fBytes];
		memcpy(ucData, importer->readAllStandardOutput().data(), fBytes);
		QImage img(ucData,
				   video->file()->width(),
				   video->file()->height(),
				   QImage::Format_RGB888);

		background = QPixmap::fromImage(img);
		this->setPixmap(background.scaled(960, 540, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
		emit import_finished();
	});

	importer->start(importCommand);
}

void BLPreview::color_changed(QColor color){
	QPixmap overlay(this->pixmap()->width(), this->pixmap()->height());
	if(overlay.paintEngine() != 0){
		overlay.fill(Qt::transparent);

		QPainterPath	path;
		QPointF			start(0., this->pixmap()->height() / 2.);

		path.moveTo(start);

		for(int i = 0; i < this->pixmap()->width(); i++){
			QPointF next((qreal) i, start.y() + qSin(M_PI / (this->pixmap()->width() / 8) * i) * start.y() / 2.);
			path.lineTo(next);
		}

		QPainter p(&overlay);

		p.strokePath(path, QPen(color));

		QPixmap bg = this->background.copy().scaled(this->pixmap()->width(), this->pixmap()->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

		QPainter display(&bg);

		display.drawPixmap(0, 0, overlay);

		this->setPixmap(bg);
	}
}

BLPreview::~BLPreview(){

}
