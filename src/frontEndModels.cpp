#include "frontEndModels.h"
#include <QStringList>
#include <QUrl>
#include <QFileInfo>
#include <QAbstractItemModel>
#include <QModelIndex>

//ExifProxyModel methods Implementation
ExifProxyModel::ExifProxyModel(QObject* parent) : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true); //rebuild filter when source model changes
}

void ExifProxyModel::setKeyword(const QString& keyword)
{
    const QString trimmed = keyword.trimmed();
    if (m_keyword == trimmed)
        return;//when keyword unchanged, do nothing

    beginFilterChange();
    m_keyword = trimmed;
    endFilterChange();
}

void ExifProxyModel::setSearchField(SearchField field)
{
    if (m_field == field)
        return;

    beginFilterChange();
    m_field = field;
    endFilterChange();
}

void ExifProxyModel::setCaseSensitivity(Qt::CaseSensitivity cs)
{
    if (m_caseSensitivity == cs)
        return;

    beginFilterChange();
    m_caseSensitivity = cs;
    endFilterChange();
}

bool ExifProxyModel::matches(const QString& haystack) const
{
    if (m_keyword.isEmpty())
        return true;

    if (haystack.isEmpty())
        return false;

    return haystack.contains(m_keyword, m_caseSensitivity);
}

//filter function
bool ExifProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    const QAbstractItemModel* src = sourceModel();
    if (!src)
        return false;

    const QModelIndex idx = src->index(sourceRow, 0, sourceParent);
    if (!idx.isValid())
        return false;

    // empty keywordno filtering
    if (m_keyword.isEmpty())
        return true;

    const int role = (m_field == SearchField::Tag)
        ? ExifModel::TagRole
        : ExifModel::ValueRole;

    const QString text = src->data(idx, role).toString();
    return matches(text);
}


//EntryListModel methods Implementation
int EntryListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return m_entries.size();
}

QVariant EntryListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_entries.size()) return {};
    const auto& e = m_entries[index.row()];
    switch (role) {
    case TagRole:   return e.tag;
    case ValueRole: return e.value;
    default:        return {};
    }
}

QHash<int, QByteArray> EntryListModel::roleNames() const
{
    return {
        { TagRole, "tag" },
        { ValueRole, "value" }
    };
}
void EntryListModel::setEntries(QVector<TagEntry> entries)
{
    beginResetModel();
    m_entries = std::move(entries);
    endResetModel();
}
//end of EntryListModel methods

//implementation of ExifGroupsModel
int ExifGroupsModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return m_groups.size();
}

QVariant ExifGroupsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_groups.size()) return {};
    const auto& g = m_groups[index.row()];
    switch (role) {
    case GroupNameRole:        return g.groupName;
    case EntriesModelRole: return QVariant::fromValue(static_cast<QObject*>(g.entriesModel));
    case FoldedRole: return g.folded;
    case GroupLengthRole: return g.entriesModel->rowCount();
    case GroupIndexRole: return index.row();
    default:               return {};
    }
}

QHash<int, QByteArray> ExifGroupsModel::roleNames() const
{
    return {
        { GroupNameRole, "groupName" },
        { EntriesModelRole, "entriesModel" },
        { FoldedRole, "folded" },
        {GroupLengthRole, "groupLength"},
        {GroupIndexRole, "groupIndex"}
    };
}

void ExifGroupsModel::rebuildFromExifModel(const ExifModel& exifModel)
{
    beginResetModel();//signal
    m_groups.clear();//clear container
    QStringList groupNames = exifModel.getGroups();//get groups in sorted order from exifModel
    const auto& allEntries = exifModel.entries(); //get all entries from exifModel
    QHash<QString, QVector<TagEntry>> buckets;//creat Hash LUT buckets
    buckets.reserve(groupNames.size());
    for (const auto& e : allEntries) { //sort all entries by group name
        buckets[e.group].push_back(e); //if group bucket not exist, create new in buckets, else add to bucket
    }


    m_groups.reserve(groupNames.size());
    for (const auto& name : groupNames) {
        auto* child = new EntryListModel(this);
        child->setEntries(buckets.value(name));

        GroupItem item;
        item.groupName = name;
        item.entriesModel = child;
        item.folded = false; //default expanded
        m_groups.push_back(std::move(item));//add back to groups list container
    }
    endResetModel();
}

void ExifGroupsModel::toggleFoldStatus(int groupIndex)
{
    if (groupIndex < 0 || groupIndex >= static_cast<int>(m_groups.size())) //boundary check
    {
        qWarning() << "ExifGroupsModel::toggleFoldStatus: Illegal group index " << groupIndex;
        return;
    }
    m_groups[groupIndex].folded = !m_groups[groupIndex].folded;//flip
    const QModelIndex idx = index(groupIndex, 0);
    emit dataChanged(idx, idx, { FoldedRole });
}

