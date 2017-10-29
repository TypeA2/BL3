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

#include "BLAudioData.h"

BLAudioData::BLAudioData(BLFileInfo *parent, int fps) : QObject(){
	emit set_label("Setting up AudioData");

//	if(!parent->ready())
		return;

	if(fps <= 0)
		throw std::invalid_argument("expected positive fps");

	this->_fps = fps;
	this->info = parent;


	QString loadCmd = QString("ffmpeg -hide_banner -v quiet -i \"%3\" -f s32le -acodec pcm_s32le -ar %1 -ac %2 -vn -")
					.arg(info->sampleRate())
					.arg(info->numChannels())
					.arg(info->filePath());

	emit set_label("Collecting data");

	importProcess	= new QProcess;

	importProcess->setProcessChannelMode(QProcess::MergedChannels);
	connect(importProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(update_import_progress()));
	connect(importProcess, SIGNAL(finished(int)), this, SLOT(import_finished()));

	importProcess->start(loadCmd);
}

int BLAudioData::fps(){
	return _fps;
}

QList<QList<qint32>> BLAudioData::dataArr(){
	return _dataArr;
}

QList<QList<qint32>> BLAudioData::idata(){
	return _idata;
}

void BLAudioData::import_finished(){
	emit set_label("Collected data");

	QDataStream stream(&_data, QIODevice::ReadOnly);
	stream.setByteOrder(QDataStream::LittleEndian);

	int samplesPerFrame = this->info->sampleRate() / this->_fps;

	int i = 0;

	_idata = QList<QList<qint32>>();

	QList<qint32> ch0;
	QList<qint32> ch1;

	_idata.append(ch0);
	_idata.append(ch1);
	_dataArr.append(ch0);
	_dataArr.append(ch1);

	emit set_label("Loading data");

	while(!stream.atEnd()){
		qint32 left;
		qint32 right;

		stream >> left;
		stream >> right;

		_dataArr[0].append(left);
		_dataArr[1].append(right);

		if(i % samplesPerFrame == 0){
			_idata[0].append(left);
			_idata[1].append(right);
		}

		i++;
	}

	emit set_label(QString("Collected %1 samples").arg(_dataArr[0].length() * 2));

	_ready = true;
}

void BLAudioData::update_import_progress(){
	_data.append(importProcess->readAllStandardOutput());
}

QByteArray BLAudioData::data(){
	return _data;
}

bool BLAudioData::ready(){
	return _ready;
}
