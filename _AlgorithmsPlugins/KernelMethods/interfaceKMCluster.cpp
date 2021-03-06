/*********************************************************************
MLDemos: A User-Friendly visualization toolkit for machine learning
Copyright (C) 2010  Basilio Noris
Contact: mldemos@b4silio.com

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public License,
version 3 as published by the Free Software Foundation.

This library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*********************************************************************/
#include "interfaceKMCluster.h"
#include <QPixmap>
#include <QDebug>
#include <QBitmap>
#include <QPainter>
#include <qcontour.h>

using namespace std;

ClustKM::ClustKM()
{
    params = new Ui::ParametersKM();
    params->setupUi(widget = new QWidget());
    // binds change events in the ui to the ChangeOptions fct.
    connect(params->kmeansMethodCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeOptions()));
    connect(params->kmeansNormCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeOptions()));
    connect(params->kernelTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeOptions()));
    connect(params->KMeansPlusPlusCheckBox, SIGNAL(clicked()), this, SLOT(ChangeOptions()));
    ChangeOptions();
}

ClustKM::~ClustKM()
{
    delete params;
}

void ClustKM::ChangeOptions()
{
    int method = params->kmeansMethodCombo->currentIndex();
    int metric = params->kmeansNormCombo->currentIndex();
    int kernel = params->kernelTypeCombo->currentIndex();

    switch(method) {
    case 0: // K-Means
        params->param1Label->setText("Power");
        params->param1Label->setVisible(metric == 3);
        params->param1Spin->setVisible(metric == 3);
        params->param2Spin->setVisible(false);
        params->labelCombo->setText("Metric");
        params->KMeansPlusPlusCheckBox->setVisible(true);
        params->kmeansMethodCombo->setVisible(true);
        params->kmeansNormCombo->setVisible(true);
        params->kernelTypeCombo->setVisible(false);
        break;
    case 1: // Soft K-Means
        params->param1Label->setText("Beta");
        params->param1Label->setVisible(true);
        params->param1Spin->setVisible(false);
        params->param2Spin->setVisible(true);
        params->labelCombo->setText("");
        params->KMeansPlusPlusCheckBox->setVisible(true);
        params->kmeansNormCombo->setVisible(false);
        params->kernelTypeCombo->setVisible(false);
        break;
    case 2: // Kernel K-Means
        params->param1Label->setText(kernel == 2 ? "Width" : "Degree");
        params->param1Label->setVisible(kernel > 0);
        params->param1Spin->setVisible(kernel == 1);
        params->param2Spin->setVisible(kernel == 2);
        params->labelCombo->setText("Kernel Type");
        params->KMeansPlusPlusCheckBox->setVisible(false);
        params->kmeansNormCombo->setVisible(false);
        params->kernelTypeCombo->setVisible(true);
        break;
    }
}

void ClustKM::SetParams(Clusterer *clusterer)
{
    if(!clusterer) return;
    int method = params->kmeansMethodCombo->currentIndex();
    int clusters = params->kmeansClusterSpin->value();
    if(method == 2) {// kernel kmeans
        float kernelWidth = params->param2Spin->value();
        int kernelDegree = params->param1Spin->value();
        int kernelType = params->kernelTypeCombo->currentIndex();
        float kernelOffset = kernelWidth;
        ClustererKKM *clust = dynamic_cast<ClustererKKM*>(clusterer);
        if(!clust) return;
        clust->SetParams(clusters, kernelType, kernelWidth, kernelDegree, kernelOffset);
    } else {// kmeans, soft-kmeans
        int power = params->param1Spin->value();
        int metrictype = params->kmeansNormCombo->currentIndex();
        float beta = params->param2Spin->value();
        bool kmeansPlusPlus = params->KMeansPlusPlusCheckBox->isChecked();
        if (metrictype < 3) power = metrictype;
        ClustererKM *clust = dynamic_cast<ClustererKM*>(clusterer);
        if(!clust) return;
        clust->SetParams(clusters, method, beta, power, kmeansPlusPlus);
    }
}


