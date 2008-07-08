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

#include "UnicornCommon.h"

#include "Logger.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QMap>
#include <QUrl>
#include <QProcess>

#ifdef WIN32
    #include <windows.h>
    #include <shlobj.h>
#endif

using namespace std;

namespace Unicorn
{


QString
md5( const QByteArray& src )
{
    QByteArray const digest = QCryptographicHash::hash( src, QCryptographicHash::Md5 );
    return QString::fromLatin1( digest.toHex() );
}


#if 0
//TODO move to QHttp clone
QString
QHttpStateToString( int state )
{
    switch ( state )
    {
        case QHttp::Unconnected: return QCoreApplication::translate( "WebService", "No connection." );
        case QHttp::HostLookup: return QCoreApplication::translate( "WebService", "Looking up host..." );
        case QHttp::Connecting: return QCoreApplication::translate( "WebService", "Connecting..." );
        case QHttp::Sending: return QCoreApplication::translate( "WebService", "Sending request..." );
        case QHttp::Reading: return QCoreApplication::translate( "WebService", "Downloading." );
        case QHttp::Connected: return QCoreApplication::translate( "WebService", "Connected." );
        case QHttp::Closing: return QCoreApplication::translate( "WebService", "Closing connection..." );
        default: return QString();
    }
}
#endif


QString
qtLanguageToLfmLangCode( QLocale::Language qtLang )
{
    switch ( qtLang )
    {
        case QLocale::English:    return "en";
        case QLocale::French:     return "fr";
        case QLocale::Italian:    return "it";
        case QLocale::German:     return "de";
        case QLocale::Spanish:    return "es";
        case QLocale::Portuguese: return "pt";
        case QLocale::Polish:     return "pl";
        case QLocale::Russian:    return "ru";
        case QLocale::Japanese:   return "jp";
        case QLocale::Chinese:    return "cn";
        case QLocale::Swedish:    return "sv";
        case QLocale::Turkish:    return "tr";
        default:                  return "en";
    }
}


QString
lfmLangCodeToIso639( const QString& code )
{
    if ( code == "jp" ) return "ja";
    if ( code == "cn" ) return "zh";

    return code;
}


QString
localizedHostName( const QString& code )
{
    if ( code == "en" ) return "www.last.fm"; //first as optimisation
    if ( code == "pt" ) return "www.lastfm.com.br";
    if ( code == "tr" ) return "www.lastfm.com.tr";
    if ( code == "cn" ) return "cn.last.fm";
    if ( code == "sv" ) return "www.lastfm.se";

    QStringList simple_hosts = QStringList()
            << "fr" << "it" << "de" << "es" << "pl"
            << "ru" << "jp" << "se";

    if ( simple_hosts.contains( code ) )
        return "www.lastfm." + code;

    // else default to english site
    return "www.last.fm";
}


void
parseQuotedStrings( const string& sCompound, vector<string>& separated )
{
    string sCopy(sCompound);

    string::size_type nIdxNext = 0;

    while (nIdxNext < sCopy.size())
    {
        string::size_type nIdxStart = sCopy.find_first_of('\"', nIdxNext);

        if (nIdxStart == string::npos)
        {
            // Not found
            return;
        }

        nIdxStart++;
        if (nIdxStart >= sCopy.size())
        {
            // Buggered string
            return;
        }

        string::size_type nIdxStop = nIdxStart;
        bool bStopFound = false;
        do
        {
            nIdxStop = sCopy.find_first_of('\"', nIdxStop);
            if (nIdxStop == string::npos)
            {
                // Buggered string
                return;
            }
            nIdxStop++;
            if (nIdxStop >= sCopy.size())
            {
                // True stop at end of string
                bStopFound = true;
                break;
            }

            // Check if escaped
            if (sCopy[nIdxStop] == '\"')
            {
                // Remove dupe
                sCopy.erase(nIdxStop, 1);
            }
            else
            {
                // Not escape, true stop
                bStopFound = true;
            }

        } while (!bStopFound);

        string sQuoted = sCopy.substr(nIdxStart, (nIdxStop - 1) - nIdxStart);
        separated.push_back(sQuoted);

        // Position next just after the closing "
        nIdxNext = nIdxStop;

    } // end while

}


void
trim( string& str )
{
    string::size_type pos1 = str.find_first_not_of(" \t\n\f\r");

    if (pos1 == string::npos)
    {
        return;
    }

    string::size_type pos2 = str.find_last_not_of(" \t");

    str = str.substr(pos1, pos2 - pos1 + 1);
}


void
stripBBCode( std::string& str )
{
    string::size_type nIdxNext = 0;

    while (nIdxNext < str.size())
    {
        string::size_type nIdxStart = str.find_first_of('[', nIdxNext);

        if (nIdxStart == string::npos)
        {
            // Not found
            return;
        }

        nIdxStart++;
        if (nIdxStart >= str.size())
        {
            // Buggered string
            return;
        }

        string::size_type nIdxStop = str.find_first_of(']', nIdxStart);
        if (nIdxStop == string::npos)
        {
            // Buggered string
            return;
        }

        // Remove BBCode section
        size_t numRemove = nIdxStop - nIdxStart + 2;
        str.erase(nIdxStart - 1, numRemove);

        nIdxNext = nIdxStop + 1 - numRemove;
    }

}


void
stripBBCode( QString& str )
{
    int nIdxNext = 0;

    while (nIdxNext < str.size())
    {
        int nIdxStart = str.indexOf('[', nIdxNext);

        if (nIdxStart == -1)
        {
            // Not found
            return;
        }

        nIdxStart++;
        if (nIdxStart >= str.size())
        {
            // Buggered string
            return;
        }

        int nIdxStop = str.indexOf(']', nIdxStart);
        if (nIdxStop == -1)
        {
            // Buggered string
            return;
        }

        // Remove BBCode section
        int numRemove = nIdxStop - nIdxStart + 2;
        str.remove(nIdxStart - 1, numRemove);

        nIdxNext = nIdxStop + 1 - numRemove;
    }
}


QString
urlEncodeItem( QString item )
{
    urlEncodeSpecialChars( item );
    item = QUrl::toPercentEncoding( item );

    return item;
}


QString
urlDecodeItem( QString item )
{
    item = QUrl::fromPercentEncoding( item.toLocal8Bit() );
    urlDecodeSpecialChars( item );

    return item;
}


QString&
urlEncodeSpecialChars(
    QString& str)
{
    str.replace( "&", "%26" );
    str.replace( "/", "%2F" );
    str.replace( ";", "%3B" );
    str.replace( "+", "%2B" );
    str.replace( "#", "%23" );

    return str;
}


QString&
urlDecodeSpecialChars(
    QString& str)
{
    str.replace( "%26", "&" );
    str.replace( "%2F", "/" );
    str.replace( "%3B", ";" );
    str.replace( "%2B", "+" );
    str.replace( "%23", "#" );
    str.replace( "+", " " );

    return str;
}


QStringList
sortCaseInsensitively( QStringList input )
{
    // This cumbersome bit of code here is how the Qt docs suggests you sort
    // a string list case-insensitively
    QMap<QString, QString> map;
    foreach (QString s, input)
        map.insert( s.toLower(), s );

    QStringList output;
    QMapIterator<QString, QString> i( map );
    while (i.hasNext())
        output += i.next().value();

    return output;
}


QString
applicationDataPath()
{
    QString path;

    #ifdef WIN32
        if ((QSysInfo::WindowsVersion & QSysInfo::WV_DOS_based) == 0)
        {
            // Use this for non-DOS-based Windowses
            char acPath[MAX_PATH];
            HRESULT h = SHGetFolderPathA( NULL, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE,
                                          NULL, 0, acPath );
            if ( h == S_OK )
            {
                path = QString::fromLocal8Bit( acPath );
            }
            else
            {
                path = "";
            }
        }

    #elif defined(Q_WS_MAC)
        path = Unicorn::applicationSupportFolderPath();

    #elif defined(Q_WS_X11)
        path = QDir::home().filePath( ".local/share" );

    #else
        path = QApplication::applicationDirPath();

    #endif

    QDir d( path );
    d.mkpath( path );

    return d.absolutePath();
}


QString
savePath( QString file )
{
    QString path;

    #ifdef WIN32
        path = Unicorn::applicationDataPath();
        if (path.isEmpty())
            path = QCoreApplication::applicationDirPath();
        else
            path += "/Last.fm/";
    #else
        path = Unicorn::applicationDataPath() + "/Last.fm";
    #endif

    QDir d( path );
    d.mkpath( path );

    return d.filePath( file );
}


QString
logPath( QString file )
{
    #ifndef Q_WS_MAC
        return savePath( file );
    #else
        QDir const d = QDir::home().filePath( "/Library/Logs/Last.fm" );
        return d.filePath( file );
    #endif
}


QString
verbosePlatformString()
{
    #ifdef Q_WS_WIN
    switch (QSysInfo::WindowsVersion)
    {
        case QSysInfo::WV_32s:        return "Windows 3.1 with Win32s";
        case QSysInfo::WV_95:         return "Windows 95";
        case QSysInfo::WV_98:         return "Windows 98";
        case QSysInfo::WV_Me:         return "Windows Me";
        case QSysInfo::WV_DOS_based:  return "MS-DOS-based Windows";

        case QSysInfo::WV_NT:         return "Windows NT";
        case QSysInfo::WV_2000:       return "Windows 2000";
        case QSysInfo::WV_XP:         return "Windows XP";
        case QSysInfo::WV_2003:       return "Windows Server 2003";
        case QSysInfo::WV_VISTA:      return "Windows Vista";
        case QSysInfo::WV_NT_based:   return "NT-based Windows";

        case QSysInfo::WV_CE:         return "Windows CE";
        case QSysInfo::WV_CENET:      return "Windows CE.NET";
        case QSysInfo::WV_CE_based:   return "CE-based Windows";

        default:                      return "Unknown";
    }
    #elif defined Q_WS_MAC
    switch (QSysInfo::MacintoshVersion)
    {
        case QSysInfo::MV_Unknown:    return "Unknown Mac";
        case QSysInfo::MV_9:          return "Mac OS 9";
        case QSysInfo::MV_10_0:       return "Mac OS X 10.0";
        case QSysInfo::MV_10_1:       return "Mac OS X 10.1";
        case QSysInfo::MV_10_2:       return "Mac OS X 10.2";
        case QSysInfo::MV_10_3:       return "Mac OS X 10.3";
        case QSysInfo::MV_10_4:       return "Mac OS X 10.4";
        case QSysInfo::MV_10_5:       return "Mac OS X 10.5";
        
        default:                      return "Unknown";
    }
    #else
    return "Unix";
    #endif
}


void
msleep( int ms )
{
  #ifdef WIN32
    Sleep( ms );
  #else
    ::usleep( ms * 1000 );
  #endif
}

QString
runCommand( QString cmd )
{
    QProcess process;

    process.start( cmd );
    process.closeWriteChannel();
    process.waitForFinished();

    return QString( process.readAll() );
}

QString 
systemInformation()
{
    QString information;
    
    information += "Operating system: " + Unicorn::verbosePlatformString() + "\n\n";

#ifdef Q_WS_X11
    information += "CPU: \n";
    information += runCommand( "cat /proc/cpuinfo" );
    information += "\n";
    
    information += "Memory: \n";
    information += runCommand( "cat /proc/meminfo" );
    information += "\n";
    
    information += "Diskspace: \n";
    information += runCommand( "df -h" );
    information += "\n";

#elif defined WIN32
    // CPU
    SYSTEM_INFO siSysInfo;
    GetSystemInfo(&siSysInfo); 

    information += "CPU: \n";  
    information += "Number of processors: " + QString::number( siSysInfo.dwNumberOfProcessors ) + "\n";
    information += "Page size: " + QString::number( siSysInfo.dwPageSize ) + "\n";
    information += "Processor type: " + QString::number( siSysInfo.dwProcessorType ) + "\n";
    information += "Active processor mask: " + QString::number( siSysInfo.dwActiveProcessorMask ) + "\n";
    information += "\n";

    // Memory
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof (statex);
    GlobalMemoryStatusEx (&statex);

    information += "Memory used: " + QString::number( statex.dwMemoryLoad ) + "%\n";
    information += "Total memory: " + QString::number( statex.ullTotalPhys/(1024*1024) ) + "MB\n";
    information += "Free memory: " + QString::number( statex.ullAvailPhys/(1024*1024) ) + "MB\n";
    information += "Total virtual memory: " + QString::number( statex.ullTotalVirtual/(1024*1024) ) + "MB\n";
    information += "Free virtual memory: " + QString::number( statex.ullAvailVirtual/(1024*1024) ) + "MB\n";

    // Disk space
    __int64 lpFreeBytesAvailable, lpTotalNumberOfBytes, lpTotalNumberOfFreeBytes;
    DWORD dwSectPerClust, dwBytesPerSect, dwFreeClusters, dwTotalClusters;

    GetDiskFreeSpaceEx( NULL,
                        (PULARGE_INTEGER)&lpFreeBytesAvailable,
                        (PULARGE_INTEGER)&lpTotalNumberOfBytes,
                        (PULARGE_INTEGER)&lpTotalNumberOfFreeBytes
                        ); 

    information += "Drive C:\\ \n";
    information += "   Total diskspace: " + QString::number( lpTotalNumberOfBytes/(1024*1024) )+ "MB\n";
    information += "   Free diskspace: " + QString::number( lpFreeBytesAvailable/(1024*1024) )  + "MB\n";

#elif defined Q_WS_MAC
    information += "CPU and Memory: \n";
    information += Unicorn::runCommand( "hostinfo" );
    information += "\n";
    
    information += "Diskspace: \n";
    information += Unicorn::runCommand( "df -h" );
    information += "\n";

#endif

    return information;
}

} // namespace UnicornUtils
