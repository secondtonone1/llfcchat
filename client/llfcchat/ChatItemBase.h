#ifndef CHATITEMBASE_H
#define CHATITEMBASE_H

#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include "global.h"
class BubbleFrame;

class ChatItemBase : public QWidget
{
    Q_OBJECT
public:
    explicit ChatItemBase(ChatRole role, QWidget *parent = nullptr);
    void setUserName(const QString &name);
    void setUserIcon(const QPixmap &icon);
    void setWidget(QWidget *w);
    void setStatus(int status);
    QLabel* getIconLabel();
private:
    ChatRole m_role;
    QLabel *m_pNameLabel;
    QLabel *m_pIconLabel;
    QWidget *m_pBubble;
    QLabel * m_pStatusLabel;
};

#endif // CHATITEMBASE_H
