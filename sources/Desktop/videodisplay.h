#ifndef VIDEODISPLAY_H
#define VIDEODISPLAY_H

#include <QDialog>
#include <QGraphicsScene>
#include <QImage>

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
    QGraphicsScene *scene;
    QPixmap * pixmap;
    QImage image;

public:

// square
#define kVidWidth  (32)
#define kVidHeight (32)
// C=64
//#define kVidWidth  (40)
//#define kVidHeight (25)

    void Set( int x, int y, int color );
    int Get( int x, int y );

    void SetLinear( int address, unsigned char value );
    unsigned char GetLinear( int address );

public:

#define kDisplayPatternBlank     (0)
#define kDisplayPatternStart     (1)
#define kDisplayPatternDiagonals (2)

    void DisplayPattern( int which );
    void Fill( int color );
    void UpdateScreen();
};

#endif // VIDEODISPLAY_H
