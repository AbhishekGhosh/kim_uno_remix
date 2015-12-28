#ifndef VIDEODISPLAY_H
#define VIDEODISPLAY_H

#include <QDialog>
#include <QGraphicsScene>
#include <QImage>

// number of palette entries
#define kNPalEntries (16)

// square
#define kVidWidth  (32)
#define kVidHeight (32)

// C=64
//#define kVidWidth  (40)
//#define kVidHeight (25)


namespace Ui {
class VideoDisplay;
}

class VideoDisplay : public QDialog
{
    Q_OBJECT

public:
    explicit VideoDisplay(QWidget *parent = 0);
    ~VideoDisplay();


private:
    Ui::VideoDisplay *ui;

private:
    unsigned char * mem; // raw memory (read/write indexes)
    unsigned char * gfx; // graphics memory (RGB)

    typedef struct PALENT {
        unsigned char r;
        unsigned char g;
        unsigned char b;
    } PALENT;

    PALENT * palette;
    void SetPaletteColor( int idx, unsigned char _r, unsigned char _g, unsigned char _b );

public:

    int width;
    int height;

    void Set( int x, int y, int color, bool delayUpdate = false);
    int Get( int x, int y );

    void SetLinear( int address, unsigned char value, bool delayUpdate = false );
    unsigned char GetLinear( int address );

public:

#define kDisplayPatternBlank     (0)
#define kDisplayPatternStart     (1)
#define kDisplayPatternDiagonals (2)

    void DisplayPattern( int which );
    void Fill( int color );

public:
    void UpdateScreen();
};

#endif // VIDEODISPLAY_H
