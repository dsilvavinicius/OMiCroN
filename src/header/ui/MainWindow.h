#ifndef MAIN_WINDOW__H
#define MAIN_WINDOW__H

#include <QMainWindow>
#include "PointRendererWidget.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow( QWidget *parent = 0 );
    ~MainWindow();

    void initialize( );

private:
    Ui::MainWindow *ui;

protected:
    void keyPressEvent( QKeyEvent *ke );
};

#endif // MAINWINDOW