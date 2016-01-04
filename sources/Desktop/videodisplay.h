#ifndef VIDEODISPLAY_H
#define VIDEODISPLAY_H

#include <QDialog>
#include <QGraphicsScene>
#include <QImage>

extern "C" {

/* ************************************************************************** */
/* Video */

/* square */
#define kVidWidth  (32)
#define kVidHeight (32)

/* C=64
#define kVidWidth  (40)
#define kVidHeight (25)
*/

}

// number of palette entries
#define kNPalEntries (16 * 2)

extern "C" {
    extern uint8_t videoMemory[];
}

typedef struct PALENT {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} PALENT;


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
    unsigned char * gfx; // graphics memory (RGB)
    QImage * imageGfx;   // QImage wrapper

public:
    int paletteChooser;
#define kPaletteC64   (0)
#define kPaletteAmiga (1)

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
#define kDisplayPatternStripes   (3)
#define kDisplayPatternRandom    (4)

    void DisplayPattern( int which );
    void Fill( int color );

public:
    void UpdateScreen();
};

#endif // VIDEODISPLAY_H
