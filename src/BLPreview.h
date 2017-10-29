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

#ifndef BLPREVIEW_H
#define BLPREVIEW_H

#include <QObject>
#include <QLabel>
#include <QtMath>
#include <QPainter>

#include <QDebug>

#include "BLFile.h"

class BLPreview : public QLabel{
	Q_OBJECT

	public:
		BLPreview(BLFile *video);
		~BLPreview();

	signals:
		void import_finished();

	public slots:
		void color_changed(QColor color);

	private:
		QProcess *importer;

		QPixmap background;
};

#endif // BLPREVIEW_H
