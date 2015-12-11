#include <QScrollBar>
#include <QDebug>
#include "memorybrowser.h"
#include "ui_memorybrowser.h"

extern "C" {
#include "interface.h"
}


MemoryBrowser::MemoryBrowser(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MemoryBrowser)
{
    ui->setupUi(this);
    this->refreshDisplay();
    this->ui->autoUpdates->setChecked( false );
    this->timer = NULL;
}

void MemoryBrowser::stopOurUpdates()
{
    if( this->timer ) {
        this->timer->stop();
        delete( this->timer );
        this->timer = NULL;
    }
}

void MemoryBrowser::startOurUpdates()
{
    this->stopOurUpdates();

    // set up the update timer
    this->timer = new QTimer( this );
    connect( this->timer, SIGNAL(timeout()), this, SLOT( on_timerTick() ));
    timer->start( 200 );
}

MemoryBrowser::~MemoryBrowser()
{
    this->stopOurUpdates();
    delete ui;
}

void MemoryBrowser::refreshDisplay()
{
    char buf[80];

    QString str("");

    char * data = flattenRam();

    for( int addr = 0 ; addr <= 0xFFF0 ; addr += 16 )
    {
        sprintf( buf, "%04x  %02x %02x %02x %02x %02x %02x %02x %02x   %02x %02x %02x %02x %02x %02x %02x %02x   "
                      "\n", addr,
                 data[addr+0]&0x0ff,  data[addr+1]&0x0ff,  data[addr+2]&0x0ff,  data[addr+3]&0x0ff,
                 data[addr+4]&0x0ff,  data[addr+5]&0x0ff,  data[addr+6]&0x0ff,  data[addr+7]&0x0ff,
                 data[addr+8]&0x0ff,  data[addr+9]&0x0ff,  data[addr+10]&0x0ff, data[addr+11]&0x0ff,
                 data[addr+12]&0x0ff, data[addr+13]&0x0ff, data[addr+14]&0x0ff, data[addr+15]&0x0ff
                 );
        str.append( buf );
    }

    // save the current scroll position
    const int savedPos = this->ui->hexCodeDisplay->verticalScrollBar()->value();

    // set the text
    this->ui->hexCodeDisplay->document()->setPlainText( str );

    // restore the scroll position
    this->ui->hexCodeDisplay->verticalScrollBar()->setValue( savedPos );

    /*
            plain_text_edit->document()->setPlainText(text);
            Alternative way, first clear the editor, then append new text:

            plain_text_edit->clear(); // unless you know the editor is empty
            plain_text_edit->appendPlainText(text);
    */
}


void MemoryBrowser::on_timerTick()
{
    this->refreshDisplay();
}


void MemoryBrowser::on_updateButton_clicked()
{
    this->refreshDisplay();
}

void MemoryBrowser::on_autoUpdates_clicked()
{
    if( this->ui->autoUpdates->isChecked() )
    {
        this->startOurUpdates();
    } else {
        this->stopOurUpdates();
    }
}
