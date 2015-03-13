#include <QFileDialog>
#include "MainWindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initialize()
{
    ui->pointRendererWidget->initialize();

    ui->group_effects->setId( ui->radio_phong, TucanoRenderingState< vec3, float >::PHONG );
    ui->group_effects->setId( ui->radio_jfpbr, TucanoRenderingState< vec3, float >::JUMP_FLOODING );

    connect( ui->group_effects, static_cast< void ( QButtonGroup::* )( int )>( &QButtonGroup::buttonClicked ),
			 ui->pointRendererWidget, &PointRendererWidget::toggleEffect );
    
	connect( ui->button_reload_shaders, &QPushButton::clicked, ui->pointRendererWidget,
			 &PointRendererWidget::reloadShaders );
    
	connect( ui->spinbox_first_max_distance,
			 static_cast< void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), ui->pointRendererWidget,
			 &PointRendererWidget::setJFPBRFirstMaxDistance );
//    connect(ui->slider_ssao_blur, &QSlider::valueChanged, ui->glwidget, &GLWidget::setSSAOBlur);
//    connect(ui->slider_toon_level, &QSlider::valueChanged, ui->glwidget, &GLWidget::setToonQuantLevel);
    
	connect( ui->check_trackball, &QCheckBox::stateChanged, ui->pointRendererWidget,
			 &PointRendererWidget::toggleDrawTrackball );
	
	connect( ui->pointRendererWidget, &PointRendererWidget::debugInfoDefined, ui->debug_info, &QTextBrowser::setText);
}

void MainWindow::on_bt_open_cloud_clicked()
{
	QFileDialog dialog;
	dialog.setFileMode(QFileDialog::AnyFile);
	
	if( dialog.exec() )
	{
		QString filename = dialog.selectedFiles().first();
		ui->pointRendererWidget->openMesh( filename.toStdString() );
	}
}

void MainWindow::keyPressEvent(QKeyEvent *ke)
{
    int key = ke->key();
    int modifiers = ke->modifiers();

    if (modifiers == 0 && key == Qt::Key_Escape)
    {
        close();
    }

    ke->accept();
}
