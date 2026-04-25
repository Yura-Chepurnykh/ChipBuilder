// #include "DRCWorker.hpp"
// #include <QMetaObject>
// #include <QFile>
// #include <fstream>
// #include <QDebug>
// #include "../../design_models/third_party/json.hpp"

// using JSON = nlohmann::json;

// // 30 pixels = 1 lambda
// static constexpr double SCALE = 1.0 / 30.0;

// DRCWorker::DRCWorker() : m_isProcessing(false) {
//     // Default rules in case rules.json isn't loaded yet
//     std::vector<DRC::LayerRules> lrs = {
//         { DRC::LayerType::Active,  3, 3, 9 },
//         { DRC::LayerType::Poly,    2, 2, 4 },
//         { DRC::LayerType::NWell,   4, 4, 16 },
//         { DRC::LayerType::PWell,   4, 4, 16 },
//         { DRC::LayerType::Metal1,  3, 3, 9 },
//         { DRC::LayerType::Via,     2, 2, 4 },
//         { DRC::LayerType::Contact, 2, 2, 4 }
//     };
//     m_layout.setLayerRules(lrs);
// }

// DRCWorker::~DRCWorker() {}

// void DRCWorker::updateLayout(std::vector<DRCInput> layout) {
//     {
//         std::lock_guard<std::mutex> lock(m_mutex);
//         m_pendingLayout = std::move(layout);
//     }

//     if (!m_isProcessing) {
//         m_isProcessing = true;
//         QMetaObject::invokeMethod(this, "process", Qt::QueuedConnection);
//     }
// }

// static DRC::LayerType toDRCLayer(const std::string& name) {
//     if (name == "active")  return DRC::LayerType::Active;
//     if (name == "poly")    return DRC::LayerType::Poly;
//     if (name == "nwell")   return DRC::LayerType::NWell;
//     if (name == "pwell")   return DRC::LayerType::PWell;
//     if (name == "metal1")  return DRC::LayerType::Metal1;
//     if (name == "via")     return DRC::LayerType::Via;
//     if (name == "contact") return DRC::LayerType::Contact;
//     return DRC::LayerType::Active;
// }

// static DRC::CrossLayerRule::Kind toRuleKind(const std::string& kind) {
//     if (kind == "enclosure")    return DRC::CrossLayerRule::Kind::Enclosure;
//     if (kind == "overlap")      return DRC::CrossLayerRule::Kind::Overlap;
//     if (kind == "intersection") return DRC::CrossLayerRule::Kind::Intersection;
//     return DRC::CrossLayerRule::Kind::Enclosure;
// }

// void DRCWorker::loadRules(const QString& filePath) {
//     qDebug() << "DRCWorker: Loading rules from" << filePath;
//     std::ifstream f(filePath.toStdString());
//     if (!f.is_open()) {
//         qDebug() << "DRCWorker: Failed to open rules file";
//         return;
//     }

//     try {
//         JSON j;
//         f >> j;

//         std::vector<DRC::LayerRules> lrs;
//         if (j.contains("layerRules")) {
//             for (auto& lr : j["layerRules"]) {
//                 lrs.push_back({
//                     toDRCLayer(lr.value("layer", "active")),
//                     (DRC::Coord)lr.value("minWidth", 0),
//                     (DRC::Coord)lr.value("minSpace", 0),
//                     (int64_t)lr.value("minArea", 0)
//                 });
//             }
//         }

//         std::vector<DRC::CrossLayerRule> clrs;
//         if (j.contains("crossLayerRules")) {
//             for (auto& cr : j["crossLayerRules"]) {
//                 clrs.push_back({
//                     toRuleKind(cr.value("kind", "enclosure")),
//                     toDRCLayer(cr.value("inner", "active")),
//                     toDRCLayer(cr.value("outer", "active")),
//                     (DRC::Coord)cr.value("value", 0)
//                 });
//             }
//         }

//         {
//             std::lock_guard<std::mutex> lock(m_mutex);
//             m_layout.setLayerRules(lrs);
//             m_layout.setCrossLayerRules(clrs);
//         }
        
//         qDebug() << "DRCWorker: Rules updated, triggering process";
//         if (!m_isProcessing) {
//             m_isProcessing = true;
//             QMetaObject::invokeMethod(this, "process", Qt::QueuedConnection);
//         }

