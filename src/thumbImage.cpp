#include "thumbImage.h"
#include <qstandardpaths.h>
#include <qDebug>
#include <QCryptographicHash>

//----工具函数：SHA256 Hash文件名生成————
//Tool function: SHA256 safe hash name generator to avoid hash collision

QString hashFileName(const QString& filePath)
{
    QByteArray hash = QCryptographicHash::hash(filePath.toUtf8(), QCryptographicHash::Sha256);
    return hash.toHex();   // 64 digit hex, no illegal char
}


//Use Qt API to load thumbnails (fallback solution)
QString QtThumbProvider::makeThumbnail(const QString &filePath, const QSize &targetSize, const QString &cacheDir)
{
    QString result;
    QImageReader reader(filePath);
    if (targetSize.isValid())
        reader.setScaledSize(targetSize);

    QImage img = reader.read();
    if (img.isNull()) {
        return QString();
    }

    // use cache to store thumbnail images
    //const QString cacheDirPath =
    //    QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
    //    + "/zviewer_thumbs";

    /*
    //portable version cache location
    const QString cacheDirPath =
        QCoreApplication::applicationDirPath() + "/cache/zviewer_thumbs";
    */

    // Ensure cache directory exists
    QDir dir(cacheDir);
    if (!dir.exists() && !dir.mkpath(".")) { //if dir not exist, make dir path, if fail, return null
        qWarning() << "QtThumbProvider: cannot create cache directory:" << cacheDir;
        return QString();
    }

    const QString thumbFileName =
        QString::number(qHash(filePath)) + ".jpg";
    result = dir.filePath(thumbFileName);//make result path

    if (!img.save(result, "JPG")) { //save image to result path, if failed, return null
        qWarning() << "QtThumbProvider: saving thumbnail failed!" << result;
        return QString();
    }
    return result;
}
