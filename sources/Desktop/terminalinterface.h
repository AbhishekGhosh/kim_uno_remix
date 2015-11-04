#ifndef TERMINALINTERFACE_H
#define TERMINALINTERFACE_H

#include <QDialog>
#include <console.h>

namespace Ui {
class TerminalInterface;
}

class TerminalInterface : public QDialog
{
    Q_OBJECT

public:
    explicit TerminalInterface(QWidget *parent = 0);
    ~TerminalInterface();

private:
    Ui::TerminalInterface *ui;

public:
    void putData( const QByteArray &data );

private slots:
    void gotData( const QByteArray &data );
};

#endif // TERMINALINTERFACE_H
