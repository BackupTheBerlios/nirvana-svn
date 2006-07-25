//#include <gtk/gtk.h>
//#include <glib.h>

#include "KWQButtonGroup.h"
#include "KWQButton.h"

QButtonGroup::QButtonGroup(QWidget * parent, const char * name)
    :_parent(parent)
{
    
}

int QButtonGroup::insert(QButton* button, int id)
{
    if (button == NULL) return  -1;
/*
    GtkRadioButton *gtkbtn = GTK_RADIO_BUTTON (button->getGtkWidget());
    if (gtkbtn, -1);

    GSList *group = gtk_radio_button_get_group(GTK_RADIO_BUTTON (_parent->getGtkWidget()));
    gtk_radio_button_set_group(gtkbtn, group);
    GSList* pos = g_slist_find(group, gtkbtn);
    if (!pos) return -1;
    return g_slist_position(group, pos);
*/
    return -1;
}

void QButtonGroup::remove(QButton* button)
{
    if (!button) return;
    /*
    GtkRadioButton *btn = GTK_RADIO_BUTTON(button->getGtkWidget());
    if (!btn) return ;

    gtk_radio_button_set_group(btn, NULL);
    */
}
