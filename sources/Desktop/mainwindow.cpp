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
#include "../KimUnoRemix/config.h"


#define kAppName "KIM Uno Remix"
#define kVersionNumber "021"
#define kVersionDate   "2016-02-05"
#define kVersionText   "Freedom"

/*
 * v021 - 2016-02-05 - Freedom - Full-memory space RAM for desktop
 * v020 - 2016-02-04 - Silver Rainbow - Scroll for memory positioning buttons, bigger window
 * v019 - 2016-01-31 - Magnetic - LlamaCalc v8 added (RC2016/1 entry)
 * v018 - 2016-01-11 - Bowie - Speed 0 and 1000 added
 * v017 - 2016-01-08 - Button for $0000 Memory seek, updated key shortcuts to shifted versions
 * v016 - 2016-01-05 - $FE for random (write for seed, read for new value)
 * v015 - 2016-01-04 - Added speed setting to the menus (starts at 100 always)
 * v014 - 2015-12-29 - Better file error handling, 6052 opcode lookup
 * v013 - 2015-12-28 - Amiga palette, video is vastly sped up.
 * v012 - 2015-12-12 - Signal handler only on Mac and linux
 * v011 - 2015-12-11 - Memory Browser added with timer updating
 * v010 - 2015-12-10 - SIGUSR1/SIGUSR2 handlers for CodeDrop reload (killall -SIGUSR1 KIM)
 * v009 - 2015-12-10 - Auto Run/PC added, with key injection queue
 * v008 - 2015-12-09 - Code Drop added (reads in ca65 ouptut .lst files) (w/drag and drop)
 * v007 - 2015-11-14 - Video Display added at $4000
 * v006 - 2015-11-02 - Keypad interface restructured, startup code fixed
 * v005 - 2015-10-26 - Serial terminal almost working
 * v004 - 2015-10-24 - pulled out interface.c to help it be more portable
 * v003 - 2015-10-23 - Complete build with display, keys, etc
 * v002 - 2015-10-22 - quit, about handlers
 * v001 - 2015-10-22 - initial build, gui, nothing more
 */

extern "C" {


    /* ************************************************** */
    /* Random functions $FE */


    /* random support for 0xFE zero page */
    uint8_t KimRandom()
    {
        return qrand() % 255;
    }

    void KimRandomSeed( uint8_t s )
    {
        qsrand( s );
    }


}

VideoDisplay * theScreen = NULL;
int messageFlags;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // initialize the UI
    ui->setupUi(this);

    this->setAcceptDrops( true );

    // set the starting speed
    this->SetEmulationSpeed( 100 );

    // start the randomer
    qsrand( time( NULL ));

    // adjust the titlebar
    this->SetTitleBar();

    // start up the integrated terminal
    this->term = new TerminalInterface( parent );
    // and shove out some text to it because why not
    std::ostringstream title;
    title << kAppName << std::endl;
    title << " v" << kVersionNumber << std::endl;
    title << "  " << kVersionDate << std::endl;
    title << std::endl;

    this->term->putData( title.str().c_str() );

    // start up the Video display
    this->video = new VideoDisplay( parent );
    theScreen = this->video;
    // this->video->setHidden(false);

    // start up the Memory Browser
    this->membrowse = new MemoryBrowser( parent );

    // start up the Code Drop
    messageFlags = kMessage_None;
    this->cdrop = new CodeDrop( parent, this->membrowse );
    //this->cdrop->setHidden(true );

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
        this->video->close();
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



void MainWindow::SetTitleBar( )
{
    std::ostringstream title;
    title << kAppName;

    if( this->clocksPerTick == 0 ) {
        title << " [Paused]";
    } else {
        title << " v" << kVersionNumber;
        title << " (" << kVersionText << ")";
    }

    this->setWindowTitle( title.str().c_str() );
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
    static int tickCount = 0;

    // do some work
    if( this->clocksPerTick > 0 ) {
        // if we're paused, there's a lot we don't do

        exec6502( this->clocksPerTick ); //do 6502 instructions

        // another tick on the clock
        tickCount++;

        // check for reload for CodeDrop
        if( messageFlags == kMessage_Reload )
        {
            if( this->cdrop && this->cdrop->isVisible() )
            {
                this->cdrop->ReloadFromFile();
            }
            messageFlags = kMessage_None; // important to clear it
        }

        // but let's check the CodeDrop injection queue...
        // we need about 100 ticks to process a key injected, so we need to deal with this.
        // & with 0x03 compensates for this, and works well enough everywhere.
        if( ((tickCount & 0x03) == 0x03 ) && (this->cdrop->keysToInject.size() > 0) )
        {
            int keycode = this->cdrop->keysToInject.dequeue();
            KIMKeyPress( keycode );
        }
    }

    // these things (updating ourself, not the emulation core) we always do

    // handle serial
    // take the bytes from the code shove them to our terminal
    if( serOutBufPos > 0 ) {
        this->term->putData( QByteArray( (const char * )serOutBuf, serOutBufPos ));
        serOutBufPos = 0;
    }

    // and update the display
    this->updateDisplay();

    this->video->UpdateScreen();
}