fvec ClustKM::GetParams()
{
    fvec par(5);
    int method = params->kmeansMethodCombo->currentIndex();
    if(method == 2)
    {
        par[0] = params->kmeansClusterSpin->value();
        par[1] = params->kernelTypeCombo->currentIndex();
        par[2] = params->param2Spin->value();
        par[3] = params->param1Spin->value();
        par[4] = params->param2Spin->value();
    }
    else
    {
        par.resize(4);
        par[0] = params->kmeansClusterSpin->value();
        par[1] = params->param1Spin->value();
        par[2] = params->param2Spin->value();
        par[3] = params->KMeansPlusPlusCheckBox->isChecked();
    }
    return par;
}

void ClustKM::SetParams(Clusterer *clusterer, fvec parameters)
{
    if(!clusterer) return;
    int method = params->kmeansMethodCombo->currentIndex();
    if(method == 2)
    {
        int clusters = parameters.size() > 0 ? parameters[0] : 1;
        int kernelType = parameters.size() > 1 ? parameters[1] : 0;
        float kernelWidth = parameters.size() > 2 ? parameters[2] : 0.1;
        int kernelDegree = parameters.size() > 3 ? parameters[3] : 1;
        int kernelOffset = parameters.size() > 4 ? parameters[4] : 0;
        ClustererKKM *clust = dynamic_cast<ClustererKKM*>(clusterer);
        if(!clust) return;
        clust->SetParams(clusters, kernelType, kernelWidth, kernelDegree, kernelOffset);
    }
    else
    {
        int clusters = parameters.size() > 0 ? parameters[0] : 1;
        int power = parameters.size() > 1 ? parameters[1] : 1;
        float beta = parameters.size() > 3 ? parameters[3] : 0.1;
        bool kmeansPlusPlus = parameters.size() > 4 ? parameters[4] : 0;
        ClustererKM *clust = dynamic_cast<ClustererKM*>(clusterer);
        if(!clust) return;
        clust->SetParams(clusters, method, beta, power, kmeansPlusPlus);
    }
}

void ClustKM::GetParameterList(std::vector<QString> &parameterNames,
                             std::vector<QString> &parameterTypes,
                             std::vector< std::vector<QString> > &parameterValues)
{
    int method = params->kmeansMethodCombo->currentIndex();
    parameterNames.push_back("Components Count");
    parameterTypes.push_back("Integer");
    if(method==2)
    {
        parameterNames.push_back("Kernel Type");
        parameterNames.push_back("Kernel Width");
        parameterNames.push_back("Kernel Degree");
        parameterNames.push_back("Kernel Offset");
        parameterTypes.push_back("List");
        parameterTypes.push_back("Real");
        parameterTypes.push_back("Integer");
        parameterTypes.push_back("Real");
        parameterValues.push_back(vector<QString>());
        parameterValues.back().push_back("0.000001f");
        parameterValues.back().push_back("99999");
        parameterValues.push_back(vector<QString>());
        parameterValues.back().push_back("Linear");
        parameterValues.back().push_back("Poly");
        parameterValues.back().push_back("RBF");
        parameterValues.push_back(vector<QString>());
        parameterValues.back().push_back("1");
        parameterValues.back().push_back("150");
        parameterValues.push_back(vector<QString>());
        parameterValues.back().push_back("-99999");
        parameterValues.back().push_back("99999");
    }
    else
    {
        parameterNames.push_back("Metric Power");
        parameterNames.push_back("beta");
        parameterNames.push_back("KMeans PlusPlus");
        parameterTypes.push_back("List");
        parameterTypes.push_back("Real");
        parameterTypes.push_back("List");
        parameterValues.push_back(vector<QString>());
        parameterValues.back().push_back("Manhattan");
        parameterValues.back().push_back("Euclidean");
        parameterValues.back().push_back("Lp");
        parameterValues.back().push_back("Infinite");
        parameterValues.push_back(vector<QString>());
        parameterValues.back().push_back("0.000001f");
        parameterValues.back().push_back("999999");
        parameterValues.push_back(vector<QString>());
        parameterValues.back().push_back("False");
        parameterValues.back().push_back("True");
    }
}


Clusterer *ClustKM::GetClusterer()
{
    Clusterer *clusterer;
    int method = params->kmeansMethodCombo->currentIndex();
    if(method == 2)
    {
        clusterer = new ClustererKKM();
    }
    else
    {
        clusterer = new ClustererKM();
    }
    SetParams(clusterer);
    return clusterer;
}

