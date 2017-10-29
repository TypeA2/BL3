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

#include "BLCyclicRenderer.h"

BLCyclicRenderer::BLCyclicRenderer(BLFile *audio, BLFile *video, int fps, QString out, QColor color) : QObject(){
    this->audio = audio;
    this->video = video;
    this->fps   = fps;
	this->out	= out;
	this->color = color;

	maxIndex		= qFloor(audio->file()->duration() / 0.5);
	remainderTime	= fmod(audio->file()->duration(), 0.5);
	pcm				= audio->rawData();
	nPoints			= audio->file()->sampleRate() / fps;
	scaledHeight	= nPoints / 16 * 9;

	QDataStream stream(&pcm, QIODevice::ReadOnly);
	stream.setByteOrder(QDataStream::LittleEndian);

	pcmArray = QList<QList<qint32>>();

	QList<qint32> ch0;
	QList<qint32> ch1;

	pcmArray.append(ch0);
	pcmArray.append(ch1);

	while(!stream.atEnd()){
		qint32 v0;
		qint32 v1;

		stream >> v0;
		stream >> v1;

		pcmArray[0].append(v0);
		pcmArray[1].append(v1);
	}
}

void BLCyclicRenderer::render(){
    blExport = new QProcess;
	blExport->setProcessChannelMode(QProcess::MergedChannels);

	connect(blExport, &QProcess::readyReadStandardOutput, this, [=](){
		QByteArray data = blExport->readAllStandardOutput();
		emit export_output(data);
	});

	connect(blExport, static_cast<void(QProcess::*)(int)>(&QProcess::finished), this, [=](){
		QByteArray data = blExport->readAllStandardOutput();
		emit export_output(data);
		emit render_finished();
	});

    tmpAudioFile = new QTemporaryFile();
    tmpAudioFile->open();
    tmpAudioFile->write(audio->rawData());

	QString exportCmd = QString("ffmpeg -f s32le -c:a pcm_s32le -ar %1 -ac %2 -i \"%6\" -framerate 50 -f rawvideo -pix_fmt rgb24 -video_size %3x%4 -i - -y -c:a aac -b:a 320k -c:v libx264 -b:v 16M -pix_fmt yuv420p -max_muxing_queue_size 8192 \"%5\"")
			.arg(audio->file()->sampleRate())
			.arg(audio->file()->numChannels())
			.arg(video->file()->width())
			.arg(video->file()->height())
			.arg(out)
			.arg(tmpAudioFile->fileName());

	for(index = 0; index < maxIndex; index++){
		commands.append(QString("ffmpeg -hide_banner -v quiet -ss %1 -i \"%4\" -ss %2 -t %3 -vf fps=50 -an -f rawvideo -pix_fmt rgb24 -c:v rawvideo -")
						.arg((index * 0.5 - 0.5 > 0) ? (index * 0.5 - 0.5) : 0)
						.arg((index * 0.5 - 0.5 < 0) ? 0 : 0.5)
						.arg(0.5)
						.arg(video->file()->absoluteFilePath()));
		QProcess *importer = new QProcess;
		importer->setProcessChannelMode(QProcess::MergedChannels);
		importers.append(importer);
		finished.append(false);
	}

	if(fmod(audio->file()->duration(), 1) == 0){
		commands.append(QString("ffmpeg -hide_banner -v quiet -ss %1 -i \"%4\" -ss %2 -t %3 -vf fps=50 -an -f rawvideo -pix_fmt rgb24 -c:v rawvideo -")
						.arg((index * 0.5) - 0.5)
						.arg(0.5)
						.arg(remainderTime)
						.arg(video->file()->absoluteFilePath()));
		QProcess *importer = new QProcess;
		importer->setProcessChannelMode(QProcess::MergedChannels);
		importer->setObjectName(QString::number(index));
		importers.append(importer);
		finished.append(false);

	}


	connect(blExport, &QProcess::started, this, [=](){
		for(index = 0; index < maxIndex; index++){
			connect(importers[index], static_cast<void(QProcess::*)(int)>(&QProcess::finished), this, &BLCyclicRenderer::importer_finished);

			importers[index]->start(commands[index]);

			while(!finished[index]) qApp->processEvents();
		}
		blExport->closeWriteChannel();

		tmpAudioFile->remove();
	});

	emit set_num_importers(commands.length());

	blExport->start(exportCmd);
}

void BLCyclicRenderer::importer_finished(){
	emit import_progress(index);

	QByteArray data = importers[index]->readAllStandardOutput();

	qint64 bytesPerFrame = video->file()->width() * video->file()->height() * 3;

	for(int i = 0; i < qFloor(data.size() / (float)bytesPerFrame); i++){
		QByteArray output;
		uchar *ucData = new uchar[bytesPerFrame];
		memcpy(ucData, data.mid(i * bytesPerFrame, bytesPerFrame).data(), bytesPerFrame);
		QImage background(ucData,
						video->file()->width(),
						video->file()->height(),
						QImage::Format_RGB888);

		QPixmap pm(qRound(nPoints), qRound(scaledHeight));
		if(pm.paintEngine() != 0){
			pm.fill(Qt::transparent);

			QPainterPath	path;
			qreal			startY		= scaledHeight / 2.;
			qreal			nMod		= 0xFFFFFFFF / startY;
			QPointF			start(0., startY - (pcmArray[0][qRound64(frameIndex / fps * audio->file()->sampleRate())] / nMod));

			path.moveTo(start);

			for(int f = 0; f < nPoints; f++){
				QPointF next((qreal) f, startY - (pcmArray[0][qRound64(frameIndex / fps * audio->file()->sampleRate()) + f] / nMod));
				path.lineTo(next);
			}

			QPainter p(&pm);

			p.strokePath(path, QPen(color));

			QPixmap overlay			= pm.scaled(video->file()->width(), video->file()->height());
			QPixmap backgroundPix	= QPixmap::fromImage(background);

			QPainter display(&backgroundPix);

			display.drawPixmap(0, 0, overlay);

			background = backgroundPix.toImage();
		}

		for(int y = 0; y < background.height(); y++){
			for(int x = 0; x < background.width(); x++){
				QRgb pixel = background.pixel(x, y);
				char r = static_cast<char>(qRed(pixel));
				char g = static_cast<char>(qGreen(pixel));
				char b = static_cast<char>(qBlue(pixel));

				output.append(r);
				output.append(g);
				output.append(b);
			}
		}

		delete ucData;

		blExport->write(output);

		frameIndex++;
	}
	finished[index] = true;
}

BLCyclicRenderer::~BLCyclicRenderer(){
	tmpAudioFile->remove();
}
