#include <QtWidgets/QApplication>
#include <QStringList>
#include <QFileDialog>
#include <QTextCodec>
#include <QMessageBox>
#include "FileProcess.h"
#include "Global/GlobalConfig.h"
#include "Global/minidump.h"

#pragma execution_character_set("utf-8") 

int main(int argc, char *argv[])
{
    InitMinDump();
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");//���2
    QTextCodec::setCodecForLocale(codec);

    CSpdLog::Instance()->SetLogFileName("log.txt");

    QApplication a(argc, argv);
    QStringList arguments = QCoreApplication::arguments();

    // ��ӡ���в���
    for (auto&& itor : arguments) {
        LOGGER->info("{} argument param:{}", __FUNCTION__, itor.toStdString());
    }

    if (argc < 2) {
        QMessageBox::warning(NULL, QObject::tr("����ʧ�ܣ�"),
            QObject::tr("��ָ���������ͣ��ֿ飺slice; �ϳɣ�merge"));
        return 0;
    }

    FileProcess* w = nullptr;
    if (arguments.at(1).compare("slice") == 0) {
        w = new FileProcess(true);
        w->show();
    } else {
        w = new FileProcess(false);
        w->show();
    }

    int ret = a.exec();
    delete w;
    return ret;
}
