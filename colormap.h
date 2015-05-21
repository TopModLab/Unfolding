#ifndef COLORMAP_H
#define COLORMAP_H

#include "common.h"

#include <QColor>

class ColorMap {
public:
	typedef pair<double, QColor> ColorPoint;
	ColorMap();

	QColor getColor(double val);
	QColor getColor_discrete(double val);

	void setColor(double val, QColor c);

	void buildColormap();

	static ColorMap getDefaultColorMap();

private:
	vector<ColorPoint> colormap;
	vector<QColor> colormap_discrete;
	double minVal, maxVal, rangeVal;
};


#endif // COLORMAP_H
