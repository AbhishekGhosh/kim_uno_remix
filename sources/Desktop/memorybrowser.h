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
    void seekDisplay( long address );

private slots:
    void on_timerTick();

    void on_autoUpdates_clicked();

    void on_go0000_clicked();
    void on_go0200_clicked();
    void on_go4000_clicked();
    void on_go8000_clicked();

public:
    void refreshDisplay();

};

#endif // MEMORYBROWSER_H
