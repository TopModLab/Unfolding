#include "colormap.h"

#include "mathutils.hpp"

ColorMap::ColorMap(){}

QColor ColorMap::getColor(double val) {
	if( val < colormap.front().first ) return colormap.front().second;
	if( val >= colormap.back().first ) return colormap.back().second;

	for(int i=0;i<colormap.size()-1;i++) {
		if( val >= colormap[i].first && val < colormap[i+1].first ) {
			double c = (val - colormap[i].first) / (colormap[i+1].first - colormap[i].first);
			return interpolate(c, colormap[i].second, colormap[i+1].second);
		}
	}
	return Qt::black;
}

QColor ColorMap::getColor_discrete(double val) {
	if( val < minVal ) return colormap_discrete.front();
	if( val > maxVal ) return colormap_discrete.back();
	const int SEGMENTS = 65536;
	int seg = clamp<int>((val - minVal)/rangeVal*SEGMENTS, 0, SEGMENTS-1);
	return colormap_discrete[seg];
}

void ColorMap::setColor(double val, QColor c) {
	colormap.push_back(make_pair(val, c));
}

void ColorMap::buildColormap() {
	std::sort(colormap.begin(), colormap.end(),
			  [](const ColorPoint& a, const ColorPoint& b) {
		return a.first < b.first;
	});

	minVal = colormap.front().first;
	maxVal = colormap.back().first;
	rangeVal = maxVal - minVal;

	const int SEGMENTS = 65536;
	for(int i=0;i<colormap.size()-1;++i) {
		// per segment interpolation
		int segCount = (colormap[i+1].first - colormap[i].first)/rangeVal*SEGMENTS;
		for(int j=0;j<segCount;++j) {
			double c = (j/(float)(segCount));
			colormap_discrete.push_back(interpolate(c, colormap[i].second, colormap[i+1].second));
		}
	}
}

ColorMap ColorMap::getDefaultColorMap()
{
	ColorMap cmap;

	cmap.setColor(-Pi/2.0, Qt::red);
	cmap.setColor(0.0, Qt::gray);
	cmap.setColor(Pi/2.0, Qt::blue);
	cmap.buildColormap();

	return cmap;
}
