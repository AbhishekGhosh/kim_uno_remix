#include <QtGui>
#include <stdlib.h>
#include "videodisplay.h"
#include "ui_videodisplay.h"

VideoDisplay::VideoDisplay(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::VideoDisplay)
    , gfx( NULL )
    , palette( NULL )
    , paletteChooser( kPaletteC64 )
    , width( kVidWidth )
    , height( kVidHeight )

{
    ui->setupUi(this);

    // allocate the new graphics buffer (l-r t-b, RGB)
    this->gfx = new unsigned char[ this->width * this->height * 3 ];

    // C64 color buffer starts at $D800, screen is 40x25 (rectangular)
    // ours             starts at $4000, screen is 32x32 (square)

    this->palette = new PALENT[ kNPalEntries ];

    // C64 Palette
    // ref: https://www.c64-wiki.com/index.php/Color
    this->SetPaletteColor( 0x00,    0,   0,   0 ); // black
    this->SetPaletteColor( 0x01,  255, 255, 255 ); // white
    this->SetPaletteColor( 0x02,  136,   0,   0 ); // red
    this->SetPaletteColor( 0x03,  170, 255, 238 ); // cyan
    this->SetPaletteColor( 0x04,  204,  68, 204 ); // violet
    this->SetPaletteColor( 0x05,    0, 204,  85 ); // green
    this->SetPaletteColor( 0x06,    0,   0, 170 ); // blue
    this->SetPaletteColor( 0x07,  238, 238, 119 ); // yellow
    this->SetPaletteColor( 0x08,  221, 136,  85 ); // orange
    this->SetPaletteColor( 0x09,  102,  68,   0 ); // brown
    this->SetPaletteColor( 0x0A,  225, 119, 119 ); // lt red
    this->SetPaletteColor( 0x0B,   51,  51,  51 ); // dk gray
    this->SetPaletteColor( 0x0C,  119, 119, 119 ); // gray
    this->SetPaletteColor( 0x0D,  170, 255, 102 ); // lt green
    this->SetPaletteColor( 0x0E,    0, 136, 255 ); // lt blue
    this->SetPaletteColor( 0x0F,  187, 187, 187 ); // lt gray


    // update it
    this->UpdateScreen();

    // Start with a pattern
    this->DisplayPattern( kDisplayPatternStart );//Random );
}

VideoDisplay::~VideoDisplay()
{
    delete ui;
    delete( this->palette );
    delete( this->gfx );
}

/////////////////////////////////////////////////////////////


void VideoDisplay::SetPaletteColor( int idx, unsigned char _r, unsigned char _g, unsigned char _b )
{
    this->palette[idx].r = _r;
    this->palette[idx].g = _g;
    this->palette[idx].b = _b;
}


void VideoDisplay::DisplayPattern( int which )
{
    switch( which ) // mmm. i could go for a Switchwich right now.
    {
    case( kDisplayPatternBlank ):
        this->Fill( 0 );
        break;

    case( kDisplayPatternStart ):
        this->Fill( 4 ); // Violet
        break;

    case( kDisplayPatternDiagonals ):
        for( int y=0 ; y < this->height ; y++ )
        {
            for( int x=0 ; x < this->width ; x++ )
            {
                this->Set( x, y, x+y );
            }
        }
        break;

    case( kDisplayPatternRandom ):
        for( int p=0 ; p < (this->height * this->width ) ; p++ )
        {
            this->SetLinear( p, qrand() & 0x0F );
        }
        break;

    default:
        this->Fill( 2 ); // Red
    }

    this->UpdateScreen();
}


// fill the display with a color
void VideoDisplay::Fill( int color )
{
    memset( videoMemory, color, this->width * this->height );
    this->UpdateScreen();
}

// display any updates to the image out to the screen
void VideoDisplay::UpdateScreen()
{
    int poffs = 0;
    if( paletteChooser == kPaletteAmiga )  poffs = 16;

    // 1. convert the indexed buffer to the RGB buffer
    for( int i=0 ; i < (this->width * this->height) ; i++ )
    {
        this->gfx[ (i*3)+0 ] = this->palette[ poffs + (videoMemory[ i ] & 0x0F) ].r; // r
        this->gfx[ (i*3)+1 ] = this->palette[ poffs + (videoMemory[ i ] & 0x0F) ].g; // g
        this->gfx[ (i*3)+2 ] = this->palette[ poffs + (videoMemory[ i ] & 0x0F) ].b; // b
    }

    // 2. create a QImage from the RGB buffer
    QImage *i = new QImage( this->gfx, this->width, this->height, QImage::Format_RGB888 );

    // 3. shove the image into the label

    QSize sz( this->ui->GraphicsLabel->size().width(), this->ui->GraphicsLabel->size().height() );
    this->ui->GraphicsLabel->setPixmap( QPixmap::fromImage(*i).scaled( sz,
                                                                       Qt::IgnoreAspectRatio,
                                                                       Qt::FastTransformation ) );

}

// get the pixel at x,y
int VideoDisplay::Get( int x, int y )
{
    if( x < 0 || x > this->width ) return 0;
    if( y < 0 || y > this->height ) return 0;

    return( videoMemory[ (y * this->width) + x ] );
}

// set the pixel at x,y with the specified color
void VideoDisplay::Set( int x, int y, int color, bool delayUpdate )
{
    if( x < 0 || x > this->width ) return;
    if( y < 0 || y > this->height ) return;

    videoMemory[ (y * this->width) + x ] = color;

    if( !delayUpdate ) this->UpdateScreen();
}


// set the pixel at (buffer + address)
void VideoDisplay::SetLinear( int address, unsigned char value, bool delayUpdate )
{
    videoMemory[ address ] = value;

    int y = address / kVidWidth;
    int x = address - (y * kVidWidth);

    this->Set( x, y, value );
    if( !delayUpdate ) this->UpdateScreen();
}

// get the pixel at (buffer + address)
unsigned char VideoDisplay::GetLinear( int address )
{
    return( videoMemory[address] );
}
