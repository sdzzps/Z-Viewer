#pragma once

#include <QObject>
#include <QtQml>
#include <vector>
#include <QString>
#include <QDebug> //only for debug and testing purposes
#include <QAbstractListModel>
#include <QSortFilterProxyModel>
#include <memory>

#include "getExif.h"

//platform headers
#if defined(Q_OS_WIN)
    #include "windowsShellThumbProvider.h"
#elif defined(Q_OS_MAC)
    #include "macThumbProvider.h"
#else

#endif


/*
 * This file contains the sub-classes of AbstractListModel and other data
 * structures used by QML frontend as data sources. They are dependent on
 * exiftool.exe pipeline and ExifModel class in getExif.h, and operating
 * system specific Thumbnail pipeline (windowsShellThumbProvider.h for
 * Windows x86 system).
 *
 * Backend class in backend.h is dependent on this file.
 *
 * This file contains the struct ExifFileInfo for storing info and items
 * of each imported file. All ExifFileInfo objects are placed in vector in
 * the Backend object and accessed with an int index.
 * This file also contains the fileListModel for loading thumbnail preview.
*/

/*
//Data structure topology
Backend: communication interface to QML frontend (calls exiftool pipeline on file import)
    ├ ExifList: vector of all exifFileInfo items
    │     ├ ExifFileInfo: stores all objects of a file (constructed from exiftool pipeline)
    │     ...   ├ ExifModel: all exiftool data for searching (constructed from exiftool pipeline)
    │           └ ExifGroupsModel: Exif info groups and related info (constructed from ExifModel)
    │               ├ EntryListModel: subset of ExifModel by group for display (constructed from ExifModel)
    │               ...
    │
    ├ FileListModel: all files for thumbnail display
    │     ├ FileItem: all thumbnail info and image path of a file
    │     ...
    └ ExifProxyModel： filters current ExifModel data on search queries and returns result
*/

/*
ExifProxyModel: A proxy model class to handle keyword search queries and result filtering. It is stored as 
an object of ExifFileInfo. When Backend receives search query from QML frontend, the keyword in here is
updated. It then filters the matching result from ExifModel object. The search result page in QML uses this
class object as data source. 
*/

class ExifProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public: 
    enum class SearchField {
        Tag, 
        Value
    };

    explicit ExifProxyModel(QObject* parent = nullptr);

    void setKeyword(const QString& keyword);
    QString keyword() const { return m_keyword; }

    void setSearchField(SearchField field);
    SearchField searchField() const { return m_field; }
    
    void setCaseSensitivity(Qt::CaseSensitivity cs);
    Qt::CaseSensitivity caseSensitivity() const { return m_caseSensitivity; }

    void resetFilter() {beginFilterChange(); endFilterChange();} //allow backend to reset filter

protected: 
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override; 

private: 
    bool matches(const QString& haystack) const;

private: 
    QString m_keyword;
    SearchField m_field = SearchField::Tag;
    Qt::CaseSensitivity m_caseSensitivity = Qt::CaseInsensitive;
};


/*
EntryListModel: subset of ExifModel. Initialize on file loading. Stores as members of
ExifGroupsModel.
*/
class EntryListModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        TagRole = Qt::UserRole + 1,
        ValueRole
    };
    Q_ENUM(Roles)

        explicit EntryListModel(QObject* parent = nullptr) : QAbstractListModel(parent) {} //null constructor

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setEntries(QVector<TagEntry> entries);//TagEntry defined in getExif.h

private:
    QVector<TagEntry> m_entries; //data storage
};

/*
ExifGroupsModel: All groups of Exif data of one file. Initialize on file loading. Stores as member of ExifFileInfo.
This is displayed in InfoPanel in QML frontend.
*/
class ExifGroupsModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        GroupNameRole = Qt::UserRole + 1,
        EntriesModelRole,
        FoldedRole, //for fold status persistent
        GroupLengthRole, //for UI acceleration
        GroupIndexRole,
    };
    Q_ENUM(Roles)

    explicit ExifGroupsModel(QObject* parent = nullptr) : QAbstractListModel(parent) {} //null constructor

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void rebuildFromExifModel(const ExifModel& exifModel);
    void toggleFoldStatus(int groupIndex);

