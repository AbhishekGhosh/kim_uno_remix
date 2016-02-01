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

    // scroll to top -- this doesn't work in here for some reason
    //QScrollBar *vScrollBar = this->ui->hexCodeDisplay->verticalScrollBar();
    //vScrollBar->triggerAction(QScrollBar::SliderToMinimum);
    this->ui->hexCodeDisplay->verticalScrollBar()->setValue( 0 );
    this->ui->hexCodeDisplay->moveCursor (QTextCursor::Start);
    this->ui->hexCodeDisplay->ensureCursorVisible();

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

void MemoryBrowser::on_go0000_clicked()
{
    this->ui->hexCodeDisplay->verticalScrollBar()->setValue( 0 );
    this->ui->hexCodeDisplay->moveCursor (QTextCursor::Start);
    this->ui->hexCodeDisplay->ensureCursorVisible();
}
