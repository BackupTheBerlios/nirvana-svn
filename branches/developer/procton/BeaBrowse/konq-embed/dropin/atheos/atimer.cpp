
#include <qtimer.h>
#include "amutex.h"
#include <Looper.h>
#include <MessageRunner.h>
#include <Autolock.h>

#include <kdebug.h>

static BLooper* g_pcTimerLooper = NULL;

//#define g_pcTimerLooper (GlobalMutex::GetCurLooper())

const int TIMER_MSG = 'timM';



QTimer::QTimer( QObject *parent, const char *name ) : QObject( parent, name ), BHandler()
	,mRunner( 0 )
{
	kdDebug(10) << "QTimer::" << __FUNCTION__ << "()" << endl;
    if ( g_pcTimerLooper == NULL ) {
		g_pcTimerLooper = new BLooper( "khtml_timer_looper" );
		g_pcTimerLooper->Run();
    }
    m_bIsActive = false;
    m_bDeleteWhenTrigged = false;
    m_bSingleShot        = false;
		
		g_pcTimerLooper->Lock();
    g_pcTimerLooper->AddHandler( this );
		g_pcTimerLooper->Unlock();
}
    
QTimer::~QTimer()
{
	kdDebug(10) << "QTimer::" << __FUNCTION__ << "()" << endl;
    delete mRunner;
}

void QTimer::MessageReceived(BMessage *aMsg)
{
	BAutolock al( GlobalMutex::GetMutex() );
	kdDebug(10) << "QTimer::" << __FUNCTION__ << "()" << endl;

	switch( aMsg->what ) {
	case TIMER_MSG:
	    emit timeout();
	    if ( m_bSingleShot ) {
			m_bIsActive = false;
	    }
	    if ( m_bDeleteWhenTrigged ) {
			delete this;
	    }
		break;
	
	default:
		BHandler::MessageReceived( aMsg );
	}
}

bool QTimer::isActive() const
{
	BAutolock al( GlobalMutex::GetMutex() );
	kdDebug(10) << "QTimer::" << __FUNCTION__ << "()" << endl;
    return( m_bIsActive );
}

int QTimer::start( int msec, bool sshot )
{
	BAutolock al( GlobalMutex::GetMutex() );
	kdDebug(10) << "QTimer::" << __FUNCTION__ << "()" << endl;
    m_bIsActive = true;
    m_bSingleShot = sshot;

	if( m_bSingleShot )
		mRunner = new BMessageRunner( BMessenger( this ), new BMessage( TIMER_MSG ), bigtime_t(msec) * 1000LL , 1 );
	else
		mRunner = new BMessageRunner( BMessenger( this ), new BMessage( TIMER_MSG ), bigtime_t(msec) * 1000LL );	

	if( mRunner->InitCheck() != B_OK ) {
	    m_bIsActive = false;
	    delete mRunner;
	    mRunner = 0;
	}
    return( 0 );
}

void QTimer::changeInterval( int msec )
{
	BAutolock al( GlobalMutex::GetMutex() );
	kdDebug(10) << "QTimer::" << __FUNCTION__ << "()" << endl;

    if ( m_bIsActive ) {
	    delete mRunner;
    }
	if( m_bSingleShot )
		mRunner = new BMessageRunner( BMessenger( this ), new BMessage( TIMER_MSG ), bigtime_t(msec) * 1000LL , 1 );
	else
		mRunner = new BMessageRunner( BMessenger( this ), new BMessage( TIMER_MSG ), bigtime_t(msec) * 1000LL );	
    m_bIsActive = true;
}

void QTimer::stop()
{
	BAutolock al( GlobalMutex::GetMutex() );
	kdDebug(10) << "QTimer::" << __FUNCTION__ << "()" << endl;

    if ( m_bIsActive ) {
		m_bIsActive = false;
		delete mRunner;
		mRunner = 0;
    }
}

void QTimer::singleShot( int msec, QObject *receiver, const char *member )
{
	kdDebug(10) << "QTimer::" << __FUNCTION__ << "()" << endl;
    QTimer* pcTimer = new QTimer();
    pcTimer->m_bDeleteWhenTrigged = true;    
//    pcTimer->Connect( receiver, ID_TIMEOUT, nEvent );
    receiver->connect( pcTimer, SIGNAL( timeout() ), receiver, member );
    pcTimer->start( msec, true );
}

#include "atimer.moc"

#if 0
//static os::Looper* g_pcTimerLooper = NULL;

#define g_pcTimerLooper (GlobalMutex::GetCurLooper())

enum { ID_TIMER };



QTimer::QTimer( QObject *parent, const char *name ) : QObject( parent, name ), BHandler( "timer" )
{
//    if ( g_pcTimerLooper == NULL ) {
//	g_pcTimerLooper = new os::Looper( "khtml_timer_looper" );
//	g_pcTimerLooper->Run();
//    }
    m_bIsActive = false;
    m_bDeleteWhenTrigged = false;
    m_bSingleShot        = false;
    g_pcTimerLooper->AddHandler( this );
}
    
QTimer::~QTimer()
{
    if ( m_bIsActive ) {
		g_pcTimerLooper->RemoveTimer( this, ID_TIMER );
    }
}

void QTimer::TimerTick( int /*nID*/ )
{
//    GetLooper()->Unlock();
//    GlobalMutex::Lock();
//    GetLooper()->Lock();
    GlobalMutex::PushLooper( Looper() );
    
    emit timeout();
    if ( m_bSingleShot ) {
		m_bIsActive = false;
    }
    if ( m_bDeleteWhenTrigged ) {
		delete this;
    }
    GlobalMutex::PopLooper();
//    GlobalMutex::Unlock();
}

bool QTimer::isActive() const
{
    return( m_bIsActive );
}

int QTimer::start( int msec, bool sshot )
{
    m_bIsActive = true;
    m_bSingleShot = sshot;
//    g_pcTimerLooper->Lock();
//    GlobalMutex::Lock();
    g_pcTimerLooper->AddTimer( this, ID_TIMER, bigtime_t(msec) * 1000LL, sshot );
//    GlobalMutex::Unlock();
//    g_pcTimerLooper->Unlock();
    return( 0 );
}

void QTimer::changeInterval( int msec )
{
//    g_pcTimerLooper->Lock();
//    GlobalMutex::Lock();
    if ( m_bIsActive ) {
		g_pcTimerLooper->RemoveTimer( this, ID_TIMER );
    }
    g_pcTimerLooper->AddTimer( this, ID_TIMER, bigtime_t(msec) * 1000LL, m_bSingleShot );
    m_bIsActive = true;
//    g_pcTimerLooper->Unlock();
//    GlobalMutex::Unlock();
}

void QTimer::stop()
{
//    GlobalMutex::Lock();
//    g_pcTimerLooper->Lock();
    if ( m_bIsActive ) {
	g_pcTimerLooper->RemoveTimer( this, ID_TIMER );
	m_bIsActive = false;
    }
//    g_pcTimerLooper->Unlock();
//    GlobalMutex::Unlock();
}

void QTimer::singleShot( int msec, QObject *receiver, const char *member )
{
    QTimer* pcTimer = new QTimer();
    pcTimer->m_bDeleteWhenTrigged = true;    
//    pcTimer->Connect( receiver, ID_TIMEOUT, nEvent );
    receiver->connect( pcTimer, SIGNAL( timeout() ), receiver, member );
    pcTimer->start( msec, true );
}

#include "atimer.moc"
#endif
