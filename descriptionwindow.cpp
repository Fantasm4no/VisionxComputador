#include "descriptionwindow.h"
#include "ui_descriptionwindow.h"

descriptionwindow::descriptionwindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::descriptionwindow)
{
    ui->setupUi(this);
}

descriptionwindow::~descriptionwindow()
{
    delete ui;
}

void descriptionwindow::setDescription(const QString &text) {
    ui->labelDescripcion->setText(text);
}
