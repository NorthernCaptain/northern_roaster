#ifndef DATAVIEW_HXX
#define DATAVIEW_HXX
#include <qcanvas.h>
#include "debug.hxx"
#include "dataset.hxx"

const int  zoom_def_val=100;
const int  zoom_factor=10;

class DataGraph
{
    DataSet*            dataset;
    QPen                draw_pen;
    size_t              last_idx;
public:
    DataGraph(DataSet *data_): dataset(data_) { draw_pen=QPen(Qt::green, 2); last_idx=0;};
    QPen&               pen()  { return draw_pen;};
    DataSet*            data() { return dataset;};
    size_t              get_last_idx() { return last_idx;};
    void                set_last_idx(size_t idx_) { last_idx=idx_;};
};

typedef std::vector<DataGraph*>         Graphs;
typedef Graphs::iterator                GraphsIt;

class DataViewer: public QCanvasView
{
    Q_OBJECT

    int                  zoom_value;

    QRect                draw_area;
    QPen                 grid_pen;
    QFont                grid_font;
    int                  grid_step_x, grid_step_y;

    bool                 show_vert_grid;
    bool                 show_hor_grid;

    int                  id;
    
    Graphs               graph_lst;

protected:
    void                 contentsWheelEvent(QWheelEvent *e);
    void                 customEvent(QCustomEvent* e);

    void                 create_grid();
    void                 delete_grid();
    void                 correct_canvas_size();
    void                 draw_graphs();
    void                 draw_graph(DataGraph*);

    void                 test_graphs();

public:
    DataViewer(int id_, QWidget* parent=0, const char* wname=0);
    ~DataViewer();

    int                  get_zoom() { return zoom_value;};
    void                 set_grid_lines(int ix, int iy);
    QFont                get_grid_font() const { return grid_font;};
    void                 set_grid_font(const QFont& fnt) { grid_font=fnt;};

    void                 redraw_grid();

signals:

    void                 wheel_up_sig();
    void                 wheel_down_sig();

public slots:
    void                 set_zoom_slot(int zoom_value);
    void                 show_vert_grid_slot(bool v);
    void                 show_hor_grid_slot(bool v);
    void                 change_grid_font_slot();
};

#endif
