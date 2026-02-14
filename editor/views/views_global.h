#ifndef VIEWS_GLOBAL_H
#define VIEWS_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(VIEWS_LIBRARY)
#define VIEWS_EXPORT Q_DECL_EXPORT
#else
#define VIEWS_EXPORT Q_DECL_IMPORT
#endif

#endif // VIEWS_GLOBAL_H
