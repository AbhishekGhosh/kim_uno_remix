#include <iostream>
#include <sstream> /* for ostringstream */

#include <QtCore>
#include <QtGui>
#include <QMessageBox>

#include "mainwindow.h"
extern "C" {
#include "interface.h"
}
#include "ui_mainwindow.h"
#include "../sources/KimUnoRemix/config.h"


#define kAppName "KIM Uno Remix"
#define kVersionNumber "004"
#define kVersionDate   "2015-10-22"

/*
 * v004 - 2015-10-24 - pulled out interface.c to help it be more portable
 * v003 - 2015-10-23 - Complete build with display, keys, etc
 * v002 - 2015-10-22 - quit, about handlers
 * v001 - 2015-10-22 - initial build, gui, nothing more
 */


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // initialize the UI
    ui->setupUi(this);

    // adjust the titlebar
    std::ostringstream title;
    title << kAppName;
    title << " v" << kVersionNumber;
    this->setWindowTitle( title.str().c_str() );

    // set up the timer
    this->timer = new QTimer( this );
    connect( this->timer, SIGNAL(timeout()), this, SLOT( on_timerTick() ));
    timer->start( 10 );

    // initialize the display
    this->updateDisplay();

    // initialize SST
    this->updateSSTButton();
    this->initialize_emulation();

    // other connections...
    //connect( ui->pushButton_GO, SIGNAL(pressed()),
    //         this, SLOT(on_Reset_option()) );
}

MainWindow::~MainWindow()
{
    this->timer->stop();
    delete( this->timer );

    delete ui;
}



bool MainWindow::reallyQuit()
{
    QMessageBox confirmQuitBox;

    confirmQuitBox.setWindowTitle( kAppName );
    confirmQuitBox.setText( "Are you sure you want to quit?" );
    confirmQuitBox.setStandardButtons( QMessageBox::Yes );
    confirmQuitBox.addButton( QMessageBox::No );
    confirmQuitBox.setDefaultButton( QMessageBox::No );
    confirmQuitBox.setIcon( QMessageBox::Warning ); // Question Warning Critical
    if( confirmQuitBox.exec() == (QMessageBox::Yes) ){
        return( true );
    } else {
        return( false );
    }
}

void MainWindow::closeEvent( QCloseEvent *bar )
{
    if( this->reallyQuit() ) {
        bar->accept();
    } else {
        bar->ignore();
    }
}



void MainWindow::on_actionAbout_triggered()
{
    std::ostringstream content;

    content << "Kim Uno Remix - Desktop Version" << std::endl;
    content << std::endl;
    content << " Version " << kVersionNumber << ", " << kVersionDate << std::endl;
    content << " using engine " << kVersionString << std::endl;
    content << std::endl;
    content << "Project at https://github.com/BleuLlama/kim_uno_remix" << std::endl;
    content << std::endl;
    content << "by Scott Lawrence, yorgle@gmail.com" << std::endl;
    content << "- Original Kim UNO engine by Oscar Vermeulen" << std::endl;
    content << "- 6502 emulator by Mike Chambers" << std::endl;

    QString txt( content.str().c_str() );

    QMessageBox::information( this, kAppName, txt, QMessageBox::Ok );
}


///////////////////////////////////////////////////////////////////////////
// display & timer

void MainWindow::updateDisplay()
{
    ui->lcdNumber_AD_F000->display( (int)kimHex[0] );
    ui->lcdNumber_AD_0F00->display( (int)kimHex[1] );
    ui->lcdNumber_AD_00F0->display( (int)kimHex[2] );
    ui->lcdNumber_AD_000F->display( (int)kimHex[3] );

    ui->lcdNumber_DA_F0->display( (int) kimHex[4] );
    ui->lcdNumber_DA_0F->display( (int) kimHex[5] );
}

