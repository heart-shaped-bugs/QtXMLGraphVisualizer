#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QGraphicsLineItem>
#include <QWheelEvent>
#include <QMap>
#include <QSet>
#include "xmlparser.h"

class GraphWidget : public QGraphicsView {
    Q_OBJECT

public:
    GraphWidget(QWidget *parent = nullptr);
    void loadGraph(const GraphData &graph);
    void fitGraph();

protected:
    void wheelEvent(QWheelEvent *event) override;

private:
    QGraphicsScene *scene;
    QMap<QString, QPointF> nodePositions;
    QMap<QString, QGraphicsRectItem*> nodeItems;
    QMap<QString, QGraphicsTextItem*> textItems;

    int calculateDepth(const GraphData &graph, const QString &nodeId, QSet<QString> &visited);
    void positionNodes(const GraphData &graph);
    QColor getNodeColor(int level);
    void drawGraph(const GraphData &graph);
};

#endif // GRAPHWIDGET_H
