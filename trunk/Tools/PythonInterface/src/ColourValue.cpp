#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

#include "OgrePythonLink.h"

using namespace boost::python;
using namespace Ogre;

void exportColourValue()
{
    object cv = class_<ColourValue>("ColourValue",
        init<optional<Real,Real,Real,Real> >())
        .def_readwrite("r", &ColourValue::r)
        .def_readwrite("g", &ColourValue::g)
        .def_readwrite("b", &ColourValue::b)
        .def_readwrite("a", &ColourValue::a)

        .def(self == self)
        .def(self != self)

        .def("getAsLongRGBA", &ColourValue::getAsLongRGBA)
        .def("getAsLongARGB", &ColourValue::getAsLongARGB)
        .def("getAsLongABGR", &ColourValue::getAsLongABGR)
    ;

    cv.attr("Black") = ColourValue::Black;
    cv.attr("White") = ColourValue::White;
    cv.attr("Red") = ColourValue::Red;
    cv.attr("Green") = ColourValue::Green;
    cv.attr("Blue") = ColourValue::Blue;
}

