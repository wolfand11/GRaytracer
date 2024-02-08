#include "graytracer.h"
#include "gtriangle.h"
#include "ui_graytracer.h"
#include "ui_gmodalmessage.h"
#include "glog.h"
#include <QResizeEvent>
#include "gmath.h"
#include "gutils.h"
#include <QDoubleValidator>
#include "gmaterial.h"
#include "glight.h"
#include "gsampler.h"
#include "gquad.h"
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
            color = GColor::LinearToGamma(color);
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
    isSyncedColor = true;
    renderingThread = std::thread(&GRaytracer::DoRendering, this);
    renderingThread.detach();

    ui->poxXLineEdit->setValidator(new QDoubleValidator(this));
    ui->poxYLineEdit->setValidator(new QDoubleValidator(this));
    ui->poxZLineEdit->setValidator(new QDoubleValidator(this));
    ui->rotationXLineEdit->setValidator(new QDoubleValidator(this));
    ui->rotationYLineEdit->setValidator(new QDoubleValidator(this));
    ui->rotationZLineEdit->setValidator(new QDoubleValidator(this));
    ui->fovLineEdit->setValidator(new QDoubleValidator(this));
    ui->focalDisEdit->setValidator(new QDoubleValidator(this));
    ui->lenREdit->setValidator(new QDoubleValidator(this));
    ui->sppEdit->setValidator(new QDoubleValidator(this));
    ui->depthLineEdit->setValidator(new QDoubleValidator(this));
    RefreshUI();
    connect(ui->poxXLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(on_drawProp_changed()));
    connect(ui->poxYLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(on_drawProp_changed()));
    connect(ui->poxZLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(on_drawProp_changed()));
    connect(ui->rotationXLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(on_drawProp_changed()));
    connect(ui->rotationYLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(on_drawProp_changed()));
    connect(ui->rotationZLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(on_drawProp_changed()));
    connect(ui->fovLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(on_drawProp_changed()));
    connect(ui->focalDisEdit, SIGNAL(textChanged(const QString&)), this, SLOT(on_drawProp_changed()));
    connect(ui->lenREdit, SIGNAL(textChanged(const QString&)), this, SLOT(on_drawProp_changed()));
    connect(ui->sppEdit, SIGNAL(textChanged(const QString&)), this, SLOT(on_drawProp_changed()));
    connect(ui->depthLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(on_drawProp_changed()));
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
    // camera
    auto cameraGObj = GCamera::CreateProjCamera(1, 2000, 60);
    cameraGObj->LookAt(vec3f(0,1,-4), vec3f(0,1,0), vec3f(0,1,0));
    cameraGObj->SetViewport(0, 0, GUtils::screenWidth, GUtils::screenHeight);
    scene.camera = cameraGObj;
    scene.camera->apertureRadius = 0;
    //if(true)
    if(false)
    {
        // light
        {
            auto lightGObj = std::make_shared<GDirectionalLight>();
            lightGObj->SetR(vec3f(50,0,0));
            //scene.add(lightGObj);
        }
        {
            auto skyLightGObj = std::make_shared<GSkyLight>();
            scene.add(skyLightGObj);
        }
        {
            auto sphereLight = std::make_shared<GSphereLight>(0.25, GColor::whiteF*100);
            sphereLight->SetT(vec3f(-1.4, 2.5, 1));
            //scene.add(sphereLight);
        }
        {
            string cornellPath = GUtils::GetAbsPath("models/cornell_box4/");
            using ShapeType = GQuad;
            auto lights = GMeshLight::CreateLights<ShapeType>(cornellPath + "cornell_box-toplight.obj", GColor::whiteF*100);
            for(auto light : lights)
            {
                light->SetT(vec3f(-1.4, 2.5, 1));
            }
            scene.add(lights);
        }

        CreateTestScene();
        //CreateMultiObjScene();
    }
    else
    {
        CreateCornellBox();
    }
}

