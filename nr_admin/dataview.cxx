#include <qwmatrix.h>
#include <qfontdialog.h>
#include "dataview.hxx"
#include "qt_event_notifier.hxx"
#include "data_watcher.hxx"


const int CANVAS_DEF_WIDTH=1000;
const int CANVAS_DEF_HEIGHT=1000;

DataViewer::DataViewer(int id_, QWidget *parent, const char* wname):QCanvasView(parent, wname)
{
    id=id_;
    zoom_value=zoom_def_val;
    grid_step_x=300;
    grid_step_y=50;

    show_vert_grid=true;
    show_hor_grid=true;

    setCanvas(new QCanvas(CANVAS_DEF_WIDTH, CANVAS_DEF_HEIGHT));
    canvas()->setBackgroundColor(Qt::white);
    canvas()->setDoubleBuffering(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    viewport()->setMouseTracking(TRUE);

    grid_pen=QPen(Qt::gray, 0, Qt::DotLine);
    grid_font=QFont("Times", 10);
    draw_area=QRect(40, 40, 3000, 500);
    correct_canvas_size();

    create_grid();

    test_graphs();

    draw_graphs();

    //int val1=rand()%100, val2=rand()%100;
    //for(int i=0;i<draw_area.width()-1;i++)
    //{
    //    QCanvasLine *ln1=new QCanvasLine(canvas());
    //    ln1->setPoints(draw_area.left()+i, 
    //        draw_area.bottom()-val1,
    //        draw_area.left()+i+1,
    //        draw_area.bottom()-val2);
    //    ln1->setPen(QPen(Qt::blue, 2));
    //    ln1->setZ(10);
    //    ln1->show();
    //    val1=val2;
    //    val2=i/3+rand()%10;
    //}

    //for(int i=0;i<draw_area.width()-1;i++)
    //{
    //    QCanvasLine *ln1=new QCanvasLine(canvas());
    //    ln1->setPoints(draw_area.left()+i, 
    //        draw_area.bottom()-val1,
    //        draw_area.left()+i+1,
    //        draw_area.bottom()-val2);
    //    ln1->setPen(QPen(Qt::blue, 2));
    //    ln1->setZ(11);
    //    ln1->show();
    //    val1=val2;
    //    val2=i/2+rand()%20;
    //}
}

DataViewer::~DataViewer()
{
    delete canvas();
    for(GraphsIt it=graph_lst.begin();it!=graph_lst.end();it++)
    {
        delete (*it)->data();
        delete *it;
    }
}

void DataViewer::correct_canvas_size()
{
    draw_area.setWidth((draw_area.width()+grid_step_x -1)/grid_step_x*grid_step_x);
    draw_area.setHeight((draw_area.height()+grid_step_y -1)/grid_step_y*grid_step_y);

    canvas()->resize(draw_area.right() + 50,
        draw_area.bottom() + 50);
}

void DataViewer::contentsWheelEvent(QWheelEvent *e)
{
    if(e->delta()>0)
        emit(wheel_up_sig());
    else
        emit(wheel_down_sig());
    e->accept();
}

void DataViewer::customEvent(QCustomEvent* e)
{
    if(e->type()==qt_notification_event_id)
    {
        QtNotificationEvent* ne=dynamic_cast<QtNotificationEvent*>(e);
        ne->notification();
        draw_graphs();
        canvas()->update();
    }
}

void DataViewer::set_zoom_slot(int new_scale)
{
    int old_scale=zoom_value;
    double factor=double(new_scale)/double(old_scale);
    zoom_value=new_scale;
    QWMatrix m = worldMatrix();
    m.scale( factor, factor );
    setWorldMatrix( m );
    ACE_DEBUG((LDBG "ZOOM_SET: old=%d, new=%d\n", old_scale, zoom_value));
}



//------------------------------Graph routines-------------------------------
void DataViewer::draw_graphs()
{
    for(GraphsIt it=graph_lst.begin();it!=graph_lst.end();it++)
        draw_graph(*it);
}

void DataViewer::draw_graph(DataGraph* gr)
{
    DataSet* dset=gr->data();
    size_t   sz=dset->size();
    for(size_t i=gr->get_last_idx();i<sz-1;i++)
    {
        DataValue val1=dset->get(i);
        DataValue val2=dset->get(i+1);

        QCanvasLine *ln1=new QCanvasLine(canvas());
        ln1->setPoints(draw_area.left()+i, 
            draw_area.bottom()-val1,
            draw_area.left()+i+1,
            draw_area.bottom()-val2);
        ln1->setPen(gr->pen());
        ln1->setZ(20);
        ln1->show();
    }
    gr->set_last_idx(sz-1);
}

void DataViewer::test_graphs()
{
    DataSet* dset=new DataSet();

    DataWatcher* watch=new DataWatcher(dset, new QtEventNotifier(this));
    watch->activate();

    graph_lst.push_back(new DataGraph(dset));
    //graph_lst.push_back(new DataGraph(new DataSet()));
    //graph_lst.push_back(new DataGraph(new DataSet()));
}
//------------------------------Grid routines-------------------------------

const int grid_line_rtti=19761;
const int grid_line_text_rtti=19762;

class GridLine:public QCanvasLine
{
public:
    GridLine(QCanvas* c):QCanvasLine(c) {};
    int rtti() const { return grid_line_rtti;};
};

class GridText:public QCanvasText
{
public:
    GridText(const QString& t, QCanvas* c) : QCanvasText(t, c) {};
    int rtti() const { return grid_line_text_rtti;};
};

void DataViewer::set_grid_lines(int ix, int iy)
{
    if(ix>0)
        grid_step_x=ix;
    if(iy>0)
        grid_step_y=iy;
    delete_grid();
    create_grid();
    canvas()->setAllChanged();
    canvas()->update();
}

void DataViewer::show_vert_grid_slot(bool v)
{
    show_vert_grid=v;
    redraw_grid();
}

void DataViewer::show_hor_grid_slot(bool v)
{
    show_hor_grid=v;
    redraw_grid();
}

void DataViewer::change_grid_font_slot()
{
    bool ok;
    QFont fnt = QFontDialog::getFont(&ok, grid_font, this);
    if(ok)
    {
        grid_font=fnt;
        redraw_grid();
    }
}

void DataViewer::redraw_grid()
{
    delete_grid();
    create_grid();
    canvas()->setAllChanged();
    canvas()->update();
}

void DataViewer::delete_grid()
{
    QCanvasItemList lst=canvas()->allItems();
    QCanvasItemList::iterator it=lst.begin(); 
    while(it!=lst.end())
    {
        if((*it)->rtti() == grid_line_rtti ||
            (*it)->rtti() == grid_line_text_rtti)
        {
            delete *it;
            it=lst.remove(it);
        } else
            it++;
    }
}

void DataViewer::create_grid()
{
    QCanvasLine *ruler1=new GridLine(canvas());
    ruler1->setPoints(draw_area.left(), draw_area.top(), 
        draw_area.left(), draw_area.bottom());
    ruler1->setPen(grid_pen);
    ruler1->setZ(1);
    ruler1->show();

    QCanvasLine *ruler2=new GridLine(canvas());
    ruler2->setPoints(draw_area.left(), draw_area.bottom(), 
        draw_area.right(), draw_area.bottom());
    ruler2->setPen(grid_pen);
    ruler2->setZ(1);
    ruler2->show();

    QCanvasText  *text0=new GridText(QString("0"), canvas());
    text0->setZ(2);
    text0->setFont(grid_font);
    text0->move(draw_area.left() - text0->boundingRect().width() -5, 
        draw_area.bottom() + 5);
    text0->show();


    for(int x=grid_step_x;x<=draw_area.width();x+=grid_step_x)
    {
        if(show_vert_grid)
        {
            QCanvasLine *ruler=new GridLine(canvas());
            ruler->setPoints(draw_area.left()+x, draw_area.top(), 
                draw_area.left()+x, draw_area.bottom());
            ruler->setPen(grid_pen);
            ruler->setZ(1);
            ruler->show();
        }
        QCanvasText  *text=new GridText(QString().sprintf("%d:%02d", x/60, x%60), canvas());
        text->setZ(2);
        text->setFont(grid_font);
        text->move(draw_area.left() + x - text->boundingRect().width()/2 , 
            draw_area.bottom() + 5);
        text->show();
    }

    for(int y=grid_step_y;y<=draw_area.height();y+=grid_step_y)
    {
        if(show_hor_grid)
        {
            QCanvasLine *ruler=new GridLine(canvas());
            ruler->setPoints(draw_area.left(), draw_area.bottom()-y, 
                draw_area.right(), draw_area.bottom()-y);
            ruler->setPen(grid_pen);
            ruler->setZ(1);
            ruler->show();
        }

        QCanvasText  *text=new GridText(QString().setNum(y), canvas());
        text->setZ(2);
        text->setFont(grid_font);
        text->move(draw_area.left() - 5 - text->boundingRect().width(), 
            draw_area.bottom() - y - text->boundingRect().height()/2);
        text->show();
    }
}

