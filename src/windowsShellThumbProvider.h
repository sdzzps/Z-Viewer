#pragma once

#ifdef _WIN32

#include <QString>
#include <QSize>
#include <QStandardPaths>
#include <QDir>
#include <QImage>
#include <QDebug>

#include "thumbimage.h"
/*
* This is the declaration of WindowShellThumbProvider class. 
* 
* 1. It contains the WindowsShellThumbProvider::makeThumbnail method which utilizes 
* win32 API (IThumbNailProvider) to extract system thumbnail images and convert to 
* QImage object.
* 
* 2. Results are stored in local cache folder as PNG files, and corresponding 
* ThumbImage objects which contain path and info are for QML UI utilization. 
* 
* 3. This class only works on Windows OS. It is a subclass of ThumbProvider. To use
* this class, #include this file, and import 4 files: thumbImage.cpp, thumbImage.h, 
* windowsShellThumbProvider.cpp, windowsShellThumbProvider.h into the project. 
* 
* sdzzps 2025.12.09
*/


class WindowsShellThumbProvider : public ThumbProvider
{
public:
    WindowsShellThumbProvider() = default;
    ~WindowsShellThumbProvider() override = default;

    QString makeThumbnail(const QString &filePath, 
        const QSize &targetSize, const QString &cacheDir) override;
};


#endif // _WIN32
