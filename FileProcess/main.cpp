#include <QtWidgets/QApplication>
#include <QStringList>
#include <QFileDialog>
#include <QTextCodec>
#include "FileProcess.h"
#include "Global/GlobalConfig.h"

int main(int argc, char *argv[])
{
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");//���2
    QTextCodec::setCodecForLocale(codec);

    CSpdLog::Instance()->SetLogFileName("log.txt");

    QApplication a(argc, argv);
    QStringList arguments = QCoreApplication::arguments();

    // ��ӡ���в���
    for (auto&& itor : arguments) {
        LOGGER->info("{} argument param:{}", __FUNCTION__, itor.toStdString());
    }

    FileProcess w;
    w.show();
    return a.exec();
}
