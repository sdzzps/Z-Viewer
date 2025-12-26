#include "getExif.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDebug>
#include <QHash>
#include <QProcess>
#include <QStringList>
#include <QVector>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

/*
This file contains the tool functions of the Exif file pipeline:
1. resolveExifToolProgram(): resolving exiftool program location by platform.
2. runExifToolJson(filePath): run exiftool using QProcess and get result in
QByteArray format.
3. parseExifJson(jsonData): convert QByteArray into QJsonObject.
This file contains the implementation of ExifModel:QAbstractListModel class.
This file contains the getExifModelFromFile(filepath,...) method using tool
functions mentioned above to construct ExifModel object from given local
path. It is called by Backend class on runtime to import file.
*/

//determine exiftool program path by platform information.
static QString resolveExifToolProgram()
{
#if defined(Q_OS_WIN)
    const QString appDir = QCoreApplication::applicationDirPath();
    const QString bundled = QDir(appDir).filePath("tools/exiftool.exe");
    if (QFileInfo::exists(bundled))
    {
        qDebug() << "exiftool.exe found";
        return bundled;
    }

#elif defined(Q_OS_MAC)
    // 在 .app 内，applicationDirPath() 通常是 .../ZViewer.app/Contents/MacOS
    const QString appDir = QCoreApplication::applicationDirPath();
    const QString bundled = QDir(appDir).filePath("tools/exiftool");
    if (QFileInfo::exists(bundled) && QFileInfo(bundled).isExecutable())
        return bundled;

    // 也可以考虑放到 Contents/Resources/tools
    const QString resourcesDir = QDir(appDir).filePath("../Resources");
    const QString bundled2 = QDir(QDir::cleanPath(resourcesDir)).filePath("tools/exiftool");
    if (QFileInfo::exists(bundled2) && QFileInfo(bundled2).isExecutable())
        return bundled2;

#else // Linux / others
    const QString appDir = QCoreApplication::applicationDirPath();
    const QString bundled = QDir(appDir).filePath("tools/exiftool");
    if (QFileInfo::exists(bundled) && QFileInfo(bundled).isExecutable())
        return bundled;
#endif

    // fallback：PATH
#if defined(Q_OS_WIN)
    qDebug() << "exiftool.exe program not found in default location. Using PATH dependency. ";
    return "exiftool";
#else
    qDebug() << "exiftool.exe program not found in default location. Using PATH dependency. ";
    return "exiftool";
#endif
}

//run exiftool command and get JSON output in QByteArray
static QByteArray runExifToolJson(const QString& filePath)
{
	QProcess process;
    QString program = resolveExifToolProgram();
	QStringList args;
	args << "-G" << "-a" << "-json" << "-charset" << "UTF8" << filePath;
	process.start(program, args);
	if (!process.waitForFinished()) {
		qWarning() << "Failed to run exiftool";
		return QByteArray();
	}
	QByteArray output = process.readAllStandardOutput();
	QByteArray errorOutput = process.readAllStandardError();
	if (!errorOutput.isEmpty()) {
		qWarning() << "ExifTool error:" << errorOutput;
		//return QByteArray(); // Uncomment this line if you want to treat errors as fatal
	}
	return output;
}

//convert JSON data in QByteArray to QJsonObject
static QJsonObject parseExifJson(const QByteArray& jsonData)
{
	QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);// Parse JSON data

	if (jsonDoc.isNull() || !jsonDoc.isArray()) {
		qWarning() << "Failed to parse JSON data.";
		return QJsonObject();
	}
	QJsonArray jsonArray = jsonDoc.array();
	if (jsonArray.isEmpty()) {
		qWarning() << "No metadata found in JSON data.";
		return QJsonObject();
	}
	return jsonArray.first().toObject();
}

//ExifModel Class methods
//constructor
ExifModel::ExifModel(QObject* parent)
	: QAbstractListModel(parent)
{

}
  
//override rowCount
int ExifModel::rowCount(const QModelIndex& parent) const
{
	if (parent.isValid())
		return 0; //only top-level items
	return m_entries.size();
}

//override data
QVariant ExifModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid() || index.row() < 0 || index.row() >= m_entries.size())
		return QVariant();//return empty QVariant for invalid index
	const TagEntry& entry = m_entries.at(index.row());
	switch (role) {
	case GroupRole:
		return entry.group;
	case TagRole:
		return entry.tag;
	case ValueRole:
		return entry.value;
	default:
		return QVariant();
	}
}

