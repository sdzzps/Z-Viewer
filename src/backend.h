#pragma once

#include <QObject>
#include <QtQml>
#include <vector>
#include <QString>
#include <QStringList>
#include <QDebug> //only for debug and testing purposes

#include "getExif.h"
#include "frontEndModels.h"

/*
This file contains the Backend class, which is the communication interface
between C++ backend and QML frontend of Z Viewer App. It is registered as 
CppComm in main(). All communications between frontend and backend should 
happen here. For other class objects communicating with QML, create a
Q_INVOKABLE method in Backend class and use it to forward class methods.
*/

/* Backend class definition
Communication Interface between QML and C++. 
Storage of all ExifModel objects and other data in vector. 
Load new file by calling loadExifFromFile method. 
Switch views between loaded files by calling setCurrentIndex method. 
*/
class Backend : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(ExifModel* exifModel READ exifModel NOTIFY exifModelChanged) //read only, model for all metadata of selected file
    Q_PROPERTY(ExifGroupsModel* exifGroupsModel READ exifGroupsModel NOTIFY exifGroupsModelChanged) //model of metadata by groups
    Q_PROPERTY(FileListModel* fileListModel READ fileListModel CONSTANT) //constant value, Model for file list
    Q_PROPERTY(ExifProxyModel* exifProxyModel READ exifProxyModel CONSTANT)//the proxy model of search result from current ExifModel
	Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged) //file selected for display 
	Q_PROPERTY(QString searchKeyword READ searchKeyword WRITE setSearchKeyword NOTIFY searchKeywordChanged) //search keyword
	Q_PROPERTY(SearchField searchField READ searchField WRITE setSearchField NOTIFY searchFieldChanged) //search field
	Q_PROPERTY(int fileCount READ fileCount NOTIFY fileCountChanged)// read only, number of all files
    Q_PROPERTY(QVariantMap basicInfo READ basicInfo NOTIFY basicInfoChanged) //display basic info in bottom panel

public:
	//define the search types
	enum class SearchField {
		Tag,
		Value 
	};
	Q_ENUM(SearchField)

    explicit Backend(QObject *parent = nullptr);

	/*Read only methods used by Q_PROPERTY to allow QML frontend to read data. 
	Functions binded by Q_PROPERTY need no Q_INVOKABLE prefix.
	*/
	ExifModel* exifModel() const; //load current model

    ExifGroupsModel* exifGroupsModel() const; //load current grouped model

    FileListModel* fileListModel() { return &m_fileListModel; }//load fileListModel for thumbnail view

	ExifProxyModel* exifProxyModel() { return &m_exifProxyModel; } //load current search result model
	
	int currentIndex() const { return m_currentIndex; }//read currentIndex

	QString searchKeyword() const { return m_searchKeyword; }
	void setSearchKeyword(const QString& kw);

	SearchField searchField() const { return m_searchField; }
	void setSearchField(SearchField field);

	Q_INVOKABLE void applySearchToProxy();
	
	int fileCount() const { return exifList.size(); }//read file number

    QVariantMap basicInfo() const;

    Q_INVOKABLE void importFiles(const QList<QUrl>& urls, bool setCurrent = true); //import method for multiple-file list

    Q_INVOKABLE void myFunction(); //for testing purposes

	//load exif data from a file into exifModel
	//supports QUrl and local path autoadapt
	//set as current model by default
	Q_INVOKABLE void loadExifFromFile(const QString& filePath, bool setCurrent = true);  

	//set current index, used by qml frontend when clicking another file to display
	//also used by loadExifFromFile when setCurrent is enabled
	Q_INVOKABLE void setCurrentIndex(int index);

    //when collapsed panel clicked, send status change to backend. 
    Q_INVOKABLE void toggleFoldStatus(int fileIndex, int groupIndex);

    //when setting new index by clicking another thumbnail, save the view position of current file
    Q_INVOKABLE void saveViewLocation(int fileIndex, int view_X, int view_Y);

	//get groups from current exifModel
    Q_INVOKABLE QStringList groups() const;

    //reveal source file in its location, adaptive to platform
    Q_INVOKABLE void revealInFileManager(const QString& filePath);

    //clear cache foler of thumbnail images
    Q_INVOKABLE bool clearCacheFolder();

    //get ExifModel subset of a given group in current ExifModel
    //Q_INVOKABLE ExifModel* getGroupModel(QString groupName) const;

    //management of fileListModel
    //void addFile(const QString& path);
    //void clearFiles();
    void rebuildFileListModel();
    
signals:
    void mySignal();//for testing purposes
    void exifModelChanged();//notify qml that model changed
    void exifGroupsModelChanged();
	void currentIndexChanged();
	void searchKeywordChanged();
	void searchFieldChanged();
	void fileCountChanged();//notify qml frontend to refresh thumb panel when new file added, currently obsolete
    void basicInfoChanged();

private:
	//Storage of loaded data
	//do not use QVector, imcompatible with unique_ptr
	std::vector<ExifFileInfo> exifList; //a big vector to store all ExifFileInfo of current application session
    FileListModel m_fileListModel;//the FileListModel object
	ExifProxyModel m_exifProxyModel;//the search result model, always linked to ExifModel of current file
	QString m_searchKeyword;
	SearchField m_searchField = SearchField::Tag;

	//use m_ to avoid confusion with Q_PROPERTY functions
	int m_currentIndex = -1; //index of current item on display in exifList

};
