#include "xmlparser.h"
#include <QXmlStreamReader>
#include <QFile>
#include <QDebug>
#include <QSet>

GraphData parseXmlGraph(const QString &filePath) {
    GraphData graph;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "❌ Ошибка открытия файла:" << filePath;
        return graph;
    }

    QXmlStreamReader xml(&file);
    QString currentSection;
    QString currentNodeId;

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
            if (xml.name() == "nodes") {
                currentSection = "nodes";
            }
            else if (xml.name() == "edges") {
                currentSection = "edges";
            }
            else if (xml.name() == "node" && currentSection == "nodes") {
                GraphNode node;
                node.id = xml.attributes().value("id").toString();
                node.name = xml.attributes().value("name").toString();
                graph.nodes[node.id] = node;
                currentNodeId = node.id;
            }
            else if (xml.name() == "edge" && currentSection == "edges") {
                QString from = xml.attributes().value("from").toString();
                QString to = xml.attributes().value("to").toString();

                // Добавляем связь в обе стороны (неориентированный граф)
                if (graph.nodes.contains(from)) {
                    graph.nodes[from].connections.append(to);
                }
                if (graph.nodes.contains(to)) {
                    graph.nodes[to].connections.append(from);
                }
            }
        }
    }

    if (xml.hasError()) {
        qDebug() << "❌ Ошибка XML:" << xml.errorString();
    }

    file.close();

    // Находим корневые узлы (без входящих связей или первые в списке)
    QSet<QString> hasIncoming;
    for (const auto &node : graph.nodes) {
        for (const auto &conn : node.connections) {
            hasIncoming.insert(conn);
        }
    }

    for (const auto &node : graph.nodes) {
        if (!hasIncoming.contains(node.id)) {
            graph.rootNodes.append(node.id);
        }
    }

    // Если все узлы имеют входящие связи (цикл), берём первый
    if (graph.rootNodes.isEmpty() && !graph.nodes.isEmpty()) {
        graph.rootNodes.append(graph.nodes.firstKey());
    }

    return graph;
}