//     } catch (const std::exception& e) {
//         qDebug() << "DRCWorker: Error parsing rules:" << e.what();
//     }
// }

// void DRCWorker::process() {
//     std::vector<DRCInput> currentLayout;
//     {
//         std::lock_guard<std::mutex> lock(m_mutex);
//         currentLayout = m_pendingLayout;
//         m_isProcessing = false;
//     }

//     {
//         std::lock_guard<std::mutex> lock(m_mutex);

//         // Incremental update of planes
//         for (auto it = m_lastLayoutCache.begin(); it != m_lastLayoutCache.end(); ) {
//             bool found = false;
//             bool changed = false;
//             for (const auto& newItem : currentLayout) {
//                 if (newItem.id == it->first) {
//                     found = true;
//                     if (newItem.rect != it->second.rect || newItem.layerName != it->second.layerName) {
//                         changed = true;
//                     }
//                     break;
//                 }
//             }

//             if (!found || changed) {
//                 QRectF normRect = it->second.rect.normalized();
//                 DRC::Rect r((DRC::Coord)std::round(normRect.left() * SCALE),
//                             (DRC::Coord)std::round(normRect.top() * SCALE),
//                             (DRC::Coord)std::round(normRect.right() * SCALE),
//                             (DRC::Coord)std::round(normRect.bottom() * SCALE));
//                 m_layout.removeRect(toDRCLayer(it->second.layerName), r);
//                 it = m_lastLayoutCache.erase(it);
//             } else {
//                 ++it;
//             }
//         }

//         for (const auto& newItem : currentLayout) {
//             if (m_lastLayoutCache.find(newItem.id) == m_lastLayoutCache.end()) {
//                 QRectF normRect = newItem.rect.normalized();
//                 DRC::Rect r((DRC::Coord)std::round(normRect.left() * SCALE),
//                             (DRC::Coord)std::round(normRect.top() * SCALE),
//                             (DRC::Coord)std::round(normRect.right() * SCALE),
//                             (DRC::Coord)std::round(normRect.bottom() * SCALE));
//                 m_layout.insertRect(toDRCLayer(newItem.layerName), r);
//                 m_lastLayoutCache[newItem.id] = newItem;
//             }
//         }

//         std::vector<DRC::DRCViolation> violations = m_layout.runDRC();
        
//         std::vector<int> errorIdsRaw;
//         for (const auto& v : violations) {
//             for (const auto& item : currentLayout) {
//                 QRectF normRect = item.rect.normalized();
//                 // Violation location is already in lambda. Convert item rect to lambda for overlap check.
//                 DRC::Rect itemRectLambda((DRC::Coord)std::round(normRect.left() * SCALE),
//                                          (DRC::Coord)std::round(normRect.top() * SCALE),
//                                          (DRC::Coord)std::round(normRect.right() * SCALE),
//                                          (DRC::Coord)std::round(normRect.bottom() * SCALE));
                
//                 if (v.location.overlaps(itemRectLambda)) {
//                     errorIdsRaw.push_back(item.id);
//                 }
//             }
//         }

//         if (!violations.empty()) {
//             qDebug() << "DRCWorker: Found" << violations.size() << "violations, affecting" << errorIdsRaw.size() << "objects";
//         }

//         std::sort(errorIdsRaw.begin(), errorIdsRaw.end());
//         errorIdsRaw.erase(std::unique(errorIdsRaw.begin(), errorIdsRaw.end()), errorIdsRaw.end());

//         QVector<int> errorIds;
//         for (int id : errorIdsRaw) errorIds.push_back(id);

//         QStringList messages;
//         for (const auto& v : violations) {
//             messages.append(QString::fromStdString(v.message));
//         }

//         // Collect tiles for visualization
//         QVector<DRCTile> allTiles;
//         for (int i = 0; i < (int)DRC::LayerType::_Count; ++i) {
//             const auto& plane = m_layout.plane((DRC::LayerType)i);
//             for (const auto* t : plane.allTiles()) {
//                 // Ignore "infinite" background tiles
//                 if (t->bbox.x1 < -5000 || t->bbox.x2 > 5000 ||
//                     t->bbox.y1 < -5000 || t->bbox.y2 > 5000) continue;

