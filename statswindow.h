#ifndef STATSWINDOW_H
#define STATSWINDOW_H

#include <QDialog>

namespace Ui {
class StatsWindow;
}

class StatsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit StatsWindow(QWidget *parent = nullptr);
    ~StatsWindow();

private:
    Ui::StatsWindow *ui;
};

#endif // STATSWINDOW_H
