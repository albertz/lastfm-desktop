/*
   Copyright 2005-2009 Last.fm Ltd. 
      - Primarily authored by Max Howell, Jono Cole and Doug Mansell

   This file is part of the Last.fm Desktop Application Suite.

   lastfm-desktop is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   lastfm-desktop is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with lastfm-desktop.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "UnicornMainWindow.h"
#include "UnicornCoreApplication.h"
#include "widgets/AboutDialog.h"
#include "widgets/UpdateDialog.h"
#include "UnicornSettings.h"
#include <lastfm/AuthenticatedUser>
#include <QDesktopServices>
#include <QMenuBar>
#include <QShortcut>


#define SETTINGS_POSITION_KEY "MainWindowPosition"


unicorn::MainWindow::MainWindow()
{
    new QShortcut( QKeySequence(Qt::CTRL+Qt::Key_W), this, SLOT(close()) );
    new QShortcut( QKeySequence(Qt::ALT+Qt::SHIFT+Qt::Key_L), this, SLOT(openLog()) );
    connect( qApp, SIGNAL(userGotInfo( QNetworkReply* )), SLOT(onUserGotInfo( QNetworkReply* )) );

    QVariant v = unicorn::UserSettings().value( SETTINGS_POSITION_KEY );
    if (v.isValid()) move( v.toPoint() ); //if null, let Qt decide
}


unicorn::MainWindow::~MainWindow()
{
    unicorn::UserSettings().setValue( SETTINGS_POSITION_KEY, pos() );
}


void
unicorn::MainWindow::finishUi()
{
    ui.account = menuBar()->addMenu( AuthenticatedUser().name() );
    ui.profile = ui.account->addAction( tr("Visit &Profile"), this, SLOT(visitProfile()) );
    ui.account->addSeparator();
    ui.account->addAction( tr("Log &Out && Quit"), qApp, SLOT(logout()) );
#ifndef __APPLE__
    ui.account->addAction( tr("&Quit"), qApp, SLOT(quit()) );
#endif

    menuBar()->insertMenu( menuBar()->actions().first(), ui.account );
    QMenu* help = menuBar()->addMenu( tr("Help") );
    QAction* about = help->addAction( tr("About"), this, SLOT(about()) );
    QAction* c4u = help->addAction( tr("Check for Updates"), this, SLOT(checkForUpdates()) );
#ifdef __APPLE__
    about->setMenuRole( QAction::AboutRole );
    c4u->setMenuRole( QAction::ApplicationSpecificRole );
#endif

    ui.update = new UpdateDialog( this );
}


void
unicorn::MainWindow::onUserGotInfo( QNetworkReply* reply )
{
    QString const text = AuthenticatedUser::getInfoString( reply );
    if (text.size() && ui.account) {
        QAction* a = ui.account->addAction( text );
        a->setEnabled( false );
        ui.account->insertAction( ui.profile, a );
    }
}


void
unicorn::MainWindow::visitProfile()
{
    QDesktopServices::openUrl( AuthenticatedUser().www() );
}


void
unicorn::MainWindow::about()
{
    if (!ui.about) ui.about = new AboutDialog( this );
    ui.about.show();
}


void
unicorn::MainWindow::checkForUpdates()
{
    if (!ui.update) ui.update = new UpdateDialog( this );
    ui.update.show();
}


void
unicorn::MainWindow::openLog()
{
    QDesktopServices::openUrl( QUrl::fromLocalFile( unicorn::CoreApplication::log().absoluteFilePath() ) );    
}
