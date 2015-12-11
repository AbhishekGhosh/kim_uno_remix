#ifndef MEMORYBROWSER_H
#define MEMORYBROWSER_H

#include <QDialog>
#include <QTimer>

namespace Ui {
class MemoryBrowser;
}

class MemoryBrowser : public QDialog
{
    Q_OBJECT

public:
    explicit MemoryBrowser(QWidget *parent = 0);
    ~MemoryBrowser();

private slots:
    void on_updateButton_clicked();

private:
    Ui::MemoryBrowser *ui;
    QTimer * timer;
    void stopOurUpdates();
    void startOurUpdates();

private slots:
    void on_timerTick();

    void on_autoUpdates_clicked();

public:
    void refreshDisplay();

};

#endif // MEMORYBROWSER_H
