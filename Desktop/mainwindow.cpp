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
#define kVersionNumber "005"
#define kVersionDate   "2015-10-26"

/*
 * v005 - 2015-10-26 - Serial terminal almost working
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

    // start up the integrated terminal
    this->term = new TerminalInterface( parent );
    // and shove out some text to it because why not
    title.str( "" );
    title << kAppName << std::endl;
    title << " v" << kVersionNumber << std::endl;
    title << "  " << kVersionDate << std::endl;
    title << std::endl;

    this->term->putData( title.str().c_str() );

    // set up the update timer
    this->timer = new QTimer( this );
    connect( this->timer, SIGNAL(timeout()), this, SLOT( on_timerTick() ));
    timer->start( 10 );

    // initialize the display
    this->updateDisplay();

    // initialize the SST button and indicator
    this->updateSSTButton();

    // and start up the emulator!
    this->initialize_emulation();
}

// GOZER THE DESTRUCTOR!
MainWindow::~MainWindow()
{
    this->timer->stop();
    delete( this->timer );

    delete ui;
}

// confirm with the user that we really want to exit
bool MainWindow::reallyQuit()
{
    QMessageBox confirmQuitBox;
    return true;//HACK 2015

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

// capture the window's close event [x]
void MainWindow::closeEvent( QCloseEvent *bar )
{
    if( this->reallyQuit() ) {
        bar->accept();
        this->term->close();
    } else {
        bar->ignore();
    }
}

// user wants to know more about us. aw shucks!
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

extern "C" {
uint16_t getpc();
uint8_t read6502(uint16_t address);
}

void MainWindow::on_timerTick()
{
    exec6502(100); //do 100 6502 instructions

    // keypad scanning is happening elsewhere!

    // handle serial
    // take the bytes from the code shove them to our terminal
    if( serOutBufPos > 0 ) {
        this->term->putData( QByteArray( (const char * )serOutBuf, serOutBufPos ));
        serOutBufPos = 0;
    }

    // and update the display
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

// make the console visible
void MainWindow::on_actionSerial_Console_triggered()
{
    if( this->term->isVisible() ){
        this->term->setVisible( false );
    } else {
        this->term->setVisible( true );
    }
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

void MainWindow::on_actionFloodgap_Books_triggered()
{
    QUrl url( "http://www.floodgap.com/retrobits/kim-1/darc.html" );
    (void) QDesktopServices::openUrl( url );
}

void MainWindow::on_action6502_Resources_triggered()
{
    QUrl url( "http://6502.org/documents/books/" );
    (void) QDesktopServices::openUrl( url );
}

void MainWindow::on_actionKim_Uno_Remix_Github_triggered()
{
    QUrl url( "https://github.com/bleullama/kim_uno_remix" );
    (void) QDesktopServices::openUrl( url );
}

void MainWindow::on_actionKim_UNO_Project_triggered()
{
    QUrl url( "http://obsolescence.wix.com/obsolescence#!kim-uno-summary/c1uuh" );
    (void) QDesktopServices::openUrl( url );
}


///////////////////////////////////////////////////////////////////
// keypad

// SST (Single Step) needs some additional support to indicate the switch
void MainWindow::updateSSTButton()
{
    if( SSTmode == 0 ) {
        ui->pushButton_SST->setText( "SS&T\nis Off" );
    } else {
        ui->pushButton_SST->setText( "SS&T\nis On" );
    }

    // for some reason, the above setText breaks the shortcut in the .ui
    ui->pushButton_SST->setShortcut( QKeySequence::fromString( "Shift+T" ) );
}

void MainWindow::on_pushButton_SST_clicked()  {
    if( SSTmode == 0 ) KIMKeyPress( kKimScancode_SSTON );
    else               KIMKeyPress( kKimScancode_SSTOFF );
    this->updateSSTButton();
}

// control keys
void MainWindow::on_pushButton_GO_clicked()   { KIMKeyPress( kKimScancode_GO ); }
void MainWindow::on_pushButton_ST_clicked()   { KIMKeyPress( kKimScancode_STOP ); }
void MainWindow::on_pushButton_RS_clicked()   { KIMKeyPress( kKimScancode_RESET ); }
void MainWindow::on_pushButton_AD_clicked()   { KIMKeyPress( kKimScancode_ADDR ); }
void MainWindow::on_pushButton_DA_clicked()   { KIMKeyPress( kKimScancode_DATA ); }
void MainWindow::on_pushButton_PC_clicked()   { KIMKeyPress( kKimScancode_PC ); }
void MainWindow::on_pushButton_PLUS_clicked() { KIMKeyPress( kKimScancode_PLUS); }


// keypad digits
void MainWindow::on_pushButton_HEX_0_clicked() { KIMKeyPress( kKimScancode_0); }
void MainWindow::on_pushButton_HEX_1_clicked() { KIMKeyPress( kKimScancode_1); }
void MainWindow::on_pushButton_HEX_2_clicked() { KIMKeyPress( kKimScancode_2); }
void MainWindow::on_pushButton_HEX_3_clicked() { KIMKeyPress( kKimScancode_3); }
void MainWindow::on_pushButton_HEX_4_clicked() { KIMKeyPress( kKimScancode_4); }
void MainWindow::on_pushButton_HEX_5_clicked() { KIMKeyPress( kKimScancode_5); }
void MainWindow::on_pushButton_HEX_6_clicked() { KIMKeyPress( kKimScancode_6); }
void MainWindow::on_pushButton_HEX_7_clicked() { KIMKeyPress( kKimScancode_7); }
void MainWindow::on_pushButton_HEX_8_clicked() { KIMKeyPress( kKimScancode_8); }
void MainWindow::on_pushButton_HEX_9_clicked() { KIMKeyPress( kKimScancode_9); }
void MainWindow::on_pushButton_HEX_A_clicked() { KIMKeyPress( kKimScancode_A); }
void MainWindow::on_pushButton_HEX_B_clicked() { KIMKeyPress( kKimScancode_B); }
void MainWindow::on_pushButton_HEX_C_clicked() { KIMKeyPress( kKimScancode_C); }
void MainWindow::on_pushButton_HEX_D_clicked() { KIMKeyPress( kKimScancode_D); }
void MainWindow::on_pushButton_HEX_E_clicked() { KIMKeyPress( kKimScancode_E); }
void MainWindow::on_pushButton_HEX_F_clicked() { KIMKeyPress( kKimScancode_F); }
