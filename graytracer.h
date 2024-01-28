#ifndef GRAYTRACER_H
#define GRAYTRACER_H

#include <QMainWindow>
#include <QDialog>
#include <QTimer>
#include <QImage>
#include <QLabel>
#include <thread>
#include <condition_variable>
#include <QVector2D>
#include "gscene.h"
#include "gintegrator.h"

namespace Ui {
class GRaytracer;
}

class GModalMessage;
class GRaytracer : public QMainWindow
{
    Q_OBJECT

public:
    explicit GRaytracer(QWidget *parent = 0);
    ~GRaytracer();

protected:
    virtual void resizeEvent(QResizeEvent* event) override;
    virtual void closeEvent(QCloseEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    void CreateScene();
    void SetupGRaytracer();
    void RefreshUI();
    void OnPreDraw();
    void DoDraw();
    void OnPostDraw();

private slots:
    void on_doDrawBtn_clicked();
    void on_drawProp_changed();
    void _update();

private:
    Ui::GRaytracer *ui;
    QLabel* msgLabel;
    GModalMessage *modalMsg;
    QTimer updateTimer;
    QImage middleBuffer;

    GScene scene;
    std::shared_ptr<GIntegrator> integrator;

    QVector2D prePressedPos{0,0};
    int timeCounter{0};
    bool isNeedExist;
    bool isRenderingCompleted;
    bool isSyncedColor;
    void DoRendering();
    std::mutex mtx;
    std::condition_variable hasDrawTask;
    std::thread renderingThread;
};


namespace Ui {
class GModalMessage;
}
class GModalMessage : public QDialog
{
public:
    GModalMessage(QWidget* parent);
    void SetMsg(QString msg);

private:
    Ui::GModalMessage *msgUI;
};

#endif // GRAYTRACER_H
