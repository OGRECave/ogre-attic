#ifndef __PythonFrameListener_H__
#define __PythonFrameListener_H__

#include "OgrePrerequisites.h"
#include "OgreFrameListener.h"
#include <boost/python.hpp>

using namespace boost::python;

namespace Ogre {

    /** Subclass of FrameListener to enable callbacks
        into Python code. */
    class PythonFrameListener : public FrameListener
    {
    public:
        // constructor storing initial self_ parameter
        PythonFrameListener(PyObject* self_)
            : self(self_) {}

        bool frameStarted(const FrameEvent& evt)
        {
            // Have to hardcode this
            // VC++6 falls over trying to use callback<bool>::call_method<Framevent>
            //  with an internal compiler error (ouch)
            //  Must be too templatey for it's liking
//            ref p1(to_python(evt));
//            ref result(PyEval_CallMethod(self, const_cast<char*>("frameStarted"),
//                                         const_cast<char*>("(O)"),
//                                         p1.get()));
//            detail::callback_adjust_refcount(result.get(), type<bool>());
//            return from_python(result.get(), type<bool>());
            return call_method<bool>(self, "frameStarted", evt);

            //return callback<bool>::call_method<const FrameEvent&>(self, "frameStarted", evt);
        }
        bool frameEnded(const FrameEvent& evt)
        {
            // Have to hardcode this
            // VC++6 falls over trying to use callback<bool>::call_method<Framevent>
            //  with an internal compiler error (ouch)
            //  Must be too templatey for it's liking
//            ref p1(to_python(evt));
//            ref result(PyEval_CallMethod(self, const_cast<char*>("frameEnded"),
//                                         const_cast<char*>("(O)"),
//                                         p1.get()));
//            detail::callback_adjust_refcount(result.get(), type<bool>());
//            return from_python(result.get(), type<bool>());
            return call_method<bool>(self, "frameEnded", evt);
            //return boost::python::callback<bool>::call_method<const FrameEvent&>(self, "frameEnded", evt);
        }

        // Supplies the default implementation of methods
        static bool default_frameStarted(FrameListener* self_, const FrameEvent& evt)
            { 
                return self_->FrameListener::frameStarted(evt); 
            }
        static bool default_frameEnded(FrameListener* self_, const FrameEvent& evt)
            { 
                return self_->FrameListener::frameEnded(evt); 
            }
     private:
        PyObject* self;
    };
}
#endif