//                 DRCTile dt;
//                 dt.rect = QRectF(t->bbox.x1 / SCALE, t->bbox.y1 / SCALE,
//                                  t->bbox.width() / SCALE, t->bbox.height() / SCALE);
//                 dt.layerType = i;
//                 dt.isSolid = t->solid;
//                 allTiles.push_back(dt);
//             }
//         }

//         emit errorsUpdated(errorIds);
//         emit violationsUpdated(messages);
//         emit tilesUpdated(allTiles);
//     }
// }


// #include "DRCWorker.hpp"
// #include <QMetaObject>
// #include <QFile>
// #include <fstream>
// #include <QDebug>
// #include "../../design_models/third_party/json.hpp"

// using JSON = nlohmann::json;

// static constexpr double SCALE = 1.0 / 30.0;

// DRCWorker::DRCWorker() : m_isProcessing(false) {
//     std::vector<DRC::LayerRules> lrs = {
//         { DRC::LayerType::Active,  3, 3, 9 },
//         { DRC::LayerType::Poly,    2, 2, 4 },
//         { DRC::LayerType::NWell,   4, 4, 16 },
//         { DRC::LayerType::PWell,   4, 4, 16 },
//         { DRC::LayerType::Metal1,  3, 3, 9 },
//         { DRC::LayerType::Via,     2, 2, 4 },
//         { DRC::LayerType::Contact, 2, 2, 4 }
//     };
//     m_layout.setLayerRules(lrs);
// }

// DRCWorker::~DRCWorker() {}

// void DRCWorker::process()
// {
//     std::vector<DRCInput> snapshot;

//     {
//         std::lock_guard<std::mutex> lock(m_mutex);
//         snapshot = m_pendingLayout;
//         m_isProcessing = false;
//     }

//     {
//         std::lock_guard<std::mutex> lock(m_mutex);

//         // =========================================
//         // 1. RESET LAYOUT COMPLETELY (IMPORTANT)
//         // =========================================
//         m_layout.clearAll();
//         m_lastLayoutCache.clear();

//         // =========================================
//         // 2. APPLY SNAPSHOT (ATOMIC STATE BUILD)
//         // =========================================
//         for (const auto& item : snapshot)
//         {
//             QRectF r = item.rect.normalized();

//             DRC::Rect rl(
//                 (DRC::Coord)std::round(r.left()   * SCALE),
//                 (DRC::Coord)std::round(r.top()    * SCALE),
//                 (DRC::Coord)std::round(r.right()  * SCALE),
//                 (DRC::Coord)std::round(r.bottom() * SCALE)
//                 );

//             m_layout.insertRect(toDRCLayer(item.layerName), rl);

//             // cache only final state
//             m_lastLayoutCache[item.id] = item;
//         }

//         // =========================================
//         // 3. RUN FULL DRC ON CONSISTENT STATE
//         // =========================================
//         std::vector<DRC::DRCViolation> violations = m_layout.runDRC();

//         // =========================================
//         // 4. MAP ERRORS TO IDS
//         // =========================================
//         std::vector<int> errorIdsRaw;

//         for (const auto& v : violations)
//         {
//             for (const auto& item : snapshot)
//             {
//                 QRectF r = item.rect.normalized();

//                 DRC::Rect itemRect(
//                     (DRC::Coord)std::round(r.left()   * SCALE),
//                     (DRC::Coord)std::round(r.top()    * SCALE),
//                     (DRC::Coord)std::round(r.right()  * SCALE),
//                     (DRC::Coord)std::round(r.bottom() * SCALE)
//                     );

//                 if (v.location.overlaps(itemRect))
//                 {
//                     errorIdsRaw.push_back(item.id);
//                 }
//             }
//         }

//         std::sort(errorIdsRaw.begin(), errorIdsRaw.end());
//         errorIdsRaw.erase(std::unique(errorIdsRaw.begin(), errorIdsRaw.end()),
//                           errorIdsRaw.end());

//         QVector<int> errorIds(errorIdsRaw.begin(), errorIdsRaw.end());

//         // =========================================
//         // 5. MESSAGES
//         // =========================================
//         QStringList messages;
//         for (const auto& v : violations)
//             messages.append(QString::fromStdString(v.message));

//         // =========================================
//         // 6. TILE VISUALIZATION
//         // =========================================
//         QVector<DRCTile> allTiles;

