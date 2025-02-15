#-------------------------------------------------
#
# Project created by QtCreator 2024-02-27T14:01:20
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = llfcchat
TEMPLATE = app
RC_ICONS = icon.ico
DESTDIR = ./bin
# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        BubbleFrame.cpp \
        ChatItemBase.cpp \
        ChatView.cpp \
        MessageTextEdit.cpp \
        PictureBubble.cpp \
        TextBubble.cpp \
        adduseritem.cpp \
        applyfriend.cpp \
        applyfrienditem.cpp \
        applyfriendlist.cpp \
        applyfriendpage.cpp \
        authenfriend.cpp \
        chatdialog.cpp \
        chatpage.cpp \
        chatuserlist.cpp \
        chatuserwid.cpp \
        clickedbtn.cpp \
        clickedlabel.cpp \
        clickedoncelabel.cpp \
        contactuserlist.cpp \
        conuseritem.cpp \
        customizeedit.cpp \
        customizetextedit.cpp \
        findfaildlg.cpp \
        findsuccessdlg.cpp \
        friendinfopage.cpp \
        friendlabel.cpp \
        global.cpp \
        grouptipitem.cpp \
        httpmgr.cpp \
        invaliditem.cpp \
        lineitem.cpp \
        listitembase.cpp \
        loadingdlg.cpp \
        logindialog.cpp \
        main.cpp \
        mainwindow.cpp \
        registerdialog.cpp \
        resetdialog.cpp \
        searchlist.cpp \
        statelabel.cpp \
        statewidget.cpp \
        tcpmgr.cpp \
        timerbtn.cpp \
        userdata.cpp \
        usermgr.cpp

HEADERS += \
        BubbleFrame.h \
        ChatItemBase.h \
        ChatView.h \
        MessageTextEdit.h \
        PictureBubble.h \
        TextBubble.h \
        adduseritem.h \
        applyfriend.h \
        applyfrienditem.h \
        applyfriendlist.h \
        applyfriendpage.h \
        authenfriend.h \
        chatdialog.h \
        chatpage.h \
        chatuserlist.h \
        chatuserwid.h \
        clickedbtn.h \
        clickedlabel.h \
        clickedoncelabel.h \
        contactuserlist.h \
        conuseritem.h \
        customizeedit.h \
        customizetextedit.h \
        findfaildlg.h \
        findsuccessdlg.h \
        friendinfopage.h \
        friendlabel.h \
        global.h \
        grouptipitem.h \
        httpmgr.h \
        invaliditem.h \
        lineitem.h \
        listitembase.h \
        loadingdlg.h \
        logindialog.h \
        mainwindow.h \
        registerdialog.h \
        resetdialog.h \
        searchlist.h \
        singleton.h \
        statelabel.h \
        statewidget.h \
        tcpmgr.h \
        timerbtn.h \
        userdata.h \
        usermgr.h

FORMS += \
        adduseritem.ui \
        applyfriend.ui \
        applyfrienditem.ui \
        applyfriendpage.ui \
        authenfriend.ui \
        chatdialog.ui \
        chatpage.ui \
        chatuserwid.ui \
        conuseritem.ui \
        findfaildlg.ui \
        findsuccessdlg.ui \
        friendinfopage.ui \
        friendlabel.ui \
        grouptipitem.ui \
        lineitem.ui \
        loadingdlg.ui \
        logindialog.ui \
        mainwindow.ui \
        registerdialog.ui \
        resetdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    rc.qrc

DISTFILES += \
    config.ini \
    res/add_friend.png \
    res/add_friend_hover.png \
    res/add_friend_normal.png \
    res/addtip.png \
    res/arowdown.png \
    res/chat_icon.png \
    res/chat_icon_hover.png \
    res/chat_icon_press.png \
    res/chat_icon_select_hover.png \
    res/chat_icon_select_press.png \
    res/close_search.png \
    res/close_transparent.png \
    res/contact_list.png \
    res/contact_list_hover.png \
    res/contact_list_press.png \
    res/female.png \
    res/filedir.png \
    res/filedir_hover.png \
    res/filedir_press.png \
    res/head_1.jpg \
    res/head_2.jpg \
    res/head_3.jpg \
    res/head_4.jpg \
    res/head_5.jpg \
    res/ice.png \
    res/loading.gif \
    res/male.png \
    res/msg_chat_hover.png \
    res/msg_chat_normal.png \
    res/msg_chat_press.png \
    res/red_point.png \
    res/right_tip.png \
    res/search.png \
    res/smile.png \
    res/smile_hover.png \
    res/smile_press.png \
    res/tipclose.png \
    res/unvisible.png \
    res/unvisible_hover.png \
    res/video_chat_hover.png \
    res/video_chat_normal.png \
    res/video_chat_press.png \
    res/visible.png \
    res/visible_hover.png \
    res/voice_chat_hover.png \
    res/voice_chat_normal.png \
    res/voice_chat_press.png

CONFIG(debug, debug|release) {
        #debug
    message("debug mode")
    #指定要拷贝的文件目录为工程目录下release目录下的所有dll、lib文件，例如工程目录在D:\QT\Test
    #PWD就为D:/QT/Test，DllFile = D:/QT/Test/release/*.dll
    TargetConfig = $${PWD}/config.ini
    #将输入目录中的"/"替换为"\"
    TargetConfig = $$replace(TargetConfig, /, \\)
    #将输出目录中的"/"替换为"\"
    OutputDir =  $${OUT_PWD}/$${DESTDIR}
    OutputDir = $$replace(OutputDir, /, \\)
    //执行copy命令
    QMAKE_POST_LINK += copy /Y \"$$TargetConfig\" \"$$OutputDir\" &

    # 首先，定义static文件夹的路径
    StaticDir = $${PWD}/static
    # 将路径中的"/"替换为"\"
    StaticDir = $$replace(StaticDir, /, \\)
    #message($${StaticDir})
    # 使用xcopy命令拷贝文件夹，/E表示拷贝子目录及其内容，包括空目录。/I表示如果目标不存在则创建目录。/Y表示覆盖现有文件而不提示。
     QMAKE_POST_LINK += xcopy /Y /E /I \"$$StaticDir\" \"$$OutputDir\\static\\\"
}else{
      #release
    message("release mode")
    #指定要拷贝的文件目录为工程目录下release目录下的所有dll、lib文件，例如工程目录在D:\QT\Test
    #PWD就为D:/QT/Test，DllFile = D:/QT/Test/release/*.dll
    TargetConfig = $${PWD}/config.ini
    #将输入目录中的"/"替换为"\"
    TargetConfig = $$replace(TargetConfig, /, \\)
    #将输出目录中的"/"替换为"\"
    OutputDir =  $${OUT_PWD}/$${DESTDIR}
    OutputDir = $$replace(OutputDir, /, \\)
    //执行copy命令
    QMAKE_POST_LINK += copy /Y \"$$TargetConfig\" \"$$OutputDir\"

    # 首先，定义static文件夹的路径
    StaticDir = $${PWD}/static
    # 将路径中的"/"替换为"\"
    StaticDir = $$replace(StaticDir, /, \\)
    #message($${StaticDir})
    # 使用xcopy命令拷贝文件夹，/E表示拷贝子目录及其内容，包括空目录。/I表示如果目标不存在则创建目录。/Y表示覆盖现有文件而不提示。
     QMAKE_POST_LINK += xcopy /Y /E /I \"$$StaticDir\" \"$$OutputDir\\static\\\"
}

win32-msvc*:QMAKE_CXXFLAGS += /wd"4819" /utf-8
