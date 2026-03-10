#ifndef XMLPARSER_H
#define XMLPARSER_H

#include <QString>
#include <QList>
#include <QMap>

struct GraphNode {
    QString id;
    QString name;
    QList<QString> connections;  // IDs connected nodes
};

struct GraphData {
    QMap<QString, GraphNode> nodes;
    QList<QString> rootNodes;  // Nodes without incoming edges
};

GraphData parseXmlGraph(const QString &filePath);

#endif // XMLPARSER_H
