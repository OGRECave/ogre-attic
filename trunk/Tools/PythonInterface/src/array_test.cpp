#include <boost/array.hpp>
#include <boost/python.hpp>
#include "container_conversions.h"

using namespace boost::python;

int func(int test[3])
{
    printf("1) %d\n2) %d\n3) %d\n", test[0], test[1], test[2]);
}

BOOST_PYTHON_MODULE(array_test)
{
    scitbx::boost_python::container_conversions::from_python_sequence<
      boost::array<int, 3>,
      scitbx::boost_python::container_conversions::fixed_size_policy>();

    def("test", func);
}
