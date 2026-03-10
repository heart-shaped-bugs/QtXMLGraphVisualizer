#include "graphwidget.h"
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QFontMetrics>
#include <QWheelEvent>
#include <QGraphicsDropShadowEffect>
#include <QQueue>
#include <algorithm>

GraphWidget::GraphWidget(QWidget *parent)
    : QGraphicsView(parent)
{
    scene = new QGraphicsScene(this);
    scene->setBackgroundBrush(QColor(245, 245, 245));
    setScene(scene);
    setRenderHint(QPainter::Antialiasing);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setInteractive(true);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setAlignment(Qt::AlignCenter);
}

void GraphWidget::loadGraph(const GraphData &graph) {
    scene->clear();
    scene->setBackgroundBrush(QColor(245, 245, 245));
    nodePositions.clear();
    nodeItems.clear();
    textItems.clear();

    if (graph.nodes.isEmpty()) {
        return;
    }

    positionNodes(graph);
    drawGraph(graph);

    scene->setSceneRect(scene->itemsBoundingRect());
    fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

int GraphWidget::calculateDepth(const GraphData &graph, const QString &nodeId, QSet<QString> &visited) {
    if (visited.contains(nodeId)) return 0;
    visited.insert(nodeId);

    const GraphNode &node = graph.nodes[nodeId];
    int maxDepth = 0;

    for (const QString &conn : node.connections) {
        if (!visited.contains(conn)) {
            maxDepth = qMax(maxDepth, calculateDepth(graph, conn, visited));
        }
    }

    return maxDepth + 1;
}

void GraphWidget::positionNodes(const GraphData &graph) {
    QQueue<QString> queue;
    QMap<QString, int> levels;
    QMap<QString, int> horizontalOrder;
    QSet<QString> visited;

    // Инициализация с корневых узлов
    for (const QString &rootId : graph.rootNodes) {
        queue.enqueue(rootId);
        levels[rootId] = 0;
        visited.insert(rootId);
        horizontalOrder[rootId] = horizontalOrder.size();
    }

    if (queue.isEmpty() && !graph.nodes.isEmpty()) {
        QString firstId = graph.nodes.firstKey();
        queue.enqueue(firstId);
        levels[firstId] = 0;
        visited.insert(firstId);
        horizontalOrder[firstId] = 0;
    }

    // BFS с учётом порядка обхода
    while (!queue.isEmpty()) {
        QString currentId = queue.dequeue();
        int currentLevel = levels[currentId];

        const GraphNode &node = graph.nodes[currentId];
        for (const QString &conn : node.connections) {
            if (!visited.contains(conn)) {
                visited.insert(conn);
                levels[conn] = currentLevel + 1;
                horizontalOrder[conn] = horizontalOrder.size();
                queue.enqueue(conn);
            }
        }
    }

    // Обрабатываем изолированные узлы
    for (const QString &nodeId : graph.nodes.keys()) {
        if (!visited.contains(nodeId)) {
            levels[nodeId] = 0;
            visited.insert(nodeId);
            horizontalOrder[nodeId] = horizontalOrder.size();
        }
    }

    // Группируем по уровням
    QMap<int, QList<QString>> levelNodes;
    for (const QString &nodeId : levels.keys()) {
        levelNodes[levels[nodeId]].append(nodeId);
    }

    // Сортируем узлы на каждом уровне по порядку обхода
    for (int level : levelNodes.keys()) {
        std::sort(levelNodes[level].begin(), levelNodes[level].end(),
                  [&horizontalOrder](const QString &a, const QString &b) {
                      return horizontalOrder[a] < horizontalOrder[b];
                  });
    }

    // Позиционируем с увеличенными отступами
    qreal verticalSpacing = 180;
    qreal horizontalSpacing = 250;

    for (int level : levelNodes.keys()) {
        const QList<QString> &nodes = levelNodes[level];
        qreal levelWidth = nodes.size() * horizontalSpacing;
        qreal startX = -levelWidth / 2 + horizontalSpacing / 2;

        for (int i = 0; i < nodes.size(); ++i) {
            QString nodeId = nodes[i];
            qreal x = startX + i * horizontalSpacing;
            qreal y = level * verticalSpacing;
            nodePositions[nodeId] = QPointF(x, y);
        }
    }
}

QColor GraphWidget::getNodeColor(int level) {
    QList<QColor> colors = {
        QColor(100, 149, 237),
        QColor(60, 179, 113),
        QColor(255, 165, 0),
        QColor(221, 160, 221),
        QColor(144, 238, 144),
        QColor(176, 196, 222)
    };

    return colors[qMin(level, colors.size() - 1)];
}

void GraphWidget::drawGraph(const GraphData &graph) {
    QSet<QString> drawnEdges;

    for (const QString &nodeId : graph.nodes.keys()) {
        const GraphNode &node = graph.nodes[nodeId];

        for (const QString &conn : node.connections) {
            QString edgeKey = nodeId < conn ? nodeId + "-" + conn : conn + "-" + nodeId;
            if (drawnEdges.contains(edgeKey)) continue;
            drawnEdges.insert(edgeKey);

            if (nodePositions.contains(conn)) {
                QGraphicsLineItem *line = scene->addLine(
                    nodePositions[nodeId].x(),
                    nodePositions[nodeId].y(),
                    nodePositions[conn].x(),
                    nodePositions[conn].y(),
                    QPen(QColor(70, 70, 70), 2, Qt::SolidLine)
                    );
                line->setZValue(0);
            }
        }
    }

    for (const QString &nodeId : graph.nodes.keys()) {
        const GraphNode &node = graph.nodes[nodeId];
        QPointF pos = nodePositions[nodeId];

        QFont font("Arial", 9, QFont::Bold);
        QFontMetrics metrics(font);
        int textWidth = metrics.horizontalAdvance(node.name);
        int textHeight = metrics.height();

        qreal rectWidth = qMax(120.0, static_cast<qreal>(textWidth) + 40);
        qreal rectHeight = 50;

        QSet<QString> visited;
        int level = calculateDepth(graph, nodeId, visited);
        level = qMin(level, 5);

        QGraphicsRectItem *rect = scene->addRect(
            pos.x() - rectWidth / 2, pos.y() - rectHeight / 2,
            rectWidth, rectHeight,
            QPen(Qt::black, 2),
            QBrush(getNodeColor(level))
            );
        rect->setZValue(1);

        QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect();
        shadow->setBlurRadius(10);
        shadow->setOffset(3, 3);
        shadow->setColor(QColor(0, 0, 0, 100));
        rect->setGraphicsEffect(shadow);

        QGraphicsTextItem *textItem = scene->addText(node.name);
        textItem->setFont(font);
        textItem->setDefaultTextColor(Qt::white);
        textItem->setPos(pos.x() - textWidth / 2, pos.y() - textHeight / 2);
        textItem->setZValue(2);

        nodeItems[nodeId] = rect;
        textItems[nodeId] = textItem;
    }
}

void GraphWidget::wheelEvent(QWheelEvent *event) {
    if (event->angleDelta().y() > 0) {
        scale(1.1, 1.1);
    } else {
        scale(0.9, 0.9);
    }
    event->accept();
}

void GraphWidget::fitGraph() {
    fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}
