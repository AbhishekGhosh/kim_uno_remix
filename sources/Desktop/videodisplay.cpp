#include "videodisplay.h"
#include "ui_videodisplay.h"

VideoDisplay::VideoDisplay(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::VideoDisplay)
    , scene( NULL )
    , pixmap( NULL )
    , image( kVidWidth, kVidHeight, QImage::Format_Indexed8 )

{
    ui->setupUi(this);

    // C64 color buffer starts at $D800, screen is 40x25 (rectangular)
    // ours             starts at $4000, screen is 32x32 (square)

    // C64 Palette
    // ref: https://www.c64-wiki.com/index.php/Color
    this->image.setColor( 0x00, qRgb(   0,   0,   0 )); // Black
    this->image.setColor( 0x01, qRgb( 255, 255, 255 )); // White
    this->image.setColor( 0x02, qRgb( 136,   0,   0 )); // Red
    this->image.setColor( 0x03, qRgb( 170, 255, 238 )); // Cyan
    this->image.setColor( 0x04, qRgb( 204,  68, 204 )); // Violet
    this->image.setColor( 0x05, qRgb(   0, 204,  85 )); // Green
    this->image.setColor( 0x06, qRgb(   0,   0, 170 )); // Blue
    this->image.setColor( 0x07, qRgb( 238, 238, 119 )); // Yellow
    this->image.setColor( 0x08, qRgb( 221, 136,  85 )); // Orange
    this->image.setColor( 0x09, qRgb( 102,  68,   0 )); // Brown
    this->image.setColor( 0x0A, qRgb( 225, 119, 119 )); // Lt Red
    this->image.setColor( 0x0B, qRgb(  51,  51,  51 )); // Dk Gray
    this->image.setColor( 0x0C, qRgb( 119, 119, 119 )); // Gray
    this->image.setColor( 0x0D, qRgb( 170, 255, 102 )); // Lt Green
    this->image.setColor( 0x0E, qRgb(   0, 136, 255 )); // Lt Blue
    this->image.setColor( 0x0F, qRgb( 187, 187, 187 )); // Lt Gray

    // since our display is 320x320, scale our 32x32 up 10x
    this->ui->graphicsView->scale( 10, 10 );

    // create a scene
    this->scene = new QGraphicsScene(this);

    // update it
    this->UpdateScreen();

    // Start with a pattern
    this->DisplayPattern( kDisplayPatternStart );
}

VideoDisplay::~VideoDisplay()
{
    delete ui;
    delete scene;
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
        for( int y=0 ; y < this->image.height() ; y++ )
        {
            for( int x=0 ; x < this->image.width() ; x++ )
            {
                this->Set( x, y, x+y );
            }
        }
        break;

    default:
        this->Fill( 2 ); // Red
    }

    this->UpdateScreen();
}

// display any updates to the image out to the screen
void VideoDisplay::UpdateScreen()
{
    // update the pixmap to the screen
    this->scene->addPixmap( QPixmap::fromImage( this->image ) );
    this->ui->graphicsView->setScene( this->scene );
}

// set the pixel at x,y with the specified color
void VideoDisplay::Set( int x, int y, int color )
{
    if( x < 0 || x > kVidWidth ) return;
    if( y < 0 || y > kVidHeight ) return;

    if( this->image.pixelIndex( x,y ) != color )
    {
        this->image.setPixel( x, y, color & 0x0F );
    }
}

// get the pixel at x,y
int VideoDisplay::Get( int x, int y )
{
    if( x < 0 || x > kVidWidth ) return 0;
    if( y < 0 || y > kVidHeight ) return 0;

    return( this->image.pixelIndex( x, y ) );
}

// set the pixel at (buffer + address)
void VideoDisplay::SetLinear( int address, unsigned char value )
{
    int y = address / kVidWidth;
    int x = address - (y * kVidWidth);

    this->Set( x, y, value );
    this->UpdateScreen();
}

// get the pixel at (buffer + address)
unsigned char VideoDisplay::GetLinear( int address )
{
    int y = address / kVidWidth;
    int x = address - (y * kVidWidth);

    return this->Get( x, y );
}

// fill the display with a color
void VideoDisplay::Fill( int color )
{
    this->image.fill( color );
    this->UpdateScreen();
}