//
//implement the 2nd constructor of ExifFileInfo struct
//allow auto-completion of file name info
ExifFileInfo::ExifFileInfo(const QString& path)
    : ExifFileInfo()
{
    filePath = path;
    QFileInfo info(path);

    fileName = info.fileName();
    baseName = info.baseName();
    fileType = info.suffix();

}

//
/*
Implementation of FileListModel class
*/
//null constructor
FileListModel::FileListModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

//2nd constructor: consturct from exifFileList
FileListModel::FileListModel(const std::vector<ExifFileInfo>& exifFileList,
                             QObject* parent)
    : QAbstractListModel(parent)
{
    rebuildFrom(exifFileList);
}

int FileListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return static_cast<int>(m_fileList.size());
}

//this is the method for QML frontend to access model data
QVariant FileListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return {};

    int row = index.row();
    if (row < 0 || row >= static_cast<int>(m_fileList.size()))
        return {};//boundary check

    const FileItem& item = m_fileList[row];

    switch (role) {
    case FilePathRole:
        return item.filePath;
    case FileIndexRole:
        return row; //use row as file index
    case FileNameRole:
        return item.fileName;
    case BaseNameRole:
        return item.baseName;
    case FileTypeRole:
        return item.fileType;
    case ThumbUrlRole: //convert Thumb cache path to QUrl
        return QUrl::fromLocalFile(item.thumbCachePath).toString();
    case ThumbStateRole:
        return static_cast<int>(item.thumbState);
    case ThumbVersionRole:
        return item.thumbVersion;
    default:
        return {};
    }
}

QHash<int, QByteArray> FileListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[FilePathRole] = "filePath"; //role names in QML
    roles[FileIndexRole] = "fileIndex";
    roles[FileNameRole] = "fileName";
    roles[BaseNameRole] = "baseName";
    roles[FileTypeRole] = "fileType";
    roles[ThumbUrlRole] = "thumbUrl";
    roles[ThumbStateRole] = "thumbState";
    roles[ThumbVersionRole] = "thumbVersion";
    return roles;
}

void FileListModel::clear()
{
    if (m_fileList.empty())
        return;

    beginResetModel();
    m_fileList.clear();
    endResetModel();
}

void FileListModel::rebuildFrom(const std::vector<ExifFileInfo>& exifFileList)
{
    beginResetModel(); //notify QML UI on model update

    m_fileList.clear();
    m_fileList.reserve(exifFileList.size());
    const QSize thumbSize(thumbWidth, thumbHeight);
    for (const ExifFileInfo& src : exifFileList) {
        FileItem item;
        //fill up path and names
        item.filePath = src.filePath;
        item.fileName = src.fileName;
        item.baseName = src.baseName;
        item.fileType = src.fileType;
        //generate thumbnail and fill up
        item.thumbCachePath = thumbProvider.makeThumbnail(item.filePath, thumbSize, m_cacheDir);
        if (!item.thumbCachePath.isEmpty())
            item.thumbState = FileItem::ThumbState::Ready;
        else
            item.thumbState = FileItem::ThumbState::Failed;
        item.thumbVersion++;
        //put into file list
        m_fileList.push_back(std::move(item));
    }

    endResetModel();
}

//direct copy from an ExifFileInfo object
void FileListModel::addFile(const ExifFileInfo& info)
{
    const int row = static_cast<int>(m_fileList.size());
    beginInsertRows(QModelIndex(), row, row);

    FileItem item;
    item.filePath = info.filePath;
    item.fileName = info.fileName;
    item.baseName = info.baseName;
    item.fileType = info.fileType;
    //generate thumbnail and fill up
    const QSize thumbSize(thumbWidth, thumbHeight);
    //generate thumbnail and fill up
    item.thumbCachePath = thumbProvider.makeThumbnail(item.filePath, thumbSize, m_cacheDir);
    if (!item.thumbCachePath.isEmpty())
        item.thumbState = FileItem::ThumbState::Ready;
    else
        item.thumbState = FileItem::ThumbState::Failed;
    item.thumbVersion++;
    m_fileList.push_back(std::move(item));

    endInsertRows();
}

//only using local path and parse into FileItem
void FileListModel::addFile(const QString& path)
{
    const int row = static_cast<int>(m_fileList.size());
    beginInsertRows(QModelIndex(), row, row);

    FileItem item;
    QFileInfo info(path);//convert to QFileInfo for auto parsing
    item.fileName = info.fileName();
    item.baseName = info.baseName();
    item.fileType = info.suffix();
    //generate thumbnail and fill up
    const QSize thumbSize(thumbWidth, thumbHeight);
    //generate thumbnail and fill up
    item.thumbCachePath = thumbProvider.makeThumbnail(item.filePath, thumbSize, m_cacheDir);
    if (!item.thumbCachePath.isEmpty())
        item.thumbState = FileItem::ThumbState::Ready;
    else
        item.thumbState = FileItem::ThumbState::Failed;
    item.thumbVersion++;
    m_fileList.push_back(std::move(item));

    endInsertRows();
}
