#pragma once

#include <QtWidgets/QMainWindow>
#include <QTimer>
#include "ui_FileProcess.h"
#include "ui_ProgressWidget.h"
#include "Global/GlobalConfig.h"

class IBaseProcess;

class FileProcess : public QMainWindow
{
    Q_OBJECT

public:
    FileProcess(QWidget *parent = Q_NULLPTR);
    ~FileProcess();

    // �����ļ��н��зֿ顢���ܴ���������
    // @param aes_key_ ������Կ
    // @param slice_count_ �ֿ�������˽�п鲻�ڼ��㷶Χ��
    // @param use_src_dir_ �Ƿ񽫷ֿ����ļ��洢��Դ�ļ�Ŀ¼��
    int         SliceProcessDirectory(char* aes_key_, const int& slice_count_,
        bool use_src_dir_);

    // �ϲ��ļ���������
    // ѡ��һ�����ݿ�����Ŀ¼����ѡ��һ��Ŀ���ļ�����Ŀ¼
    // @param aes_key_ ������Կ
    // @param use_src_dir_ �Ƿ񽫷ֿ����ļ��洢��Դ�ļ�Ŀ¼��
    int         MergeProcessDirectory(char* aes_key_, bool use_src_dir_);

protected:
    // ѡ��һ���ļ�
    QString     SelectFile(const QString& dlg_title_);

    // ѡ�����ļ�
    QStringList SelectFiles(const QString& dlg_title_);

    // ѡ��Ի���
    QString     SelectDir(const QString& dlg_title_);

    // ����ָ���ļ����������ļ�
    int         QueryDirectory(const string& path_, list<string>& dest_file_vec_);

signals:

public slots:
    void        UpdateProgressSlot();

private:
    Ui::FileProcessClass ui;
    //Ui::Process_Form*        _progress_ui;
    shared_ptr<IBaseProcess> _base_process_ptr;

    std::thread  _slice_thread;
    std::thread  _merge_thread;
    QTimer       _timer;
};
