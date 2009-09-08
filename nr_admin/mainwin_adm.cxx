#include "mainwin_adm.hxx"
#include "debug.hxx"
#include "configman.hxx"
#include <qapplication.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qtoolbar.h>
#include <qsplitter.h>
#include <qdockwindow.h>
#include <qvbox.h>
#include <qscrollview.h>

int NRMainWinAdm::view_idx=1;

NRMainWinAdm::NRMainWinAdm(QWidget* parent, const char* name, WFlags f):
    QMainWindow(parent, name, f)
{
    QSplitter  *hbox=new QSplitter(this);

    view=new DataViewer(view_idx, hbox);

    QDockWindow *dwin=new QDockWindow(QDockWindow::InDock, this);
    
    QScrollView* dock_view=new QScrollView(hbox);
    QVBox*    dock_vbox=new QVBox(dock_view->viewport());
    dock_vbox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    dwin->setCaption(tr("Info") + QString("[%1]").arg(view_idx));
    moveDockWindow(dwin, Qt::DockRight);

    info_win[0]=new InfoWin(dock_vbox);
    info_win[1]=new InfoWin(dock_vbox);
    dock_view->addChild(dock_vbox);
    dock_view->setResizePolicy(QScrollView::AutoOneFit);
    dwin->setWidget(dock_view);
    dwin->setVerticallyStretchable(true);

    connect(view, SIGNAL(wheel_up_sig()), SLOT(zoom_in_slot()));
    connect(view, SIGNAL(wheel_down_sig()), SLOT(zoom_out_slot()));

    create_menu();
    create_toolbar();
    statusBar();
    setCentralWidget(hbox);
    setCaption(tr("Northern Roast Admin") + QString("[%1]").arg(view_idx));

    view_idx++;

    load_config();
}

NRMainWinAdm::~NRMainWinAdm()
{
    CFGMAN::instance()->set_int_value("appearence", "main_win_width", width());
    CFGMAN::instance()->set_int_value("appearence", "main_win_height", height());
    CFGMAN::instance()->set_cstring_value("appearence", "grid_font", (const char*) (view->get_grid_font().toString().local8Bit()));
}

void NRMainWinAdm::new_view_slot()
{
    NRMainWinAdm *m = new NRMainWinAdm(0, 0, WDestructiveClose);
    qApp->setMainWidget(m);
    m->show();
    qApp->setMainWidget(0);
}

void NRMainWinAdm::load_config()
{
    QSize siz;
    siz.setWidth(CFGMAN::instance()->get_int_value("appearence", "main_win_width", 400));
    siz.setHeight(CFGMAN::instance()->get_int_value("appearence", "main_win_height", 350));
    resize(siz);
    char buf[1024]="";
    CFGMAN::instance()->get_cstring_value("appearence", "grid_font", buf);
    if(buf[0])
    {
	QFont fnt;
	fnt.fromString(QString(buf));
	view->set_grid_font(fnt);
	view->redraw_grid();
    }
}

void NRMainWinAdm::create_toolbar()
{
    QToolBar* tool_bar=new QToolBar(this, "toolbar" );
    moveDockWindow(tool_bar, Qt::DockTop);
    zoom_spin=new QSpinBox(zoom_factor,800,zoom_factor, tool_bar);
    zoom_spin->setValue(view->get_zoom());
    zoom_spin->setSuffix("%");
    connect(zoom_spin, SIGNAL(valueChanged(int)), view, SLOT(set_zoom_slot(int)));
}

