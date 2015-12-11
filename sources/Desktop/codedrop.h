#ifndef CODEDROP_H
#define CODEDROP_H

#include <QDialog>
#include <QQueue>

namespace Ui {
class CodeDrop;
}

class CodeDrop : public QDialog
{
    Q_OBJECT

public:
    explicit CodeDrop(QWidget *parent = 0);
    ~CodeDrop();

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

private slots:
    void on_BrowseFiles_clicked();
    void on_LoadToRAM_clicked();

    void on_AutoPC_clicked();
    void on_AutoRun_clicked();

public:
    QQueue<int> keysToInject;

private:
    int loadedBytes;
    int loadedAddr;
    int GetLineAddress( const char * line );
    int GetDataByte( const char * line, int which );
    int ParseCC65LstFile( QString path );
public:
    void ReloadFromFile();

private:
    Ui::CodeDrop *ui;

private:
    void LoadSettings();
    void SaveSettings();
    QString lastFile;
};

#endif // CODEDROP_H
