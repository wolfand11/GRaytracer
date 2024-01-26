#include "graytracer.h"
#include "ui_graytracer.h"
#include "ui_gmodalmessage.h"
#include "glog.h"
#include <QResizeEvent>
#include "gmath.h"
#include "gutils.h"
#include <QDoubleValidator>
using namespace GMath;

static void CopyColorBufferToImage(GColorBuffer* colorBuffer, QImage* image)
{
    image->setColorCount(colorBuffer->width*colorBuffer->height);
    for(int i=0; i<colorBuffer->width; i++)
    {
        for(int j=0; j<colorBuffer->width; j++)
        {
            // image origin is top left corner
            GColor color = colorBuffer->GetColor(i, colorBuffer->height - j - 1);
            image->setPixelColor(i, j, QColor(color.r,color.g,color.b,color.a));
        }
    }
}

GRaytracer::GRaytracer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::GRaytracer),
    updateTimer(this),
    middleBuffer(GUtils::screenWidth,GUtils::screenHeight,QImage::Format_RGBA8888),
    scene(GUtils::screenWidth,GUtils::screenHeight)
{
    ui->setupUi(this);
    msgLabel = new QLabel;
    modalMsg = new GModalMessage(this);
    modalMsg->setModal(true);
    modalMsg->hide();

    SetupGRaytracer();
    CreateScene();

    connect(&updateTimer, SIGNAL(timeout()), this, SLOT(_update()));
    updateTimer.start(1000);

    isNeedExist = false;
    isRenderingCompleted = true;
    renderingThread = std::thread(&GRaytracer::DoRendering, this);
    renderingThread.detach();

    ui->poxXLineEdit->setValidator(new QDoubleValidator(this));
    ui->poxYLineEdit->setValidator(new QDoubleValidator(this));
    ui->poxZLineEdit->setValidator(new QDoubleValidator(this));
    ui->rotationXLineEdit->setValidator(new QDoubleValidator(this));
    ui->rotationYLineEdit->setValidator(new QDoubleValidator(this));
    ui->rotationZLineEdit->setValidator(new QDoubleValidator(this));
    ui->fovLineEdit->setValidator(new QDoubleValidator(this));
    RefreshUI();
    connect(ui->poxXLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(on_drawProp_changed()));
    connect(ui->poxYLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(on_drawProp_changed()));
    connect(ui->poxZLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(on_drawProp_changed()));
    connect(ui->rotationXLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(on_drawProp_changed()));
    connect(ui->rotationYLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(on_drawProp_changed()));
    connect(ui->rotationZLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(on_drawProp_changed()));
    connect(ui->fovLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(on_drawProp_changed()));
    connect(ui->fillModeCB, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(on_drawProp_changed()));
    connect(ui->cullModeCB, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(on_drawProp_changed()));
}

GRaytracer::~GRaytracer()
{
    delete ui;
}

void GRaytracer::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    GLog::LogInfo("size changed! w = ", event->oldSize().width(), " h = ", event->oldSize().height());
}

void GRaytracer::closeEvent(QCloseEvent *event)
{
    isNeedExist = true;
    hasDrawTask.notify_one();
}

void GRaytracer::mousePressEvent(QMouseEvent *event)
{
    int x = event->x() - ui->canvas->x();
    int y = GUtils::screenHeight - (event->y() - ui->canvas->y());
    prePressedPos.setX(x);
    prePressedPos.setY(y);
}

