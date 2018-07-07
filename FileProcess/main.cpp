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
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");//情况2
    QTextCodec::setCodecForLocale(codec);

    CSpdLog::Instance()->SetLogFileName("log.txt");

    QApplication a(argc, argv);
    QStringList arguments = QCoreApplication::arguments();

    // 打印所有参数
    for (auto&& itor : arguments) {
        LOGGER->info("{} argument count:{} param:{}", 
            __FUNCTION__, arguments.count(), itor.toStdString());
    }
    LOGGER->flush();

    if (argc < 2) {
        QMessageBox::warning(NULL, QObject::tr("启动失败："),
            QObject::tr("需指定启动类型，分块：slice 分块数量; 合成：merge"));
        return 0;
    }
    if (arguments.at(1).compare(QObject::tr("test")) == 0) {
        QMessageBox::information(NULL, QObject::tr("测试"), QObject::tr("客户端已正确安装！"));
        return 0;
    }
    
    FileProcess* w = nullptr;
    if (arguments.at(1).compare("slice") == 0) {
        if (arguments.size() <= 2) {
            QMessageBox::warning(NULL, QObject::tr("启动失败："),
                QObject::tr("启动参数错误，分块：slice 分块数量; 合成：merge"));
            return -0;
        }
        w = new FileProcess(true);
        if (w->SliceProcessDirectory((char *)GLOBALCONFIG->GetAES128Key(), arguments.at(2).toInt(), false) < 0) {
            delete w;
            return -1;
        }
    } else if(arguments.at(1).compare("merge") == 0) {
        w = new FileProcess(false);
        if (w->MergeProcessDirectory((char *)GLOBALCONFIG->GetAES128Key(), true) < 0) {
            delete w;
            return -1;
        }
    } else {
        QMessageBox::warning(NULL, QObject::tr("启动失败："),
            QObject::tr("启动参数错误，分块：slice 分块数量; 合成：merge"));
        return 0;
    }
    w->show();
    w->activateWindow();

    int ret = a.exec();
    delete w;
    return ret;
}
