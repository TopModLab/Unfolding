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
}

void ColorMap::setColor(double val, QColor c) {
  colormap.push_back(make_pair(val, c));
}

void ColorMap::buildColormap() {
  std::sort(colormap.begin(), colormap.end(),
            [](const ColorPoint& a, const ColorPoint& b) {
    return a.first < b.first;
  });
}
