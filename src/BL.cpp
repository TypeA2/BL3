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

#include "BL.h"

BL::BL() : QMainWindow(){
	central			= new QWidget;
	centralLayout	= new QGridLayout;
	theStart		= new QPushButton("Start Balkinizing");

	theStart->setObjectName("theStartButton");
	theStart->setCursor(QCursor(Qt::PointingHandCursor));

	connect(theStart, &QPushButton::released, this, &BL::start_balkinizing);

	centralLayout->addWidget(theStart, 0, 0, 1, 1, Qt::AlignHCenter);

	central->setLayout(centralLayout);

	QFile style(":/qss/window.css");
	style.open(QFile::ReadOnly);
	QString styleSheet = QLatin1String(style.readAll());
	style.close();

	this->setCentralWidget(central);
	this->setWindowTitle("Balkinizer v3");
	this->setMinimumSize(640, 480);
	this->setStyleSheet(styleSheet);
}

void BL::load_files(QFileInfo background, QFileInfo audio, bool video){
	theStart->setText("Gathering info...");

	BLFileInfo *bgInfo = new BLFileInfo(background, false, video);
	while(!bgInfo->ready()) qApp->processEvents();

	BLFileInfo *audioInfo = new BLFileInfo(audio, true);
	while(!audioInfo->ready()) qApp->processEvents();

	theStart->setText("Loading files...");

	bgFile = new BLFile(bgInfo);
	while(!bgFile->ready()) qApp->processEvents();

	audioFile = new BLFile(audioInfo);
	while(!audioFile->ready()) qApp->processEvents();
}

void BL::setup_controls(){
    centralLayout->removeWidget(theStart);
    delete theStart;

	/*connect(&videoCodec, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &BL::video_codec_changed);
	connect(&audioCodec, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &BL::audio_codec_changed);
	*/

	color = new QColorDialog(QColor(0xFF00FFFF));
	color->setOptions(QColorDialog::ShowAlphaChannel | QColorDialog::NoButtons);
	color->setStyleSheet(this->styleSheet());

	preview = new BLPreview(bgFile);

	connect(preview, &BLPreview::import_finished, this, [=](){
		preview->color_changed(color->currentColor());
	});
	connect(color, &QColorDialog::currentColorChanged, preview, &BLPreview::color_changed);

	render = new QPushButton("Render");
	render->setObjectName("render");
	render->setCursor(QCursor(Qt::PointingHandCursor));
	connect(render, &QPushButton::released, this, &BL::setup_render_form);

	centralLayout->addWidget(preview, 0, 0);
	centralLayout->addWidget(render, 1, 0);

	centralLayout->setAlignment(preview, Qt::AlignHCenter);
	centralLayout->setAlignment(color, Qt::AlignHCenter);

	centralLayout->setRowMinimumHeight(0, 540);

	color->show();

	this->setMinimumSize(1024, 720);
	this->setGeometry(QStyle::alignedRect(
						Qt::LeftToRight, Qt::AlignCenter,
						this->size(), qApp->desktop()->availableGeometry()));
	color->move(qMax(0, this->x() - color->frameGeometry().width()), color->y());
}

void BL::setup_render_form(){
	centralLayout->removeWidget(preview);
	centralLayout->removeWidget(render);

	color->close();
	selectedColor = color->currentColor();

	delete color;
	delete preview;
	delete render;

	pathErr = new QLabel;
	pathErr->setObjectName("pathErr");

	setOutputPath = new QPushButton("Select output file");
	setOutputPath->setObjectName("setOutputPath");
	setOutputPath->setCursor(QCursor(Qt::PointingHandCursor));

	outputPath = new QLineEdit;
	outputPath->setObjectName("outputPath");

	connect(setOutputPath, &QPushButton::released, this, [=](){
		QString path =
				QFileDialog::getSaveFileName(
					this,
					"Select output file",
					outputPath->text(),
					"*.avi *.mkv *.mp4 *.webm");

		if(path.isEmpty()) return;

		outputPath->setText(path);
	});

	connect(outputPath, &QLineEdit::textChanged, this, [=](QString t){
		pathErr->setText("");
		outputExists = false;
		if(!t.endsWith("webm") && !t.endsWith("avi") && !t.endsWith("mkv") && !t.endsWith("mp4")){
			pathErr->setText("Unspported container format");
		}else if(QFile(outputPath->text()).exists()){
			pathErr->setText("Output file already exists");
			outputExists = true;
		}

	});

	outputPath->setText(QStandardPaths::standardLocations(QStandardPaths::MoviesLocation)[0] +
			"/" + audioFile->file()->completeBaseName() + ".mp4");

	create = new QPushButton("Start rendering");
	create->setCursor(QCursor(Qt::PointingHandCursor));

	connect(create, &QPushButton::released, this, &BL::setup_rendering_layout);

	form = new QFormLayout;

	form->addRow("", pathErr);
	form->addRow(setOutputPath, outputPath);
	form->addRow(create);

	centralLayout->addLayout(form, 0, 0);
}