//override roleNames
QHash<int, QByteArray> ExifModel::roleNames() const
{
	QHash<int, QByteArray> roles;
	roles[GroupRole] = "group";
	roles[TagRole] = "tag";
	roles[ValueRole] = "value";
	return roles;
}

//I/O methods
//setEntries method on change and mark groups cache as dirty
void ExifModel::setEntries(const QVector<TagEntry>& entries)
{
	beginResetModel();
	m_entries = entries;
	m_groupsDirty = true; //mark groups cache as dirty before UI access
	endResetModel();//end model reset, UI will update on this signal
}

//query methods
QStringList ExifModel::getGroups() const
{
	if (!m_groupsDirty) {
		return m_groupsCache; //return cached groups if not dirty
	}
	//rebuild group list
	QSet<QString> groupSet;
	for (const TagEntry& entry : m_entries) {
		groupSet.insert(entry.group);
	}
	QStringList list = groupSet.values(); 

	std::sort(list.begin(), list.end(), &ExifModel::groupLessThan);

	//store result in cache, set flag to clean
	m_groupsCache = list;
	m_groupsDirty = false;

	return m_groupsCache;
}

QVariantMap ExifModel::getBasicInfo() const {
    return {
        { "fileName",      m_fileName },
        { "fileSize",      m_fileSize },
        { "imageSize",     m_imageSize },
        { "dateTaken",     m_dateTaken },
        { "aperture",      m_aperture },
        { "shutterSpeed",  m_shutterSpeed },
        { "iso",           m_iso },
        { "focalLength",   m_focalLength },
        { "camera",   m_camera },
        { "lensModel",   m_lensModel },
        { "duration",   m_duration },
        { "frameRate",   m_frameRate }
    };
}

//helper methods
//for sorting groups in defined priority order
bool ExifModel::groupLessThan(const QString& a, const QString& b)
{
	static const QStringList PRIORITY = {
		"EXIF",
		"MakerNotes",
        "Composite",
        "QuickTime",
		"GPS",
		"XMP",
        "ICC_Profile",
        "File",
        "MPF",
        "Photoshop",
        "IPTC",
        "APP14",
        "PNG",
        "GIF",
        "SVG",
        "JFIF",
        "RIFF",
        "ExifTool"
	};//define priority order here
	//non-defined groups are sorted alphabetically after defined ones
	int ia = PRIORITY.indexOf(a);
	int ib = PRIORITY.indexOf(b);

	bool aIn = ia != -1;
	bool bIn = ib != -1;

	if (aIn && bIn)
		return ia < ib;
	if (aIn && !bIn)
		return true;
	if (!aIn && bIn)
		return false;
	return a < b;
}

//Traverse m_entries and rebuild basic info values
//helper method to normalize text
//define inline to tradeoff runtime with program size
static inline QString normTag(QString t)
{
    t.remove(' ');
    return t.toLower();
}

//define fields for searching
enum class FieldId {
    FileName,
    FileSize,
    ImageSize,
    DateTaken,

    Aperture,
    ShutterSpeed,
    ISO,
    FocalLength,

    Camera,
    LensModel,

    Duration,
    FrameRate,

    Count //when casts to int it equals number of fields
};

//record aliases of each variable for matching
static const QHash<FieldId, QStringList>& aliasTable()
{
    // add other aliases if needed
    static const QHash<FieldId, QStringList> t = {
      { FieldId::FileName,    { "FileName"} },
      { FieldId::FileSize,    { "FileSize"} },
      { FieldId::ImageSize,   { "ImageSize" } },
      { FieldId::DateTaken,   { "DateTimeOriginal", "CreateDate", "MediaCreateDate", "TrackCreateDate" } },

      { FieldId::Aperture,    { "FNumber", "Aperture", "ApertureValue" } },
      { FieldId::ShutterSpeed,{ "ExposureTime", "ShutterSpeed", "ShutterSpeedValue" } },
      { FieldId::ISO,         { "ISO", "ISOSetting", "ISOSpeedRatings" } },
      { FieldId::FocalLength, { "FocalLength", "FocalLengthIn35mmFormat" } },

      { FieldId::Camera, { "Model", "CameraModelName" } },
      { FieldId::LensModel,   { "LensModel", "LensInfo", "Lens", "LensID", "LensSpec"} },

      { FieldId::Duration,    { "Duration", "MediaDuration" } },
      { FieldId::FrameRate,   { "VideoFrameRate", "FrameRate", "FPS" } },
    };
    return t;
}

