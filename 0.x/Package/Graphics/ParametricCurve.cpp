#include "ParametricCurve.h"

using namespace std;

ParametricCurve::ParametricCurve(F1P *ffx, F1P *ffy, double start, double end) {
    LineStrip* lineStrip = new LineStrip;
    lineStrips.add(lineStrip);
    fx = ffx;
    fy = ffy;
    tmin = start;
    tmax = end;
    spt = tmax - tmin;
    makePoints();
}

ParametricCurve::ParametricCurve(F1P *ffx, F1P *ffy, double start, double end, F1P *cff) {
    LineStrip* lineStrip = new LineStrip;
    lineStrips.add(lineStrip);
    fx = ffx;
    fy = ffy;
    tmin = start;
    tmax = end;
    spt = tmax - tmin;
    //color function
    cf = cff;
    isColorfulCurve = true;
    makePoints();
    useColorFunction();
}

ParametricCurve::~ParametricCurve() {
}

void ParametricCurve::makePoints() {
    dataNum = getBaseNum();
    double *t = new double[dataNum];
    double *x = new double[dataNum];
    double *y = new double[dataNum];
    for (int i = 0; i < dataNum; i++) {
        //split the domain
        t[i] = tmin + (tmax - tmin) * i / (dataNum - 1);
    }
    fx->getArrayData(t, x, dataNum);
    fy->getArrayData(t, y, dataNum);
    for (int i = 0; i < dataNum; i++) {
        lineStrips[0]->vert.add(new Point2d(x[i], y[i]));
        arrayT.add(t[i]);
    }
    delete[] t;
    delete[] x;
    delete[] y;
    xmin = lineStrips[0]->vert[0]->x;
    xmax = lineStrips[0]->vert[0]->x;
    ymin = lineStrips[0]->vert[0]->y;
    ymax = lineStrips[0]->vert[0]->y;
    for (int i = 0; i < dataNum; i++) {
        if (lineStrips[0]->vert[i]->y > ymax) ymax = lineStrips[0]->vert[i]->y;
        if (lineStrips[0]->vert[i]->y < ymin) ymin = lineStrips[0]->vert[i]->y;
        if (lineStrips[0]->vert[i]->x > xmax) xmax = lineStrips[0]->vert[i]->x;
        if (lineStrips[0]->vert[i]->x < xmin) xmin = lineStrips[0]->vert[i]->x;
    }
    spx = xmax - xmin;
    spy = ymax - ymin;
    graphScale = spy / spx;
    dt = spt / LARGE;
    addVerts(lineStrips[0]);
}

int ParametricCurve::getBaseNum() {
    return 20;
}

void ParametricCurve::addVerts(LineStrip* lineStrip) {
    ArrayList<double> kcaled;
    kcaled.add(getK(arrayT[0], dt) / graphScale);
    unsigned int i = 0;
    while (i < lineStrip->vert.size() - 1) {
        double dis = sqrt(((lineStrip->vert[i+1]->x - lineStrip->vert[i]->x) / spx) * ((lineStrip->vert[i+1]->x - lineStrip->vert[i]->x) / spx) + ((lineStrip->vert[i+1]->y - lineStrip->vert[i]->y) / spy) * ((lineStrip->vert[i+1]->y - lineStrip->vert[i]->y) / spy));
        if (dis < MIN_DELTA_DIS) {
            i++;
            kcaled.add(getK(arrayT[i], dt) / graphScale);
            continue;
        }
        double netk1 = kcaled[i];
        double netk2 = getK(arrayT[i+1], -dt) / graphScale;
        double dangle = acos((1 + netk1 * netk2) / sqrt(1 + netk1 * netk1) / sqrt(1 + netk2 * netk2));
        if (dangle > ADD_POINT_ANGLE) {
            double t = (arrayT[i] + arrayT[i+1]) / 2;
            double x = fx->getSingleData(t);
            double y = fy->getSingleData(t);
            lineStrip->vert.add(i + 1, new Point2d(x, y));
            arrayT.add(i + 1, t);
            if (x > xmax) xmax = x;
            if (x < xmin) xmin = x;
            if (y > ymax) ymax = y;
            if (y < ymin) ymin = y;
        } else {
            double t = (arrayT[i] + arrayT[i+1]) / 2;
            double netk3 = getK(t, -dt) / graphScale;
            if ((netk3 < netk1 && netk3 < netk2) || (netk3 > netk1 && netk3 > netk2)) {
                double t = (arrayT[i] + arrayT[i+1]) / 2;
                double x = fx->getSingleData(t);
                double y = fy->getSingleData(t);
                lineStrip->vert.add(i + 1, new Point2d(x, y));
                arrayT.add(i + 1, t);
                if (x > xmax) xmax = x;
                if (x < xmin) xmin = x;
                if (y > ymax) ymax = y;
                if (y < ymin) ymin = y;
            } else {
                i++;
                kcaled.add(getK(arrayT[i], dt) / graphScale);
            }
        }
    }
    spx = xmax - xmin;
    spy = ymax - ymin;
    graphScale = spy / spx;
}

void ParametricCurve::useColorFunction() {
    double cfmin, cfmax;
    cfmin = cf->getSingleData(arrayT[0]);
    cfmax = cfmin;
    for (unsigned int j = 0; j < lineStrips[0]->vert.size(); j++) {
        double tmp = cf->getSingleData(arrayT[j]);
        if (tmp > cfmax)
            cfmax = tmp;
        if (tmp < cfmin)
            cfmin = tmp;
    }
    for (unsigned int j = 0; j < lineStrips[0]->vert.size(); j++) {
        double tmp = cf->getSingleData(arrayT[j]);
        lineStrips[0]->color.add(j, getRainbowColor((tmp - cfmin) / (cfmax - cfmin)));
    }
}

inline double ParametricCurve::getK(double t, double dt) {
    return fy->getDerivative(t, dt) / fx->getDerivative(t, dt);
}
