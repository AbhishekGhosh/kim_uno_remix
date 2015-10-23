#include <iostream>
#include <sstream> /* for ostringstream */

#include <QtCore>
#include <QtGui>
#include <QMessageBox>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "../sources/KimUnoRemix/config.h"


#define kAppName "KIM Uno Remix"
#define kVersionNumber "003"
#define kVersionDate   "2015-10-22"

/*
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

/********************/
/* EEPROM FUNCTIONS */
extern "C" {

    void reset6502();
    void nmi6502();
    void loadProgramsToRam();
    void exec6502( int nCycles );

  /* serial fakeo */
  void serout( uint8_t ch )
  {
      ch = ch;
      //printf( "%c", ch );
  }
  void serouthex( uint8_t ch )
  {
      ch = ch;
      //printf( "%01x", ch );
  }

  uint8_t curkey = 0;

  extern uint8_t serialEnable;
  extern uint8_t SSTmode;

  uint8_t getAkey()   { return(curkey);  }
  void clearkey()     { curkey = 0; }

  uint8_t getKIMkey() {
    if (curkey==0)
      return (0xFF);	//0xFF: illegal keycode

    // numeric hex
    if ((curkey>='0') & (curkey <='9'))
      return (curkey-'0');
    if ((curkey>='A') & (curkey <='F'))
      return(curkey-'A'+10);
    if ((curkey>='a') & (curkey <='f'))
      return(curkey-'a'+10);

    // control presses
    if (curkey==1) // ctrlA
      return(0x10); // AD address mode
    if (curkey==4) // ctrlD
      return(0x11); // DA data mode
    if (curkey=='+') // +
      return(0x12); // step
    if (curkey==7) // ctrlG
      return(0x13); // GO
    if (curkey==16) // ctrlP
      return(0x14); // PC mode
    // curkey==ctrlR for hard reset (/RST) (KIM's RS key) is handled in main loop!
    // curkey==ctrlT for ST key (/NMI) is handled in main loop!

    // additional control press shortcuts
    if( curkey == 'g' ) return( 0x07 ); /* GO */
    if( curkey == 'l' ) return( 0x10 ); /* AD - address Location */
    if( curkey == 'v' ) return( 0x11 ); /* DA - data Value */
    if( curkey == 'p' ) return( 0x14 ); /* PC */

    return(curkey); // any other key, should not be hit but ignored by KIM
  }

  // translate keyboard commands to KIM-I keycodes or emulator actions
  void interpretkeys()
  {
    // round 1: keys that always have the same meaning
    switch (curkey) {
      case 18:  // CtrlR = RS key = hardware reset (RST)
        reset6502();
        clearkey();
        break;

      case 20: // CtrlT = ST key = throw an NMI to stop execution of user program
        nmi6502();
        clearkey();
        break;

      case '[': // SST off
        SSTmode = 0;
        clearkey();
        break;

      case ']': // SST on
        SSTmode = 1;
        clearkey();
        break;

      case 9: // TAB pressed, toggle between serial port and onboard keyboard/display
        /*
        if (useKeyboardLed==0)
        {
          useKeyboardLed=1;    Serial.print(F("                    Keyboard/Hex Digits Mode "));
        } else {
          useKeyboardLed=0;    Serial.print(F("                        Serial Terminal Mode         "));
        }
        */
        reset6502();  clearkey();  break;

    }
  }



  uint8_t xkeyPressed()    // just see if there's any keypress waiting
  {
    return (curkey==0?0:1);
  }


  /* display fakeo */
  char kimHex[6];        // seLED display

  void driveLEDs( void )
  {
      /* trigger the LED update */

  }
} /* C */


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