private:
    struct GroupItem {
        QString groupName;
        QPointer<EntryListModel> entriesModel;
        bool folded = false; //true=folded

    };
    QVector<GroupItem> m_groups; //data storage
};


/*ExifFileInfo: Object to store multiple items of a single file for display.
It is saved in ExifList (Backend class member), which is an std::vector of
all ExifFileInfo objects. */
struct ExifFileInfo
{
    //set default value to avoid loading error
    QString filePath = ""; //local file path (not file:///)
    QString fileName = ""; //file name + suffix
    QString baseName = ""; //only file name without suffix
    QString fileType = ""; //suffix
    std::unique_ptr<ExifModel> exifModel = nullptr; //pointer to ExifModel for searching and other purposes
    std::unique_ptr<ExifGroupsModel> exifGroupsModel = nullptr; //pointer to exif groups model for direct infopanel display

    //view location of infoPanel（pending change）
    int viewX = 0;
    int viewY = 0;

    //thumbnail is not managed here, thumbnail is owned by FileListModel

    //constructors
    //explicit defined to avoid accidental construction
    //default constructor
    ExifFileInfo() = default;

    //2nd constructor: allow auto-completion of file name info
    explicit ExifFileInfo(const QString& path);
};

/*FileListModel class for thumbnail loading in QML.
This class only stores file name, path and thumbnail path.
It shares index value with ExifList and can be reconstructed solely from
the exifList object, but it is a QtListModel instead of std::vector.
All exif data are stored in ExifList[i].ExifFileInfo.exifModel, not here.
This class is used as a member value m_fileListModel in Backend Class.
*/
class FileListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        FilePathRole = Qt::UserRole + 1, //full path
        FileIndexRole,
        FileNameRole, //full name with ext
        BaseNameRole, //name without ext
        FileTypeRole, //ext name
        ThumbUrlRole, //for thumbnail loading
        ThumbStateRole, //  int: 0=NotRequested,1=Generating,2=Ready,3=Failed
        ThumbVersionRole, // update UI when version renewed
    };
    Q_ENUM(Roles)

        // null constructor, use rebuildFrom() to fill up
        explicit FileListModel(QObject* parent = nullptr);

    //2nd constructor, construct FileListModel from exifFileList vector
    explicit FileListModel(const std::vector<ExifFileInfo>& exifList, QObject* parent = nullptr);

    // override implementation of QAbstractListModel mandatory interfaces
    int rowCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    //clear all
    void clear();

    // clear and rebuild data of this from an std::vector<ExifFileInfo>
    // not to be confused with 2nd constructor
    void rebuildFrom(const std::vector<ExifFileInfo>& exifFileList);

    // add operation. used in Backend::loadExifFromFile()
    void addFile(const QString& path); //add using local path
    void addFile(const ExifFileInfo& info); //add using ExifFileInfo

private:

    //define the struct to store data of all roles
    struct FileItem
    {
        //no need to store fileIndex as int because equivalent to row number
        QString filePath;
        QString fileName;
        QString baseName;
        QString fileType;

        //thumbnail part contains 3 members: thumbCachePath, thumbState, and thumbVersion
        QString thumbCachePath;//cache path of thumb image

        enum class ThumbState { //define status of thumb
            NotRequested = 0, //designate number value for static cast
            Generating = 1,
            Ready = 2,
            Failed = 3
        } thumbState = ThumbState::NotRequested;

        int thumbVersion = 0; //default 0, ++ when update

        //helper method: check if cache file exists
        bool hasValidThumbnail() const {
            return thumbState == ThumbState::Ready && QFile::exists(thumbCachePath);
        }

    };

    //m_fileList: storage of data
    std::vector<FileItem> m_fileList;
    //Thumb Provider: WIN32 only
#if defined(Q_OS_WIN)
    WindowsShellThumbProvider thumbProvider;
#elif defined(Q_OS_MAC)
    MacThumbProvider thumbProvider;
#else
    QtThumbProvider thumbProvider;
#endif
    //cache dir: for portable version, put under path of the .exe
    const QString m_cacheDir = QCoreApplication::applicationDirPath() + "/cache/zviewer_thumbs";
    //default thumbnail image width and height
    const int thumbWidth = 300;
    const int thumbHeight = 200;
};
