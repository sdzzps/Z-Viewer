#include "backend.h"

#include <QUrl>
#include <QProcess>
#include <QFileInfo>
#include <QDir>
//
/*
Implementation of Backend class methods
*/
//
Backend::Backend(QObject *parent)
    : QObject{parent}
    , m_fileListModel(this)//set Backend object as parent of m_fileListModel
{}

//all file-level models retrievals need to check index legitimacy
ExifModel* Backend::exifModel() const
{
    //boundary check
    if (m_currentIndex < 0 || m_currentIndex >= static_cast<int>(exifList.size()))
        return nullptr;
    return exifList[m_currentIndex].exifModel.get();
}

ExifGroupsModel* Backend::exifGroupsModel() const
{
    if (m_currentIndex < 0 || m_currentIndex >= static_cast<int>(exifList.size()))
        return nullptr;
    return exifList[m_currentIndex].exifGroupsModel.get();
}

QVariantMap Backend::basicInfo() const
{
    if (m_currentIndex < 0 || m_currentIndex >= static_cast<int>(exifList.size()))
        return QVariantMap();
    return exifList[m_currentIndex].exifModel->getBasicInfo();
}

//Search model setting methods
void Backend::setSearchKeyword(const QString& kw)
{
    const QString trimmed = kw.trimmed();
    if (m_searchKeyword == trimmed)
        return;

    m_searchKeyword = trimmed;
    emit searchKeywordChanged();
}

void Backend::setSearchField(SearchField field)
{
    if (m_searchField == field)
        return;

    m_searchField = field;
    emit searchFieldChanged();
}

void Backend::applySearchToProxy()
{
    // 1) keyword
    m_exifProxyModel.setKeyword(m_searchKeyword);

    // 2) field：Backend::SearchField -> ExifProxyModel::SearchField
    const auto proxyField =
        (m_searchField == SearchField::Tag)
        ? ExifProxyModel::SearchField::Tag
        : ExifProxyModel::SearchField::Value;

    m_exifProxyModel.setSearchField(proxyField);
}

void Backend::importFiles(const QList<QUrl>& urls, bool setCurrent)
{
    QStringList paths;
    paths.reserve(urls.size());

    for (const QUrl& u : urls) {
        if (!u.isValid())
            continue;

        if (!u.isLocalFile()) {
            qWarning() << "importFileUrls: non-local url ignored:" << u;
            continue;
        }

        const QString p = u.toLocalFile(); // 关键：file:/// -> C:\...
        if (!p.isEmpty())
            paths.push_back(p);
    }

    if (paths.isEmpty())
        return;

    const int last = paths.size() - 1;

    for (int i = 0; i < paths.size(); ++i) {
        const bool setCurForThis = setCurrent && (i == last);
        loadExifFromFile(paths[i], setCurForThis); // 直接调用原有函数
    }
}


void Backend::myFunction()
{
    emit mySignal();//member function for comm testing purposes
}

//method called by QML to load new file and display
//if QML does not specify, new file will be set on current display
void Backend::loadExifFromFile(const QString& filePath, bool setCurrent)
{
    //step 1: file path processing
    //filePath may contain file// heading, parsing needed
    QUrl url(filePath);
    QString localPath;


    if (url.isValid() && url.scheme().startsWith("file")){
        localPath = url.toLocalFile();//convert if path is file:// format
    } else {
        localPath = filePath;//skip if path is local format
    }

    qDebug() << "path from QML = " << filePath
             <<", localPath = " << localPath; //for debug purposes


    //step 2: reading exif data using pipeline
    std::unique_ptr<ExifModel> loadModel = getExifModelFromFile(localPath);//use local path to run exiftool and load Model
    //when pipeline fails to read, nullptr will be returned
    if (!loadModel) {
        qWarning() << "Failed to load model from pipeline. Local path: " << localPath;
        return;
    }

    //step 3: create new ExifFileInfo object and fill up
    auto groupsModel = std::make_unique<ExifGroupsModel>();
    groupsModel->rebuildFromExifModel(*loadModel);
    ExifFileInfo info(filePath);
    info.exifModel = std::move(loadModel);
    info.exifGroupsModel = std::move(groupsModel);

    //step 4: push back into exifList, update file count and fileListModel
    //qml UI will update thumbnail panel
    exifList.push_back(std::move(info));
    m_fileListModel.addFile(exifList.back());// add file to fileListModel, which also creates thumb image
    emit fileCountChanged();
    
    //step 5: if setCurrent enabled, update m_exifModel and change current index
    if (setCurrent)
    {
        //when setting current index, the setCurrentIndex method would emit change signals
        setCurrentIndex(static_cast<int>(exifList.size()) - 1);
    }
    else if (m_currentIndex < 0) //no current file yet, initialize to first file
    {   
        setCurrentIndex(0);
    }
}