void GRaytracer::CreateTestScene()
{
    // models
    auto redTex = std::make_shared<GSolidColor>(GColor::redF);
    auto lightGrayTex = std::make_shared<GSolidColor>(GColor::grayF*1.5);
    auto goldTex = std::make_shared<GSolidColor>(GFColor(1.0, 0.782, 0.344, 1));
    auto ironTex = std::make_shared<GSolidColor>(GFColor(0.562, 0.565, 0.578, 1));
    auto roughTex = std::make_shared<GSolidColor>(GFColor(0.1, 0., 0., 1));
    auto checkerTex = std::make_shared<GCheckerTexture>(0.8, GColor::whiteF, GColor::blackF);
    auto redLambertMat = std::make_shared<GLambertianMaterial>(GColor::redF);
    auto greenLambertMat = std::make_shared<GLambertianMaterial>(GColor::greenF);
    auto grayLambertMat = std::make_shared<GLambertianMaterial>(GColor::grayF);
    auto checkerLambertMat = std::make_shared<GLambertianMaterial>(checkerTex);
    auto lightGrayMetalMat = std::make_shared<GSpecularReflectionMaterial>(lightGrayTex);
    auto goldSpecMat = std::make_shared<GSpecularReflectionMaterial>(goldTex);
    auto goldGlossyMat = std::make_shared<GGlossyMaterial>(goldTex, roughTex);
    auto ironSpecMat = std::make_shared<GSpecularReflectionMaterial>(ironTex);
    auto glassSpecRefractMat = std::make_shared<GSpecularRefractionMaterial>(GColor::grayF * 0.5, GColor::whiteF, 1.3);
    {
        auto sphereGObj0 = std::make_shared<GSphereModel>(0.5, redLambertMat);
        sphereGObj0->SetT(vec3f(0, 0.5, 1));
        scene.add(sphereGObj0);
    }
    {
        //auto sphereGObj2 = std::make_shared<GSphereModel>(0.5, greenLambertMat);
        auto sphereGObj2 = std::make_shared<GSphereModel>(vec3(0, 0.5, 0), 0.5, ironSpecMat);
        sphereGObj2->SetT(vec3f(-1.2, 0.5, 1));
        scene.add(sphereGObj2);
    }
    {
        //auto sphereGObj2 = std::make_shared<GSphereModel>(0.5, greenLambertMat);
        auto sphereGObj2 = std::make_shared<GSphereModel>(0.2, ironSpecMat);
        sphereGObj2->SetT(vec3f(-2.5, 0.2, 1));
        scene.add(sphereGObj2);
    }
    {
        //auto sphereGObj3 = std::make_shared<GSphereModel>(0.5, goldSpecMat);
        auto sphereGObj3 = std::make_shared<GSphereModel>(0.5, goldGlossyMat);
        sphereGObj3->SetT(vec3f(1.2, 0.5, 1));
        scene.add(sphereGObj3);
    }
    {
        auto sphereGObj = std::make_shared<GSphereModel>(0.5, glassSpecRefractMat);
        sphereGObj->SetT(vec3f(0, 0.5, -1));
        scene.add(sphereGObj);
    }
    {
        auto sphereGObj1 = std::make_shared<GSphereModel>(100, checkerLambertMat);
        //auto sphereGObj1 = std::make_shared<GSphereModel>(100, redLambertMat);
        sphereGObj1->SetT(vec3f(0, -100, 1));
        scene.add(sphereGObj1);
    }
    {
        auto objMesh = GMeshModel::CreateMesh<GQuad>(GUtils::GetAbsPath("models/cornell_box4/cornell_box.obj"), redLambertMat);
        //auto objMesh = GMeshModel::CreateMesh(GUtils::GetAbsPath("models/cornell_box/cornell_box.obj"), whiteMat);
        for(auto tri : objMesh)
        {
            tri->SetT(vec3f(0, 1, 0));
            //tri->SetS(scale);
            //tri->SetT(vec3f(0, 0.5, 0));
            //tri->SetT(vec3f(-1, 0.5, 1));
            tri->SetR(vec3f(0, 90, 0));
        }
        //scene.add(objMesh);
    }
    {
        //auto mat = make_shared<GLambertianMaterial>();
        /*
        auto triMesh0 = GMeshModel::CreateMesh(GUtils::GetAbsPath("models/cube.obj"), mat);
        //auto triMesh0 = GMeshModel::CreateMesh(GUtils::GetAbsPath("models/cube-front.obj"), mat);
        //auto triMesh1 = GMeshModel::CreateMesh(GUtils::GetAbsPath("models/sphereTriangle.obj"), mat);
        //auto triMesh1 = GMeshModel::CreateMesh(GUtils::GetAbsPath("models/diablo3_pose/diablo3_pose.obj"), mat);
        auto triMesh1 = GMeshModel::CreateMesh(GUtils::GetAbsPath("models/girl/DancingGirl.obj"), mat);
        //auto triMesh1 = GMeshModel::CreateMesh(GUtils::GetAbsPath("models/plane.obj"), mat);
        for(auto tri : triMesh1)
        {
            tri->SetT(vec3f(0, 0, 0));
            //tri->SetT(vec3f(0, 0.5, 0));
            //tri->SetT(vec3f(-1, 0.5, 1));
            //tri->SetR(vec3f(0, 180, 0));
        }
        //scene.add(triMesh1);
        */
    }
    scene.BuildBVHTree();
}

