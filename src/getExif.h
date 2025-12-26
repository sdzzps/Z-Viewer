#pragma once

#include <QDebug> //only for debug and testing purposes
#include <QVector>
#include <QString>
#include <QAbstractListModel>
#include <memory>

/*
This file contains pipeline of reading exif data using
exiftool.exe by Phil Harvey. It uese QProcess to run
exiftool.exe and convert it to QJson object, then finally
to ExifModel object.
This file also implement the ExifModel class, an QObject
for storage of exif data in the form of List Model for 
frontend display in QML. 
This module should only contain implementations of data
structures and pipelines. All interfaces to QML frontend 
and objects and methods directly accessed by QML should 
not be here. 
*/

//define struct for tag entry
struct TagEntry
{
	QString group;
	QString tag;
	QString value;
};

//define ExifModel class inheriting from QAbstractListModel
//for storage and access of exif tag entries
class ExifModel : public QAbstractListModel 
{
	Q_OBJECT
public:
	enum Roles {
        //for plain data:
		GroupRole = Qt::UserRole + 1, //define custom roles starting from Qt::UserRole + 1
		TagRole,
        ValueRole,
	};

	explicit ExifModel(QObject* parent = nullptr);//constructor

	//override implementation of necessary parent class methods
	//override rowCount
	int rowCount(const QModelIndex& parent) const override;
	//override data
	QVariant data(const QModelIndex& index, int role) const override;
	//override roleNames
	QHash<int, QByteArray> roleNames() const override;

	//I/O methods
	void setEntries(const QVector<TagEntry>& entries); //set entries from QVector<TagEntry>
	const QVector<TagEntry>& entries() const { return m_entries; } //get all entries

	//query methods
	QStringList getGroups() const; //get list of unique groups in entries
	//ExifModel getGroupModel(QString group); //get a sub model for a given group

    //provide basicInfo in a single QVar, called by Backend class
    QVariantMap getBasicInfo() const;

    //traverse m_entries fill up basic info, call on import.
    void rebuildBasicInfo();

private:
	QVector<TagEntry> m_entries; //storage for tag entries
	mutable QStringList m_groupsCache; //cached list of unique groups

	mutable bool m_groupsDirty = true; //flag to indicate if cached groups need updating

	//helper methods
	static bool groupLessThan(const QString& a, const QString& b);//sorting method for groups

    //Basic Info
    // ===== Basic file info =====
    QString m_fileName;          // file name, for bottom panel info
    QString m_fileSize;          // formatted file size, e.g. "24.3 MB"
    QString m_imageSize;         // image resolution, e.g. "6048 × 4024"
    QString m_dateTaken;         // date taken, formatted string

    // ===== Exposure (photo) =====
    QString m_aperture;          // e.g. "f/2.8"
    QString m_shutterSpeed;      // e.g. "1/125"
    QString m_iso;               // e.g. "ISO 800"
    QString m_focalLength;       // e.g. "35 mm"

    // ===== Equipment =====
    QString m_camera;       // camera model, e.g. "Nikon Z 8"
    QString m_lensModel;         // lens model, e.g. "NIKKOR Z 24-120mm f/4 S"

    // ===== Video =====
    QString m_duration;          // video duration, e.g. "00:01:23"
    QString m_frameRate;         // e.g. "29.97 fps"

};

//convert QJsonObject to QVector of TagEntry structs
QVector<TagEntry> parseExifTags(const QJsonObject& jsonObject);

//function from file path to ExifModel object
//use unique pointer to manage ownership
//single-threaded version （AbstractListModel items are not thread-safe）
std::unique_ptr<ExifModel> getExifModelFromFile(const QString& filePath, QObject *parent = nullptr);