void GRaytracer::CreateScene()
{
    // light
    auto lightGObj = GLight::CreateLightGObj(GLightType::kLTDirection);
    lightGObj->SetR(vec3f(50,-50,0));
    scene.lights.push_back(lightGObj);

    // camera
    auto cameraGObj = GCamera::CreateProjCamera(1, 2000, 60);
    cameraGObj->LookAt(vec3f(0,0,-2), vec3f(0,0,0), vec3f(0,1,0));
    cameraGObj->SetViewport(0, 0, GUtils::screenWidth, GUtils::screenHeight);
    scene.camera = cameraGObj;

    // models
    // cube
    auto cubeGObj = GModel::CreateModelGObj(GModelType::kMTObj, GUtils::GetAbsPath("models/cube.obj"));
    cubeGObj->SetR(vec3f(-25,0.0f,0.0f));
    //scene.models.push_back(cubeGObj);

    // floor
    auto floorGObj = GModel::CreateModelGObj(GModelType::kMTObj, GUtils::GetAbsPath("models/floor.obj"), false);
    floorGObj->model.init_texture(GMipmapType::kMipmapAnisotropy);
    //floorGObj.SetS(vec3f(1, 1, 1));
    //floorGObj.SetT(vec3f(0,1,0));
    //floorGObj.SetR(vec3f(45,0,0));
    scene.models.push_back(floorGObj);

    // diablo
    auto diablo3GObj = GModel::CreateModelGObj(GModelType::kMTObj, GUtils::GetAbsPath("models/diablo3_pose/diablo3_pose.obj"), false);
    diablo3GObj->model.init_texture(GMipmapType::kMipmapAnisotropy, GMipmapType::kMipmapAnisotropy);
    diablo3GObj->SetR(vec3f(0,180,0));
    scene.models.push_back(diablo3GObj);

    // sphere
    auto sphereGObj = GModel::CreateModelGObj(GModelType::kMTObj, GUtils::GetAbsPath("models/sphere.obj"));
    sphereGObj->SetT(vec3f(0,0,-0.9));
    //scene.models.push_back(sphereGObj);

    // african head
    auto africanHeadGObj = GModel::CreateModelGObj(GModelType::kMTObj, GUtils::GetAbsPath("models/african_head/african_head.obj"));
    //scene.models.push_back(africanHeadGObj);

    // triangle
    auto triangleGObj = GModel::CreateModelGObj(GModelType::kMTObj, GUtils::GetAbsPath("models/triangle.obj"));
    //triangleGObj.InitShader(GLAPI, GShaderType::kSTDefault, {std::make_tuple(GRenderBufferType::kRBFront, true)});
    triangleGObj->depthMask = false;
    //triangleGObj.modelShader->diffuseColor = GColor(255,255,255,100);
    //scene.models.push_back(triangleGObj);

    for(auto model: scene.models)
    {
        //model.SetupDraw(GLAPI);
    }
}

void GRaytracer::SetupGRaytracer()
{
    //GLAPI = new GGraphicLibAPI();
    //frameBuffer = GLAPI->GenFrameBuffer();
    //assert(frameBuffer!=nullptr);
    //GLAPI->BindFrameBuffer(frameBuffer);
    //colorBuffer = GLAPI->GenRenderBuffer(GUtils::screenWidth, GUtils::screenHeight);
    //assert(colorBuffer!=nullptr);
    //GLAPI->AttachRenderBufferToFrameBuffer(frameBuffer, colorBuffer, GRenderBufferType::kRBFront);
    //GLAPI->DrawRenderBuffer({GRenderBufferType::kRBFront});
    //GLAPI->Clear(GColor::black);
    //depthBuffer = GLAPI->GenDepthStencilBuffer(GUtils::screenWidth, GUtils::screenHeight);
    //GLAPI->AttachRenderBufferToFrameBuffer(frameBuffer, depthBuffer);
    //GLAPI->ClearDepth(1.0f);
    // obj vertex is CounterClockwise, but obj face is Clockwise
    //GLAPI->SetFrontFace(GFrontFace::kClockwise);
    //GLAPI->SetCullFace(GCullFaceType::kFTBack);
}

void GRaytracer::RefreshUI()
{
    if(scene.camera==nullptr) return;

    auto mainCamera = scene.camera;
    ui->poxXLineEdit->setText(QString("%1").arg(mainCamera->position().x()));
    ui->poxYLineEdit->setText(QString("%1").arg(mainCamera->position().y()));
    ui->poxZLineEdit->setText(QString("%1").arg(mainCamera->position().z()));
    ui->rotationXLineEdit->setText(QString("%1").arg(mainCamera->rotation().x()));
    ui->rotationYLineEdit->setText(QString("%1").arg(mainCamera->rotation().y()));
    ui->rotationZLineEdit->setText(QString("%1").arg(mainCamera->rotation().z()));
    ui->fovLineEdit->setText(QString("%1").arg(mainCamera->fov));
    //ui->fillModeCB->setCurrentIndex((int)GLAPI->activePolygonMode);
    //ui->depthBModeCB->setCurrentIndex(GLAPI->enableWBuffer ? 1 : 0);
    //ui->cullModeCB->setCurrentIndex((int)GLAPI->cullFaceType);
}

