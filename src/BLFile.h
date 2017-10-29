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

#ifndef BLFILE_H
#define BLFILE_H

#include <QObject>

#include "BLFileInfo.h"

class BLFile : public QObject{
	Q_OBJECT

	public:
		BLFile(BLFileInfo *file);

		BLFileInfo	*file();
		bool		ready();
        bool        panic();
		QByteArray	rawData();

	private:
		BLFileInfo	*_file;
		bool		_ready = false;
        bool        _panic = false;

		QProcess	*loader;

		QByteArray	_rawData;

	private slots:
		void loader_read();
		void loader_finished();

};

#endif // BLFILE_H