void GRaytracer::CreateMultiObjScene()
{
    scene.camera->focalDistance = 6;
    // models
    auto redTex = std::make_shared<GSolidColor>(GColor::redF);
    auto lightGrayTex = std::make_shared<GSolidColor>(GColor::grayF*1.5);
    auto goldTex = std::make_shared<GSolidColor>(GFColor(1.0, 0.782, 0.344, 1));
    auto ironTex = std::make_shared<GSolidColor>(GFColor(0.562, 0.565, 0.578, 1));
    auto roughTex = std::make_shared<GSolidColor>(GFColor(0.1, 0., 0., 1));
    auto checkerTex = std::make_shared<GCheckerTexture>(0.8, GColor::whiteF, GColor::grayF);
    auto redLambertMat = std::make_shared<GLambertianMaterial>(GColor::redF);
    auto greenLambertMat = std::make_shared<GLambertianMaterial>(GColor::greenF);
    auto grayLambertMat = std::make_shared<GLambertianMaterial>(GColor::grayF);
    auto checkerLambertMat = std::make_shared<GLambertianMaterial>(checkerTex);
    auto lightGrayMetalMat = std::make_shared<GSpecularReflectionMaterial>(lightGrayTex);
    auto goldSpecMat = std::make_shared<GSpecularReflectionMaterial>(goldTex);
    auto goldGlossyMat = std::make_shared<GGlossyMaterial>(goldTex, roughTex);
    auto glassSpecRefractMat = std::make_shared<GSpecularRefractionMaterial>(GColor::grayF, GColor::whiteF, 1.3);
    auto ironSpecMat = std::make_shared<GSpecularReflectionMaterial>(ironTex);
    float centerZ = 1.5;
    {
        auto sphereGObj0 = std::make_shared<GSphereModel>(0.5, redLambertMat);
        sphereGObj0->SetT(vec3f(-0.9, 0.5, 3));
        scene.add(sphereGObj0);
    }
    {
        auto sphereGObj2 = std::make_shared<GSphereModel>(0.5, ironSpecMat);
        sphereGObj2->SetT(vec3f(-0.3, 0.5, 2));
        scene.add(sphereGObj2);
    }
    {
        //auto sphereGObj3 = std::make_shared<GSphereModel>(0.5, goldSpecMat);
        auto sphereGObj3 = std::make_shared<GSphereModel>(0.5, goldGlossyMat);
        sphereGObj3->SetT(vec3f(0.3, 0.5, 1));
        scene.add(sphereGObj3);
    }
    {
        auto sphereGObj4 = std::make_shared<GSphereModel>(0.5, glassSpecRefractMat);
        sphereGObj4->SetT(vec3f(0.9, 0.5, 0));
        scene.add(sphereGObj4);
    }
    {
        auto sphereGObj1 = std::make_shared<GSphereModel>(100, checkerLambertMat);
        sphereGObj1->SetT(vec3f(0, -100, centerZ));
        scene.add(sphereGObj1);
    }
    {
        // eta: ice, water, fused quartz, glass, diamond
        std::vector<double> etaArr = {1.31, 1.333, 1.46, 1.6, 2.42};
        for(int i=-5; i<=5; i++)
        {
            for(int j=-5; j<=5; j++)
            {
                auto randomMat = GSampler::Random();
                vec3 center(i + 0.9*GSampler::Random(), 0.2, centerZ + j + 0.9*GSampler::Random());
                if((center - vec3(0, 0.2, centerZ)).length() > 0.9)
                {
                    std::shared_ptr<GMaterial> mat;
                    std::shared_ptr<GModel> model;
                    std::shared_ptr<GTexture> tex;
                    if(randomMat < 0.4)
                    {
                        auto Kd = GColor::RandomF();
                        mat = make_shared<GLambertianMaterial>(Kd);
                    }
                    else if(randomMat < 0.6)
                    {
                        auto Ks = GColor::RandomF();
                        mat = make_shared<GSpecularReflectionMaterial>(Ks);
                    }
                    else if(randomMat < 0.8)
                    {
                        GFColor Ks(0.024, 0.024, 0.024, 1);
                        auto Kt = GColor::RandomF(0.5);
                        auto eta = etaArr[GSampler::RandomInt(0, etaArr.size()-1)];
                        mat = make_shared<GSpecularRefractionMaterial>(Ks, Kt, eta);
                    }
                    else
                    {
                        auto Ks = GColor::RandomF();
                        auto rough = GSampler::Random(0.1, 0.9);
                        mat = make_shared<GGlossyMaterial>(Ks, rough);
                    }
                    model = make_shared<GSphereModel>(0.2, mat);
                    model->SetT(center);
                    scene.add(model);
                }
            }
        }
    }
    scene.BuildBVHTree();
}