void GRaytracer::OnPreDraw()
{
}

void GRaytracer::DoDraw()
{
    OnPreDraw();
    timeCounter = 0;
    isRenderingCompleted = false;

    GLog::LogInfo("==> notify");
    hasDrawTask.notify_one();
}

void GRaytracer::OnPostDraw()
{
    CopyColorBufferToImage(&scene.film, &middleBuffer);
    ui->canvas->setPixmap(QPixmap::fromImage(middleBuffer));
}

void GRaytracer::on_doDrawBtn_clicked()
{
    this->setEnabled(false);
    modalMsg->show();
    DoDraw();
}

void GRaytracer::on_drawProp_changed()
{
    GLog::LogInfo("trs changed!");

    if(scene.camera==nullptr) return;
    auto mainCamera = scene.camera;

    vec3f pos;
    pos.SetX(ui->poxXLineEdit->text().toFloat());
    pos.SetY(ui->poxYLineEdit->text().toFloat());
    pos.SetZ(ui->poxZLineEdit->text().toFloat());
    mainCamera->SetT(pos);
    vec3f rot;
    rot.SetX(ui->rotationXLineEdit->text().toFloat());
    rot.SetY(ui->rotationYLineEdit->text().toFloat());
    rot.SetZ(ui->rotationZLineEdit->text().toFloat());
    mainCamera->SetR(rot);
    mainCamera->SetFov(ui->fovLineEdit->text().toFloat());
}

QString _msg;
void GRaytracer::_update()
{
    _msg.clear();
    if(isRenderingCompleted)
    {
        this->setEnabled(true);
        modalMsg->hide();
        OnPostDraw();
        if(timeCounter!=0)
        {
            ui->statusBar->addWidget(msgLabel);
        }
        _msg = QString::asprintf("rendering completed. cost %d seconds! previous pressed pos = %d,%d", timeCounter,(int)prePressedPos.x(), (int)prePressedPos.y());
    }
    else
    {
        _msg = QString::asprintf("waited %d seconds!", timeCounter);
        modalMsg->SetMsg(_msg);
        timeCounter++;
    }
    msgLabel->setText(_msg);
    ui->statusBar->addWidget(msgLabel);
}

void GRaytracer::DoRendering()
{
    while(true)
    {
        if(isNeedExist)
        {
            break;
        }
        GLog::LogInfo("==> wait");
        std::unique_lock<std::mutex> lock(mtx);
        hasDrawTask.wait(lock);
        GLog::LogInfo("==> wait over");

        if(isNeedExist)
        {
            break;
        }

        //GLAPI->Clear(GColor::black);
        //GLAPI->ClearDepth(1.0f);
        if(scene.camera==nullptr) continue;
        auto currentCam = scene.camera.get();
        GCamera::activeCamera = currentCam;

        for(auto& model: scene.models)
        {
            //model.DrawModel(GLAPI);
        }

        for(int j=currentCam->viewportY; j<currentCam->viewportH; j++)
        {
            GLog::LogInfo("%d/%d", j, currentCam->viewportH);
            for(int i=currentCam->viewportX; i<currentCam->viewportW; i++)
            {
                auto ray = currentCam->GetRay(i, j);
                GColor color = currentCam->RayColor(ray, 1);
                scene.film.SetColor(i,j, color);
            }
        }
        isRenderingCompleted = true;
    }
    GLog::LogInfo("==> exist!");
}

GModalMessage::GModalMessage(QWidget *parent)
    :QDialog(parent),msgUI(new Ui::GModalMessage)
{
    msgUI->setupUi(this);
}

void GModalMessage::SetMsg(QString msg)
{
    msgUI->msg->setText(msg);
}