#ifndef USERINFOPAGE_H
#define USERINFOPAGE_H

#include <QWidget>

namespace Ui {
class UserInfoPage;
}

class UserInfoPage : public QWidget
{
    Q_OBJECT

public:
    explicit UserInfoPage(QWidget *parent = nullptr);
    ~UserInfoPage();
protected:
private slots:
    void slot_up_load();

private:
    Ui::UserInfoPage *ui;
};

#endif // USERINFOPAGE_H
