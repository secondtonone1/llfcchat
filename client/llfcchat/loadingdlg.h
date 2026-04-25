#ifndef LOADINGDLG_H
#define LOADINGDLG_H

#include <QDialog>

namespace Ui {
class LoadingDlg;
}

class LoadingDlg : public QDialog
{
    Q_OBJECT

public:
    explicit LoadingDlg(QWidget *parent = nullptr);
    ~LoadingDlg();

private:
    Ui::LoadingDlg *ui;
};

#endif // LOADINGDLG_H