void NRMainWinAdm::create_menu()
{
    QMenuBar* menu = menuBar();

    QPopupMenu* file = new QPopupMenu( menu );
    file->insertItem(tr("New window"), this, SLOT(new_view_slot()), CTRL+Key_N);
    file->insertSeparator();
    file->insertItem(tr("E&xit"), qApp, SLOT(quit()), CTRL+Key_Q);
    menu->insertItem(tr("&File"), file);
    QPopupMenu* gridmenu= new QPopupMenu( menu);
    grid_vert_a=new QAction(tr("Show vertical grid lines"), CTRL+Key_Y, this);
    grid_vert_a->setToggleAction(true);
    grid_vert_a->setOn(true);
    connect(grid_vert_a, SIGNAL(toggled(bool)), view, SLOT(show_vert_grid_slot(bool)));
    grid_hor_a=new QAction(tr("Show horizontal grid lines"), CTRL+Key_U, this);
    grid_hor_a->setToggleAction(true);
    grid_hor_a->setOn(true);
    connect(grid_hor_a, SIGNAL(toggled(bool)), view, SLOT(show_hor_grid_slot(bool)));
    
    grid_vert_a->addTo(gridmenu);
    grid_hor_a->addTo(gridmenu);
    menu->insertItem(tr("&Grid"), gridmenu);

    QPopupMenu* grid_v_menu=new QPopupMenu(gridmenu);
    
    grid_v_grp=new QActionGroup(grid_v_menu);
    grid_v_grp->setExclusive(true);
    grid_v_a[0]=new QAction(tr("every 1 min"), 0, grid_v_grp);
    grid_v_a[0]->setToggleAction(true);
    grid_v_grp->add(grid_v_a[0]);
    grid_v_a[1]=new QAction(tr("every 5 min"), 0, grid_v_grp);
    grid_v_a[1]->setToggleAction(true);
    grid_v_a[1]->setOn(true);
    grid_v_grp->add(grid_v_a[1]);
    grid_v_a[2]=new QAction(tr("every 10 min"), 0, grid_v_grp);
    grid_v_a[2]->setToggleAction(true);
    grid_v_grp->add(grid_v_a[2]);
    grid_v_a[3]=new QAction(tr("every 30 min"), 0, grid_v_grp);
    grid_v_a[3]->setToggleAction(true);
    grid_v_grp->add(grid_v_a[3]);
    grid_v_a[4]=new QAction(tr("every 60 min"), 0, grid_v_grp);
    grid_v_a[4]->setToggleAction(true);
    grid_v_grp->add(grid_v_a[4]);
    grid_v_grp->addTo(grid_v_menu);
    gridmenu->insertItem(tr("Vertical grid size"), grid_v_menu);

    connect(grid_v_grp, SIGNAL(selected(QAction*)), this, SLOT(set_vert_grid_lines_slot(QAction*)));

    QPopupMenu* grid_h_menu=new QPopupMenu(gridmenu);
    
    grid_h_grp=new QActionGroup(grid_h_menu);
    grid_h_grp->setExclusive(true);
    grid_h_a[0]=new QAction(tr("every 5"), 0, grid_h_grp);
    grid_h_a[0]->setToggleAction(true);
    grid_h_grp->add(grid_h_a[0]);
    grid_h_a[1]=new QAction(tr("every 10"), 0, grid_h_grp);
    grid_h_a[1]->setToggleAction(true);
    grid_h_grp->add(grid_h_a[1]);
    grid_h_a[2]=new QAction(tr("every 20"), 0, grid_h_grp);
    grid_h_a[2]->setToggleAction(true);
    grid_h_grp->add(grid_h_a[2]);
    grid_h_a[3]=new QAction(tr("every 50"), 0, grid_h_grp);
    grid_h_a[3]->setToggleAction(true);
    grid_h_a[3]->setOn(true);
    grid_h_grp->add(grid_h_a[3]);
    grid_h_a[4]=new QAction(tr("every 100"), 0, grid_h_grp);
    grid_h_a[4]->setToggleAction(true);
    grid_h_grp->add(grid_h_a[4]);
    grid_h_grp->addTo(grid_h_menu);
    gridmenu->insertItem(tr("Horizontal grid size"), grid_h_menu);

    gridmenu->insertSeparator();
    gridmenu->insertItem(tr("Change grid font"), view, SLOT(change_grid_font_slot()));

    connect(grid_h_grp, SIGNAL(selected(QAction*)), this, SLOT(set_hor_grid_lines_slot(QAction*)));
}

void NRMainWinAdm::set_vert_grid_lines_slot(QAction* a)
{
    int  val[5]={60, 300, 600, 1800, 3600};
    for(int i=0;i<5;i++)
	if(grid_v_a[i]==a)
	{
	    view->set_grid_lines(val[i], -1);
	    break;
	}
}

void NRMainWinAdm::set_hor_grid_lines_slot(QAction* a)
{
    int  val[5]={5, 10, 20, 50, 100};
    for(int i=0;i<5;i++)
	if(grid_h_a[i]==a)
	{
	    view->set_grid_lines(-1, val[i]);
	    break;
	}
}

void NRMainWinAdm::zoom_in_slot()
{
    zoom_spin->setValue(view->get_zoom() + zoom_factor);
}

void NRMainWinAdm::zoom_out_slot()
{
    zoom_spin->setValue(view->get_zoom() - zoom_factor);
}
