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

#ifndef BLAUDIODATA_H
#define BLAUDIODATA_H

#include <stdexcept>

#include <QObject>
#include <QList>

#include "BLFileInfo.h"

class BLAudioData : public QObject{
	Q_OBJECT
	public:
		explicit BLAudioData(BLFileInfo *parent, int fps);

		BLFileInfo				*info;

		bool					ready();
		QByteArray				data();
		QList<QList<qint32>>	idata();
		QList<QList<qint32>>	dataArr();
		int						fps();

	signals:
		void set_label(QString str);

	private:
		bool _ready		= false;

		QProcess				*importProcess;
		QByteArray				_data;
		QList<QList<qint32>>	_idata;
		QList<QList<qint32>>	_dataArr;
		int						_fps;

	private slots:
		void update_import_progress();
		void import_finished();
};

#endif // BLAUDIODATA_H
