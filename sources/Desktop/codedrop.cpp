#include "codedrop.h"
#include "ui_codedrop.h"
// ref: http://doc.qt.io/qt-5/qtwidgets-widgets-codeeditor-example.html

#include <QtCore/QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QSettings>
#include <QDragEnterEvent>
#include <QMimeData>

extern "C" {
#include "interface.h"
}


///////////////////////////////////////////////////////////////////////////////
// Class stuff
CodeDrop::CodeDrop(QWidget *parent, MemoryBrowser *_mb )
    : QDialog(parent)
    , mb( _mb )
    , ui(new Ui::CodeDrop)
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

CodeDrop::~CodeDrop()
{
    delete ui;
}


///////////////////////////////////////////////////////////////////////////////
// Settings handlers (load, save )
void CodeDrop::SaveSettings()
{
    QSettings settings( "UmlautLlama.com", "KimUnoRemix" );
    settings.beginGroup( "CodeDrop" );
    settings.setValue( "Version", "2" );
    settings.setValue( "AutoReset", this->ui->AutoPC->isChecked() );
    settings.setValue( "AutoRun", this->ui->AutoRun->isChecked() );
    settings.setValue( "LastFile", this->lastFile );
    settings.endGroup();
}

void CodeDrop::LoadSettings()
{
    bool autoPC = false;
    bool autoRun = false;

    QSettings settings( "UmlautLlama.com", "KimUnoRemix" );
    settings.beginGroup( "CodeDrop" );

    // 1. check to see if we've already got version info saved
    int vers = settings.value( "Version", 0 ).toInt();
    if( vers != 2 ) {
        // nope. set some defaults
        settings.setValue( "Version", "2" );
        this->lastFile = "file.lst";
        this->ui->AutoPC->setChecked( false );
        this->ui->AutoRun->setChecked( false );

        // and save them
        this->SaveSettings();
    }
    // now load the settings
    this->lastFile = settings.value( "LastFile" ).toString();
    autoPC = settings.value( "AutoPC" ).toBool();
    autoRun = settings.value( "AutoRun" ).toBool();
    settings.endGroup();

    // and populate the UI
    this->ui->SelectedFilepath->setText( this->lastFile );
    this->ui->AutoPC->setChecked( autoPC );
    this->ui->AutoRun->setChecked( autoRun );
}


///////////////////////////////////////////////////////////////////////////////
// Drag and drop support
//
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

    // and force the reload!
    this->ReloadFromFile();
}

///////////////////////////////////////////////////////////////////////////////
// when these are clicked, just save it

void CodeDrop::on_AutoPC_clicked()
{
    this->SaveSettings();
}

void CodeDrop::on_AutoRun_clicked()
{
    this->SaveSettings();
}

///////////////////////////////////////////////////////////////////////////////
// Browse for a new file (request file from user)
//
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


// Helpers for parsing the file
// is ch a hex character [0-9a-fA-F]
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

// get the address from a line (first 6 bytes need to be hex in ascii)
// this was only tested with cc65's output .lst files!
int CodeDrop::GetLineAddress( const char * line )
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


// get the requested data byte. 0..3
// this was only tested with cc65's output .lst files!
int CodeDrop::GetDataByte( const char * line, int which )
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

// load in a CA65/CC65 "list" file.  This ONLY is confirmed to work
// with the current software.  No idea if it works with other stuff
// perhaps it can be adapted to others later.
int CodeDrop::ParseCC65LstFile( QString filepath )
{
    this->loadedBytes = 0;
    this->loadedAddr = -1;

    // open up a CC65 ASM List file
    QFile inputFile( filepath );
    if (inputFile.open(QIODevice::ReadOnly))
    {
       QTextStream in(&inputFile);
       while (!in.atEnd())
       {
          QString line = in.readLine();
          const char * d = line.toLatin1().data();
          int addr = this->GetLineAddress( d );

          if( addr >= 0 ) {
              // set the bookmark
              if( this->loadedAddr < 0 ) {
                  this->loadedAddr = addr;
              }

              int b0, b1, b2, b3;
              b0 = this->GetDataByte( d, 0 );
              b1 = this->GetDataByte( d, 1 );
              b2 = this->GetDataByte( d, 2 );
              b3 = this->GetDataByte( d, 3 );

              if( b0 >= 0 ) { write6502( addr++, b0 ); this->loadedBytes++; }
              if( b1 >= 0 ) { write6502( addr++, b1 ); this->loadedBytes++; }
              if( b2 >= 0 ) { write6502( addr++, b2 ); this->loadedBytes++; }
              if( b3 >= 0 ) { write6502( addr++, b3 ); this->loadedBytes++; }
          }
       }
       inputFile.close();
    }
    return this->loadedBytes;
}


void CodeDrop::ReloadFromFile()
{
    // make sure the file exists...
    QFileInfo checkFile( this->lastFile );
    if( !checkFile.exists() ) {
        this->ui->StatusText->setText( "Error: No file exists at the specified path." );
        return;
    }

    // and it's a file
    if( !checkFile.isFile() ) {
        this->ui->StatusText->setText( "Error: The specified path is not a file." );
        return;
    }

    // now let's parse it in and see what we get...
    this->loadedBytes = this->ParseCC65LstFile( this->lastFile );
    QString infotext;

    infotext.sprintf( "%d bytes loaded at $%04x.", this->loadedBytes, this->loadedAddr );

    if( this->loadedBytes <0 ) {
        infotext = "Error: Unable to load specified file.";
    } else if( this->loadedBytes == 0 ) {
        infotext = "Error: No data found in specified file.";
    } else {
        // now trigger autopc and autorun if applicable...
        if( this->ui->AutoPC->isChecked() || this->ui->AutoRun->isChecked() )
        {
            infotext.append( " ADDR Seek." );

            // seek the PC
            this->keysToInject.enqueue( kKimScancode_STOP );
            this->keysToInject.enqueue( kKimScancode_ADDR );
            this->keysToInject.enqueue( (this->loadedAddr >> 12) & 0x0F );
            this->keysToInject.enqueue( (this->loadedAddr >> 8) & 0x0F );
            this->keysToInject.enqueue( (this->loadedAddr >> 4) & 0x0F );
            this->keysToInject.enqueue( (this->loadedAddr >> 0) & 0x0F );

            if( this->ui->AutoRun->isChecked() )
            {
                infotext.append( " GO." );
                this->keysToInject.enqueue( kKimScancode_GO );
            }
        }
    }

    this->ui->StatusText->setText( infotext );

    if( this->mb ) this->mb->refreshDisplay();
}

// ui button was clicked, let's parse in the selected file...
void CodeDrop::on_LoadToRAM_clicked()
{
    this->ReloadFromFile();
}
