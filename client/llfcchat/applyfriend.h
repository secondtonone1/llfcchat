#ifndef APPLYFRIEND_H
#define APPLYFRIEND_H

#include <QDialog>
#include "clickedlabel.h"
#include "friendlabel.h"

namespace Ui {
class ApplyFriend;
}

class ApplyFriend : public QDialog
{
    Q_OBJECT

public:
    explicit ApplyFriend(QWidget *parent = nullptr);
    ~ApplyFriend();
    void InitTipLbs();
    void AddTipLbs(ClickedLabel*, QPoint cur_point, QPoint &next_point, int text_width, int text_height);

private:
    void resetLabels();
    Ui::ApplyFriend *ui;
    QMap<QString, ClickedLabel*> _add_labels;
    std::vector<QString> _add_label_keys;
    QPoint _label_point;
    QMap<QString, FriendLabel*> _friend_labels;
    std::vector<QString> _friend_label_keys;
    void addLabel(QString name);
    std::vector<QString> _tip_data;
    QPoint _tip_cur_point;
public slots:
    //��ʾ����label��ǩ
    void ShowMoreLabel();
    //����label���»س���������ǩ����չʾ��
    void SlotLabelEnter();
    //�Ƴ�չʾ�����ѱ�ǩ
    void SlotRemoveFriendLabel(QString);
    //ͨ�����tipʵ�����Ӻͼ��ٺ��ѱ�ǩ
    void SlotChangeFriendLabelByTip(QString, ClickLbState);
    //������ı��仯��ʾ��ͬ��ʾ
    void SlotLabelTextChange(const QString& text);
    //������������
    void SlotLabelEditFinished();
   //�����ʾ����Ӻ��ѱ�ǩ
    void SlotAddFirendLabelByClickTip(QString text);
};

#endif // APPLYFRIEND_H
