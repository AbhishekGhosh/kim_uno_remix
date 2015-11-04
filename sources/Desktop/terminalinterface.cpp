#include "terminalinterface.h"
#include "ui_terminalinterface.h"

extern "C" {
#include "interface.h"
}

// ref: http://doc.qt.io/qt-5/qtserialport-terminal-example.html

TerminalInterface::TerminalInterface(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TerminalInterface)
{
    ui->setupUi(this);

    this->setWindowTitle( "KIM Serial Terminal" );

    connect( this->ui->term, SIGNAL( getData( const QByteArray & ) ),
             this, SLOT( gotData( const QByteArray & ) ) );
}

TerminalInterface::~TerminalInterface()
{
    delete ui;
}

void TerminalInterface::gotData(const QByteArray &data)
{
    int j=0;

    for( j=0 ; j< data.count() ; j++ ) {
        KimSerialInPush( data.at(j) );
    }
}


void TerminalInterface::putData( const QByteArray &data )
{
    this->ui->term->putData( data );
}
