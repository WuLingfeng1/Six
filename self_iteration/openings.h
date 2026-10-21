#pragma once
#include <QPoint>
#include <QVector>

// Keep the first four IDs stable for comparisons with earlier reports.
inline QVector<QPoint> experimentOpening(int id)
{
    static const char *pairs[][2] = {
        {"I10", "K12"}, {"L7", "H14"}, {"H11", "L9"}, {"G8", "M12"},
        {"I11", "K11"}, {"J11", "L10"}, {"H10", "K11"}, {"I12", "L9"},
    };
    if (id < 0 || id >= 8) return {};
    QVector<QPoint> result;
    for (const char *text : pairs[id]) {
        const QString value = QString::fromLatin1(text);
        result.append(QPoint(19 - value.mid(1).toInt(), value[0].unicode() - 'A'));
    }
    return result;
}
