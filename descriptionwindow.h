#ifndef DESCRIPTIONWINDOW_H
#define DESCRIPTIONWINDOW_H

#include <QDialog>

namespace Ui {
class descriptionwindow;
}

class descriptionwindow : public QDialog
{
    Q_OBJECT

public:
    explicit descriptionwindow(QWidget *parent = nullptr);
    ~descriptionwindow();

    void setDescription(const QString &text);

private:
    Ui::descriptionwindow *ui;
};

#endif // DESCRIPTIONWINDOW_H
