#include <QtGui>
#include <stdlib.h>
#include "videodisplay.h"
#include "ui_videodisplay.h"

const PALENT colorPalette [] =
{
    // C=64 Palette
    {   0,   0,   0 },  // 0x00: black
    { 255, 255, 255 },  // 0x01: white
    { 136,   0,   0 },  // 0x02: red
    { 170, 255, 238 },  // 0x03: cyan
    { 204,  68, 204 },  // 0x04: violet
    {   0, 204,  85 },  // 0x05: green
    {   0,   0, 170 },  // 0x06: blue
    { 238, 238, 119 },  // 0x07: yellow
    { 221, 136,  85 },  // 0x08: orange
    { 102,  68,   0 },  // 0x09: brown
    { 225, 119, 119 },  // 0x0A: lt red
    {  51,  51,  51 },  // 0x0B: dk gray
    { 119, 119, 119 },  // 0x0C: gray
    { 170, 255, 102 },  // 0x0D: green
    {   0, 136, 255 },  // 0x0E: blue
    { 187, 187, 187 },  // 0x0F: gray

    // Amiga Palette
    // My 16 color version of the Deluxe Paint palette - NTSC safe ;)
    {   0,   0,   0 },  // 0x10: black
    {  80,  80,  80 },  // 0x11: dk gray
    { 176, 176, 176 },  // 0x12: lt gray
    { 240, 240, 240 },  // 0x13: white
    { 208,  60,  40 },  // 0x14: red
    { 149,  40,  26 },  // 0x15: dk red
    { 152,  86,  43 },  // 0x16: brown
    { 200, 131,  38 },  // 0x17: orange
    { 240, 219,  48 },  // 0x18: yellow
    { 158, 228,  28 },  // 0x19: green
    {  56, 120,   5 },  // 0x1A: dk green
    {  87, 186,  96 },  // 0x1B: blue green
    {  87, 202, 205 },  // 0x1C: cyan
    {   0,  67, 236 },  // 0x1D: blue
    { 174,  80, 222 },  // 0x1E: violet
    { 183,  62, 130 }   // 0x1F: red violet
};

VideoDisplay::VideoDisplay(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::VideoDisplay)
    , gfx( NULL )
    , paletteChooser( kPaletteAmiga )
    , width( kVidWidth )
    , height( kVidHeight )

{
    ui->setupUi(this);

    // allocate our internal graphics buffer (l-r t-b, RGB)
    this->gfx = new unsigned char[ this->width * this->height * 3 ];

    // Start with a pattern on the screen
    this->DisplayPattern( kDisplayPatternRandom );
}

VideoDisplay::~VideoDisplay()
{
    delete ui;
    delete( this->gfx );
}

/////////////////////////////////////////////////////////////


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
        int paletteIndex = (videoMemory[i] & 0x0F) + poffs;
        this->gfx[ (i*3)+0 ] = colorPalette[ paletteIndex ].r; // r
        this->gfx[ (i*3)+1 ] = colorPalette[ paletteIndex ].g; // g
        this->gfx[ (i*3)+2 ] = colorPalette[ paletteIndex ].b; // b
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
