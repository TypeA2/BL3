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

#include <QApplication>
#include <QProcess>
#include <QMessageBox>

int main(int argc, char *argv[]){
	QApplication a(argc, argv);

    QProcess *ffprobe_test	= new QProcess;
    QProcess *ffmpeg_test	= new QProcess;
#if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QStringList envlist = env.toStringList();

    for(int i=0; i < envlist.size(); i++)
    {
        QString entry = envlist[i];

        if(entry.startsWith("PATH="))
        {
            int index = entry.indexOf("=");

            if(index != -1)
            {
                QString value = entry.right(entry.length() - (index+1));
                value += ":/usr/texbin:/usr/local/bin";

                setenv("PATH", value.toLatin1().constData(), true);
            }

            break;
        }
    }

    ffprobe_test->start("which ffprobe");
    ffmpeg_test->start("which ffmpeg");
#elif defined(Q_OS_WIN32)
    ffprobe_test->start("where ffprobe");
    ffmpeg_test->start("where ffmpeg");
#endif

    ffprobe_test->waitForFinished();
    ffmpeg_test->waitForFinished();

    bool ffprobe_exists	= !ffprobe_test->exitCode();
    bool ffmpeg_exists	= !ffmpeg_test->exitCode();

    if(!ffprobe_exists || !ffmpeg_exists){
        QMessageBox err;
        if(!ffprobe_exists && ffmpeg_exists){
            err.setText("ffprobe was not deteced in your PATH.\nPlease make sure it is installed and added to your PATH.");
        }else if(ffprobe_exists && !ffmpeg_exists){
            err.setText("ffmpeg was not deteced in your PATH.\nPlease make sure it is installed and added to your PATH.");
        }else{
            err.setText("ffprobe and ffmpeg were not deteced in your PATH.\nPlease make sure they is installed and added to your PATH.");
        }
        err.setIcon(QMessageBox::Critical);
        err.addButton("Close", QMessageBox::AcceptRole);
        err.exec();
        return 1;
    }
    delete ffprobe_test;
    delete ffmpeg_test;

	BL bl;

	bl.show();

	return a.exec();
}
