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

#ifndef BLCYCLICRENDERER_H
#define BLCYCLICRENDERER_H

#include <QObject>
#include <QTemporaryFile>
#include <QtMath>
#include <math.h>
#include <QPainter>

#include "BLFile.h"

class BLCyclicRenderer : public QObject{
    Q_OBJECT

    public:
		BLCyclicRenderer(BLFile *audio, BLFile *video, int fps, QString out, QColor color);
		~BLCyclicRenderer();

	public slots:
		void render();

	signals:
		void render_finished();
		void import_progress(int index);
		void export_output(QByteArray data);
		void set_num_importers(int n);

    private:
        BLFile  *audio;
        BLFile  *video;
        int     fps;
		QString	out;
		QColor	color;

		QByteArray				pcm;
		QList<QList<qint32>>	pcmArray;
		qreal					nPoints;
		qreal					scaledHeight;
		qreal					frameIndex	= 0;

        QByteArray      *videoBuffer;
        QProcess        *blExport;
        QTemporaryFile  *tmpAudioFile;

		qint64			index;
        QString         tempAudioFileName;
        qint64          maxIndex;
        qreal           remainderTime;

		QList<QString>		commands;
		QList<QProcess*>	importers;
		QList<bool>			finished;

	private slots:
		void importer_finished();
};

#endif // BLCYCLICRENDERER_H
