#ifndef KWQBUTTONGROUP_H_
#define KWQBUTTONGROUP_H_

class QButton;
class QWidget;

class QButtonGroup
{
 public:
  QButtonGroup(QWidget * parent = 0, const char * name = 0);
  int insert(QButton* button, int id=-1);
  void remove(QButton* button);
 private:
#ifdef KWIQ
  QWidget *_parent;
#endif
};

#endif
