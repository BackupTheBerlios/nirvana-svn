/*
    Copyright (c) 2006 Maxim Sokhatsky (maxim@sokhatsky.com)
*/

#ifndef QBUTTONGROUP_H
#define QBUTTONGROUP_H

class QButton;
class QWidget;

class QButtonGroup {
public:
    QButtonGroup(QWidget * parent = 0, const char * name = 0);
    int insert(QButton* button, int id=-1);
    void remove(QButton* button);
private:
    QWidget *_parent;
};

#endif
