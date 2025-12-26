#pragma once
#include <QtGlobal>

namespace UiScale {

#ifdef Q_OS_MACOS
inline constexpr qreal FontScale = 1.15;
#elif defined(Q_OS_WIN)
inline constexpr qreal FontScale = 1.0;
#else
inline constexpr qreal FontScale = 1.0;
#endif

}
