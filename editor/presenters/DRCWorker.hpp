#ifndef DRC_WORKER_HPP
#define DRC_WORKER_HPP

#include <QObject>
#include <QThread>
#include <QRectF>
#include <QVector>
#include <mutex>
#include <queue>
#include <unordered_map>
#include <QStringList>
#include "../../incremental_drc/incremental_drc.hpp"

struct DRCInput {
    int id;
    std::string layerName;
    QRectF rect;
    int zLevel;
};

struct DRCTile {
    QRectF rect;
    int layerType;
    bool isSolid;
    int zLevel;
};

class DRCWorker : public QObject {
    Q_OBJECT
public:
    DRCWorker();
    ~DRCWorker();

public slots:
    void updateLayout(std::vector<DRCInput> layout);
    void loadRules(const QString& filePath);
    void process();

signals:
    void errorsUpdated(QVector<int> errorIds);
    void violationsUpdated(QStringList messages);
    void tilesUpdated(QVector<DRCTile> tiles);

private:
    DRC::Layout m_layout;
    std::vector<DRCInput> m_pendingLayout;
    std::unordered_map<int, DRCInput> m_lastLayoutCache;
    std::mutex m_mutex;
    bool m_isProcessing;
};

#endif // DRC_WORKER_HPP