void ClustKM::DrawInfo(Canvas *canvas, QPainter &painter, Clusterer *clusterer)
{
    if(!canvas || !clusterer) return;
    painter.setRenderHint(QPainter::Antialiasing);
    ClustererKM * _kmeans = dynamic_cast<ClustererKM*>(clusterer);
    if(_kmeans) // standard k-means!
    {
        KMeansCluster *kmeans = _kmeans->kmeans;
        painter.setBrush(Qt::NoBrush);
        FOR(i, kmeans->GetClusters())
        {
            fvec mean = kmeans->GetMean(i);
            QPointF point = canvas->toCanvasCoords(mean);

            QColor color = SampleColor[(i+1)%SampleColorCnt];
            painter.setPen(QPen(Qt::black, 12));
            painter.drawEllipse(point, 8, 8);
            painter.setPen(QPen(color,4));
            painter.drawEllipse(point, 8, 8);
        }
        return;
    }
    ClustererKKM * _kkmeans = dynamic_cast<ClustererKKM*>(clusterer);
    if(!_kkmeans) return;
    int W = painter.viewport().width();
    int H = painter.viewport().height();
    int w = 129;
    int h = 129;
    int nbClusters = _kkmeans->NbClusters();

    int bMethod = 1; // 0: cluster decision function, 1: cluster weight
    if(bMethod==0)
    {
        double *values = new double[w*h];
        FOR(i, w)
        {
            FOR(j, h)
            {
                int x = i*W/w;
                int y = j*H/h;
                fvec sample = canvas->fromCanvas(x,y);
                fvec res = _kkmeans->TestUnnormalized(sample);
                double value = res[0];
                FOR(k, res.size())
                {
                    if(value < res[k])
                    {
                        value = res[k];
                    }
                }
                values[j*w + i] = value;
            }
        }
        QContour contour(values, w, h);
        contour.bDrawColorbar = false;
        contour.plotColor = Qt::black;
        contour.plotThickness = 3;
        contour.style = Qt::DotLine;
        double vmin, vmax;
        contour.GetLimits(vmin, vmax);
        //vmin += (vmax - vmin)/4; // we take out the smallest levels to avoid numerical issues
        contour.SetLimits(vmin, vmax);
        contour.Paint(painter, 10);
        delete [] values;
    }
    else if(bMethod==1)
    {
        double **valueList = new double*[nbClusters];
        FOR(i, nbClusters)
        {
            valueList[i] = new double[w*h];
        }

        FOR(k, nbClusters)
        {
            FOR(i, w)
            {
                FOR(j, h)
                {
                    int x = i*W/w;
                    int y = j*H/h;
                    fvec sample = canvas->fromCanvas(x,y);
                    double value = _kkmeans->TestScore(sample, k);
                    // to avoid some numerical weird stuff
                    value = value*1000.;
                    valueList[k][j*w + i] = value;
                }
            }
        }
        FOR(k, nbClusters)
        {
            QContour contour(valueList[k], w, h);
            if(canvas->bDisplayMap)
            {
                contour.bDrawColorbar = false;
                contour.plotColor = Qt::black;
                contour.plotThickness = 4;
                contour.style = Qt::DotLine;
            }
            else
            {
                contour.bDrawColorbar = false;
                contour.plotColor = SampleColor[(k+1)%SampleColorCnt];
                contour.plotThickness = 3;
                contour.style = Qt::SolidLine;
            }
            double vmin, vmax;
            contour.GetLimits(vmin, vmax);
            vmin += (vmax - vmin)/5; // we take out the smallest levels to avoid numerical issues
            contour.SetLimits(vmin, vmax);
            contour.Paint(painter, 10);
            /*
            QImage image(w,h,QImage::Format_RGB32);
            double vmin, vmax;
            contour.GetLimits(vmin, vmax);
            double vdiff = vmax - vmin;
            FOR(i, w)
            {
                FOR(j, h)
                {
                    int value = (int)((valueList[k][j*w + i]-vmin)/vdiff*255);
                    value = max(0,min(255,value));
                    image.setPixel(i,j, qRgb((int)value,value,value));
                }
            }
            QPixmap contourPixmap = QPixmap::fromImage(image).scaled(512,512, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            QLabel *lab = new QLabel();
            lab->setPixmap(contourPixmap);
            lab->show();
            */
            delete [] valueList[k];
            valueList[k] = 0;
        }
        delete [] valueList;
    }
}

