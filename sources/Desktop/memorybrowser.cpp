#include <QScrollBar>
#include <QTextDocument>
#include <QTextBlock>
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

    // start at the top
    this->ui->hexCodeDisplay->verticalScrollBar()->setSliderPosition( 0 );

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

    this->ui->hexCodeDisplay->verticalScrollBar()->setValue( 100 );
    this->ui->hexCodeDisplay->moveCursor(QTextCursor::Start);
    this->ui->hexCodeDisplay->ensureCursorVisible();

}

MemoryBrowser::~MemoryBrowser()
{
    this->stopOurUpdates();
    delete ui;
}

void MemoryBrowser::refreshDisplay()
{
    char buf[80];
    char bufhex[80];
    char bufasc[80];
    char bufx[80];
    QString str("");

    char * data = flattenRam();

    for( int addr = 0 ; addr <= 0xFFF0 ; addr += 16 )
    {
        bufhex[0] = '\0';
        bufasc[0] = '\0';

        if( (addr & 0xFF) == 0 )
        {
            if( addr ) str.append( "\n" );
        }

        /* special headings */
        switch( addr ) {
        case( 0x0000 ): str.append( "Zero Page:\n" ); break;
        case( 0x0200 ): str.append( "User Program:\n" ); break;
        case( 0x0400 ): str.append( "EEPROM: (on AVR)\n" ); break;

        case( 0x2000 ): str.append( "Disasm (ROM):\n" ); break;
        case( 0xc000 ): str.append( "Chess (ROM):\n" ); break;

        case( 0x4000 ): str.append( "Video Buffer 0:\n" ); break;

        case( 0x1780 ): str.append( "\nRAM 0003:\n" ); break;
        case( 0x17C0 ): str.append( "\nRAM 0002:\n" ); break;
        case( 0x1800 ): str.append( "ROM 003:\n" ); break;
        case( 0x1C00 ): str.append( "ROM 002:\n" ); break;
        }

        for( int offset=0 ; offset<16 ; offset++ ) {
            unsigned char ch = data[addr+offset];
            sprintf( bufx, " %02x", ch );
            strcat( bufhex, bufx );

            if( ch >= ' ' && ch <= '?' ) {
                sprintf( bufx, "%c", ch );
            } else {
                sprintf( bufx, "." );
            }
            strcat( bufasc, bufx );

            if( offset == 7 ){
                strcat( bufhex, " " );
                strcat( bufasc, " " );
            }
        }

        sprintf( buf, "%04x  %s  %s\n", addr, bufhex, bufasc );
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

void MemoryBrowser::seekDisplay( long line_number )
{
    // first, move the cursor to the selected line
    QTextCursor text_cursor( this->ui->hexCodeDisplay->document()->findBlockByLineNumber(line_number - 1) );
    text_cursor.select( QTextCursor::LineUnderCursor );
    this->ui->hexCodeDisplay->setTextCursor( text_cursor );

    // try to move the current cursor to the center for now
    this->ui->hexCodeDisplay->centerCursor();
/*
    int cursorY = this->ui->hexCodeDisplay->cursorRect().top();
    QScrollBar *vbar = this->ui->hexCodeDisplay->verticalScrollBar();
    vbar->setValue( vbar->value() + cursorY -100 );
    */
}

/* hardcode the line numbers */
void MemoryBrowser::on_go0000_clicked() { this->seekDisplay( 0 ); }
void MemoryBrowser::on_go0200_clicked() { this->seekDisplay( 37 ); }
void MemoryBrowser::on_go4000_clicked() { this->seekDisplay( 1099 ); }
void MemoryBrowser::on_go8000_clicked() { this->seekDisplay( 2188 ); }