//change current index and update m_exifModel
//this is the backend method of switching views from different loaded files
void Backend::setCurrentIndex(int index)
{
    //sanity check of index 
    if (index < 0 || index >= static_cast<int>(exifList.size()))
    {
        qWarning() << "Set index failed. " << index << " exceeds list size. ";
        return;
    }
    
    //avoid overly refresh
    if (m_currentIndex == index)
        return;

    //update index, notify qml to refresh thumb panel
    m_currentIndex = index;
    emit currentIndexChanged();

    //set source of exifProxyModel to exifModel of current file
    m_exifProxyModel.setSourceModel(exifList[m_currentIndex].exifModel.get());
    //m_exifProxyModel.resetFilter();

    //update current model, notify qml to refresh info panel
    emit exifModelChanged();
    emit basicInfoChanged();
    emit exifGroupsModelChanged();
}

//when collapsed panel clicked, send status change to backend. 
void Backend::toggleFoldStatus(int fileIndex, int groupIndex)
{
    //qWarning() << "received param " << fileIndex << " " <<groupIndex;//debug only

    if (fileIndex >= exifList.size() || fileIndex < 0)
    {
        qWarning() << "Illegal file index " << fileIndex ;
        return;
    }
    exifList[fileIndex].exifGroupsModel->toggleFoldStatus(groupIndex);
}

//when setting new index by clicking another thumbnail, save the view position of current file
void Backend::saveViewLocation(int fileIndex, int view_X, int view_Y)
{
    if (fileIndex >= exifList.size() || fileIndex < 0)
    {
        qWarning() << "Backend::saveViewLocation: Illegal file index " << fileIndex;
        return;
    }
    if (view_X < 0 || view_Y < 0)
    {
        qWarning() << "Backend::saveViewLocation: Illegal position " << view_X << view_Y;
        return;
    }
    exifList[fileIndex].viewX = view_X;
    exifList[fileIndex].viewY = view_Y;
}

QStringList Backend::groups() const //currently obsolete
{
    //boundary check
    if (m_currentIndex < 0 || m_currentIndex >= static_cast<int>(exifList.size()))
        return {};

    ExifModel* model = exifList[m_currentIndex].exifModel.get();
    if (!model) //if no model, return null
        return {};

    return model->getGroups();
}

void Backend::revealInFileManager(const QString& filePath)
{
    const QFileInfo fi(filePath);
    if (!fi.exists()) return;

#if defined(Q_OS_WIN)
    // Explorer /select, "C:\path\file.ext"
    QStringList args;
    args << "/select," << QDir::toNativeSeparators(fi.absoluteFilePath());
    QProcess::startDetached("explorer.exe", args); //startDetached: call unmanaged new process

#elif defined(Q_OS_MAC)
    // Finder reveal
    QProcess::startDetached("open", {"-R", fi.absoluteFilePath()});

#else
    // Linux: open folder (best-effort)
    QProcess::startDetached("xdg-open", {fi.absolutePath()});
#endif
}

bool Backend::clearCacheFolder()
{
    const QString m_cacheDir = QCoreApplication::applicationDirPath() + "/cache/zviewer_thumbs";
    if (m_cacheDir.isEmpty()) {
        qWarning() << "Cache directory path is empty.";
        return false;
    }

    QDir dir(m_cacheDir);

    if (!dir.exists()) {
        qDebug() << "Cache directory does not exist:" << m_cacheDir;
        return true; // if non-exist, reture complete
    }

    // avoid deleting program main dir
    if (dir.absolutePath() == QCoreApplication::applicationDirPath()) {
        qCritical() << "Refusing to delete application directory!";
        return false;
    }

    bool ok = dir.removeRecursively();//delete all files in dir
    if (!ok) {
        qWarning() << "Failed to remove cache directory:" << m_cacheDir;
        return false;
    }

    // 重新创建空目录，方便后续继续用
    if (!dir.mkpath(".")) {
        qWarning() << "Failed to recreate cache directory:" << m_cacheDir;
        return false;
    }

    qDebug() << "Cache folder cleared:" << m_cacheDir;
    return true;
}

//management of fileListModel


void Backend::rebuildFileListModel()
{
    m_fileListModel.rebuildFrom(exifList);
}
