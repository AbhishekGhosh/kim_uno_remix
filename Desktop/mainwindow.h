#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QTimer * timer;

private:
    void updateDisplay();

private:
    bool reallyQuit();
    void closeEvent( QCloseEvent *bar );

private:
    void initialize_emulation();

private slots:
    // timer
    void on_timerTick();

private slots:
    // menus
    void on_actionSerial_Monitor_triggered();
    void on_actionReset_triggered();
    void on_actionQuit_triggered();

private:
    void button_pressed( int b );
    void updateSSTButton( void );

private slots:
    // keypad
    void on_pushButton_GO_clicked();
    void on_pushButton_ST_clicked();
    void on_pushButton_RS_clicked();
    void on_pushButton_SST_clicked();

    void on_pushButton_AD_clicked();
    void on_pushButton_DA_clicked();
    void on_pushButton_PC_clicked();
    void on_pushButton_PLUS_clicked();

    void on_pushButton_HEX_0_clicked();
    void on_pushButton_HEX_1_clicked();
    void on_pushButton_HEX_2_clicked();
    void on_pushButton_HEX_3_clicked();
    void on_pushButton_HEX_4_clicked();
    void on_pushButton_HEX_5_clicked();
    void on_pushButton_HEX_6_clicked();
    void on_pushButton_HEX_7_clicked();
    void on_pushButton_HEX_8_clicked();
    void on_pushButton_HEX_9_clicked();
    void on_pushButton_HEX_A_clicked();
    void on_pushButton_HEX_B_clicked();
    void on_pushButton_HEX_C_clicked();
    void on_pushButton_HEX_D_clicked();
    void on_pushButton_HEX_E_clicked();
    void on_pushButton_HEX_F_clicked();
    void on_actionAbout_triggered();
};

#endif // MAINWINDOW_H