void ClustKM::DrawModel(Canvas *canvas, QPainter &painter, Clusterer *clusterer)
{
    if(!canvas || !clusterer) return;
    painter.setRenderHint(QPainter::Antialiasing);

    FOR(i, canvas->data->GetSamples().size())
    {
        fvec sample = canvas->data->GetSampleDim(i, canvas->sourceDims);
        fvec fullSample = canvas->data->GetSample(i);
        QPointF point = canvas->toCanvasCoords(fullSample);
        fvec res = clusterer->Test(sample);
        float r=0,g=0,b=0;
        if(res.size() > 1)
        {
            FOR(j, res.size())
            {
                r += SampleColor[(j+1)%SampleColorCnt].red()*res[j];
                g += SampleColor[(j+1)%SampleColorCnt].green()*res[j];
                b += SampleColor[(j+1)%SampleColorCnt].blue()*res[j];
            }
        }
        else if(res.size())
        {
            r = (1-res[0])*255 + res[0]* 255;
            g = (1-res[0])*255;
            b = (1-res[0])*255;
        }
        painter.setBrush(QColor(max(0.f,min(255.f,r)),max(0.f,min(255.f,g)),max(0.f,min(255.f,b))));
        painter.setPen(Qt::black);
        painter.drawEllipse(point,5,5);
    }
}

void ClustKM::SaveOptions(QSettings &settings)
{
    settings.setValue("kmeansParam1", params->param1Spin->value());
    settings.setValue("kmeansParam2", params->param2Spin->value());
    settings.setValue("kmeansCluster", params->kmeansClusterSpin->value());
    settings.setValue("kmeansMethod", params->kmeansMethodCombo->currentIndex());
    settings.setValue("kernelType", params->kernelTypeCombo->currentIndex());
}

bool ClustKM::LoadOptions(QSettings &settings)
{
    if(settings.contains("kmeansParam1")) params->param1Spin->setValue(settings.value("kmeansParam1").toFloat());
    if(settings.contains("kmeansParam2")) params->param2Spin->setValue(settings.value("kmeansParam2").toFloat());
    if(settings.contains("kmeansCluster")) params->kmeansClusterSpin->setValue(settings.value("kmeansCluster").toFloat());
    if(settings.contains("kmeansMethod")) params->kmeansMethodCombo->setCurrentIndex(settings.value("kmeansMethod").toInt());
    if(settings.contains("kernelType")) params->kernelTypeCombo->setCurrentIndex(settings.value("kernelType").toInt());
    ChangeOptions();
    return true;
}

void ClustKM::SaveParams(QTextStream &file)
{
    file << "clusterOptions" << ":" << "kmeansParam1" << " " << params->param1Spin->value() << "\n";
    file << "clusterOptions" << ":" << "kmeansParam2" << " " << params->param2Spin->value() << "\n";
    file << "clusterOptions" << ":" << "kmeansCluster" << " " << params->kmeansClusterSpin->value() << "\n";
    file << "clusterOptions" << ":" << "kmeansMethod" << " " << params->kmeansMethodCombo->currentIndex() << "\n";
    file << "clusterOptions" << ":" << "kernelType" << " " << params->kernelTypeCombo->currentIndex() << "\n";
}

bool ClustKM::LoadParams(QString name, float value)
{
    if(name.endsWith("kmeansParam1")) params->param1Spin->setValue(value);
    if(name.endsWith("kmeansParam2")) params->param2Spin->setValue(value);
    if(name.endsWith("kmeansCluster")) params->kmeansClusterSpin->setValue((int)value);
    if(name.endsWith("kmeansMethod")) params->kmeansMethodCombo->setCurrentIndex((int)value);
    if(name.endsWith("kernelType")) params->kernelTypeCombo->setCurrentIndex((int)value);
    ChangeOptions();
    return true;
}
