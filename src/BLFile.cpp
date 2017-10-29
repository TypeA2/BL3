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

#include "BLFile.h"

BLFile::BLFile(BLFileInfo *file) : QObject(){
	this->_file = file;

	loader = new QProcess;
	loader->setProcessChannelMode(QProcess::MergedChannels);

	connect(loader, &QProcess::readyReadStandardOutput, this, &BLFile::loader_read);
	connect(loader, static_cast<void(QProcess::*)(int,QProcess::ExitStatus)>(&QProcess::finished), this, &BLFile::loader_finished);

	QString command("ffmpeg -hide_banner -v quiet -i \"" + file->filePath() + "\" ");
	QString extension;

	if(file->isAudio()){
		extension = "-f s32le -c:a pcm_s32le -ar %1 -ac %2 -vn";
        extension = extension.arg(file->sampleRate()).arg(file->numChannels());
	}else if(file->isImage()){
		return;
	}else if(!file->isImage()){
        _panic = true;
		extension = "-f image2pipe -pix_fmt rgb24 -c:v rawvideo -an -r 50";
	}

	command += extension + " -";

    if(!_panic){
        loader->start(command);
    }else{
        _ready = true;
    }
}

void BLFile::loader_finished(){
	_ready = true;
}

void BLFile::loader_read(){
	_rawData.append(loader->readAllStandardOutput());
}

bool BLFile::panic(){
    return _panic;
}

QByteArray BLFile::rawData(){
	return _rawData;
}

bool BLFile::ready(){
	return _ready;
}

BLFileInfo *BLFile::file(){
	return _file;
}