//         for (int i = 0; i < (int)DRC::LayerType::_Count; ++i)
//         {
//             const auto& plane = m_layout.plane((DRC::LayerType)i);

//             for (const auto* t : plane.allTiles())
//             {
//                 if (t->bbox.x1 < -5000 || t->bbox.x2 > 5000 ||
//                     t->bbox.y1 < -5000 || t->bbox.y2 > 5000)
//                     continue;

//                 DRCTile dt;
//                 dt.rect = QRectF(
//                     t->bbox.x1 / SCALE,
//                     t->bbox.y1 / SCALE,
//                     t->bbox.width() / SCALE,
//                     t->bbox.height() / SCALE
//                     );

//                 dt.layerType = i;
//                 dt.isSolid = t->solid;

//                 allTiles.push_back(dt);
//             }
//         }

//         // =========================================
//         // 7. EMIT RESULTS
//         // =========================================
//         emit errorsUpdated(errorIds);
//         emit violationsUpdated(messages);
//         emit tilesUpdated(allTiles);
//     }
// }

#include "DRCWorker.hpp"

#include <QMetaObject>
#include <QFile>
#include <QDebug>
#include <QRectF>
#include <QStringList>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <mutex>
#include <optional>
#include <vector>

#include "../../design_models/third_party/json.hpp"

using JSON = nlohmann::json;

// 30 pixels = 1 lambda
static constexpr double SCALE = 1.0 / 30.0;

static DRC::LayerType toDRCLayer(const std::string& name) {
    if (name == "active")  return DRC::LayerType::Active;
    if (name == "poly")    return DRC::LayerType::Poly;
    if (name == "nwell")   return DRC::LayerType::NWell;
    if (name == "pwell")   return DRC::LayerType::PWell;
    if (name == "metal1")  return DRC::LayerType::Metal1;
    if (name == "via")     return DRC::LayerType::Via;
    if (name == "contact") return DRC::LayerType::Contact;
    return DRC::LayerType::Active;
}

static DRC::CrossLayerRule::Kind toRuleKind(const std::string& kind) {
    if (kind == "enclosure")    return DRC::CrossLayerRule::Kind::Enclosure;
    if (kind == "overlap")      return DRC::CrossLayerRule::Kind::Overlap;
    if (kind == "intersection") return DRC::CrossLayerRule::Kind::Intersection;
    return DRC::CrossLayerRule::Kind::Enclosure;
}

static DRC::Rect toLambdaRect(const QRectF& rect) {
    QRectF r = rect.normalized();
    return DRC::Rect(
        (DRC::Coord)std::round(r.left()   * SCALE),
        (DRC::Coord)std::round(r.top()    * SCALE),
        (DRC::Coord)std::round(r.right()  * SCALE),
        (DRC::Coord)std::round(r.bottom() * SCALE)
        );
}

DRCWorker::DRCWorker() : m_isProcessing(false) {
    std::vector<DRC::LayerRules> lrs = {
        { DRC::LayerType::Active,  3, 3,  9 },
        { DRC::LayerType::Poly,    2, 2,  4 },
        { DRC::LayerType::NWell,   4, 4, 16 },
        { DRC::LayerType::PWell,   4, 4, 16 },
        { DRC::LayerType::Metal1,  3, 3,  9 },
        { DRC::LayerType::Via,     2, 2,  4 },
        { DRC::LayerType::Contact, 2, 2,  4 }
    };
    m_layout.setLayerRules(lrs);
}

DRCWorker::~DRCWorker() {}

void DRCWorker::updateLayout(std::vector<DRCInput> layout) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_pendingLayout = std::move(layout);
    }

    if (!m_isProcessing) {
        m_isProcessing = true;
        QMetaObject::invokeMethod(this, "process", Qt::QueuedConnection);
    }
}

