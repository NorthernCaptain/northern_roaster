#ifndef MAINWIN_ADM_HXX
#define MAINWIN_ADM_HXX

#include <qmainwindow.h>
#include <qhbox.h>
#include <qspinbox.h>
#include <qaction.h>

#include "dataview.hxx"
#include "infowin/infowin.h"

const int max_graphs=2;
class NRMainWinAdm: public QMainWindow
{
    Q_OBJECT
protected:
    static int                 view_idx;

    DataViewer                 *view;

    QSpinBox                   *zoom_spin;

    QAction                    *grid_vert_a, *grid_hor_a;
    QActionGroup               *grid_v_grp;
    QAction                    *grid_v_a[5];
    QActionGroup               *grid_h_grp;
    QAction                    *grid_h_a[5];

    InfoWin                    *info_win[max_graphs];

    void                        create_menu();
    void                        create_toolbar();
    void                        load_config();

protected slots:
    void                        zoom_in_slot();
    void                        zoom_out_slot();
    void                        set_vert_grid_lines_slot(QAction* a);
    void                        set_hor_grid_lines_slot(QAction* a);
    void                        new_view_slot();

public:
    NRMainWinAdm(QWidget* parent=0, const char* name=0, WFlags f=0);
    ~NRMainWinAdm();
};

#endif
