/***************************************************************************
 *   Copyright 2005-2008 Last.fm Ltd.                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.          *
 ***************************************************************************/

#ifndef LASTFM_ARTIST_H
#define LASTFM_ARTIST_H

#include <lastfm/DllExportMacro.h>
#include <lastfm/core/WeightedStringList.h>
#include <QString>
#include <QUrl>
class WsReply;


class LASTFM_TYPES_DLLEXPORT Artist
{
    QString m_name;

public:
    Artist()
    {}

    Artist( const QString& name ) : m_name( name )
    {}

    bool isNull() const { return m_name.isEmpty(); }
    
	/** the url for this artist's page at www.last.fm */
	QUrl www() const;
    
    QUrl smallImageUrl(){ return m_smallImage; }
    QUrl imageUrl(){ return m_image; }
	
	bool operator==( const Artist& that ) const { return m_name == that.m_name; }
	bool operator!=( const Artist& that ) const { return m_name != that.m_name; }
	
    operator QString() const 
    {
        /** if no artist name is set, return the musicbrainz unknown identifier
          * in case some part of the GUI tries to display it anyway. Note isNull
          * returns false still. So you should have queried this! */
        return m_name.isEmpty() ? "[unknown]" : m_name; 
    }

    WsReply* share( const class User& recipient, const QString& message = "" );
	WsReply* getInfo() const;	
	WsReply* getSimilar() const;
	static WeightedStringList getSimilar( WsReply* );
    
    /** use Tag::list to get the tag list out of the finished reply */
    WsReply* getTags() const;
    WsReply* getTopTags() const;
    
    /** Last.fm dictates that you may submit at most 10 of these */
    WsReply* addTags( const QStringList& ) const;
	
	WsReply* search( int limit = -1 ) const;
	static QList<Artist> list( WsReply* );
    
private:
	QUrl m_smallImage;
	QUrl m_image;
};

#endif