void DRCWorker::loadRules(const QString& filePath) {
    qDebug() << "DRCWorker: Loading rules from" << filePath;

    std::ifstream f(filePath.toStdString());
    if (!f.is_open()) {
        qDebug() << "DRCWorker: Failed to open rules file";
        return;
    }

    try {
        JSON j;
        f >> j;

        std::optional<std::vector<DRC::LayerRules>> newLayerRules;
        std::optional<std::vector<DRC::CrossLayerRule>> newCrossLayerRules;

        if (j.contains("layerRules") && j["layerRules"].is_array()) {
            std::vector<DRC::LayerRules> lrs;
            for (const auto& lr : j["layerRules"]) {
                lrs.push_back({
                    toDRCLayer(lr.value("layer", "active")),
                    (DRC::Coord)lr.value("minWidth", 0),
                    (DRC::Coord)lr.value("minSpace", 0),
                    (int64_t)lr.value("minArea", 0)
                });
            }
            newLayerRules = std::move(lrs);
        }

        if (j.contains("crossLayerRules") && j["crossLayerRules"].is_array()) {
            std::vector<DRC::CrossLayerRule> clrs;
            for (const auto& cr : j["crossLayerRules"]) {
                clrs.push_back({
                    toRuleKind(cr.value("kind", "enclosure")),
                    toDRCLayer(cr.value("inner", "active")),
                    toDRCLayer(cr.value("outer", "active")),
                    (DRC::Coord)cr.value("value", 0)
                });
            }
            newCrossLayerRules = std::move(clrs);
        }

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (newLayerRules) {
                m_layout.setLayerRules(*newLayerRules);
            }
            if (newCrossLayerRules) {
                m_layout.setCrossLayerRules(*newCrossLayerRules);
            }
        }

        qDebug() << "DRCWorker: Rules updated, triggering process";
        if (!m_isProcessing) {
            m_isProcessing = true;
            QMetaObject::invokeMethod(this, "process", Qt::QueuedConnection);
        }
    } catch (const std::exception& e) {
        qDebug() << "DRCWorker: Error parsing rules:" << e.what();
    }
}

void DRCWorker::process() {
    std::vector<DRCInput> snapshot;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        snapshot = m_pendingLayout;
        m_isProcessing = false;
    }

    {
        std::lock_guard<std::mutex> lock(m_mutex);

        // Полная пересборка layout из финального снапшота.
        // Никаких remove+insert по частям — только consistent state.
        m_layout.clearAll();

        for (const auto& item : snapshot) {
            m_layout.insertRect(toDRCLayer(item.layerName), toLambdaRect(item.rect), item.zLevel);
        }

        // Полный DRC уже по согласованному состоянию
        std::vector<DRC::DRCViolation> violations = m_layout.runDRC();

        // Привязка нарушений к объектам по пересечению bbox
        std::vector<int> errorIdsRaw;
        errorIdsRaw.reserve(snapshot.size());

        for (const auto& v : violations) {
            for (const auto& item : snapshot) {
                DRC::Rect itemRect = toLambdaRect(item.rect);
                if (v.location.overlaps(itemRect)) {
                    errorIdsRaw.push_back(item.id);
                }
            }
        }

        std::sort(errorIdsRaw.begin(), errorIdsRaw.end());
        errorIdsRaw.erase(std::unique(errorIdsRaw.begin(), errorIdsRaw.end()), errorIdsRaw.end());

        QVector<int> errorIds;
        errorIds.reserve((int)errorIdsRaw.size());
        for (int id : errorIdsRaw) {
            errorIds.push_back(id);
        }

        QStringList messages;
        for (const auto& v : violations) {
            messages.append(QString::fromStdString(v.message));
        }

        QVector<DRCTile> allTiles;
        for (int z : m_layout.activeZLevels()) {
            for (int i = 0; i < (int)DRC::LayerType::_Count; ++i) {
                const auto& plane = m_layout.plane((DRC::LayerType)i, z);
                for (const auto* t : plane.allTiles()) {
                    if (t->bbox.x1 < -5000 || t->bbox.x2 > 5000 ||
                        t->bbox.y1 < -5000 || t->bbox.y2 > 5000) {
                        continue;
                    }

                    DRCTile dt;
                    dt.rect = QRectF(
                        t->bbox.x1 / SCALE,
                        t->bbox.y1 / SCALE,
                        t->bbox.width() / SCALE,
                        t->bbox.height() / SCALE
                        );
                    dt.layerType = i;
                    dt.isSolid = t->solid;
                    dt.zLevel = z;
                    allTiles.push_back(dt);
                }
            }
        }

        emit errorsUpdated(errorIds);
        emit violationsUpdated(messages);
        emit tilesUpdated(allTiles);
    }
}