void GRaytracer::CreateCornellBox()
{
    integrator->spp = 1;
    scene.camera->fov = 40;
    scene.camera->LookAt(vec3f(0,0,-160), vec3f(0,0,0), vec3f(0,1,0));
    scene.camera->apertureRadius = 0;
    auto redMat = std::make_shared<GLambertianMaterial>(GFColor(0.65, 0.05, 0.05, 1.0));
    auto whiteMat = std::make_shared<GLambertianMaterial>(GFColor(0.73, 0.73, 0.73, 1.0));
    auto greenMat = std::make_shared<GLambertianMaterial>(GFColor(0.12, 0.45, 0.15, 1.0));
    auto blueMat = std::make_shared<GLambertianMaterial>(GFColor(0.12, 0.15, 0.45, 1.0));
    vec3f scale = vec3f(40.0,40.0,40.0);
    string cornellPath = GUtils::GetAbsPath("models/cornell_box4/");
    using ShapeType = GQuad;
    //using ShapeType = GTriangle;
    GFColor lightColor(20,20,20,1);
    {
        auto sphereLight = std::make_shared<GSphereLight>(12, lightColor);
        sphereLight->SetT(vec3f(0, 60, 0));
        //scene.add(sphereLight);

        auto lights = GMeshLight::CreateLights<ShapeType>(cornellPath + "cornell_box-toplight.obj", lightColor*20, false);
        //auto lights = GMeshLight::CreateLights<ShapeType>(cornellPath + "cornell_box-toplight.obj", lightColor);
        for(auto light : lights)
        {
            light->SetT(vec3f(0, scale.x()*0.9, 0));
            light->SetS(vec3f(scale.x()*0.2, 1, scale.x()*0.2));
        }
        scene.add(lights);
    }
    {
        auto objMesh = GMeshModel::CreateMesh<ShapeType>(cornellPath + "cornell_box.obj", whiteMat);
        //auto objMesh = GMeshModel::CreateMesh<ShapeType>(cornellPath + "cornell_box.obj", blueMat);
        for(auto tri : objMesh)
        {
            tri->SetS(scale);
            tri->SetR(vec3f(0, 90, 0));
        }
        scene.add(objMesh);
        auto objMesh1 = GMeshModel::CreateMesh<ShapeType>(cornellPath + "cornell_box-left.obj", redMat);
        for(auto tri : objMesh1)
        {
            tri->SetS(scale);
            tri->SetR(vec3f(0, 90, 0));
        }
        scene.add(objMesh1);
        auto objMesh2 = GMeshModel::CreateMesh<ShapeType>(cornellPath + "cornell_box-right.obj", greenMat);
        for(auto tri : objMesh2)
        {
            tri->SetS(scale);
            tri->SetR(vec3f(0, 90, 0));
        }
        scene.add(objMesh2);
        auto objMesh3 = GMeshModel::CreateMesh<ShapeType>(cornellPath + "cornell_box-box.obj", whiteMat);
        for(auto tri : objMesh3)
        {
            tri->SetT(vec3f(0.3*scale.x(), -0.7*scale.x(), -0.4*scale.x()));
            tri->SetS(scale*0.3);
            tri->SetR(vec3f(0, 15, 0));
        }
        scene.add(objMesh3);
        auto objMesh4 = GMeshModel::CreateMesh<ShapeType>(cornellPath + "cornell_box-box.obj", whiteMat);
        for(auto tri : objMesh4)
        {
            tri->SetT(vec3f(-0.3*scale.x(), -0.4*scale.x(), 0.2*scale.x()));
            vec3 tScale = scale*0.3;
            tScale.SetY(tScale.y()*2);
            tri->SetS(tScale);
            tri->SetR(vec3f(0, -15, 0));
        }
        scene.add(objMesh4);
    }
    scene.BuildBVHTree();
}