void BL::setup_rendering_layout(){
	form->removeWidget(pathErr);
	form->removeWidget(setOutputPath);
	form->removeWidget(outputPath);
	form->removeWidget(create);

	QString outPath = outputPath->text();

	delete pathErr;
	delete setOutputPath;
	delete outputPath;
	delete create;
	delete form;

	this->setMinimumSize(640, 480);

	output = new QTextEdit;
	output->setReadOnly(true);

	progress = new QProgressBar;
	progress->setOrientation(Qt::Horizontal);
	progress->setMinimum(0);

	renderThread = new QThread(this);

	renderer = new BLCyclicRenderer(audioFile, bgFile, 50, outPath, selectedColor);

	renderer->moveToThread(renderThread);

	connect(renderThread, &QThread::started, renderer, &BLCyclicRenderer::render);
	connect(renderer, &BLCyclicRenderer::render_finished, renderThread, &QThread::quit);
	connect(renderThread, &QThread::finished, this, [=](){
		renderer->deleteLater();
		progress->setValue(progress->maximum());
	});
	connect(renderThread, &QThread::finished, renderThread, &QProcess::deleteLater);
	connect(renderer, &BLCyclicRenderer::import_progress, this, [=](int i){
		progress->setValue(i);
	});
	connect(renderer, &BLCyclicRenderer::export_output, this, [=](QByteArray data){
		outString.append(data);
		output->setText(outString);
		output->verticalScrollBar()->setValue(output->verticalScrollBar()->maximum());
	});
	connect(renderer, &BLCyclicRenderer::set_num_importers, this, [=](int n){
		progress->setMaximum(n - 1);
	});

	centralLayout->addWidget(output, 0, 0, 1, 2);
	centralLayout->addWidget(progress, 1, 0, 1, 2, Qt::AlignVCenter);

	renderThread->start();

}

void BL::start_balkinizing(){
    disconnect(theStart, &QPushButton::released, this, &BL::start_balkinizing);
    theStart->setCursor(QCursor(Qt::ArrowCursor));

    QString backgroundFile	= QFileDialog::getOpenFileName(
                                this,
                                "Select background source",
                                QStandardPaths::standardLocations(QStandardPaths::MoviesLocation)[0],
                                "Image/Video files (*.mp4 *.mkv *.avi *.webm *.mov *.png *.jpg *.jpeg *.bmp)",
                                nullptr,
                                QFileDialog::DontResolveSymlinks
                                );

    if(backgroundFile.isEmpty()) return;
    QFileInfo backgroundFileInfo(backgroundFile);

    QString audioFile		= QFileDialog::getOpenFileName(
                                this,
                                "Select audio source",
                                QStandardPaths::standardLocations(QStandardPaths::MusicLocation)[0],
								"Audio files (*.wav *.flac *.aiff *.aif *.mp3 *.mp4 *.ogg *.m4a *.3gp)",
                                nullptr,
                                QFileDialog::DontResolveSymlinks
                                );

    if(audioFile.isEmpty()) return;
    QFileInfo audioFileInfo(audioFile);

    bool video = backgroundFileInfo.suffix().at(0) == QChar('m') ||
                        backgroundFileInfo.suffix().at(0) == QChar('a') ||
                        backgroundFileInfo.suffix().at(0) == QChar('w');

    this->load_files(backgroundFileInfo, audioFileInfo, video);

    this->setup_controls();
}

//void BL::output_path_changed(){
	//pathErr.setText("");
	//QString path = outputPath.text().toLower();

	//while(form.rowCount() > 2) form.takeRow(2);
	//if(!videoCodec.isEnabled()) videoCodec.setEnabled(true);
	//if(!audioCodec.isEnabled()) audioCodec.setEnabled(true);

	/*while(videoCodec.count() > 0) videoCodec.removeItem(0);
	while(audioCodec.count() > 0) audioCodec.removeItem(0);

qInfo() << 2;
	if(path.endsWith("webm")){
		videoCodec.addItem("VP8",		"webm_vp8");
		videoCodec.addItem("VP9",		"webm_vp9");
		videoCodec.setCurrentIndex(1);

		audioCodec.addItem("Opus",		"webm_opus");
		audioCodec.addItem("Vorbis",	"webm_vorbis");
		audioCodec.setCurrentIndex(0);
	}else if(path.endsWith("avi") || path.endsWith("mkv")){
		videoCodec.addItem("H.264",		"matroska_h264");
		videoCodec.addItem("H.265",		"matroska_h265");
		videoCodec.addItem("VP8",		"matroska_vp8");
		videoCodec.addItem("VP9",		"matroska_vp9");
		videoCodec.setCurrentIndex(2);

		audioCodec.addItem("FLAC",		"matroska_flac");
		audioCodec.addItem("MP3",		"matroska_mp3");
		audioCodec.addItem("Opus",		"matroska_opus");
		audioCodec.addItem("Vorbis",	"matroska_vorbis");
		audioCodec.setCurrentIndex(1);
	}else if(path.endsWith("mp4")){
		videoCodec.addItem("H.264",		"mp4_h264");
		videoCodec.addItem("H.265",		"mp4_h265");
		videoCodec.setCurrentIndex(1);

		audioCodec.addItem("MP3",		"mp4_mp3");
		audioCodec.setCurrentIndex(0);
	}else{*/
	//if(!path.endsWith("webm") && !path.endsWith("avi") && !path.endsWith("mkv") && !path.endsWith("mp4")){
		//qInfo() << 3;
		//while(form.rowCount() > 2) form.takeRow(2);
		//videoCodec.setEnabled(false);
		//audioCodec.setEnabled(false);
		//pathErr.setText("Unspported container format");
	//}
//}

BL::~BL() {

}
