#pragma once

#include <QString>
#include <QSize>
#include <QImageReader>
#include <QStandardPaths>
#include <QDir>
#include <QImage>
/*
This file contains the thumbnail pipelines and ThumbImage object def of Z Viewer application.
Thumbnail pipeline only generate thumb image on given file path and cache path.
All management and status of thumb files should be done externally. 
*/

/*
//Platform-independent abstract object of thumbnails (obsolete)
struct ThumbImage
{
    QString thumbPath;  //local path of thumbnail image
    QSize   size;       // size(optional)
    bool    ready = false;
    bool    failed = false;

    bool isValid() const { return ready && !thumbPath.isEmpty(); }
};
*/

/*Abstract layer of image pipeline classes. Return a ThumbImage object on given
local path and size. All platform dependent pipelines should inherit from this
class.
*/
class ThumbProvider
{
public:
    virtual ~ThumbProvider() = default;

    //
    virtual QString makeThumbnail(const QString &filePath, const QSize &targetSize, const QString &cacheDir) = 0;
};

//1. thumb provider based on Qt library
class QtThumbProvider : public ThumbProvider
{
public:
    QString makeThumbnail(const QString &filePath, const QSize &targetSize, const QString &cacheDir) override;
};

QString hashFileName(const QString& filePath);
