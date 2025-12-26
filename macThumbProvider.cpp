#include "macThumbProvider.h"
#include <QDir>
#include <QFileInfo>

MacThumbProvider::MacThumbProvider() {}

// implement in .mm, call system API to generate QuickLook thumbnail images
bool macGenerateThumbnailToPng(const QString& filePath,
                               const QSize& targetSize,
                               const QString& outPngPath);

QString MacThumbProvider::makeThumbnail(const QString& filePath,
                                        const QSize& targetSize,
                                        const QString& cacheDir)
{
    if (filePath.isEmpty() || targetSize.isEmpty() || cacheDir.isEmpty())
        return {};

    QDir().mkpath(cacheDir);

    const QString key = hashFileName(filePath);
    const QString outPath = QDir(cacheDir).filePath(key + ".png");

    // 1) Use QuickLook to generate system thumbnail image
    if (macGenerateThumbnailToPng(filePath, targetSize, outPath))
        return outPath;

    // 2) if fails, return null and use fallback in outerlayer
    return {};
}
