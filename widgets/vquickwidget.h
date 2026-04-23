#ifndef VQUICKWIDGET_H
#define VQUICKWIDGET_H

#include <QWidget>

#ifdef Q_OS_WASM

class QQmlEngine;
class QQuickItem;
class QUrl;

class VQuickWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VQuickWidget(QWidget *parent = nullptr) : QWidget(parent) {}
    
    enum ResizeMode { SizeViewToRootObject, SizeRootObjectToView };
    
    void setResizeMode(ResizeMode) {}
    void setClearColor(const QColor&) {}
    QQmlEngine* engine() const { return nullptr; }
    void setSource(const QUrl&) {}
    QQuickItem* rootObject() const { return nullptr; }

    QSize sizeHint() const override { return QSize(400, 300); }
    QSize minimumSizeHint() const override { return QSize(50, 50); }
};

#else

#include <QQuickWidget>

class VQuickWidget : public QQuickWidget
{
    Q_OBJECT
public:
    explicit VQuickWidget(QWidget *parent = nullptr) : QQuickWidget(parent) {}
};

#endif

#endif // VQUICKWIDGET_H
