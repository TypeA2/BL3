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

#ifndef BLFILEINFO_H
#define BLFILEINFO_H

#include <QObject>
#include <QProcess>
#include <QFileDialog>
#include <QCoreApplication>
#include <QVariantMap>
#include <QJsonDocument>
#include <QJsonObject>
#include <QScriptEngine>

#include <exception>

#include <QDebug>

class ProperyNotSetException : public std::exception {
	public:
		ProperyNotSetException(QString msg) : e_msg(msg){ }
		const char * what() const throw(){
			return e_msg.toStdString().c_str();
		}

	private:
		QString e_msg;
};

class BLFileInfo : public QObject, public QFileInfo{
	Q_OBJECT

	public:
		BLFileInfo(QFileInfo file, bool audio, bool video = false);

		bool	isAudio();
		bool	isImage();
		bool	ready();

		int		streamCount();
		QString	codecName();
		QString codecNameLong();
		QString fmt();
		int		sampleRate();
		int		numChannels();
		QString	channelLayout();
		bool	isPCM();
		int		bitDepth();
		int		frameCount();
		double	duration();
		int		bitDepthRaw();
		QString	filePath();
		int		fileSize();
		int		fileBitRate();
		double	srcFps();
		int		width();
		int		height();

	private:
		bool _isAudio;
		bool _isImage;
		bool _ready	= false;

		int		_streamCount;
		QString	_codecName;
		QString	_codecNameLong;
		QString	_codecType;
		QString	_fmt;
		int		_sampleRate;
		int		_numChannels;
		QString	_channelLayout;
		bool	_isPCM;
		int		_bitDepth;
		int		_frameCount;
		double	_duration;
		int		_bitDepthRaw;
		QString	_filePath;
		int		_fileSize;
		int		_fileBitRate;
		int		_width;
		int		_height;
		double	_srcFps;

		QProcess	*parser;

	private slots:
		void parse_info();
};

#endif // BLFILEINFO_H