static const QHash<QString, FieldId>& tagToFieldIndex()
{
    // normalized tag -> FieldId
    static const QHash<QString, FieldId> idx = []{
        QHash<QString, FieldId> m;
        const auto& a = aliasTable();
        for (auto it = a.begin(); it != a.end(); ++it) {
            const FieldId fid = it.key();
            for (const QString& name : it.value()) {
                m.insert(normTag(name), fid);
            }
        }
        return m;
    }();
    return idx;
}

//info formatters
static inline QString fmtImageSize(QString v)
{
    v = v.trimmed();//use multiply sign
    v.replace('x', QStringLiteral(" × "), Qt::CaseInsensitive);
    return v;
}

static inline QString fmtAperture(QString v)
{
    v = v.trimmed();
    if (v.isEmpty()) return v;
    if (v.startsWith(QStringLiteral("f/"), Qt::CaseInsensitive)) return v;

    bool ok = false;
    const double d = v.toDouble(&ok);
    if (ok) return QStringLiteral("f/%1").arg(QString::number(d, 'g', 3));
    return v;
}

static inline QString fmtShutterSpeed(QString v)
{
    v = v.trimmed();
    if (v.isEmpty()) return v;

    if (v.endsWith(QLatin1Char('s'), Qt::CaseInsensitive))
        return v;

    return QStringLiteral("%1 s").arg(v);
}

static inline QString fmtISO(QString v)
{
    v = v.trimmed();
    if (v.isEmpty()) return v;
    if (v.startsWith(QStringLiteral("ISO"), Qt::CaseInsensitive)) return v;
    return QStringLiteral("ISO %1").arg(v);
}

static inline QString fmtFocal(QString v)
{
    v = v.trimmed();
    if (v.isEmpty()) return v;
    if (v.contains(QStringLiteral("mm"), Qt::CaseInsensitive)) return v;

    bool ok = false;
    const double d = v.toDouble(&ok);
    if (ok) return QStringLiteral("%1 mm").arg(QString::number(d, 'f', 0));
    return v;
}

static inline QString fmtFps(QString v)
{
    v = v.trimmed();
    if (v.isEmpty()) return v;
    if (v.contains(QStringLiteral("fps"), Qt::CaseInsensitive)) return v;
    return QStringLiteral("%1 FPS").arg(v);
}

void ExifModel::rebuildBasicInfo()
{
    // 1. clear all basic variables
    m_fileName.clear();
    m_fileSize.clear();
    m_imageSize.clear();
    m_dateTaken.clear();

    m_aperture.clear();
    m_shutterSpeed.clear();
    m_iso.clear();
    m_focalLength.clear();

    m_camera.clear();
    m_lensModel.clear();

    m_duration.clear();
    m_frameRate.clear();

    // 2) first-match indicator, if ture, skip this var
    bool filled[static_cast<int>(FieldId::Count)] = { false };

    //lambda void method to set found value to corresponding variable
    auto setOnce = [&](FieldId fid, QString v) {
        const int i = static_cast<int>(fid);
        if (filled[i]) return; //when var already written, skip

        v = v.trimmed();
        if (v.isEmpty()) return;//when empty, skip

        QString out; // content to write

        switch (fid) {
        case FieldId::FileName:     out = v; break;
        case FieldId::FileSize:     out = v; break;
        case FieldId::ImageSize:    out = fmtImageSize(v); break;
        case FieldId::DateTaken:    out = v; break;

        case FieldId::Aperture:     out = fmtAperture(v); break;
        case FieldId::ShutterSpeed: out = fmtShutterSpeed(v); break;
        case FieldId::ISO:          out = fmtISO(v); break;
        case FieldId::FocalLength:  out = fmtFocal(v); break;

        case FieldId::Camera:  out = v; break;
        case FieldId::LensModel:    out = v; break;

        case FieldId::Duration:     out = v; break;
        case FieldId::FrameRate:    out = fmtFps(v); break;

        case FieldId::Count:
            return;
        }

        out = out.trimmed();
        if (out.isEmpty())
            return;            // 关键：格式化后为空 -> 不写入、不锁定

        // 真正写入成员变量
        switch (fid) {
        case FieldId::FileName:     m_fileName = out; break;
        case FieldId::FileSize:     m_fileSize = out; break;
        case FieldId::ImageSize:    m_imageSize = out; break;
        case FieldId::DateTaken:    m_dateTaken = out; break;

        case FieldId::Aperture:     m_aperture = out; break;
        case FieldId::ShutterSpeed: m_shutterSpeed = out; break;
        case FieldId::ISO:          m_iso = out; break;
        case FieldId::FocalLength:  m_focalLength = out; break;

        case FieldId::Camera:  m_camera = out; break;
        case FieldId::LensModel:    m_lensModel = out; break;

        case FieldId::Duration:     m_duration = out; break;
        case FieldId::FrameRate:    m_frameRate = out; break;

        case FieldId::Count:
            break;
        }

        filled[i] = true;      //only mark as written if non-empty value found
    };

    const auto& index = tagToFieldIndex();

    //lambda void method to process each entry
    auto processEntry = [&](const TagEntry& e) {
        const QString key = normTag(e.tag);
        auto it = index.constFind(key); //use QHash constFind to search
        if (it == index.constEnd())
            return; //if not found, return.
        setOnce(it.value(), e.value);
    };

    // 3) 为了让“第一个命中”有确定性：按 group 优先级分轮扫描（仍是 O(k) 级别）
    static const QStringList GROUP_ORDER = {
        "EXIF",
        "Composite",
        "MakerNotes",
        "QuickTime",
        "XMP",
        "File"
    };

    // 3.1 优先组
    for (const QString& g : GROUP_ORDER) {
        for (const TagEntry& e : m_entries) {
            if (e.group == g)
                processEntry(e);
        }
    }

    // 3.2 其它组（兜底）
    for (const TagEntry& e : m_entries) {
        if (!GROUP_ORDER.contains(e.group))
            processEntry(e);
    }
}