extern "C" {
uint8_t getPixel( uint16_t address )
{
    return( theScreen->GetLinear( address ));
}

void setPixel( uint16_t address, uint8_t value )
{
    theScreen->SetLinear( address, value );
}

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

void MainWindow::on_action_Video_Display_triggered()
{
    if( this->video->isVisible() ){
        this->video->setVisible( false );
    } else {
        this->video->setVisible( true );
    }
}


void MainWindow::on_actionReset_triggered()
{
    this->initialize_emulation();

    for( int x=0 ; x<1024 * 64 ; x++ )
        write6502( x, 0 );
    loadProgramsToRam();

    this->cdrop->keysToInject.enqueue( kKimScancode_STOP );
    this->cdrop->keysToInject.enqueue( kKimScancode_ADDR );
    this->cdrop->keysToInject.enqueue( 0 );
    this->cdrop->keysToInject.enqueue( 0 );
    this->cdrop->keysToInject.enqueue( 0 );
    this->cdrop->keysToInject.enqueue( 0 );

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


void MainWindow::on_action6502_Opcodes_triggered()
{
    QUrl url( "http://www.6502.org/tutorials/6502opcodes.html" );
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

void MainWindow::on_actionCode_Drop_triggered()
{
    if( this->cdrop->isVisible() ){
        this->cdrop->setVisible( false );
    } else {
        this->cdrop->setVisible( true );
    }
}

void MainWindow::on_actionMemory_triggered()
{
    if( this->membrowse->isVisible() ){
        this->membrowse->setVisible( false );
    } else {
        this->membrowse->refreshDisplay();
        this->membrowse->setVisible( true );
    }
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

extern "C" {
void KIMKeyPress( uint8_t keycode );
}

void MainWindow::on_pushButton_SST_clicked()  {
    if( SSTmode == 0 ) KIMKeyPress( kKimScancode_SSTON );
    else               KIMKeyPress( kKimScancode_SSTOFF );
    this->updateSSTButton();
}

////////////////////////////////////////////
// control keys
void MainWindow::on_pushButton_GO_clicked()   { KIMKeyPress( kKimScancode_GO ); }
void MainWindow::on_pushButton_ST_clicked()   { KIMKeyPress( kKimScancode_STOP ); }
void MainWindow::on_pushButton_RS_clicked()   { KIMKeyPress( kKimScancode_RESET ); }
void MainWindow::on_pushButton_AD_clicked()   { KIMKeyPress( kKimScancode_ADDR ); }
void MainWindow::on_pushButton_DA_clicked()   { KIMKeyPress( kKimScancode_DATA ); }
void MainWindow::on_pushButton_PC_clicked()   { KIMKeyPress( kKimScancode_PC ); }
void MainWindow::on_pushButton_PLUS_clicked() { KIMKeyPress( kKimScancode_PLUS); }

////////////////////////////////////////////
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

////////////////////////////////////////////
// speed adjustments...
void MainWindow::SetEmulationSpeed( int value )
{
    this->clocksPerTick = value;

    this->ui->speed_0->setChecked( (value == 0)?true:false );
    this->ui->speed_25->setChecked( (value == 25)?true:false );
    this->ui->speed_47->setChecked( (value == 47)?true:false );
    this->ui->speed_50->setChecked( (value == 50)?true:false );
    this->ui->speed_100->setChecked( (value == 100)?true:false );
    this->ui->speed_200->setChecked( (value == 200)?true:false );
    this->ui->speed_500->setChecked( (value == 500)?true:false );
    this->ui->speed_1000->setChecked( (value == 1000)?true:false );
    this->SetTitleBar();
}

void MainWindow::on_speed_0_triggered()    { this->SetEmulationSpeed( 0 ); }
void MainWindow::on_speed_25_triggered()   { this->SetEmulationSpeed( 25 ); }
void MainWindow::on_speed_47_triggered()   { this->SetEmulationSpeed( 47 ); }
void MainWindow::on_speed_50_triggered()   { this->SetEmulationSpeed( 50 ); }
void MainWindow::on_speed_100_triggered()  { this->SetEmulationSpeed( 100 ); }
void MainWindow::on_speed_200_triggered()  { this->SetEmulationSpeed( 200 ); }
void MainWindow::on_speed_500_triggered()  { this->SetEmulationSpeed( 500 ); }
void MainWindow::on_speed_1000_triggered() { this->SetEmulationSpeed( 1000 ); }
