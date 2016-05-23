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
    ui->pointRendererWidget->initialize( ui->sld_frame_rate->value(), ui->sld_frame_tolerance->value() );

    ui->group_effects->setId( ui->radio_phong, TucanoRenderingState::PHONG );
    ui->group_effects->setId( ui->radio_jfpbr, TucanoRenderingState::JUMP_FLOODING );

    connect( ui->group_effects, static_cast< void ( QButtonGroup::* )( int )>( &QButtonGroup::buttonClicked ),
			 ui->pointRendererWidget, &PointRendererWidget::toggleEffect );
    
	connect( ui->button_reload_shaders, &QPushButton::clicked, ui->pointRendererWidget,
			 &PointRendererWidget::reloadShaders );
    
	connect( ui->sld_frame_rate, &QSlider::valueChanged, ui->pointRendererWidget, &PointRendererWidget::setFrameRate );
	
	connect( ui->sld_frameskip, &QSlider::valueChanged, ui->pointRendererWidget, &PointRendererWidget::setJfpbrFrameskip );
	
	connect( ui->sld_frame_tolerance, &QSlider::valueChanged, ui->pointRendererWidget,
			 &PointRendererWidget::setRenderingTimeTolerance );
	
	connect( ui->spinbox_first_max_distance,
			 static_cast< void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), ui->pointRendererWidget,
			 &PointRendererWidget::setJFPBRFirstMaxDistance );
    
	connect( ui->check_trackball, &QCheckBox::stateChanged, ui->pointRendererWidget, &PointRendererWidget::toggleDrawTrackball );
	
	connect( ui->ckbx_draw_viewports, &QCheckBox::stateChanged, ui->pointRendererWidget,
			 &PointRendererWidget::toggleDrawAuxViewports );
	
	connect( ui->bt_write_frames, &QCheckBox::stateChanged, ui->pointRendererWidget, &PointRendererWidget::toggleWriteFrames );
	
	connect( ui->bt_draw_node_debug, &QCheckBox::stateChanged, ui->pointRendererWidget, &PointRendererWidget::toggleNodeDebugDraw );
	
	connect( ui->pointRendererWidget, &PointRendererWidget::debugInfoDefined, ui->debug_info, &QTextBrowser::setText);
}

void MainWindow::on_bt_open_cloud_clicked()
{
	QFileDialog dialog;
	dialog.setFileMode( QFileDialog::ExistingFile );
	dialog.setNameFilter( "Octree files or point files (*.oct *.ply);;Octree files (*.oct);;Point files (*.ply)" );
	dialog.setDirectory( QDir::currentPath().append( "/../../src/data/real/" ) );
	
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
