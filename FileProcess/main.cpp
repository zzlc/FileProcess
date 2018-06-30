#include <QtWidgets/QApplication>
#include <QStringList>
#include <QFileDialog>
#include "FileProcess.h"
#include "Global/GlobalConfig.h"

int main(int argc, char *argv[])
{
    CSpdLog::Instance()->SetLogFileName("log.txt");

    QApplication a(argc, argv);
    QStringList arguments = QCoreApplication::arguments();

    // 打印所有参数
    for (auto&& itor : arguments) {
        LOGGER->info("{} argument param:{}", __FUNCTION__, itor.toStdString());
    }

    FileProcess w;
    w.show();
    return a.exec();
}
