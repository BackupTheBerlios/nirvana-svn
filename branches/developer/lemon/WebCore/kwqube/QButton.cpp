/*
    Copyright (c) 2006 Maxim Sokhatsky (maxim@sokhatsky.com)
*/

#include <SupportKit.h>
#include "KWQButton.h"
#include "KWQAssertions.h"

QButton::QButton(QWidget * parent, const char * name, int f) :
    QWidget(parent,name,f),
    m_clicked(this,SIGNAL(clicked())),
    m_group(this)
{
}

QButton::~QButton() { }

void QButton::setText(const QString &s)
{
    QCString cstr = s.utf8();
    BControl *w = static_cast<BControl*>(getBView());
    if (w != NULL) {
	w->SetLabel(static_cast<const char*>(cstr));
    }
}

QString QButton::text()
{
    const gchar* cstr = NULL;
    BControl *w = static_cast<BControl*>(getBView());
    if (w != NULL) cstr = w->Label();
    if (cstr != NULL) return QString::fromUtf8(cstr);
    else return QString();
}

void QButton::clicked()
{
    // Order of signals is:
    //   1) signals in subclasses (stateChanged, not sure if there are any others)
    //   2) mouse up
    //   3) clicked
    // Proper behavior of check boxes, at least, depends on this order.
    // [above is WebCore-specific comment, KWIQ status unknown]
    
    m_clicked.call();
}

void QButton::click()
{
/*
    GtkButton *b = GTK_BUTTON( getGtkWidget() );
    ASSERT(b);
    gtk_button_clicked(b);
*/
    // SendMessage CLICK
}

void QButton::setFont(const QFont &f)
{
    QWidget::setFont(f);
}

/*
void QButton::setWritingDirection(QPainter::TextDirection d)
{
    QWidget::setWritingDirection(d);
}
*/

QWidget::FocusPolicy QButton::focusPolicy()
{
    return QWidget::focusPolicy();
}  

void QButton::MessageReceived(BMessage* message)
{
/*
    switch (message->what) {
        case SELECTION_MSG:
	    click();
	    break;
	default:
	    BView::MessageReceived(message);
	    break;
    }
*/
}
