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

#include "BLFileInfo.h"

BLFileInfo::BLFileInfo(QFileInfo file, bool audio, bool video) : QObject(), QFileInfo(file){
	this->_isAudio = audio;
	this->_isImage = !video;

	QString parseCmd = "ffprobe -hide_banner -print_format json"
							" -show_format -show_streams -v quiet "
							+ file.absoluteFilePath().prepend("\"").append("\"");

	parser = new QProcess;

    parser->setProcessChannelMode(QProcess::MergedChannels);
    connect(parser, static_cast<void(QProcess::*)(int,QProcess::ExitStatus)>(&QProcess::finished), this, &BLFileInfo::parse_info);

	parser->start(parseCmd);
}

void BLFileInfo::parse_info(){
	QVariantMap map		= QJsonDocument::fromJson(parser->readAll()).object().toVariantMap();

	QVariantMap stream0 = map["streams"].toList()[0].toMap();
	QVariantMap format	= map["format"].toMap();

	if(_isAudio){
		_streamCount	= map["streams"].toList().length();
		_codecName		= stream0["codec_name"].toString();
		_codecNameLong	= stream0["codec_long_name"].toString();
		_fmt			= stream0["sample_fmt"].toString();
		_sampleRate		= stream0["sample_rate"].toInt();
		_numChannels	= stream0["channels"].toInt();
		_channelLayout	= stream0["channel_layout"].toString();
		_isPCM			= stream0["bits_per_sample"].toInt() > 0;
		_bitDepth		= stream0["bits_per_sample"].toInt();
		_frameCount		= stream0["duration_ts"].toInt();
		_duration		= stream0["duration"].toDouble();
		_bitDepthRaw	= stream0["bits_per_raw_sample"].toInt();
		_filePath		= format["filename"].toString();
		_fileSize		= format["size"].toInt();
		_fileBitRate	= format["bit_rate"].toInt();
	}else if(_isImage){
		_streamCount	= map["streams"].toList().length();
		_codecName		= stream0["codec_name"].toString();
		_codecNameLong	= stream0["codec_long_name"].toString();
		_width			= stream0["width"].toInt();
		_height			= stream0["height"].toInt();
		_fmt			= stream0["pix_fmt"].toString();
		_filePath		= format["filename"].toString();
		_fileSize		= format["size"].toInt();
	}else if(!_isImage){
		_streamCount	= map["streams"].toList().length();
		_codecName		= stream0["codec_name"].toString();
		_codecNameLong	= stream0["codec_long_name"].toString();
		_fmt			= stream0["pix_fmt"].toString();
		_width			= stream0["width"].toInt();
		_height			= stream0["height"].toInt();
		_srcFps			= QScriptEngine().evaluate(stream0["r_frame_rate"].toString()).toNumber();
		_duration		= format["duration"].toDouble();
		_filePath		= format["filename"].toString();
		_fileSize		= format["size"].toInt();
	}

	_ready				= true;
}

int BLFileInfo::height(){
	if(_isAudio) throw ProperyNotSetException("file " + _filePath + " has no height");
	return _height;
}

int BLFileInfo::width(){
	if(_isAudio) throw ProperyNotSetException("file " + _filePath + " has no width");
	return _width;
}

bool BLFileInfo::isAudio(){
	return _isAudio;
}

bool BLFileInfo::isImage(){
	return _isImage;
}

double BLFileInfo::srcFps(){
	if(!_isAudio && !_isImage) throw ProperyNotSetException("file " + _filePath + " has no fps");
	return _srcFps;
}

int BLFileInfo::fileBitRate(){
	if(!_isAudio) throw ProperyNotSetException("file " + _filePath + " has no file bitrate");
	return _fileBitRate;
}

int BLFileInfo::fileSize(){
	return _fileSize;
}

QString BLFileInfo::filePath(){
	return _filePath;
}

int BLFileInfo::bitDepthRaw(){
	if(!_isAudio) throw ProperyNotSetException("file " + _filePath + " has no raw bit depth");
	return _bitDepthRaw;
}

double BLFileInfo::duration(){
	if(!_isAudio && _isImage) throw ProperyNotSetException("file " + _filePath + " has no duration");
	return _duration;
}

int BLFileInfo::frameCount(){
	if(!_isAudio) throw ProperyNotSetException("file " + _filePath + " has no frame count");
	return _frameCount;
}

int BLFileInfo::bitDepth(){
	if(!_isAudio) throw ProperyNotSetException("file " + _filePath + " has no bit depth");
	return _bitDepth;
}

bool BLFileInfo::isPCM(){
	if(!_isAudio) throw ProperyNotSetException("file " + _filePath + " cannot be PCM");
	return _isPCM;
}

QString BLFileInfo::channelLayout(){
	if(!_isAudio) throw ProperyNotSetException("file " + _filePath + " has no channel layout");
	return _channelLayout;
}

int BLFileInfo::numChannels(){
	if(!_isAudio) throw ProperyNotSetException("file " + _filePath + " has no channels");
	return _numChannels;
}

int BLFileInfo::sampleRate(){
	if(!_isAudio) throw ProperyNotSetException("file " + _filePath + " has no samplerate");
	return _sampleRate;
}

QString BLFileInfo::fmt(){
	return _fmt;
}

QString BLFileInfo::codecNameLong(){
	return _codecNameLong;
}

QString BLFileInfo::codecName(){
	return _codecName;
}

int BLFileInfo::streamCount(){
	return _streamCount;
}

bool BLFileInfo::ready(){
	return _ready;
}