//convert QJsonObject to QVector of TagEntry structs
QVector<TagEntry> parseExifTags(const QJsonObject& jsonObject)
{
	QVector<TagEntry> entries;
	entries.reserve(jsonObject.size());
	for (auto it = jsonObject.constBegin(); it != jsonObject.constEnd(); ++it) //traverse all 
	{
		const QString fullKey = it.key(); // e.g., "EXIF:DateTimeOriginal"
		const QJsonValue value = it.value();

		//skip SourceFile key
		if (fullKey == QLatin1String("SourceFile"))
			continue;

		//create a TagEntry object for current row result
		TagEntry entry;

		// Split the key into group and tag
		const int colonIndex = fullKey.indexOf(QLatin1Char(':'));
		if (colonIndex > 0) {
			entry.group = fullKey.left(colonIndex);
			entry.tag = fullKey.mid(colonIndex + 1);
		}
		else {
			entry.group = QStringLiteral("Other");
			entry.tag = fullKey;
		} //for keys without a colon, assign "Other" as group

		// Convert QJsonValue to QString by type
		if (value.isString()) {
			entry.value = value.toString();
		}
		else if (value.isDouble()) {
			entry.value = QString::number(value.toDouble());
		}
		else if (value.isBool()) {
			entry.value = value.toBool() ? QStringLiteral("true") : QStringLiteral("false");
		}
		else if (value.isNull() || value.isUndefined()) {
			entry.value = QString();
		}
		else if (value.isArray()) { //convert array or object to compact JSON string
			QJsonDocument doc(value.toArray());
			entry.value = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
		}
		else if (value.isObject()) {
			QJsonDocument doc(value.toObject());
			entry.value = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
		}
		else {
			entry.value = QStringLiteral("<unsupported json type>");
		}

		//add to entries vector
		entries.append(entry);
	}
	return entries;

}

//Single-threaded method of getting ExifModel object from given local file path.
std::unique_ptr<ExifModel> getExifModelFromFile(const QString& filePath, QObject* parent)
{
	auto modelptr = std::make_unique<ExifModel>();// do not set parent object, or will double delete

	QByteArray jsonData = runExifToolJson(filePath);
	if (jsonData.isEmpty()) {
		//return modelptr; //return pointer to empty model on failure
		return nullptr;// return nullptr on failure
	}

	QJsonObject jsonObject = parseExifJson(jsonData);
	if (jsonObject.isEmpty()) {
		//return modelptr; //return pointer to empty model on failure
		return nullptr; // return nullptr on failure
	}

	QVector<TagEntry> entries = parseExifTags(jsonObject);
	//this step does not do sanity check because some files does not contain metadata

	modelptr->setEntries(entries);

    //set basic values
    modelptr->rebuildBasicInfo();

	return modelptr;//it is safe to return unique_ptr
}
