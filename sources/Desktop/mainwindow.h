#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <terminalinterface.h>
#include "videodisplay.h"
#include "codedrop.h"
#include "memorybrowser.h"

#define kMessage_None    (0)
#define kMessage_Reload  (1)
extern int messageFlags;

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
    TerminalInterface * term;
    VideoDisplay * video;
    CodeDrop * cdrop;
    MemoryBrowser * membrowse;
    int clocksPerTick;

private:
    void SetTitleBar();
    void updateDisplay();
    void SetEmulationSpeed( int value );

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
    void on_actionReset_triggered();
    void on_actionQuit_triggered();
    void on_actionAbout_triggered();

    void on_actionSerial_Console_triggered();
    void on_actionFloodgap_Books_triggered();
    void on_action6502_Resources_triggered();
    void on_actionKim_Uno_Remix_Github_triggered();
    void on_actionKim_UNO_Project_triggered();

    void on_action_Video_Display_triggered();
    void on_actionCode_Drop_triggered();

private:
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
    void on_actionMemory_triggered();
    void on_action6502_Opcodes_triggered();
    void on_speed_25_triggered();
    void on_speed_50_triggered();
    void on_speed_100_triggered();
    void on_speed_200_triggered();
    void on_speed_500_triggered();
    void on_speed_47_triggered();
    void on_speed_0_triggered();
    void on_speed_1000_triggered();
};

#endif // MAINWINDOW_H
