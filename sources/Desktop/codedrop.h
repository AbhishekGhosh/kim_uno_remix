#ifndef CODEDROP_H
#define CODEDROP_H

#include <QDialog>

namespace Ui {
class CodeDrop;
}

class CodeDrop : public QDialog
{
    Q_OBJECT

public:
    explicit CodeDrop(QWidget *parent = 0);
    ~CodeDrop();

private slots:
    void on_BrowseFiles_clicked();
    void on_LoadToRAM_clicked();

private:
    Ui::CodeDrop *ui;

    void LoadSettings();
    void SaveSettings();
    QString lastFile;
};

#endif // CODEDROP_H
