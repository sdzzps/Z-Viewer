#pragma once

#include "thumbImage.h"

class MacThumbProvider final : public ThumbProvider
{
public:
    MacThumbProvider();

    QString makeThumbnail(const QString& filePath,
                          const QSize& targetSize,
                          const QString& cacheDir) override;
};