void MainWindow::on_timerTick()
{
    exec6502(100); //do 100 6502 instructions
    // handle sst, NMI, etc...
    if( xkeyPressed() != 0 ) {
        interpretkeys();
    }

    this->updateDisplay();
}



void MainWindow::initialize_emulation()
{
    /* clear the display buffer */
    for( int i=0 ; i<6 ; i++) kimHex[i] = '0';

    /* reset the CPU */
    reset6502();

    /* set up vectors and load programs to RAM */
    loadProgramsToRam();

}


///////////////////////////////////////////////////////////////////
// menu
void MainWindow::on_actionSerial_Monitor_triggered()
{
}

void MainWindow::on_actionReset_triggered()
{
    reset6502();
}

void MainWindow::on_actionQuit_triggered()
{
    if( this->reallyQuit() ) {
        qApp->quit();
    }
}

///////////////////////////////////////////////////////////////////
// keypad

void MainWindow::button_pressed( int btn )
{
    curkey = btn;
    interpretkeys(); // catch NMI etc.
}

void MainWindow::updateSSTButton()
{
    if( SSTmode == 0 ) {
        ui->pushButton_SST->setText( "SST\nis Off" );
    } else {
        ui->pushButton_SST->setText( "SST\nis On" );
    }

    // for some reason, the above setText breaks the shortcut in the .ui
    ui->pushButton_SST->setShortcut( QKeySequence::fromString( "Shift+T" ) );
}

void MainWindow::on_pushButton_GO_clicked()   { this->button_pressed( 7 ); }
void MainWindow::on_pushButton_ST_clicked()   { this->button_pressed( 20 ); }
void MainWindow::on_pushButton_RS_clicked()   { this->button_pressed( 18 ); }
void MainWindow::on_pushButton_AD_clicked()   { this->button_pressed( 1 ); }
void MainWindow::on_pushButton_DA_clicked()   { this->button_pressed( 4 ); }
void MainWindow::on_pushButton_PC_clicked()   { this->button_pressed( 16 ); }
void MainWindow::on_pushButton_PLUS_clicked() { this->button_pressed( '+' ); }
void MainWindow::on_pushButton_SST_clicked()  {
    this->button_pressed( (SSTmode==0)?']':'[' );
    this->updateSSTButton();
}

/////////////////////////////////////////////////////
// keypad digits

void MainWindow::on_pushButton_HEX_0_clicked() { this->button_pressed( '0' ); }
void MainWindow::on_pushButton_HEX_1_clicked() { this->button_pressed( '1' ); }
void MainWindow::on_pushButton_HEX_2_clicked() { this->button_pressed( '2' ); }
void MainWindow::on_pushButton_HEX_3_clicked() { this->button_pressed( '3' ); }
void MainWindow::on_pushButton_HEX_4_clicked() { this->button_pressed( '4' ); }
void MainWindow::on_pushButton_HEX_5_clicked() { this->button_pressed( '5' ); }
void MainWindow::on_pushButton_HEX_6_clicked() { this->button_pressed( '6' ); }
void MainWindow::on_pushButton_HEX_7_clicked() { this->button_pressed( '7' ); }
void MainWindow::on_pushButton_HEX_8_clicked() { this->button_pressed( '8' ); }
void MainWindow::on_pushButton_HEX_9_clicked() { this->button_pressed( '9' ); }
void MainWindow::on_pushButton_HEX_A_clicked() { this->button_pressed( 'A' ); }
void MainWindow::on_pushButton_HEX_B_clicked() { this->button_pressed( 'B' ); }
void MainWindow::on_pushButton_HEX_C_clicked() { this->button_pressed( 'C' ); }
void MainWindow::on_pushButton_HEX_D_clicked() { this->button_pressed( 'D' ); }
void MainWindow::on_pushButton_HEX_E_clicked() { this->button_pressed( 'E' ); }
void MainWindow::on_pushButton_HEX_F_clicked() { this->button_pressed( 'F' ); }
