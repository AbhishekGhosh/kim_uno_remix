#include "codedrop.h"
#include "ui_codedrop.h"
// ref: http://doc.qt.io/qt-5/qtwidgets-widgets-codeeditor-example.html

#include <QtCore/QDebug>
#include <QFileDialog>
#include <QSettings>
#include <QDragEnterEvent>
#include <QMimeData>

extern "C" {
#include "interface.h"
}

void CodeDrop::SaveSettings()
{
    QSettings settings( "UmlautLlama.com", "KimUnoRemix" );
    settings.beginGroup( "CodeDrop" );
    settings.setValue( "Version", "1" );
    settings.setValue( "LastFile", this->lastFile );
    settings.endGroup();
}

void CodeDrop::LoadSettings()
{
    QSettings settings( "UmlautLlama.com", "KimUnoRemix" );
    settings.beginGroup( "CodeDrop" );

    // 1. check to see if we've already got version info saved
    int vers = settings.value( "Version", 0 ).toInt();
    if( vers != 1 ) {
        // nope. set some defaults
        settings.setValue( "Version", "1" );
        this->lastFile = "file.lst";
        // and save them
        this->SaveSettings();
    }
    // now load the settings
    this->lastFile = settings.value( "LastFile" ).toString();
    settings.endGroup();

    // and populate the UI
    this->ui->SelectedFilepath->setText( this->lastFile );
}

CodeDrop::CodeDrop(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CodeDrop)
{
    ui->setupUi(this);

    ui->SelectedFilepath->setAcceptDrops( true );
    ui->BrowseFiles->setAcceptDrops( true );
    this->setAcceptDrops( true );

    this->LoadSettings();

    // set up the settings file
    //QDir pth( QDir::home() );
    //this->settingsFile = pth.absolutePath() + "/kim_uno_remix.ini";
    //qDebug() << "Settings file is " << settingsFile;

    // now load some stuff from the settings file if it's there
    //QSettings settings( this->settingsFile, QSettings::NativeFormat );

}

void CodeDrop::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}

void CodeDrop::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty())
        return;

    QString fileName = urls.first().toLocalFile();
    if (fileName.isEmpty())
        return;

    this->ui->SelectedFilepath->setText( fileName );
    this->lastFile = fileName;
    this->SaveSettings();
}


CodeDrop::~CodeDrop()
{
    delete ui;
}

void CodeDrop::on_BrowseFiles_clicked()
{
    QFileDialog qfd(this);
    qfd.setFileMode( QFileDialog::ExistingFile );

    qfd.setDirectory( QFileInfo( this->lastFile ).absolutePath() );

    qfd.setNameFilter( tr("Asm Listing (*.lst)" ));
    //;Hex Files(*.ihx *.hex)" ));
    qfd.setViewMode( QFileDialog::Detail );

    if( qfd.exec() )
    {
        // OK!
        this->lastFile = qfd.selectedFiles()[0] ;
        this->ui->SelectedFilepath->setText( this->lastFile );
        this->SaveSettings();

    } else {
        // cancelled
    }
}

bool byteIsHex( char ch )
{
    if (   (ch >= '0' && ch <= '9')
        || (ch >= 'A' && ch <= 'F')
        || (ch >= 'a' && ch <= 'f')
        ) {
        return true;
    }
    return false;
}

// this was only tested with cc65's output .lst files!
int getLineAddress( const char * line )
{
    char buf[8];

    if( !line || strlen( line ) < 8 ) return -1;
    if( line[6] == 'r' ) return -2;

    for( int i=0 ; i<6 ; i++ ) {
        // copy and validate
        char ch = line[i];

        // make sure it's valid
        if( !byteIsHex( ch )) return -10;
        // and copy it

        buf[i] = ch;
    }
    // terminate the string
    buf[7] = '\0';

    // and convert hex string to int
    return strtol( buf, NULL, 16 );
}

int getDataByte( const char * line, int which )
{
    int idx = 11 + (which *3);

    if( !line || (int)strlen( line ) < (idx+1) ) return -1;

    if( !byteIsHex( line[idx] )) return -2;
    if( !byteIsHex( line[idx+1] )) return -3;

    char buf[3];
    buf[0] = line[idx++];
    buf[1] = line[idx++];
    buf[2] = '\0';

    return strtol( buf, NULL, 16 );
}

void CodeDrop::on_LoadToRAM_clicked()
{
    int nbytes = 0;
    // this is where we'd parse in the file.
    QFile inputFile( this->lastFile );
    if (inputFile.open(QIODevice::ReadOnly))
    {
       QTextStream in(&inputFile);
       while (!in.atEnd())
       {
          QString line = in.readLine();
          const char * d = line.toLatin1().data();
          int addr = getLineAddress( d );

          if( addr >= 0 ) {
              int b0, b1, b2, b3;
              b0 = getDataByte( d, 0 );
              b1 = getDataByte( d, 1 );
              b2 = getDataByte( d, 2 );
              b3 = getDataByte( d, 3 );

              if( b0 >= 0 ) { write6502( addr++, b0 ); nbytes++; }
              if( b1 >= 0 ) { write6502( addr++, b1 ); nbytes++; }
              if( b2 >= 0 ) { write6502( addr++, b2 ); nbytes++; }
              if( b3 >= 0 ) { write6502( addr++, b3 ); nbytes++; }
          }
       }
       inputFile.close();
    }
}
