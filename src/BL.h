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

#ifndef BL_H
#define BL_H

#include <QMainWindow>
#include <QStandardPaths>
#include <QGridLayout>
#include <QPushButton>
#include <QFile>
#include <QFileDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QComboBox>
#include <QLabel>
#include <QColorDialog>
#include <QTextEdit>
#include <QScrollBar>
#include <QThread>
#include <QProgressBar>
#include <QApplication>
#include <QDesktopWidget>

#include <QDebug>

#include "BLFileInfo.h"
#include "BLFile.h"
#include "BLCyclicRenderer.h"
#include "BLPreview.h"

class BL : public QMainWindow{
    Q_OBJECT

	public:
		BL();
		~BL();

	private:
		QWidget		*central;
		QGridLayout	*centralLayout;
		QPushButton	*theStart;

		BLFile		*bgFile;
		BLFile		*audioFile;

		QFormLayout *form;

		QColorDialog	*color;
		BLPreview		*preview;
		QPushButton		*render;

		QColor		selectedColor;
		QLabel		*pathErr;
		QPushButton	*setOutputPath;
		QLineEdit	*outputPath;
		bool		outputExists = false;
		QPushButton	*create;

		QTextEdit	*output;
		QString		outString;
		QThread		*renderThread;

		BLCyclicRenderer	*renderer;
		QProgressBar		*progress;

		void load_files(QFileInfo background, QFileInfo audio, bool video);
        void setup_controls();

	private slots:
		void start_balkinizing();
		void setup_render_form();
		void setup_rendering_layout();
};

#endif // BL_H
