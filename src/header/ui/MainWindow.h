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

public slots:
	void on_bt_open_cloud_clicked();
	void on_bt_save_octree_clicked();

protected:
    void keyPressEvent( QKeyEvent *ke );
	void closeEvent( QCloseEvent * event ) override;
	
private:
	using NodeLoader = model::NodeLoader< typename PointRendererWidget::Point >;
	
	Ui::MainWindow *ui;
	PointRendererWidget* m_pointRenderWidget;
	NodeLoader* m_loader;
};

#endif // MAINWINDOW