void GRaytracer::SetupGRaytracer()
{
    integrator = std::make_shared<GIntegrator>();
    integrator->maxDepth = 5;
    integrator->spp = 5;
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
    ui->focalDisEdit->setText(QString("%1").arg(mainCamera->focalDistance));
    ui->lenREdit->setText(QString("%1").arg(mainCamera->apertureRadius));
    ui->sppEdit->setText(QString("%1").arg(integrator->spp));
    ui->depthLineEdit->setText(QString("%1").arg(integrator->maxDepth));
}

void GRaytracer::OnPreDraw()
{
}

void GRaytracer::DoDraw()
{
    OnPreDraw();
    timeCounter = 0;
    isRenderingCompleted = false;
    isSyncedColor = false;

    GLog::LogInfo("==> notify");
    hasDrawTask.notify_one();
}

void GRaytracer::OnPostDraw()
{
    if(!isSyncedColor)
    {
        CopyColorBufferToImage(&scene.film, &middleBuffer);
        ui->canvas->setPixmap(QPixmap::fromImage(middleBuffer));

        isSyncedColor = true;
    }
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
    mainCamera->focalDistance = std::max(0.01f, ui->focalDisEdit->text().toFloat());
    mainCamera->apertureRadius = std::max(0.00f, ui->lenREdit->text().toFloat());
    integrator->spp = std::max(ui->sppEdit->text().toInt(), 1);
    integrator->maxDepth = std::max(ui->depthLineEdit->text().toInt(), 1);
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

        if(scene.camera==nullptr) continue;
        auto currentCam = scene.camera.get();
        GCamera::activeCamera = currentCam;

        integrator->Render(scene);

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
